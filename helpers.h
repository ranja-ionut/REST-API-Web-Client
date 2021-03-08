#ifndef _HELPERS_
#define _HELPERS_

#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>

/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

// HOST si PORT specificate in PDF
#define HOST "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
#define HOST_HDR "HOST: "
#define HOST_HDR_SIZE (sizeof(HOST_HDR))
#define PORT 8080

// URL-uri pentru fiecare comanda
#define REGISTER_URL "/api/v1/tema/auth/register"
#define LOGIN_URL "/api/v1/tema/auth/login"
#define LIBRARY_ACCESS_URL "/api/v1/tema/library/access"
#define LOGOUT_URL "/api/v1/tema/auth/logout"
/**
 * Chiar daca acestea 4 sunt acelasi lucru, in cazul in care erau
 * la alte adrese ar fi fost nevoie de o astfel de delimitare
 */
#define VIEW_BOOKS_URL "/api/v1/tema/library/books"
#define VIEW_BOOK_URL "/api/v1/tema/library/books/"
#define ADD_BOOK_URL "/api/v1/tema/library/books"
#define REMOVE_BOOK_URL "/api/v1/tema/library/books/"

/**
 * Elemente necesare pentru producerea JSON-urilor.
 */
#define CONTENT_APP_JSON "application/json"
#define LINKER ","
#define JSON_START "{"
#define USERNAME "\"username\":"
#define PASSWORD "\"password\":"
#define JSON_END "}"
#define JSON_STRING "\""
#define TITLE "\"title\":"
#define AUTHOR "\"author\":"
#define GENRE "\"genre\":"
#define PAGE_COUNT "\"page_count\":"
#define PUBLISHER "\"publisher\":"
#define JSON_SEARCH "{\""

#define forever while(1)

#define JWT_EXPIRATION "500 Internal Server Error"

// Coduri pentru fiecare comanda in parte
#define BAD_CMD -500
#define REGISTER 1
#define LOGIN 2
#define LIBRARY_ACCESS 3
#define VIEW_BOOKS 4
#define VIEW_BOOK 5
#define ADD_BOOK 6
#define DELETE_BOOK 7
#define LOGOUT 8
#define EXIT 500

// Comenzile suportate de client.
#define REGISTER_CMD "register"
#define LOGIN_CMD "login"
#define LIBRARY_ACCESS_CMD "enter_library"
#define VIEW_BOOKS_CMD "get_books"
#define VIEW_BOOK_CMD "get_book"
#define ADD_BOOK_CMD "add_book"
#define DELETE_BOOK_CMD "delete_book"
#define LOGOUT_CMD "logout"
#define EXIT_CMD "exit"

// Folosite in requests, buffer sau helper
#define BUFLEN 4096
#define LINELEN 2000
#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH_HDR "Content-Length: "
#define CONTENT_LENGTH_HDR_SIZE (sizeof(CONTENT_LENGTH_HDR) - 1)
#define CONTENT_TYPE_HDR "Content-Type: "
#define CONTENT_TYPE_HDR_SIZE (sizeof(CONTENT_TYPE_HDR))
#define COOKIE "Cookie: "
#define COOKIE_SIZE (sizeof(COOKIE))
#define SET_COOKIE "Set-Cookie: "
#define SET_COOKIE_SIZE (sizeof(SET_COOKIE))
#define AUTHORIZATION_HDR "Authorization: Bearer "
#define AUTHORIZATION_HDR_SIZE (sizeof(AUTHORIZATION_HDR))

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

#endif
