#include <fcntl.h>
#include <sys/event.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <unistd.h>
#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "DataType.hpp"
#include <Tokenizer.hpp>

GlobalConfig::GlobalConfig()
{
	this->autoIndex = -1;
	this->errorPages["."] = "";
	this->IsAlias = false; // INFO: this is art Do not touch unless you have a permit.
}


GlobalConfig::GlobalConfig(int autoIndex, const std::string &upload_file_path)
{
	this->autoIndex = autoIndex;
	this->upload_file_path = upload_file_path;
}

GlobalConfig::GlobalConfig(const GlobalConfig &other)
{
	*this = other;
}
GlobalConfig &GlobalConfig::operator=(const GlobalConfig &other)
{
	std::map<std::string, std::string>::const_iterator kv = other.errorPages.begin();
	if (this == &other)
		return *this;

	if (root.empty())
	{
		root = other.root;
		this->IsAlias = other.IsAlias;
	}
	if (upload_file_path.empty())
		upload_file_path = other.upload_file_path;
	if (autoIndex == -1)
		autoIndex = other.autoIndex;
	for (; kv != other.errorPages.end(); kv++)
	{
		if (this->errorPages.find(kv->first) == this->errorPages.end())
			this->errorPages.insert(*kv);
	}
	for (size_t i = 0; i < other.indexes.size(); i++)
	{
		std::vector<std::string>::const_iterator it =
			std::find(this->indexes.begin(), this->indexes.end(), other.indexes[i]);
		if (it == this->indexes.end())
			this->indexes.push_back(other.indexes[i]);
	}
	return *this; // Return *this to allow chained assignments
}
GlobalConfig::~GlobalConfig() {}

void GlobalConfig::setRoot(Tokens &token, Tokens &end)
{
	struct stat buf;

	if (!this->root.empty())
		throw Tokenizer::ParserException("Root directive is duplicate");
	validateOrFaild(token, end);
	this->root = consume(token, end);
	if (stat(this->root.c_str(), &buf) != 0)
		throw Tokenizer::ParserException("Root directory does not exist");
	if (S_ISDIR(buf.st_mode) == 0)
		throw Tokenizer::ParserException("Root is not a directory");
	CheckIfEnd(token, end);
}

std::string GlobalConfig::getRoot() const
{
	return this->root; // Return the root
}

void GlobalConfig::setAutoIndex(Tokens &token, Tokens &end)
{
	validateOrFaild(token, end);

	if (*token == "on")
		this->autoIndex = true;
	else if (*token == "off")
		this->autoIndex = false;
	else
		throw Tokenizer::ParserException("Invalid value for autoindex");
	token++;
	CheckIfEnd(token, end);
}

bool GlobalConfig::getAutoIndex() const
{
	return this->autoIndex; // Return autoIndex
}

void GlobalConfig::setIndexes(Tokens &token, Tokens &end)
{
	validateOrFaild(token, end);
	while (token != end && *token != ";")
		this->indexes.push_back(consume(token, end));
	CheckIfEnd(token, end);
}

bool GlobalConfig::IsId(std::string &token)
{
	return (token == ";" || token == "}" || token == "{");
}

void GlobalConfig::validateOrFaild(Tokens &token, Tokens &end)
{
	token++;
	if (token == end || IsId(*token))
		throw Tokenizer::ParserException("Unexpected token: " + (token == end ? "end of file" : *token));
}

void GlobalConfig::CheckIfEnd(Tokens &token, Tokens &end)
{
	if (token == end)
		throw Tokenizer::ParserException("Unexpected end of file");
	else if (*token != ";")
		throw Tokenizer::ParserException("Unexpected `;` found: " + *token);
	token++;
}

std::string &GlobalConfig::consume(Tokens &token, Tokens &end)
{
	if (token == end)
		throw Tokenizer::ParserException("Unexpected end of file");
	if (IsId(*token))
		throw Tokenizer::ParserException("Unexpected token: " + *token);
	return *token++;
}
bool GlobalConfig::parseTokens(Tokens &token, Tokens &end)
{
	if (token == end)
		throw Tokenizer::ParserException("Unexpected end of file");
	else if (*token == "root")
		this->setRoot(token, end);
	else if (*token == "autoindex")
		this->setAutoIndex(token, end);
	else if (*token == "index")
		this->setIndexes(token, end);
	else if (*token == "error_page")
		this->setErrorPages(token, end);
	else if (*token == "alias")
		this->setAlias(token, end);
	else
		throw Tokenizer::ParserException("Invalid token: " + *token);
	return (true);
}

void GlobalConfig::setErrorPages(Tokens &token, Tokens &end)
{
	std::vector<std::string> vec;
	std::string content;

	std::string str;
	this->validateOrFaild(token, end);
	while (token != end && *token != ";")
		vec.push_back(this->consume(token, end));
	if (vec.size() <= 1)
		throw Tokenizer::ParserException("Invalid error page define");
	this->CheckIfEnd(token, end);
	if (access(vec.back().data(), F_OK | R_OK) == -1)
		throw Tokenizer::ParserException("file does not exist" + vec.back());
	for (size_t i = 0; i < vec.size() - 1; i++)
	{
		if (!this->isValidStatusCode(vec[i]))
			throw Tokenizer::ParserException("Invalid status Code " + vec[i]);
		this->errorPages[vec[i]] = vec.back().data();
	}
}

const std::vector<std::string> &GlobalConfig::getIndexes()
{
	return (this->indexes);
}

const std::string &GlobalConfig::getErrorPage(std::string &StatusCode)
{
	const static std::string  empty = "";

	const std::map<std::string, std::string>::iterator &kv = this->errorPages.find(StatusCode);
	if (kv == this->errorPages.end())
		return empty;
	return (kv->second);
}

void GlobalConfig::setAlias(Tokens &token, Tokens &end)
{
	struct stat buf;

	if (!this->root.empty())
		throw Tokenizer::ParserException("Alias directive is duplicate");
	validateOrFaild(token, end);
	this->root = consume(token, end);
	if (stat(this->root.c_str(), &buf) != 0)
		throw Tokenizer::ParserException("Alias directory does not exist");
	if (S_ISDIR(buf.st_mode) == 0)
		throw Tokenizer::ParserException("Alias is not a directory");
	this->IsAlias = true;
	CheckIfEnd(token, end);
}
bool GlobalConfig::getAliasOffset() const 
{
	return (this->IsAlias);
}

GlobalConfig::Proc::Proc()
{
	this->fin = -1;
	this->woffset = 0;
	this->fout = -1;
	this->pid = -1;
	this->state = NONE;
}

GlobalConfig::Proc &GlobalConfig::Proc::operator=(Proc &other)
{
	this->pid = other.pid;
	this->fin = other.fin;
	this->fout = other.fout;
	this->state = other.state;
	return (*this);
}

void GlobalConfig::Proc::die()
{
	// assert(this->pid > 0 && "Major Error Need to be fix: with proc");

	if (this->pid > 0)
		::kill(this->pid, SIGKILL);
	// this->pid = -1;
	
}

void GlobalConfig::Proc::clean()
{
	if (this->fin < 0 || this->fout < 0 )
		return ;
	close(this->fin);
	close(this->fout);
	this->fout = -1;
	this->fin = -1;
}

