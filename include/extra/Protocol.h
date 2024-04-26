#pragma once

#include <string>

namespace extra::protocol
{

enum class State : int
{
	Unknown,
	Success,
	SystemError,
	NetworkError,
	Last,
};

enum Error : int
{
	NoError,
	Timeout,
	ParseError,
};

class Request
{
	[[nodiscard]] virtual std::string format() const = 0;
};

class Response
{
	virtual bool parse(std::string_view &) = 0;

	/*
	virtual bool eof()
	{
		return false;
	}
	 */
};

}