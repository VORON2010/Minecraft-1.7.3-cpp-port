#pragma once

#include <string>
#include <vector>

#include "java/Type.h"

namespace HttpGet
{

struct Response
{
	
	int_t status = 0;
	std::vector<byte_t> body;
	std::string error;

	bool ok() const
	{
		return error.empty() && status >= 200 && status < 400;
	}
};







Response fetch(const std::string &url, int_t timeoutSeconds = 30,
	const std::string &proxyHost = std::string(), int_t proxyPort = 80);

}
