#ifndef DataType_H
# define DataType_H

#include <map>
#include <string>
#include <vector>

typedef std::vector<std::string>::iterator Tokens;

struct ErrorPage
{
	bool IsRedirection;
	std::map<int, std::string> errorPages;
	ErrorPage(); // todo default pages
};


struct GlobalParam
{
	private:
		std::string							accessLog;
		std::string							errorLog;

		std::string							root;
		bool								autoIndex;
		long								maxBodySize; // in bytes
		long								maxHeaderSize; // in bytes
		std::vector<ErrorPage>				errorPages;
		std::map<std::string, std::string>	cgi_map;
		std::vector<std::string>			indexes;

		void validateOrFaild(Tokens &token, Tokens &end);
	public:
		static bool IsId(std::string &token);

		// getter and setter
		void setRoot(Tokens &token, Tokens &end);
		std::string getRoot() const;

		void setAutoIndex(Tokens &token, Tokens &end);
		bool getAutoIndex() const;

		void setAccessLog(Tokens &token, Tokens &end);
		std::string getAccessLog() const;

		void setErrorLog(Tokens &token, Tokens &end);
		std::string getErrorLog() const;

		void setMaxBodySize(Tokens &token, Tokens &end);
		long getMaxBodySize() const ;

		void setMaxHeaderSize(Tokens &token, Tokens &end);
		long getMaxHeaderSize() const;

		void setIndexes(Tokens &token, Tokens &end);
		// TODO Indexes getter may be with caching ??

		void setCGI(Tokens &token, Tokens &end);
		// TODO CGI getter may be with caching ??

		void setErrorPages(Tokens &token, Tokens &end);

 

};



#endif	
