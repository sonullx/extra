#include "Http.h"
#include <sstream>

namespace extra::protocol::http
{
std::string HttpRequest<Version::_1_0>::format() const
{
	std::ostringstream oss;
	oss << method << " " << uri << " " << version_string << CRLF;
	for (const auto & [key, value]: headers)
		oss << key << ": " << value << CRLF;

	oss << CRLF;
	oss << body;
	return oss.str();
}

std::string_view HttpResponse<Version::_1_0>::parse(std::string_view sv)
{
	return sv;
}
}
