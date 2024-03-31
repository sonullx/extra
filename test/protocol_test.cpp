#include "gtest/gtest.h"
#include "extra/HttpV1D0.h"

TEST(HttpV1D0, RequestFormat_0)
{
	using namespace extra::protocol::http;
	HttpRequestV1D0 req;
	req.set_method<Method::Get>();
	req.set_uri("/test");
	auto actual = req.format();
	std::string expected =
		"GET /test HTTP/1.0\r\n"
		"\r\n";

	EXPECT_EQ(actual, expected);
}

TEST(HttpV1D0, RequestFormat_1)
{
	using namespace extra::protocol::http;
	HttpRequestV1D0 req;
	req.set_method<Method::Get>();
	req.set_uri("/test");
	req.append_header("hello", "world");
	req.set_body_at_last("The quick brown fox jumps over the lazy dog.");
	auto actual = req.format();
	std::string expected =
		"GET /test HTTP/1.0\r\n"
		"hello: world\r\n"
		"Content-Length: 44\r\n"
		"\r\n"
		"The quick brown fox jumps over the lazy dog.";

	EXPECT_EQ(actual, expected);
}

TEST(HttpV1D0, ResponseParse_0)
{
	std::string s =
		"HTTP/1.0 200 OK\r\n"
		"hello: world\r\n"
		"A:       \t     B\r\n"
		"C:\r\n"
		" D\r\n"
		" E\r\n"
		"Content-Length: 44\r\n"
		"\r\n"
		"The quick brown fox jumps over the lazy dog.";

	std::string_view raw = s;

	using namespace extra::protocol::http;
	HttpResponseV1D0 resp;
	resp.parse(raw);
	auto version = resp.get_version();
	auto status = resp.get_status();
	auto phrase = resp.get_phrase();
	auto headers = resp.get_headers();
	auto body = resp.get_body();

	EXPECT_EQ(version, "HTTP/1.0");
	EXPECT_EQ(status, "200");
	EXPECT_EQ(phrase, "OK");
	EXPECT_EQ(body, "The quick brown fox jumps over the lazy dog.");
	auto iter = headers.find("Content-Length");
	EXPECT_NE(iter, headers.end());
	EXPECT_EQ(iter->second, "44");
	iter = headers.find("hello");
	EXPECT_NE(iter, headers.end());
	EXPECT_EQ(iter->second, "world");
	iter = headers.find("A");
	EXPECT_NE(iter, headers.end());
	EXPECT_EQ(iter->second, "B");
	iter = headers.find("C");
	EXPECT_NE(iter, headers.end());
	EXPECT_EQ(iter->second, "D, E");
}
