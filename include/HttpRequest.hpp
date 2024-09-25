#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

#define URI_MAX		 2048
#define REQSIZE_MAX  50000
#define BODY_MAX	 30000

enum reqMethode
{
	GET,
	POST,
	DELETE,
	NONE
};

enum reqState
{
	METHODE,
	PATH,
	HTTP_VERSION,
	REQUEST_LINE_FINISH,
	HEADER_NAME,
	HEADER_VALUE,
	HEADER_FINISH,
	BODY,
	BODY_FINISH,
	REQUEST_FINISH,
	ERROR,
	DEBUG
};

enum crlfState {
	READING,
	NLINE,
	RETURN,
	LNLINE,
	LRETURN
};

typedef struct httpError 
{
	int         code;
	std::string description;
} httpError;

typedef struct methodeStr
{
	std::string							tmpMethodeStr;
	std::string							eqMethodeStr;
	
} methodeStr;


class HttpRequest 
{
	private:
		const int							fd;
		enum crlfState						crlfState;

		enum reqMethode						methode;
		std::string                         path;

		std::map<std::string, std::string>	headers;
		std::string							currHeaderName;

		std::string							body; // TODO : body  may be binary (include '\0') fix this;
		int									bodySize;

		int                                 reqSize;
		int									reqBufferSize;
		size_t								reqBufferIndex;
		std::string							reqBuffer; // buffer  may be binary (include '\0') fix this;

		httpError							error;

		methodeStr							methodeStr;
		std::string							httpVersion;

		void		readRequest();

		void		parseMethod();
		void		parsePath();
		void		parseHttpVersion();
		void		parseHeaderName();
		void		parseHeaderVal();
		void		parseBody();
		void		crlfGetting();

		int			verifyUriChar(char c);
		void		checkHttpVersion(int *state);


		void returnHandle();
		void nLineHandle();

	public:
		enum reqState state;

		HttpRequest();
		HttpRequest(int fd);
		~HttpRequest();

		void		setHttpError(int code, std::string str);
		void		feed();

		void		setFd(int fd);


};

#endif
