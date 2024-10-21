#include "CgiHandler.hpp"
#include "DataType.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <cstddef>
#include <iostream>
#include <sstream>
#include <sys/unistd.h>
#include <sys/wait.h>
#include <unistd.h>


// CgiHandler::CgiHandler(std::string reqPath, httpError& status, Location *location, ServerContext *ctx) : reqPath(reqPath), status(status), location(location), ctx(ctx)
// {
// }

CgiHandler::CgiHandler(HttpResponse& response) : response(&response)
{
	envArr = NULL;
	argv = NULL;
}

CgiHandler::~CgiHandler()
{
	//TODO:free the fucking env array
	size_t i = 0;//free the env array;
	while (envArr != NULL && envArr[i] != NULL) {
	    delete [] envArr[i];
		i++;
		envArr[i] = NULL;
	}
	delete [] envArr;
	envArr = NULL;
	i = 0;//free argv;
	while (argv != NULL && argv[i] != NULL) {
	    delete [] argv[i];
		i++;
		argv[i] = NULL;
	}
	delete [] argv;
	argv = NULL;
}

int		CgiHandler::checkCgiFile()
{
	scriptPath = response->location->globalConfig.getRoot() + response->path;
	if (access(scriptPath.c_str(), F_OK) == -1)
		return (response->setHttpResError(404, "Not Found"), 0);
	if (access(scriptPath.c_str(), X_OK) == -1)
		return (response->setHttpResError(403, "Forbidden"), 0);
	return (1);
}

int			CgiHandler::initEnv()
{
	if (!checkCgiFile())
		return (0);
	env["SERVER_SOFTWARE"] = "macOS";
	env["SERVER_NAME"] = "";//TODO:server
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_PORT"] = "8000";//TODO
	env["REQUEST_METHOD"] = response->strMethod;
	// std::cout << "---------------------> " << response->strMethod << std::endl; 
	env["SCRIPT_NAME"] = response->path;
	env["PATH_INFO"] = response->path;
	env["QUERY_STRING"] = response->queryStr;
	env["CONTENT_TYPE"] = response->headers["Content-type"];
	if (env["REQUEST_METHOD"] == "POST")
	{
		std::ostringstream oss;
		oss << response->getBody().size();
		env["CONTENT_LENGTH"] = oss.str();
		// std::cout << "---------------------> " << env["CONTENT_LENGTH"] << std::endl; 
	}
	else
		env["CONTENT_LENGTH"] = "0";
	env["REMOTE_ADDR"] = "";//TODO: 
	env["HTTP_HOST"] = response->headers["Host"];
	return (1);
}

void	CgiHandler::envMapToArr(std::map<std::string, std::string> mapEnv)
{
	map_it it;

	if (!mapEnv.size())
		return ;
	this->envArr = new char *[mapEnv.size() + 1];
	size_t i = 0;
	for (it = mapEnv.begin(); it != mapEnv.end(); it++)
	{
		envArr[i] = new char[it->first.size() + it->second.size() + 2];
		std::string tmp = it->first + "=" + it->second;
		for (size_t j = 0; j < tmp.size(); j++)
		{
			envArr[i][j] = tmp[j];
		}
		envArr[i][tmp.size()] = 0;
		i++;
	}	
	this->envArr[mapEnv.size()] = NULL;
}

void		CgiHandler::initArgv()
{
	argv = new char *[3];
	argv[0] = new char[cgiPath.size() + 1];
	for (size_t i = 0; i < cgiPath.size();i++)
	{
		argv[0][i] = cgiPath[i];
	}
	argv[0][cgiPath.size()] = 0;
	argv[1] = new char [(response->location->globalConfig.getRoot() + response->path).size() + 1];
	std::string tmp = response->location->globalConfig.getRoot() + response->path;
	for (size_t i = 0; i < tmp.size();i++)
	{
		argv[1][i] = tmp[i];
	}
	argv[1][tmp.size()] = 0;
	argv[2] = NULL;
}

static int closeFds(int *fd1, int *fd2)
{
	if (fd1 != NULL)
	{
		close(fd1[0]);
		close(fd1[1]);
	}
	if (fd2 != NULL)
	{
		close(fd2[0]);
		close(fd2[1]);
	}
	return (1);
}

void		CgiHandler::execute(std::string cgiPath)
{
	int pipefdOut[2];
	int pipefdIn[2];

	this->cgiPath = cgiPath;
	if (!initEnv())
		return ;
	envMapToArr(env);
	initArgv();
	if (pipe(pipefdOut) < 0)
	{
		response->setHttpResError(500, "Internal Server Error");
		return ;
	}
	if (pipe(pipefdIn) < 0)
	{
		closeFds(NULL, pipefdOut);
		response->setHttpResError(500, "Internal Server Error");
		return ;
	}
	for (size_t i = 0; i < response->getBody().size(); i++)
	{
		if (env["REQUEST_METHOD"] == "POST")
			write(pipefdIn[1], &response->getBody()[i], 1);
	}
	int pid = fork();
	if (pid < 0)
	{
		closeFds(pipefdIn, pipefdOut);
		response->setHttpResError(500, "Internal Server Error");
		return ;
	}
	close(pipefdIn[1]);
	if (pid == 0)
	{
		close(pipefdOut[0]);
		if (dup2(pipefdOut[1], 1) < 0 && !close(pipefdOut[1]) && !close(pipefdIn[0]))
			return ;
		if (dup2(pipefdIn[0], 0) < 0 && !close(pipefdOut[1]) && !close(pipefdIn[0]))
			return ;
		if (execve(*argv, argv, envArr) < 0 && !close(pipefdOut[1]) && !close(pipefdIn[0]))
			return ;
	}
	int status;
	waitpid(pid, &status, 0);
	if (status)
		response->setHttpResError(500, "Internal Server Error");
	close(pipefdIn[0]);
	close(pipefdOut[1]);
	response->loadFile(pipefdOut[0]);
	close(pipefdOut[0]);
}
