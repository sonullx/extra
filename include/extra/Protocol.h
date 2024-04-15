#pragma once

#include <string>

namespace extra::protocol
{

enum class State : int
{
	Unknown = 0,
	Success,
	SystemError,
	NetworkError,
	LAST,
};

enum class Error : int
{
	NoError = 0,
	NoConnection,
	Timeout,
	ParseError,
	LAST,
};

class Request
{
public:
	Request() = default;

	virtual ~Request() = default;

	[[nodiscard]] virtual std::string format() const = 0;

};

class Response
{
public:
	Response() = default;

	virtual ~Response() = default;

	virtual bool parse(std::string_view &) = 0;

	/*
	virtual bool eof()
	{
		return false;
	}
	 */
};

}