#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include "Protocol.h"
#include "HttpConst.h"

namespace extra::protocol::http
{
template <Version>
class HttpRequest;

template <Version>
class HttpResponse;


template <>
class HttpRequest<Version::_1_0> : public Request
{
public:
	constexpr static Version version = Version::_1_0;
	constexpr static std::string_view version_string = VersionString<version>;

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

	void set_body(std::string body_)
	{
		body = std::move(body_);
	}

	void set_header(std::string key, std::string value)
	{
		headers.insert_or_assign(std::move(key), std::move(value));
	}

	[[nodiscard]]
	std::optional<std::string> get_header(const std::string & key) const
	{
		auto iter = headers.find(key);
		if (iter == headers.end())
			return std::nullopt;
		else
			return iter->second;
	}

	[[nodiscard]]
	const std::map<std::string, std::string> & get_all_headers() const
	{
		return headers;
	}

	[[nodiscard]] std::string format() const override;

private:
	std::string method;
	std::string uri;
	std::map<std::string, std::string> headers;
	std::string body;
};

template <>
class HttpResponse<Version::_1_0> : public Response
{
public:
	[[nodiscard]] const std::string & get_status() const
	{
		return status;
	}

	[[nodiscard]] const std::string & get_phrase() const
	{
		return phrase;
	}

	[[nodiscard]] const std::string & get_body() const
	{
		return body;
	}

	[[nodiscard]] std::string_view get_body_view() const
	{
		return body;
	}

	std::string_view parse(std::string_view) override;

private:
	std::string status;
	std::string phrase;
	std::string body;

private:
};

template <Version>
class HttpChat;

template <Version>
class HttpPool;

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


}

