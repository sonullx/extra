#include <stack>

#include "TcpTarget.h"
#include "Protocol.h"

namespace extra::protocol::http
{
using Connection = int;
constexpr static inline Connection no_connection = -1;

class HttpHub;

class HttpChat
{
private:
	friend HttpHub;
	State state;
	Error error;
	Request * request;
	Response * response;
	HttpHub * hub;
	Connection connection;

	explicit HttpChat(HttpHub * hub_)
		: state{State::Unknown}, error{Error::NoError}, request{nullptr}, response{nullptr}, hub{hub_}, connection{}
	{
	}

	~HttpChat()
	{
		// TODO Really?
		delete request;
		delete response;
	}

public:
	[[nodiscard]] State get_state() const
	{
		return state;
	}

	[[nodiscard]] Error get_error() const
	{
		return error;
	}

	Request & get_request()
	{
		return *request;
	}

	[[nodiscard]] const Request & get_request() const
	{
		return *request;
	}

	[[nodiscard]] const Response & get_response() const
	{
		return *response;
	}

	void go()
	{
	}
};

class HttpHub
{
public:
	HttpChat new_chat()
	{
		return HttpChat{this};
	}

	void send_chat(HttpChat & chat)
	{
		if (chat.connection = use_connection(); chat.connection != no_connection)
		{
		}
		else
		{
			chat.state = State::NetworkError;
			chat.error = Error::NoConnection;
			complete_chat(chat);
		}
	}

	void complete_chat(HttpChat & chat)
	{
		if (chat.connection != no_connection)
		{
			if (chat.state == State::Success)
				recycle_connection(chat.connection);
			else
				abandon_connection(chat.connection);
		}
	}


private:
	std::string domain;
	unsigned short port;

	TcpTarget * target;
	std::stack<Connection> idle;


	int use_connection()
	{
		if (!idle.empty())
		{
			Connection c = idle.top();
			idle.pop();
			return c;
		}
		else
			return target->open_connection(port);
	}

	void recycle_connection(Connection c)
	{
		idle.push(c);
	}

	void abandon_connection(Connection c)
	{
		target->close_connection(c, true);
	}
};
}