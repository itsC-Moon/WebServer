

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 512

void response(int fd)
{
	const char *http_200_response =
		"HTTP/1.1 204 OK\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Connection: Close\r\n"
		"Server: YOUR DADDY\r\n"
		"Content-Length: 0\r\n"
		"\r\n";
		// "<!DOCTYPE html>\n"
		// "<html>\n"
		// "<head><title>200 OK</title></head>\n"
		// "<body>\n"
		// "<h1>200 OK</h1>\n"
		// "<p>The request has succeeded.</p>\n"
		// "</body>\n"
		// "</html>";

	const char *http_404_response =
		"HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Connection: Close\r\n"
		"Content-Length: 175\r\n"
		"\r\n"
		"<!DOCTYPE html>\n"
		"<html>\n"
		"<head><title>404 Not Found</title></head>\n"
		"<body>\n"
		"<h1>404 Not Found</h1>\n"
		"<p>The requested resource could not be found on this server.</p>\n"
		"</body>\n"
		"</html>";
	// if (p != "/")
		write(fd, http_404_response, strlen(http_404_response));
	// else
	// 	write(fd, http_200_response, strlen(http_200_response));
}
// Set a socket to non-blocking mode
int set_non_blocking(int sockfd)
{
	int flags = fcntl(sockfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl(F_GETFL)");
		return -1;
	}
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		perror("fcntl(F_SETFL)");
		return -1;
	}
	return 0;
}

int main()
{
	int server_fd, client_fd, kq;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);

	// Create a server socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	int optval = 1;
	 setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	// Set the server socket to non-blocking mode
	if (set_non_blocking(server_fd) == -1)
	{
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// Configure server address
	address.sin_family = AF_INET;
	if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0) return 1;
	address.sin_port = htons(PORT);

	// Bind the server socket
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// Start listening for incoming connections
	if (listen(server_fd, 10) < 0)
	{
		perror("listen failed");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// Create a kqueue
	if ((kq = kqueue()) == -1)
	{
		perror("kqueue");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	// Add server socket to kqueue for monitoring incoming connections
	struct kevent ev_set;
	EV_SET(&ev_set, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	if (kevent(kq, &ev_set, 1, NULL, 0, NULL) == -1)
	{
		perror("kevent");
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	printf("Server listening on port %d...\n", PORT);

	while (1)
	{
		struct kevent ev_list[MAX_EVENTS];
		int nev = kevent(kq, NULL, 0, ev_list, MAX_EVENTS, NULL);
		if (nev == -1)
		{
			perror("kevent");
			close(server_fd);
			exit(EXIT_FAILURE);
		}

		// Process the events
		for (int i = 0; i < nev; i++)
		{
			if (ev_list[i].ident == (u_long)server_fd)
			{
				// Incoming connection
				client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
				if (client_fd == -1)
				{
					perror("accept");
					continue;
				}

				// Set the client socket to non-blocking mode
				if (set_non_blocking(client_fd) == -1)
				{
					close(client_fd);
					continue;
				}

				// Add the client socket to kqueue for monitoring readable data
				EV_SET(&ev_set, client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
				if (kevent(kq, &ev_set, 1, NULL, 0, NULL) == -1)
				{
					perror("kevent");
					close(client_fd);
				}

				printf("Accepted new connection\n");
			}
			else if (ev_list[i].filter == EVFILT_READ)
			{
				// Data available to read from a client socket
				char buffer[BUFFER_SIZE];
				ssize_t n = read(ev_list[i].ident, buffer, sizeof(buffer) - 1);
				if (n <= 0)
				{
					if (n < 0)
						perror("read");
					// Client disconnected or error
					close(ev_list[i].ident);
					printf("Client disconnected\n");
				}
				else
				{
					buffer[n] = '\0'; // Null-terminate the string
					printf("Received message: %s\n", buffer);
				}
			}
			else if (ev_list[i].filter == EVFILT_READ)
			{
				printf("write\n");
				response(ev_list[i].ident);
			}
		}
	}

	close(server_fd);
	return 0;
}
