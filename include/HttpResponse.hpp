#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "HttpRequest.hpp"
#include "Location.hpp"
#include "ServerContext.hpp"
#include <string>


enum responseState 
{
	START,
	ERROR
};

enum pathType
{
	_DIR,
	INDEX,
	DEF_INDEX
};


class HttpResponse
{
	private:
		enum responseBodyType 
		{
			LOAD_FILE,	
			NO_TYPE,
			AUTO_INDEX,
			CGI
		};
		enum reqMethode
		{
			GET  = 0b1,
			POST = 0b10,
			DELETE = 0b100,
			NONE = 0
		};
		struct errorResponse 
		{
			std::string		statusLine;
			std::string		headers;
			std::string		connection;
			std::string		contentLen;

			std::string		bodyHead;
			std::string		title;
			std::string		body;
			std::string		htmlErrorId;
			std::string		bodyfoot;
		};

		enum cgiResponeState
		{
			HEADERS,
			BODY,
			PARSING_DONE
		};
		struct cgiRespone 
		{
			cgiResponeState	state;
			size_t			bodyStartIndex;
			std::string		cgiStatusLine;
			std::vector<std::vector<unsigned char > > lines;
		};

		cgiRespone							cgiRes;
		int									fd;
		enum reqMethode						methode;
		std::vector<unsigned char>							body;
		httpError							status;	
		bool								isCgiBool;
		errorResponse						errorRes;
		enum responseBodyType				bodyType;

		std::string							fullPath;
		static const int					fileReadingBuffer = 10240;
		std::string							autoIndexBody;
		ServerContext						*ctx;
		HttpRequest							*request;
	public:
		std::string							queryStr;
		std::string											getCgiContentLenght();
		int												parseCgiHaders(std::string str);
		std::string											strMethod;
		std::vector<std::vector<unsigned char> >			responseBody;
		int													keepAlive; // bool? // bool?
		Location											*location;
		enum responseState									state;
		std::string											path;
		std::map<std::string, std::string>					headers;
		std::map<std::string, std::string>					resHeaders;

		HttpResponse(int fd, ServerContext *ctx, HttpRequest *request);
		HttpResponse	operator=(const HttpRequest& req);

		void							responseCooking();
		bool							isCgi();
		void							cgiCooking();

		int								getStatusCode() const;
		std::string						getStatusDescr() const;

		bool							isPathFounded();
		bool							isMethodAllowed();
		int								pathChecking();
		void							setHttpResError(int code, std::string str);

		std::string						getErrorRes();
		std::string						getContentLenght(); // TYPO
		int								directoryHandler();
		int								loadFile(const std::string& pathName);
		int								loadFile(int _fd);//for cgi

		void							writeResponse();

		std::string						getStatusLine();
		std::string						getConnectionState();
		std::string						getContentType();
		std::string						getDate();
		int								sendBody(int _fd, enum responseBodyType type);
		std::string						getContentLenght(enum responseBodyType type); // TYPO

		int								autoIndexCooking();
		static std::string				getExtension(std::string str);
		std::vector<unsigned char>		getBody() const;


		void							parseCgiOutput();
		void							writeCgiResponse();

		void							decodingUrl();
		void							splitingQuery();
		int								uploadFile();
};

std::string			decimalToHex(int	decimal);

#endif
