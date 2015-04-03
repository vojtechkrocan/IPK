/***** C knihovny *****/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/***** C++ knihovny ******/
#include <iostream>
#include <vector>

/****** POMOCNA MAKRA ******/
#define DEBUG 1
#define BUFF_SIZE 100

/*
 * TO DO LIST:
 *	- /etc/passwd
 *	- pouzit vektor
 *	- dokumentace
 * 	- polozky oddelit mezerou
 *	- kdyz sestavujes odpoved, tak kdybys nasel vic matchu, tak pridej dalsi radek a dej to na nej
 */

using namespace std;
 
/****************************************************************************************************
 * Deklarace pomocnych funkci a promennych.
 ****************************************************************************************************
 */

typedef struct {
	
	
	int port;
	string hostname;
	} tOptions;
 
int Get_port(int, char* []);
int setNetwork(int*, struct sockaddr_in*, tOptions*);
int createSocket();
//int Message_send(string message, int socket);
//int Process_answer(string message);

/****************************************************************************************************
 * Main.
 ****************************************************************************************************
 */
int main(int argc, char* argv[])
{
	int soc, t, sin_len, msg_len;
	struct sockaddr_in sin;
	struct in_addr;
	char msg[80];
	//char recived_message[BUFF_SIZE];
	struct hostent* hp;
	int j;
	tOptions options;
	
	options.port = Get_port(argc, argv);
	
	// hodit do funkce
	setNetwork(&soc, &sin, &options);
	
	
	// pocud
	
	// cout << sin.sin_addr.s_addr << endl;
	if (bind(soc, (struct sockaddr *)&sin, sizeof(sin) ) < 0 )
	{
		cerr << "error on bind" << endl;
		return -1;												// bind error
	}
	if (listen(soc, 5))
	{ 
		cerr << "error on listen" << endl;						// listen error
		return -1;
	}
	sin_len = sizeof(sin);
	while (1)
	{
		/* accepting new connection request from client,
		socket id for the new connection is returned in t */
		
		if ( (t = accept(soc, (struct sockaddr *)&sin, (socklen_t*)&sin_len) ) < 0 )
		{
			cerr << "Error on accept." << endl;					// accept error
			return -1;
		}
		hp = (struct hostent *)gethostbyaddr((char *)&sin.sin_addr, 4, AF_INET);
		j = (int)(hp->h_length);
		options.hostname = inet_ntoa(sin.sin_addr);
		options.port = htons(sin.sin_port);
		cout << "From " << options.port << " (" << hp->h_name << ") : " << options.hostname << "." << endl;
		bzero(msg, 80*sizeof(char) );
		
		if ( read( t, msg, 80 ) <0)
		{  /* read message from client */
			cerr << "error on read" << endl;					// read error
			return -1;
		}
		msg_len = strlen(msg);
		cout << "length of message is " << msg_len << endl << "Message from client is:" << msg << endl;
		if ( write(t, msg, strlen(msg) ) < 0 )
		{														// echo message back
			cerr << "Error on write." << endl;
			return -1;											// write error
		}
		/* close connection, clean up sockets */
		if ( close(t) < 0 )
		{
			cerr << "Error on close." << endl;
			return -1;
		}
	} // not reach below
	if (close(soc) < 0)
	{
		cerr << "close error" << endl;
		return -1;
	}
	return 0;
}

/****************************************************************************************************
 * Funkce zpracovavajici argument, vracejici cislo portu.
 ****************************************************************************************************
 * @param argc, argv - ...
 * @return cislo portu z argumentu
 */
int Get_port(int argc, char* argv[])
{
	int port = -1;
	int option;
	while ( (option = getopt (argc, argv, "p:")) != -1 )					// pouziti getoptu
	{
		switch(option)
		{
			case 'p':
				// mozna pridat podminku, jestli to je cislo
				port = atoi(argv[2]);
				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}
	return port;
}

/****************************************************************************************************
 * Funkce.
 ****************************************************************************************************
 * @param sin - pointer na strukturu
 * @param soc_p - socket
 * @return - se stejne zahodi
 * - z prikladu k predmetu, jen vlozeno do funkce
 */
int setNetwork(int *soc_p, struct sockaddr_in* sin, tOptions* opts)
{
	*soc_p = createSocket();
	
	const char* host;
	struct hostent *hptr;
	sin->sin_family = PF_INET;	// set protocol family to Internet
	sin->sin_port = opts->port; // sin->sin_port = htons(options->port);
	sin->sin_addr.s_addr = INADDR_ANY;
	host = opts->hostname.c_str();
	if ( ( hptr = gethostbyname(host) ) == NULL)	// ziskani adresy
	{
		cerr << "Gethostbyname error: " << host << endl;
		exit(EXIT_FAILURE);
	}
	memcpy( &sin->sin_addr, hptr->h_addr, hptr->h_length);
	return 0;
}

/****************************************************************************************************
 * Funkce pro vytvoreni socketu.
 ****************************************************************************************************
 * @return - vytvoreny socket
 */
int createSocket()
{
	int new_soc;
	if ( (new_soc = socket(PF_INET, SOCK_STREAM, 0 ) ) < 0)	// create socket
	{
		cerr << "Error on socket." << endl;	// socket error
		exit(EXIT_FAILURE);
	}
	return new_soc;
}