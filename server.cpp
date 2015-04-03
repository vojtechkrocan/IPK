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
	string parameters;
	int port;
	} tOptions;

void initOptions(tOptions*);
int getPort(int, char* [], tOptions*);
int setNetwork(int*, struct sockaddr_in*, tOptions*);
int createSocket();
int acceptConnection(struct sockaddr_in*, struct sockaddr_in*, int*, int*);
int communicate(int*);
string createAnswer(string);

/****************************************************************************************************
 * Main.
 ****************************************************************************************************
 */
int main(int argc, char* argv[])
{
	int welcome_soc, in_soc;
	struct sockaddr_in sin, client_sin;
	tOptions options;
	memset(&sin, 0, sizeof(sin));
	
	initOptions(&options);
	getPort(argc, argv, &options);
	setNetwork(&welcome_soc, &sin, &options);
	
	/*
	WHILE START
	while(1)
	{
		
	*/
	
	while (1)
	{
		int pid;
		/* accepting new connection request from client, socket id for the new connection is returned in t */
		acceptConnection(&client_sin, &sin, &welcome_soc, &in_soc);
		/*
		if( in_soc <= 0 ) continue;
		pid = fork();	// fork
		if ( pid < 0 )
		{
          cerr << "Error on fork." << endl;
          exit(EXIT_FAILURE);
        }
		
		if ( pid == 0 ) // child process
		{
			communicate(&in_soc);
		}
		else
		*/
		/* close connection, clean up sockets */
		if ( close(in_soc) < 0 )
		{
			cerr << "Error on close." << endl;
			return EXIT_FAILURE;
		}
	} // not reach below
	if (close(welcome_soc) < 0)
	{
		cerr << "Close error." << endl;
		return EXIT_FAILURE;
	}
	/*
	}
	WHILE END
	*/
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce zpracovavajici argument, vracejici cislo portu.
 ****************************************************************************************************
 * @param argc, argv - ...
 * @return cislo portu z argumentu
 */
int getPort(int argc, char* argv[], tOptions* opts)
{
	int option;
	if ( argc != 3 )
	{
		cerr << "Error: Too few or Too many parameters." << endl;
		exit(EXIT_FAILURE);
	}
	while ( (option = getopt (argc, argv, "p:")) != -1 )	// pouziti getoptu
	{
		switch(option)
		{
			case 'p':
				// mozna pridat podminku, jestli to je cislo
				opts->port = atoi(optarg);
				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}
	// if port neni cislo, skonci
	return EXIT_SUCCESS;
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
	sin->sin_family = PF_INET;	// set protocol family to Internet
	sin->sin_port = opts->port; // sin->sin_port = htons(options->port);
	sin->sin_addr.s_addr = INADDR_ANY;
	if ( bind(*soc_p, (struct sockaddr *)sin, sizeof(*sin) ) < 0 )
	{
		cerr << "Error on bind." << endl;
		exit(EXIT_FAILURE);	// bind error
	}
	if ( listen(*soc_p, 5) )
	{ 
		cerr << "Error on listen." << endl;	// listen error
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce pro vytvoreni socketu.
 ****************************************************************************************************
 * @return - vytvoreny socket
 */
int createSocket()
{
	int new_soc;
	if ( (new_soc = socket(PF_INET, SOCK_STREAM, 0 )) < 0 )	// create socket
	{
		cerr << "Error on socket." << endl;	// socket error
		exit(EXIT_FAILURE);
	}
	return new_soc;
}
/****************************************************************************************************
 * .
 ****************************************************************************************************
 * @param
 * @param
 * @param
 * @param
 * @return			zahodi se
 */
int acceptConnection(struct sockaddr_in* client_sin, struct sockaddr_in* sin, int* welcome_soc, int* in_soc)
{
	/* accepting new connection request from client, socket id for the new connection is returned in in_soc */
	int sin_len, port;
	string hostname;
	sin_len = sizeof(*sin);
	*in_soc = accept(*welcome_soc, (struct sockaddr *)sin, (socklen_t*)&sin_len);
	if(DEBUG)
	{
		hostname = inet_ntoa(sin->sin_addr);
		port = htons(sin->sin_port);
		cout << "From " << port << ": " << hostname << "." << endl;
	}
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * .
 ****************************************************************************************************
 * @param
 * @return			vytvoreny socket
 */
int communicate(int* in_soc)
{
	char recived_message[BUFF_SIZE];
	int msg_len;
	string request, answer;
	bzero(recived_message, BUFF_SIZE*sizeof(char) );
	
	/***** READ REQUEST *****/
	if ( read(*in_soc, recived_message, BUFF_SIZE ) <0)
	{
		cerr << "error on read" << endl;	// read error
		return -1;
	}
	msg_len = strlen(recived_message);
	cout << "Length of message is " << msg_len << ". Message from client is:" << endl << recived_message << endl;
	
	/***** CREATE ANSWER *****/
	request = recived_message;
	answer = createAnswer(request);
	/***** WRITE ANSWER *****/
	if ( send(*in_soc, answer.c_str(), answer.length(), 0) < 0 )	// odeslani odpovedi klientovi
	{
		perror("Error on sending.\n");	// chyba pri odesilani zpravy
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * .
 ****************************************************************************************************
 * @param
 * @return			vytvorena zprava
 */
string createAnswer(string request)
{
	int length;
	string answer;
	length = request.size();
	answer = "PIZDA\n";
	return answer;
}

/****************************************************************************************************
 * Funkce na inicializaci struktury s prepinaci.
 ****************************************************************************************************
 * @param tOptions* - uk. na strukturu k inicializaci
 */
void initOptions(tOptions* opts)
{
	opts->port = -1;
	opts->parameters = "";
}