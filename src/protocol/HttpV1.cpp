
#include <sstream>
#include "Http.h"

namespace extra::protocol::http
{
std::string HttpRequest<Version::_1_0>::format() const
{
	std::ostringstream oss;
	oss << method << " " << uri << " " << version << CRLF;
	for (const auto & [key, value]: headers)
		oss << key << ": " << value << CRLF;

	oss << CRLF;
	oss << body;
	return oss.str();
}

HttpResponse<Version::_1_0>::HttpResponse()
	: version{}, status{}, phrase{}, headers{}, body{}
	, line{}, key{}, content_length{static_cast<size_t>(-1)}, parsing{ParsingStatusLine}, next_parsing{ParsingStatusLine}
{
	line.reserve(line_initial_capacity);
}

bool HttpResponse<Version::_1_0>::parse(std::string_view & raw)
{
	bool good = true;
	while (good && !raw.empty())
	{
		switch (parsing)
		{
		case ParsingLF:
			good = parse_lf(raw);
			break;

		case ParsingStatusLine:
			good = parse_status_line(raw);
			break;

		case ParsingHeader:
			good = parse_header(raw);
			break;

		case ParsingBody:
			good = parse_body(raw);
			break;
		}
	}
	return good;
}

bool HttpResponse<Version::_1_0>::parse_lf(std::string_view & raw)
{
	if (raw.front() == LF)
	{
		raw.remove_prefix(1);
		parsing = next_parsing;
		return true;
	}
	else
		return false;
}

/**
 * @return false if buffer has become too large before found CRLF, or cannot split status line to 3 parts
 */
bool HttpResponse<Version::_1_0>::parse_status_line(std::string_view & raw)
{
	auto pos = std::min(raw.find(CR), raw.size());
	if (line.size() + pos > status_line_length_limit)
		return false;

	line += raw.substr(0, pos);
	raw.remove_prefix(pos);
	if (raw.empty()) // CR not found
		return true;

	raw.remove_prefix(1); // remove CR
	parsing = ParsingLF;

	std::string_view sv = line;

	if (sv.empty())
		return false;

	if (pos = sv.find(' '); pos == std::string_view::npos)
		return false;

	version = sv.substr(0, pos);
	sv.remove_prefix(pos + 1);

	if (sv.empty())
		return false;

	if (pos = sv.find(' '); pos == std::string_view::npos)
		return false;

	status = sv.substr(0, pos);
	sv.remove_prefix(pos + 1);

	if (sv.empty())
		return false;

	phrase = sv;

	line.clear();
	next_parsing = ParsingHeader;

	return true;
}

bool HttpResponse<Version::_1_0>::parse_header(std::string_view & raw)
{
	auto pos = std::min(raw.find(CR), raw.size());
	if (line.size() + pos > header_length_limit)
		return false;

	line += raw.substr(0, pos);
	raw.remove_prefix(pos);
	if (raw.empty())
		return true;

	raw.remove_prefix(1);
	parsing = ParsingLF;

	if (line.empty())
	{
		body.reserve(body_initial_capacity);
		next_parsing = ParsingBody;
		return true;
	}

	if (line.front() != ' ' && line.front() != '\t')
	{
		pos = line.find(':');
		if (pos == std::string::npos)
			return false;

		key = line.substr(0, pos);
		pos++;
	}
	else
		pos = 0;

	pos = line.find_first_not_of(" \t", pos);
	if (pos != std::string::npos)
	{
		auto value = line.substr(pos);
		if (auto iter = headers.find(key); iter != headers.end())
		{
			iter->second += ", ";
			iter->second += value;
		}
		else
			headers.insert({key, std::move(value)});
	}

	line.clear();
	// next_parsing = ParsingHeader;

	return true;
}

bool HttpResponse<Version::_1_0>::parse_body(std::string_view & raw)
{
	if (content_length == static_cast<size_t>(-1))
	{
		if (const auto iter = headers.find("Content-Length"); iter != headers.end())
		{
			const auto & value = iter->second;
			char * e = nullptr;
			size_t l = std::strtoul(value.data(), &e, 10);
			if (e != value.data() + value.size())
				return false;

			content_length = l;
		}
		else
			content_length = static_cast<size_t>(-2);
	}

	auto pos = std::min(raw.size(), content_length - body.size());
	if (body.size() + pos > body_length_limit)
		return false;

	body += raw.substr(0, pos);
	raw.remove_prefix(pos);
	return true;
}
}
