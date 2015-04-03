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
	} tUrl;
 
int Get_port(int argc, char* argv[]);
//int Message_send(string message, int socket);
//int Process_answer(string message);

/****************************************************************************************************
 * Main.
 ****************************************************************************************************
 */
int main(int argc, char* argv[])
{
	int s, t, sin_len, msg_len;
	struct sockaddr_in sin;
	struct in_addr;
	char msg[80];
	// char recived_message;
	struct hostent* hp;
	int j;
	tUrl url;
	
	url.port = Get_port(argc, argv);
	
	// hodit do funkce
	
	if ( (s = socket(PF_INET, SOCK_STREAM, 0 ) ) < 0)
	{														
		cerr << "Error on socket" << endl;						// socket error
		return -1;
	}
	sin.sin_family = PF_INET;									// set protocol family to Internet */
	sin.sin_port = url.port;										// set port no.
	sin.sin_addr.s_addr = INADDR_ANY;   						// set IP addr to any interface
	
	// pocud
	
	// cout << sin.sin_addr.s_addr << endl;
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin) ) < 0 )
	{
		cerr << "error on bind" << endl;
		return -1;												// bind error
	}
	if (listen(s, 5))
	{ 
		cerr << "error on listen" << endl;						// listen error
		return -1;
	}
	sin_len = sizeof(sin);
	while (1)
	{
		/* accepting new connection request from client,
		socket id for the new connection is returned in t */
		
		if ( (t = accept(s, (struct sockaddr *)&sin, (socklen_t*)&sin_len) ) < 0 )
		{
			cerr << "Error on accept." << endl;					// accept error
			return -1;
		}
		hp = (struct hostent *)gethostbyaddr((char *)&sin.sin_addr, 4, AF_INET);
		j = (int)(hp->h_length);
		url.hostname = inet_ntoa(sin.sin_addr);
		url.port = htons(sin.sin_port);
		cout << "From " << url.port << " (" << hp->h_name << ") : " << url.hostname << "." << endl;
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
			cerr << "error on write" << endl;
			return -1;											// write error
		}
		/* close connection, clean up sockets */
		if ( close(t) < 0 )
		{
			cerr << "error on close" << endl;
			return -1;
		}
	} // not reach below
	if (close(s) < 0)
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