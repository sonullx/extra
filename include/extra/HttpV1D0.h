#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include "Http.h"

namespace extra::protocol::http
{

template <>
class HttpRequest<Version::_1_0> : public Request
{
public:
	constexpr static inline Version V = Version::_1_0;
	constexpr static inline std::string_view version = VersionString<V>;

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

	void append_header(const std::string & key, const std::string & value)
	{
		headers += key;
		headers += ": ";
		headers += value;
		headers += CRLF;
	}

	void set_body_at_last(std::string body_)
	{
		body = std::move(body_);
		if (!body.empty())
			append_header("Content-Length", std::to_string(body.size()));
	}

	[[nodiscard]] std::string format() const override;

private:
	std::string method;
	std::string uri;
	std::string headers;
	std::string body;
};

using HttpRequestV1D0 = HttpRequest<Version::_1_0>;

template <>
class HttpResponse<Version::_1_0> : public Response
{
public:
	constexpr static Version V = Version::_1_0;

	HttpResponse();

	[[nodiscard]] const std::string & get_version() const
	{
		return version;
	}

	[[nodiscard]] bool same_version() const
	{
		return version == VersionString<V>;
	}

	[[nodiscard]] const std::string & get_status() const
	{
		return status;
	}

	[[nodiscard]] const std::string & get_phrase() const
	{
		return phrase;
	}

	[[nodiscard]] const std::unordered_map<std::string, std::string> & get_headers() const
	{
		return headers;
	}

	[[nodiscard]] const std::string & get_body() const
	{
		return body;
	}

	[[nodiscard]] std::string_view get_body_view() const
	{
		return body;
	}

	bool parse(std::string_view &) override;

private:
	std::string version;
	std::string status;
	std::string phrase;
	std::unordered_map<std::string, std::string> headers;
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
	enum
	{
		ParsingLF,
		ParsingStatusLine,
		ParsingHeader,
		ParsingBody,
	} parsing, next_parsing;


	bool parse_lf(std::string_view &);

	bool parse_status_line(std::string_view &);

	bool parse_header(std::string_view &);

	bool parse_body(std::string_view &);
};

using HttpResponseV1D0 = HttpResponse<Version::_1_0>;

template <>
class HttpChat<Version::_1_0>
{
public:
	using Request = HttpRequest<Version::_1_0>;
	using Response = HttpResponse<Version::_1_0>;

	HttpChat() = default;

private:
	int state;
	int error;
	Request request;
	Response response;
};

using HttpChatV1D0 = HttpChat<Version::_1_0>;

template <typename C>
class HttpSession
{
public:
	using Chat = C;
	using Request = typename Chat::Request;
	using Response = typename Chat::Response;

	Chat make_chat();

};


}

