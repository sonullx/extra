#pragma once
#include <string>

namespace extra::protocol
{
class Request
{
	[[nodiscard]]
	virtual std::string format() const = 0;
};

class Response
{
	virtual std::string_view parse(std::string_view) = 0;
};

}