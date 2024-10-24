
#include "ServerContext.hpp"
#include "DataType.hpp"
#include "ParserException.hpp"
#include "VirtualServer.hpp"
#include <vector>

ServerContext::ServerContext()
{
	this->maxBodySize = 100 * 1024 * 1024;
	this->maxHeaderSize = 100 * 1024 * 1024;
}

ServerContext::~ServerContext()
{
	for (size_t i = 0; i < this->servers.size(); i++)
		this->servers[i].deleteRoutes();
}
// TODO : check if value is was set  for duplicates
//
std::vector<VirtualServer> &ServerContext::getServers()
{
	return this->servers;
}
void ServerContext::parseTokens(Tokens &token, Tokens &end)
{
	if (*token == "max_body_size")
		this->setMaxBodySize(token, end);
	else if (*token == "max_header_size")
		this->setMaxHeaderSize(token, end);
	else
	this->globalParam.parseTokens(token, end);
}
void ServerContext::pushServer(Tokens &token, Tokens &end)
{
	token++;
	if (token == end)
		throw ParserException("Unexpected end of file");
	else if (*token != "{")
		throw ParserException("Unexpact token: " + *token);
	token++;

	this->servers.push_back(VirtualServer());  // push empty VirtualServer to keep 
											   // the reference in http object in case of exception to cause memory leak

	VirtualServer &server = this->servers.back(); // grants access to the last element

	while (token != end && *token != "}")
	{
		if (*token == "location")
			server.pushLocation(token, end);
		else
			server.parseTokens(token, end);
	}
	if (token == end || *token != "}")
		throw ParserException("Unexpected end of file");
	token++;
}

std::vector<VirtualServer> &ServerContext::getVirtualServers()
{
	return this->servers;
}

static long toBytes(std::string &size)
{
	long long sizeValue = 0;

	for (size_t i = 0; i < size.size() - 1; i++)
	{
		if (size[i] < '0' || size[i] > '9')
			throw ParserException("Invalid size");
		sizeValue = sizeValue * 10 + size[i] - '0';
		if (sizeValue > (1 << 16))
			throw ParserException("Size too large");
	}
	switch (size[size.size() - 1])
	{
		case 'k':
		case 'K': sizeValue *= 1024; break;
		case 'M':
		case 'm': sizeValue *= 1024 * 1024; break;
		default: return (-1);
	}
	// if (sizeValue > MAX_REQ_SIZE) // max size 30M;
	// 	return (-1);
	return (sizeValue);
}
void ServerContext::setMaxBodySize(Tokens &token, Tokens &end)
{
	globalParam.validateOrFaild(token, end);
	this->maxBodySize = toBytes(*token);
	if (this->maxBodySize == -1)
		throw ParserException("Invalid max body size or too large max is 100");
	token++;
	globalParam.CheckIfEnd(token, end);
}

long ServerContext::getMaxBodySize() const
{
	return this->maxBodySize; // Return max body size
}

void ServerContext::setMaxHeaderSize(Tokens &token, Tokens &end)
{
	globalParam.validateOrFaild(token, end);
	this->maxHeaderSize = toBytes(*token);

	if (this->maxHeaderSize == -1)
		throw ParserException("Invalid max header size or too large max is 100");
	token++;
	globalParam.CheckIfEnd(token, end);
}

long ServerContext::getMaxHeaderSize() const
{
	return this->maxHeaderSize; // Return max header size
}



