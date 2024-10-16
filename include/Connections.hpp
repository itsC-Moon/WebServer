#ifndef CONNECTIONS_HPP
#define CONNECTIONS_HPP

#include "Client.hpp"
#include "ServerContext.hpp"
#include <map>

typedef std::map<int, Client *>::iterator clients_it; // WARNING 

class Connections 
{
	private:
		ServerContext	*ctx;
	public:
		std::map<int, Client *> clients;

		Connections(ServerContext *ctx);
		~Connections();
		
		void		closeConnection(int	fd);
		void		addConnection(int	fd);
		void		addConnection(int	fd, int server);
		void		connecting(int	fd);
		void		requestHandler(int	fd);
};

#endif