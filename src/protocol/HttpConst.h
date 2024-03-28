#pragma once

namespace extra::protocol::http
{
enum class Version
{
	_1_0,
	_1_1,
	_2,
	_3,
};

template <Version>
constexpr inline std::string_view VersionString;

template <> constexpr inline std::string_view VersionString<Version::_1_0> = "HTTP/1.0";
template <> constexpr inline std::string_view VersionString<Version::_1_1> = "HTTP/1.1";
template <> constexpr inline std::string_view VersionString<Version::_2> = "HTTP/2";
template <> constexpr inline std::string_view VersionString<Version::_3> = "HTTP/3";

enum class Method
{
	Connect,
	Delete,
	Get,
	Head,
	Options,
	Post,
	Put,
	Trace,
};

template <Method>
constexpr inline std::string_view MethodString;

template <> constexpr inline std::string_view MethodString<Method::Connect> = "CONNECT";
template <> constexpr inline std::string_view MethodString<Method::Delete> = "DELETE";
template <> constexpr inline std::string_view MethodString<Method::Get> = "GET";
template <> constexpr inline std::string_view MethodString<Method::Head> = "HEAD";
template <> constexpr inline std::string_view MethodString<Method::Options> = "OPTIONS";
template <> constexpr inline std::string_view MethodString<Method::Post> = "POST";
template <> constexpr inline std::string_view MethodString<Method::Put> = "PUT";
template <> constexpr inline std::string_view MethodString<Method::Trace> = "TRACE";

enum class Status
{
	_200,
};

template <Status>
constexpr inline std::string_view StatusString;

template <> constexpr inline std::string_view StatusString<Status::_200> = "200";

constexpr inline std::string_view CRLF = "\r\n";

}