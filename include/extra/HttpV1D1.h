#pragma once

#include <string>
#include <string_view>
#include "Http.h"

namespace extra::protocol::http
{
template <>
class HttpRequest<Version::_1_1> : public Request
{
public:
	constexpr static inline Version V = Version::_1_1;
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

	void set_keep_alive()
	{
		keep_alive = true;
		append_header("Connection", "Keep-Alive");
	}

	void set_body(std::string body_)
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
	bool keep_alive;
};
}