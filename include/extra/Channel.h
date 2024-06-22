#pragma once

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Util.h"

namespace extra::kernel
{
namespace detail
{
class alignas(util::cache_line_size) ChannelLayout
{
public:
	constexpr static size_t null_index = 0;

	static size_t total_size(size_t content_size, size_t capacity)
	{
		return sizeof(ChannelLayout) + calculate_cell_size(content_size) * capacity;
	}

private:
	struct alignas(util::cache_line_size)
	{
		size_t mark;
		size_t cell_size;
		size_t capacity;
		int state_;
	};

	alignas(util::cache_line_size) size_t last;
	alignas(util::cache_line_size) size_t unused;
	alignas(util::cache_line_size) char base[0];

private:
	constexpr static int state_available = 0;
	constexpr static int state_not_available = 1;

	static size_t calculate_cell_size(size_t content_size)
	{
		return util::pow_of_2(content_size + sizeof(size_t));
	}

	std::atomic_ref<size_t> next_of(size_t index)
	{
		return std::atomic_ref<size_t>(*reinterpret_cast<size_t *>(base + cell_size * (index + 1) - sizeof(size_t)));
	}

	[[nodiscard]]
	std::atomic_ref<const size_t> next_of(size_t index) const
	{
		return std::atomic_ref<const size_t>(
			*reinterpret_cast<const size_t *>(base + cell_size * (index + 1) - sizeof(size_t)));
	}

public:
	bool initialize(size_t mark_, size_t content_size_, size_t capacity_)
	{
		int expected = state_available;
		int desired = state_not_available;
		std::atomic_ref s{state_};
		if (!s.compare_exchange_strong(expected, desired, std::memory_order::acq_rel))
			return false;

		mark = mark_;
		cell_size = calculate_cell_size(content_size_);
		capacity = capacity_;
		last = 0;
		unused = 1;
		s.store(state_available, std::memory_order::release);
		return true;
	}

	bool check(size_t mark_, size_t content_size_)
	{
		return std::atomic_ref{state_}.load(std::memory_order::acquire) == state_available
		       && mark == mark_
		       && cell_size == calculate_cell_size(content_size_);
	}

	size_t allocate()
	{
		std::atomic_ref u{unused};
		size_t index;
		do
		{
			index = u.load(std::memory_order::acquire);
			if (index >= capacity)
				return null_index;
		} while (!u.compare_exchange_strong(index, index + 1, std::memory_order::acq_rel));
		return index;
	}

	void append(size_t index)
	{
		std::atomic_ref l{last};
		size_t current_last = l.load(std::memory_order::acquire);
		size_t expected_next;
		size_t advanced_last;
		do
		{
			expected_next = null_index;

			// CAS 1. Try to link [index] after [last] if [last].next == null_index.
			// Succeed or fail, we got a better last. Try to link again in next loop if failed.
			advanced_last =
				next_of(current_last).compare_exchange_strong(expected_next, index, std::memory_order::acq_rel)
				? index : expected_next;

			// CAS 2. Advance last. If process crushes here, other process calling append() will fix it.
			// Succeed or fail, we got a better last, again. Will use it in next loop if necessary.
			current_last =
				l.compare_exchange_strong(current_last, advanced_last, std::memory_order::acq_rel)
				? advanced_last : current_last;
		} while (expected_next != null_index); // Until CAS 1 succeeds.
	}

	[[nodiscard]]
	size_t first() const
	{
		return null_index;
	}

	[[nodiscard]]
	size_t next(size_t index) const
	{
		return next_of(index).load(std::memory_order::acquire);
	}

	void * content(size_t index)
	{
		return base + cell_size * index;
	}

	[[nodiscard]]
	const void * content(size_t index) const
	{
		return const_cast<ChannelLayout *>(this)->content(index);
	}
};

static_assert(std::is_trivial_v<ChannelLayout>);
static_assert(std::is_standard_layout_v<ChannelLayout>);
static_assert(alignof(ChannelLayout) == util::cache_line_size);

class ChannelShm
{
private:
	static std::string get_filename(const std::string & name, unsigned long version)
	{
		return name.substr(0, 128) + '-' + std::to_string(version);
	}

	static size_t get_mark(const std::string & name, unsigned long version)
	{
		size_t mark = 0;
		mark ^= std::hash<std::string>{}(name);
		mark ^= std::hash<unsigned long>{}(version);
		return mark;
	}

public:
	ChannelShm() = delete;

	static ChannelLayout * create(const std::string & name, unsigned long version, size_t content_size, size_t capacity)
	{
		auto filename = get_filename(name, version);
		auto mark = get_mark(name, version);

		if (auto fd = shm_open(filename.data(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR); fd != -1)
		{
			if (auto size = ChannelLayout::total_size(content_size, capacity);
				ftruncate(fd, static_cast<off_t>(size)) != -1)
			{
				if (auto p = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0); p != MAP_FAILED)
				{
					close(fd);
					if (auto layout = reinterpret_cast<ChannelLayout *>(p);
						layout->initialize(mark, content_size, capacity))
						return layout;

					munmap(p, size);
				}
				close(fd);
			}
			shm_unlink(filename.data());
		}
		return nullptr;
	}

	static ChannelLayout * attach(const std::string & name, unsigned long version, size_t content_size)
	{
		auto filename = get_filename(name, version);
		auto mark = get_mark(name, version);

		if (auto fd = shm_open(filename.data(), O_RDWR, S_IRUSR | S_IWUSR); fd != -1)
		{
			if (struct stat st{}; fstat(fd, &st) != -1)
			{
				auto size = static_cast<size_t>(st.st_size);
				if (auto p = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0); p != MAP_FAILED)
				{
					close(fd);
					if (auto layout = reinterpret_cast<ChannelLayout *>(p); layout->check(mark, content_size))
						return layout;

					munmap(p, size);
				}
				close(fd);
			}
			shm_unlink(filename.data());
		}
		return nullptr;
	}

	static void detach(const std::string & name, unsigned long version)
	{
		auto filename = get_filename(name, version);
		shm_unlink(filename.data());
	}
};
} // namespace detail

template <typename T>
class ChannelIterator
{
protected:
	using Layout = detail::ChannelLayout;
	Layout * layout;
	size_t index;
	T * content;

	ChannelIterator(Layout * layout_, size_t index_)
		: layout{layout_}, index{index_}
	{
		content = reinterpret_cast<T *>(layout->content(index));
	}

	void update(size_t index_)
	{
		index = index_;
		content = reinterpret_cast<T *>(layout->content(index));
	}

public:
	T * operator->()
	{
		return content;
	}

	const T * operator->() const
	{
		return content;
	}

	T & operator*()
	{
		return *content;
	}

	const T & operator*() const
	{
		return *content;
	}
};

template <typename T>
class ChannelWriteIterator : public ChannelIterator<T>
{
private:
	using Base = ChannelIterator<T>;
	using typename Base::Layout;

public:
	explicit ChannelWriteIterator(Layout * layout_)
		: Base{layout_, layout_->allocate()}
	{
	}

	~ChannelWriteIterator()
	{
		Base::layout->append(this->index);
	}
};

template <typename T>
class ChannelReadIterator : public ChannelIterator<T>
{
private:
	using Base = ChannelIterator<T>;
	using typename Base::Layout;

public:
	explicit ChannelReadIterator(Layout * layout_)
		: Base{layout_, layout_->first()}
	{
	}

	void reset()
	{
		this->update(Base::layout->first());
	}

	bool next()
	{
		auto n = Base::layout->next(this->index);
		if (n == Layout::null_index)
			return false;

		this->update(n);
		return true;
	}
};

template <typename T>
class Channel
{
private:
	std::string name;
	unsigned long version;
	constexpr static size_t content_size = sizeof(T);
	size_t capacity;
	detail::ChannelLayout * layout;

public:
	using WriteIterator = ChannelWriteIterator<T>;
	using ReadIterator = ChannelReadIterator<T>;

public:
	Channel(std::string name_, unsigned long version_, size_t capacity_)
		: name{std::move(name_)}, version{version_}, capacity{capacity_}, layout{nullptr}
	{
	}

	~Channel()
	{
		detach();
	}

	[[nodiscard]]
	bool good() const
	{
		return layout != nullptr;
	}

	bool create()
	{
		layout = detail::ChannelShm::create(name, version, content_size, capacity);
		return good();
	}

	bool attach()
	{
		layout = detail::ChannelShm::attach(name, version, content_size);
		return good();
	}

	void detach()
	{
		if (good())
			detail::ChannelShm::detach(name, version);
	}

	auto write_iterator()
	{
		return WriteIterator{layout};
	}

	auto read_iterator()
	{
		return ReadIterator{layout};
	}
};

}
