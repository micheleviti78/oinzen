/* J. David's webserver */
/* This is a simple webserver.
 * Created November 1999 by J. David Blackstone.
 * CSE 4344 (Network concepts), Prof. Zeigler
 * University of Texas at Arlington
 */

/* C standard */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>

/* networking */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

/*
 isspace()
          checks  for  white-space  characters.   In  the  "C" and "POSIX" locales, these are:
          space, form-feed ('\f'), newline ('\n'), carriage return ('\r'), horizontal tab
	  ('\t'), and vertical tab ('\v').
	  */

#define ISspace(x) isspace((int)(x))
#define STRLEN(s) ((sizeof(s)/sizeof(s[0]))-1)
#define SEND(s) (send(client, s, STRLEN(s), 0))
#define SERVER_STRING "Server: J. David's webserver httpd/0.1.0\r\n"
#define SERVER_PORT 9999
#define SERVEFILE 0

void accept_request(int); /* Needed */
void bad_request(int);
void cat(int, FILE *); /* Needed */
void cannot_execute(int);
void error_die(const char *);
void execute_cgi(int, const char *, const char *, const char *);
int get_line(int, char *, int); /* Needed */
void headers(int, const char *); /* Needed */
void not_found(int);
void serve_file(int, const char *); /* Needed */
// int startup(uint32_t *);
void unimplemented(int);
/* Moreno */
void serve_index(int);

/**********************************************************************/
/* A request has caused a call to accept() on the server port to
 * return.  Process the request appropriately.
 * Parameters: the socket connected to the client */
/**********************************************************************/
int32_t isSpace(char c){ /* RETURN 1 IF THE CHAR IS NOT ALPHA-NUMERIC */

	if      (c ==  ' ') return 1;
	else if (c == '\f') return 1;
	else if (c == '\n') return 1;
	else if (c == '\r') return 1;
	else if (c == '\t') return 1;
	else if (c == '\v') return 1;

        return 0;
}

void accept_request(int client) /* LISTEN THE SOCKET */
{
	char buf[256];
	int numchars;
	char method[256];
	// char url[256];
	// char path[512];
	size_t i, j;
	// struct stat st;
	// int cgi = 0;      /* becomes true if server decides this is a CGI program */
	// char *query_string = NULL;

	numchars = get_line(client, buf, sizeof(buf)); /* buf contains the input string */
        printf("[ LOG ] buf %s\n", buf);
	i = 0;
	j = 0;

	while (!isSpace(buf[j]) && (i < sizeof(method) - 1))
	{
		method[i] = buf[j];
		i++;
		j++;
	}
	method[i] = '\0';
        printf("[ LOG ] method %s\n", method);
	if (strcasecmp(method, "GET") == 0) {
#if SERVEFILE
		serve_file(client, "index.html");
#else
		serve_index(client);
#endif
	}

	close(client);
}


/**********************************************************************/
/* Put the entire contents of a file out on a socket.  This function
 * is named after the UNIX "cat" command, because it might have been
 * easier just to do something like pipe, fork, and exec("cat").
 * Parameters: the client socket descriptor
 *             FILE pointer for the file to cat */
/**********************************************************************/
void cat(int client, FILE *resource)
{
	char buf[1024];

	fgets(buf, sizeof(buf), resource);
	while (!feof(resource))
	{
		send(client, buf, strlen(buf), 0);
		fgets(buf, sizeof(buf), resource);
	}
}


/**********************************************************************/
/* Print out an error message with perror() (for system errors; based
 * on value of errno, which indicates system call errors) and exit the
 * program indicating an error. */
/**********************************************************************/
void error_die(const char *sc)
{
	perror(sc);
	exit(1);
}


/**********************************************************************/
/* Get a line from a socket, whether the line ends in a newline,
 * carriage return, or a CRLF combination.  Terminates the string read
 * with a null character. If no newline indicator is found before the
 * end of the buffer, the string is terminated with a null. If any of
 * the above three line terminators is read, the last character of the
 * string will be a linefeed and the string will be terminated with a
 * null character.
 * Parameters: the socket descriptor
 *             the buffer to save the data in
 *             the size of the buffer
 * Returns: the number of bytes stored (excluding null) */
/**********************************************************************/
int get_line(int sock, char *buf, int size) /* READ FROM THE SOCKET THE INPUT STRING */
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n'))
	{
	    n = recv(sock, &c, 1, 0);
	    /* recv( ... ) receive a message from a socket
	     ssize_t recv(int sockfd, void *buf, size_t len, int flags);
	     */
	        if (n > 0)
		{
			if (c == '\r')
			{
			    n = recv(sock, &c, 1, MSG_PEEK);
	        	    if ((n > 0) && (c == '\n'))
				recv(sock, &c, 1, 0);
			    else
				c = '\n';
			}
			buf[i] = c;
			i++;
		}
		else
		    c = '\n';
	}
	buf[i] = '\0'; /* NULL terminate the string */

	return(i);
}

/**********************************************************************/
/* Return the informational HTTP headers about a file. */
/* Parameters: the socket t\o print the headers on
 *             the name of the file */
/**********************************************************************/
inline int32_t size_of(char *buf){

    int32_t i = 0;

    while (buf[i] != '\0') i++;

    return i;
}
#if SERVERILE
void headers(int client, const char *filename)
{
#if 0

	char buf[1024];
	(void) filename;  /* could use filename to determine file type */
	static const char msg[] = "HTTP/1.1 200 OK\r\n";
        printf("sizeof %d, strlen %d \n", sizeof(msg)/sizeof(msg[0]), strlen(msg));
	strcpy(buf, "HTTP/1.1 200 OK\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, SERVER_STRING);
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	strcpy(buf, "\r\n");
	send(client, buf, strlen(buf), 0);

#else

	static const char msg_0[] = "HTTP/1.0 200 OK\r\n";
	static const char msg_1[] = SERVER_STRING;
	static const char msg_2[] = "Content-Type: text/html\r\n";
	static const char msg_3[] = "\r\n";
#if 0

	send(client, msg_0, STRLEN(msg_0), 0);
	send(client, msg_1, STRLEN(msg_1), 0);
	send(client, msg_2, STRLEN(msg_2), 0);
	send(client, msg_3, STRLEN(msg_3), 0);

#else

	SEND( msg_0 );
	SEND( msg_1 );
	SEND( msg_2 );
        SEND( msg_3 );

#endif

#endif
        return;
}
#endif

/**********************************************************************/
/* Send a regular file to the client.  Use headers, and report
 * errors to client if they occur.
 * Parameters: a pointer to a file structure produced from the socket
 *              file descriptor
 *             the name of the file to serve */
/**********************************************************************/
#if SERVEFILE
void serve_file(int client, const char *filename)
{
	FILE *resource = NULL;
	int numchars = 1;
	char buf[256];

	buf[0] = 'A';
	buf[1] = '\0';

	while ((numchars > 0) && strcmp("\n", buf)) {  /* read & discard headers */
		numchars = get_line(client, buf, sizeof(buf));
		printf("numchars %d %s\n", numchars, buf);
	}
        printf("[ LOG ] serve_file buf %s\n", buf);
	resource = fopen(filename, "r");
	if (resource == NULL)
		printf("File not found\n"); //not_found(client);
	else
	{
		headers(client, filename);
		cat(client, resource);
	}
	fclose(resource);
}

#endif

void serve_index(int client){

	static int numchars = 1;
	static int32_t s_len = 0;
        static const char Agent[] = "USER-AGENT";
	static char buf[256];
	static char out[256];

	buf[0] = 'A';
	buf[1] = '\0';

	while ((numchars > 0) && strcmp("\n", buf))  {/* read & discard headers */
		numchars = get_line(client, buf, sizeof(buf));

#if 1

		printf("%d %s \n", numchars, buf);
                {
		const char OFFSET = 'a' - 'A';
		char *q = &buf[0];
#if 0
		while (*q) {
			*q = (*q >= 'a' && *q <= 'z') ? *q -= OFFSET : *q;
                        q++;
		}
#else
		if ((*q = 'a') || (*q= 'A')) {
			for (uint32_t i=0; i < STRLEN( Agent ); i++) {
				*q = (*q >= 'a' && *q <= 'z') ? *q -= OFFSET : *q;
				q++;
			}
		}
#endif
                printf("BUFFER %s\n", buf);
		}

		if (strncmp(Agent, buf, STRLEN( Agent )) == 0) {
                        printf("strcmp %d \n", strncmp(Agent, buf, STRLEN( Agent )));
			s_len = sprintf(out, "<p> User-Agent: %s </p>", &buf[STRLEN( Agent )+2]);
                        printf("%s \n", out);
		}

#endif

	}

	printf("[ LOG ] serve_file buf %s\n", buf);

#include "index.h"
	static const char msg_0[] = "HTTP/1.0 200 OK\r\n";
	static const char msg_1[] = SERVER_STRING;
	static const char msg_2[] = "Content-Type: text/html\r\n";
	static const char msg_3[] = "\r\n";
        /* Sending Header */
	SEND( msg_0 );
	send(client, msg_1, STRLEN(msg_1), 0);
	send(client, msg_2, STRLEN(msg_2), 0);
	send(client, msg_3, STRLEN(msg_3), 0);

	/* Sending index.html */
	send(client, index_1, STRLEN(index_1), 0);
	SEND( index_2 );
	SEND( index_3 );
	SEND( index_4 );
	SEND( index_5 );
	SEND( index_6 );
	SEND( index_7 );
	SEND( index_8 );
	SEND( index_8 );
	SEND( index_9 );
	SEND( index_10 );
	SEND( index_11 );
        send(client, out, s_len, 0);
	SEND( index_12 );
	SEND( index_13 );
	SEND( index_14 );
	SEND( index_15 );
	SEND( index_16 );

	return;
}


/**********************************************************************/

#if 1
int main(void)
#else
int tinyd(void)
#endif
{
	int server_sock = -1;
	uint32_t port = SERVER_PORT;
	int client_sock = -1;
	struct sockaddr_in client_name;
        int client_name_len = sizeof(client_name);
        int httpd = -1;
        struct sockaddr_in name;
	int name_len = sizeof(name);
	struct protoent *p;

	p = getprotobyname("tcp"); /* get the protocol numer for tcp */
	/*
	 int socket(int domain, int type, int protocol);
	 */
	httpd = socket(AF_INET, SOCK_STREAM, p->p_proto); /* open a socket for IP4, type stream bidirectional, tcp */

	if (httpd == -1) {
		error_die("socket");
	}

	memset(&name, 0, name_len);
	name.sin_family = AF_INET;
	name.sin_port = htons(port);              /* convert values between host and network byte order */
	name.sin_addr.s_addr = htonl(INADDR_ANY); /* convert values between host and network byte order */
        /*
	 int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
	*/
	if (bind(httpd, (struct sockaddr*) &name, name_len) < 0) { /*  bind a name to a socket */
		error_die("bind");
	}
	/* int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen); */
	if (getsockname(httpd, (struct sockaddr*) &name, (uint32_t*) &name_len) < 0) {
                error_die("getsockname");
	}
	/*
	 listen - listen for connections on a socket
	 #include <sys/types.h>
	 #include <sys/socket.h>

	 int listen(int sockfd, int backlog);
	 */
	if (listen(httpd, 1) < 0) {
		error_die("listen");
	}

	printf("[ LOG ] httpd running on port %d\n", port);

	while (1)
	{
		/*
		 accept â€accept a new connection on a socket

		 #include <sys/socket.h>

		 int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
		 */
		client_sock = accept(httpd,
				     (struct sockaddr*) &client_name,
				     (uint32_t*) &client_name_len);
		if (client_sock == -1) {
			error_die("accept");
		}

		accept_request(client_sock);
	}
	/*
	 close â€” close a file descriptor

	 #include <unistd.h>

	 int close(int fildes);
	 */
	close(server_sock);

}
