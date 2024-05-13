#pragma once

#include <algorithm>
#include <array>
#include <limits>
#include <string_view>
#include <type_traits>

namespace extra::protocol::http::header
{
using Key = std::string_view;

consteval bool good_key(Key k)
{
	// TODO
	return !k.empty();
}

template <const Key & key>
class LowercaseKey
{
private:
	constexpr static auto buffer = []()
	{
		constexpr size_t s = key.size();
		std::array<char, s + 1> a{};
		for (size_t i = 0; i < s; i++)
		{
			char c = key[i];
			if (c >= 'A' && c <= 'Z')
				c = static_cast<char>(c - 'A' + 'a');

			a[i] = c;
		}
		a[s] = '\0';
		return a;
	}();
public:
	constexpr static Key value{buffer.data(), buffer.size() - 1};
};

template <const Key & ... KS>
class HeaderTrie
{
public:
	static_assert((true && ... && good_key(KS)));
	constexpr static auto raw_keys = std::array{KS ...};
	constexpr static auto lowercase_keys = std::array{LowercaseKey<KS>::value ...};
	constexpr static auto sorted_lowercase_keys = []()
	{
		std::array a = lowercase_keys;
		std::sort(a.begin(), a.end());
		return a;
	}();
	constexpr static auto dedup_sorted_lowercase_keys_length = []() -> size_t
	{
		size_t c = 0;
		Key prev;
		for (auto k: sorted_lowercase_keys)
		{
			if (k != prev)
				c++;

			prev = k;
		}
		return c;
	}();
	constexpr static auto dedup_sorted_lowercase_keys = []()
	{
		std::array<Key, dedup_sorted_lowercase_keys_length> array{};
		size_t j = 0;
		Key prev;
		for (auto k: sorted_lowercase_keys)
		{
			if (k != prev)
				array[j++] = k;

			prev = k;
		}
		return array;
	}();

public:
	constexpr static auto keys = dedup_sorted_lowercase_keys;

	template <const Key & K>
	constexpr static size_t index = []() -> size_t
	{
		Key key = LowercaseKey<K>::value;
		size_t i = 0;
		while (i < keys.size() && keys[i] != key)
			i++;

		return i;
	}();

	constexpr static size_t no_index = keys.size();

public:
	constexpr static auto table_length = []() -> size_t
	{
		size_t c = 2;
		Key prev;
		for (auto k: keys)
		{
			size_t s = std::min(k.size(), prev.size());
			size_t i = 0;
			while (i < s && k[i] == prev[i])
				i++;

			c += k.size() - i;
			prev = k;
		}
		return c;
	}();

	using Index = std::conditional_t<table_length < std::numeric_limits<int16_t>::max(),
		std::conditional_t<table_length < std::numeric_limits<int8_t>::max(), int8_t, int16_t>,
		std::conditional_t<table_length < std::numeric_limits<int32_t>::max(), int32_t, int64_t>
	>;

	constexpr static Index begin = 1;
	constexpr static Index end = 0;

	constexpr static auto table = []()
	{
		std::array<std::array<Index, 128>, table_length> t{};
		for (auto & line: t)
		{
			for (auto & c: line)
				c = end;

			line[0] = no_index;
		}

		t[begin][' '] = begin;
		t[begin]['\t'] = begin;

		Index expand = begin + 1;
		for (size_t i = 0; i < keys.size(); i++)
		{
			Key key = keys[i];
			Index current = begin;
			for (auto c: key)
			{
				Index & next = t[current][c];
				if (next == end)
					next = expand++;

				current = next;
			}
			t[current][0] = i;
			t[current][' '] = current;
			t[current]['\t'] = current;
		}

		return t;
	}();

	Index state;

	constexpr HeaderTrie()
		: state{begin}
	{
	}

	void reset()
	{
		state = begin;
	}

	constexpr HeaderTrie & operator<<(char c)
	{
		state = table[state][c];
		return *this;
	}

	constexpr HeaderTrie & operator<<(Key key)
	{
		for (auto c : key)
			*this << c;

		return *this;
	}

	constexpr HeaderTrie & operator>>(size_t & i)
	{
		i = table[state][0];
	}
};

constexpr static inline Key connection = "Connection";
constexpr static inline Key content_length = "Content-Length";
constexpr static inline Key host = "Host";
constexpr static inline Key what = "What";

using F = HeaderTrie<host, content_length, connection, host>;
static_assert(F::raw_keys == std::array{host, content_length, connection, host});
static_assert(F::lowercase_keys == std::array<Key, 4>{"host", "content-length", "connection", "host"});
static_assert(F::sorted_lowercase_keys == std::array<Key, 4>{"connection", "content-length", "host", "host"});
static_assert(F::dedup_sorted_lowercase_keys_length == 3);
static_assert(F::dedup_sorted_lowercase_keys == std::array<Key, 3>{"connection", "content-length", "host"});
static_assert(F::table_length == 27);
static_assert(std::is_same_v<F::Index, int8_t>);
static_assert(F::index < connection > != F::no_index);
static_assert(F::index < content_length > != F::no_index);
static_assert(F::index < host > != F::no_index);
static_assert(F::index < what > == F::no_index);
static_assert(!F::table.empty());

F f;
}
