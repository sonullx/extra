#pragma once

#include <string>

class TcpTarget
{
public:
	explicit TcpTarget(std::string domain_)
		: domain{std::move(domain_)}
	{
	}

	void resolve_domain();

	int open_connection(unsigned short port);

	void close_connection(int fd, bool error);

private:
	std::string domain;

	const struct addr & choose_addr();
};