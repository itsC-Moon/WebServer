#include "ServerContext.hpp"
#include <unistd.h>
#include <ctime>
#include "Tokenizer.hpp"
#include "Log.hpp"

ServerContext::ServerContext() : globalParam(0, "/tmp")
{
	// TODO: add reverse mimes types;
	std::string types[] = {
		"application/octet-stream",
		"text/plain",
		"text/html",
		"text/html",
		"text/html",
		"text/css",
		"text/xml",
		"image/gif",
		"image/jpeg",
		"image/jpeg",
		"application/javascript",
	};

	const std::string ext[] = {
		".",
		"txt",
		"html",
		"shtml",
		"htm",
		"css",
		"xml",
		"gif",
		"jpeg",
		"jpg",
		"js",
	};
	for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++)
		this->types[ext[i]] = types[i];
	this->maxBodySize = 100 * 1024 * 1024;
	this->maxHeaderSize = 100 * 1024 * 1024;
	this->keepAliveTimeout = 75; // second
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
		throw Tokenizer::ParserException("Unexpected end of file");
	else if (*token != "{")
		throw Tokenizer::ParserException("Unexpact token: " + *token);
	token++;

	this->servers.push_back(VirtualServer()); // push empty VirtualServer to keep
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
		throw Tokenizer::ParserException("Unexpected end of file");
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
			throw Tokenizer::ParserException("Invalid size");
		sizeValue = sizeValue * 10 + size[i] - '0';
		if (sizeValue > (1 << 16))
			throw Tokenizer::ParserException("Size too large");
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
		throw Tokenizer::ParserException("Invalid max body size or too large max is 100");
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
		throw Tokenizer::ParserException("Invalid max header size or too large max is 100");
	token++;
	globalParam.CheckIfEnd(token, end);
}

long ServerContext::getMaxHeaderSize() const
{
	return this->maxHeaderSize; // Return max header size
}

void ServerContext::addTypes(Tokens &token, Tokens &end)
{
	std::string type = this->globalParam.consume(token, end);
	std::string ext = this->globalParam.consume(token, end);
	this->globalParam.CheckIfEnd(token, end);
	this->types[ext] = type;
}

void ServerContext::pushTypes(Tokens &token, Tokens &end)
{
	token++;
	if (token == end || *token != "{")
		throw Tokenizer::ParserException("Unexpected  " + ((token == end) ? "end of file" : "token " + *token));
	token++;
	while (token != end && *token != "}")
		this->addTypes(token, end);
	if (token == end)
		throw Tokenizer::ParserException("Unexpected end of file");
	token++;
}
const std::string &ServerContext::getType(const std::string &ext)
{
	std::map<std::string, std::string>::iterator kv = this->types.find(ext);

	if (kv == this->types.end())
		return (this->types.find(".")->second);
	return (kv->second);
}
void ServerContext::init()
{
	if (this->servers.size() == 0)
		throw Tokenizer::ParserException("No Virtual Server has been define");
	for (size_t i = 0; i < this->servers.size(); i++)
	{
		this->servers[i].globalConfig = this->globalParam;
		this->servers[i].init();
	}
}

void ServerContext::setErrorLog(Tokens &token, Tokens &end)
{
	this->globalParam.validateOrFaild(token, end);
	std::string error_log = this->globalParam.consume(token, end);
	Log::setErrorLogFile(error_log);
	this->globalParam.CheckIfEnd(token, end);
}
void ServerContext::setAccessLog(Tokens &token, Tokens &end)
{
	this->globalParam.validateOrFaild(token, end);
	std::string access_log = this->globalParam.consume(token, end);
	Log::setAccessLogFile(access_log);
	this->globalParam.CheckIfEnd(token, end);
}

void ServerContext::setKeepAlive(Tokens &token, Tokens &end)
{
	this->globalParam.validateOrFaild(token, end);
	std::string s_time = this->globalParam.consume(token, end);
	this->keepAliveTimeout = 0;
	for (size_t i = 0;i<s_time.size();i++)
	{
		if (s_time[i] <'0' || s_time[i] > '9' )
			throw Tokenizer::ParserException("Invalid keep alive Timeout value");
		this->keepAliveTimeout = this->keepAliveTimeout * 10 + s_time[i] - '0';
		if (this->keepAliveTimeout > (1 << 30))
			throw Tokenizer::ParserException("Invlaid keep alive Timeout value");
	}
	this->globalParam.CheckIfEnd(token, end);
}

int ServerContext::getCGITimeOut() const
{
	return (this->CGITimeOut);
}

int ServerContext::getClientReadTime() const 
{
	return (this->ClientReadTime);
}

int	ServerContext::getKeepAliveTime() const
{
	return (this->keepAliveTimeout);
}
