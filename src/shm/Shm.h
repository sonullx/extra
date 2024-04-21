#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <cstddef>
#include <cstring>

#include <atomic>
#include <string>

constexpr static inline size_t cache_line_size = 64;

class alignas(cache_line_size) Raw
{
public:
	char name[cache_line_size];
	unsigned long version;
	size_t cell_size;
	size_t length;
	// bool initializing;

	alignas(cache_line_size) size_t last;
	alignas(cache_line_size) size_t unused;
	alignas(cache_line_size) char base[0];
};

static_assert(std::is_standard_layout_v<Raw>);
static_assert(std::is_trivial_v<Raw>);


template <typename T>
requires std::is_standard_layout_v<T> && std::is_trivial_v<T>
class alignas(cache_line_size) ShmNode
{
public:
	alignas(size_t) T t;
	size_t next;
};

template <typename T, size_t N>
class ShmMeta
{
private:
	constexpr static size_t expand(size_t n)
	{
		n--;
		size_t m = 1;
		while (n != 0)
		{
			n >>= 1;
			m <<= 1;
		}
		return m;
	}

public:
	constexpr static size_t type_size = sizeof(T);
	using Node = ShmNode<T>;
	constexpr static size_t node_size = sizeof(Node);
	constexpr static size_t length = N;
	constexpr static size_t cell_size = expand(node_size);
	constexpr static size_t array_size = cell_size * length;
	constexpr static size_t raw_size = sizeof(Raw) + array_size;

	static Node & get(const char * base, size_t index)
	{
		return *reinterpret_cast<Node *>(base + index * cell_size);
	}
};

template <typename T, size_t N>
class ShmReader
{
public:
	using Meta = ShmMeta<T, N>;

	explicit ShmReader(Raw * raw_)
		: raw{raw_}, current{0}, t{nullptr}
	{
	}

	ShmReader(const ShmReader & other)
		: raw{other.raw}, current{other.current}, t{other.t}
	{
	}

	ShmReader & operator=(const ShmReader & other)
	{
		if (&other != this)
		{
			raw = other.raw;
			current = other.current;
			t = other.t;
		}
		return *this;
	}

	[[nodiscard]]
	bool good() const
	{
		return current != 0;
	}

	explicit operator bool() const
	{
		return good();
	}

	T & operator*()
	{
		return *t;
	}

	const T & operator*() const
	{
		return *t;
	}

	T * operator->()
	{
		return t;
	}

	const T * operator->() const
	{
		return t;
	}

	bool next()
	{
		auto next = std::atomic_ref{Meta::get(raw->base, current).next}.load(std::memory_order::acquire);
		if (next != 0)
		{
			current = next;
			t = &Meta::get(raw->base, current);
			return true;
		}
		else
			return false;
	}

private:
	Raw * raw;
	size_t current;
	T * t;
};

template <typename T, size_t N>
class ShmWriter
{
public:
	using Meta = ShmMeta<T, N>;

	explicit ShmWriter(Raw * raw_)
		: raw{raw_}, index{std::atomic_ref{raw->unused}++}, t{&Meta::get(raw->base, index).t}
	{
	}

	~ShmWriter()
	{
		for (auto last = std::atomic_ref{raw->last}.load();;)
		{
			size_t expected = 0;
			if (std::atomic_ref{Meta::get(raw->base, last).next}.compare_exchange_strong(expected, index))
			{
				std::atomic_ref{raw->last}.compare_exchange_strong(last, index);
				break;
			}

			if (std::atomic_ref{raw->last}.compare_exchange_strong(last, expected))
				last = expected;
		}
	}

	ShmWriter(const ShmWriter &) = delete;

	ShmWriter & operator=(const ShmWriter &) = delete;

	ShmWriter(ShmWriter &&) noexcept = delete;

	ShmWriter & operator=(ShmWriter &&) noexcept = delete;

	[[nodiscard]]
	bool good() const
	{
		return index < Meta::length;
	}

	explicit operator bool() const
	{
		return good();
	}


private:
	Raw * raw;
	size_t index;
	T * t;
};

template <typename T, size_t N>
class Shm
{
	using Meta = ShmMeta<T, N>;
	using Meta::cell_size;
	using Meta::length;
	using Meta::raw_size;

	Shm(std::string name_, unsigned long version_)
		: name{std::move(name_)}, version{version_}, raw{nullptr}
	{
	}

	~Shm() = default;

	bool create()
	{
		int fd = shm_open(name.data(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
		if (fd == -1)
			return false;

		do
		{
			if (ftruncate(fd, raw_size) == -1)
				break;

			auto p = mmap(nullptr, raw_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
			if (p == MAP_FAILED)
				break;

			raw = reinterpret_cast<Raw *>(p);
			std::strncpy(raw->name, name.data(), sizeof(raw->name));
			raw->version = version;
			raw->cell_size = cell_size;
			raw->length = length;
			raw->last = 0;
			raw->unused = 1;

			return true;
		} while (false);

		raw = nullptr;
		shm_unlink(name.data());
		return false;
	}

	bool link()
	{
		int fd = shm_open(name.data(), O_RDWR, S_IRUSR | S_IWUSR);
		if (fd == -1)
			return false;

		do
		{
			struct stat st{};
			if (fstat(fd, &st) == -1)
				break;

			if (st.st_size != raw_size)
				break;

			auto p = mmap(nullptr, raw_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
			if (p == MAP_FAILED)
				break;

			raw = reinterpret_cast<Raw *>(p);
			if (std::strncmp(raw->name, name.data(), sizeof(raw->name)) != 0 || raw->version != version ||
			    raw->cell_size != cell_size || raw->length != length)
				break;

			return true;
		} while (false);

		raw = nullptr;
		shm_unlink(name.data());
		return false;
	}

	[[nodiscard]]
	bool good() const
	{
		return raw != nullptr;
	}

	explicit operator bool() const
	{
		return good();
	}

private:
	std::string name;
	unsigned long version;
	Raw * raw;
};

