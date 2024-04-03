#pragma once

#include <cassert>
#include <cstring>
#include <sstream>
#include <unordered_map>

#include "Protocol.h"
#include "HttpConst.h"

namespace extra::protocol::http
{
class HttpRequestBasic : public Request
{
private:
	std::string method;
	std::string uri;
	std::string version;
	std::string headers;
	std::string body;

public:
	HttpRequestBasic() = default;

	~HttpRequestBasic() = default;

	template <Method m>
	void set_method()
	{
		method = MethodString<m>;
	}

	void set_method(std::string method_)
	{
		method = std::move(method_);
	}

	void set_uri(std::string uri_)
	{
		uri = std::move(uri_);
	}

	template <Version v>
	void set_version()
	{
		version = VersionString<v>;
	}

	void set_version(std::string version_)
	{
		version = std::move(version_);
	}

	void append_header(const std::string & key, const std::string & value)
	{
		headers += key;
		headers += ": ";
		headers += value;
		headers += CRLF;
	}

	void set_body_once(std::string body_)
	{
		assert(body.empty());
		body = std::move(body_);
		if (!body.empty())
			append_header("Content-Length", std::to_string(body.size()));
	}

	[[nodiscard]] std::string format() const override
	{
		std::ostringstream oss;
		oss << method << " " << uri << " " << version << CRLF << headers << CRLF << body;
		return oss.str();
	}
};

class HttpResponseBasic : public Response
{
public:
	class CaseInsensitiveHash
	{
	public:
		size_t operator()(std::string s) const
		{
			for (auto & c: s)
			{
				if (std::isupper(c))
					c -= 'A' - 'a';
			}
			return std::hash<std::string>{}(s);
		}
	};

	class CaseInsensitiveEqualTo
	{
	public:
		bool operator()(const std::string & lhs, const std::string & rhs) const
		{
			return strcasecmp(lhs.data(), rhs.data()) == 0;
		}
	};

	using HeaderMap = std::unordered_map<std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEqualTo>;

private:
	std::string version;
	std::string status;
	std::string phrase;
	HeaderMap headers;
	std::string body;

private:
	constexpr static size_t line_initial_capacity = 1024;
	constexpr static size_t body_initial_capacity = 1024;
	constexpr static size_t status_line_length_limit = 1024;
	constexpr static size_t header_length_limit = 1024;
	constexpr static size_t body_length_limit = 1024 * 1024;

	std::string line;
	std::string key;
	size_t content_length;
	constexpr static size_t content_length_unknown = 0;
	constexpr static size_t content_length_unlimited = -1;
	enum
	{
		ParsingStatusLine,
		ParsingHeader,
		ParsingBody,
		ParsingLF,
	} parsing, will_parse;

public:
	HttpResponseBasic()
		: version{}, status{}, phrase{}, headers{}, body{}
		, line{}, key{}, content_length{content_length_unknown}
		, parsing{ParsingStatusLine}, will_parse{ParsingStatusLine}
	{
		line.reserve(line_initial_capacity);
	}


	[[nodiscard]] const std::string & get_version() const
	{
		return version;
	}

	[[nodiscard]] const std::string & get_status() const
	{
		return status;
	}

	[[nodiscard]] const std::string & get_phrase() const
	{
		return phrase;
	}

	[[nodiscard]] const HeaderMap & get_headers() const
	{
		return headers;
	}

	[[nodiscard]] const std::string & get_body() const
	{
		return body;
	}

	bool parse(std::string_view &) override;

private:

	bool parse_lf(std::string_view &);

	bool parse_status_line(std::string_view &);

	bool parse_header(std::string_view &);

	bool parse_body(std::string_view &);
};

class HttpRequestV1D0 : protected HttpRequestBasic
{
public:
	using HttpRequestBasic::set_method;
	using HttpRequestBasic::set_uri;
	using HttpRequestBasic::append_header;
	using HttpRequestBasic::set_body_once;
	using HttpRequestBasic::format;

	HttpRequestV1D0()
		: HttpRequestBasic()
	{
		HttpRequestBasic::set_version<Version::_1_0>();
	}
};

class HttpResponseV1D0 : public HttpResponseBasic
{
};

class HttpRequestV1D1 : protected HttpRequestBasic
{
public:
	using HttpRequestBasic::set_method;
	using HttpRequestBasic::set_uri;
	using HttpRequestBasic::append_header;
	using HttpRequestBasic::set_body_once;
	using HttpRequestBasic::format;

	HttpRequestV1D1()
		: HttpRequestBasic()
	{
		HttpRequestBasic::set_version<Version::_1_1>();
	}
};

class HttpResponseV1D1 : public HttpResponseBasic
{
public:
	bool is_keep_alive() const
	{
		auto iter = get_headers().find("Connection");
		return iter != get_headers().end() && CaseInsensitiveEqualTo{}(iter->second, "Keep-Alive");
	}
};

}