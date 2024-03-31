#include <sstream>
#include "HttpV1D1.h"

namespace extra::protocol::http
{
std::string HttpRequest<Version::_1_1>::format() const
{
	std::ostringstream oss;
	oss << method << " " << uri << " " << version << CRLF << headers << CRLF << body;
	return oss.str();
}
}