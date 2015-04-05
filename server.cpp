/***** C knihovny *****/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/***** C++ knihovny ******/
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>

/****** POMOCNA MAKRA ******/
#define BUFF_SIZE 100

/*
 * TO DO LIST:
 *	- dokumentace
 */

using namespace std;
 
/****************************************************************************************************
 * Deklarace pomocnych funkci a promennych.
 ****************************************************************************************************
 */

typedef struct {
	int u_or_l;
	string parameters;
	string search;
	vector<string> goals;
	int port;
	} tOptions;

void initOptions(tOptions*);
int getPort(int, char* [], tOptions*);
int setNetwork(int*, struct sockaddr_in*, tOptions*);
int createSocket();
int acceptConnection(struct sockaddr_in*, int*, int*);
int communicate(int*, tOptions*);
string createAnswer(char *, tOptions*, int*);
int processRequest(string, tOptions*, int*);
int find(tOptions*);

/****************************************************************************************************
 * Main.
 ****************************************************************************************************
 */
int main(int argc, char* argv[])
{
	int welcome_soc, in_soc;
	struct sockaddr_in sin;
	tOptions options;
	memset(&sin, 0, sizeof(sin));

	initOptions(&options);
	getPort(argc, argv, &options);
	setNetwork(&welcome_soc, &sin, &options);
	while (1)
	{
		int pid;
		/* accepting new connection request from client, socket id for the new connection is returned in in_soc */
		acceptConnection(&sin, &welcome_soc, &in_soc);
		if( in_soc <= 0 ) continue;
		
		/***** FORK PROCESS *****/
		pid = fork();	// fork
		if ( pid < 0 )
		{
          cerr << "Error on fork." << endl;
          exit(EXIT_FAILURE);
        }
		if ( pid == 0 ) // child process
		{
			if (close(welcome_soc) < 0)	// uzavreni welcome soketu, ktery u potomka uz neni potreba
			{
				cerr << "Close error." << endl;
				return EXIT_FAILURE;
			}
			/***** SAMOTNA KOMUNIKACE *****/
			communicate(&in_soc, &options);
			
			if (close(in_soc) < 0)	// uzavreni komunkacniho soketu
			{
				cerr << "Close error." << endl;
				return EXIT_FAILURE;
			}
			break;
		}
		else	// parent process
		{
			// closing communication socket
			if ( close(in_soc) < 0 )
			{
				cerr << "Error on close." << endl;
				exit(EXIT_FAILURE);
			}
		}
	}
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
	int length;
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
				length = strlen(optarg);
				for (int j = 0; j < length; j++)
				{
					if ( !isdigit(optarg[j]) )
					{
						cerr << "Port has to be a number." << endl;
						exit(EXIT_FAILURE);
					}
				}
				opts->port = atoi(optarg);
				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce pro nabindovani a prijmuti prvniho soketu.
 ****************************************************************************************************
 * @param sin 		pointer na strukturu
 * @param soc_p		soket
 * @param opts		pomocna struktura
 * @return			se stejne zahodi
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
 * Funkce na prijeti pripojeni.
 ****************************************************************************************************
 * @param sin			struktura
 * @param welcome_soc	welcome socket
 * @param in_soc		komunikacni soket
 * @return				zahodi se
 */
int acceptConnection(struct sockaddr_in* sin, int* welcome_soc, int* in_soc)
{
	int sin_len;
	string hostname;
	sin_len = sizeof(*sin);
	*in_soc = accept(*welcome_soc, (struct sockaddr *)sin, (socklen_t*)&sin_len);
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce slouzici pro komunikaci s klientem.
 ****************************************************************************************************
 * @param in_soc		komunkikacni socket
 * @param opts			pomocna struktura s prepinaci
 * @return				se stejne zahodi
 */
int communicate(int* in_soc, tOptions* opts)
{
	char recieved_message[BUFF_SIZE];
	string request, answer;
	bzero(recieved_message, BUFF_SIZE*sizeof(char) );
	
	int i = 1;
	while(i)
	{		
		/***** READ REQUEST *****/
		if ( read(*in_soc, recieved_message, BUFF_SIZE ) < 0 )
		{
			cerr << "Error on read." << endl;	// read error
			return EXIT_FAILURE;
		}
		/***** CREATE ANSWER *****/
		answer = createAnswer(recieved_message, opts, &i);	// vytvoreni odpovedi pro klienta
		if ( i == 0 )	// pokud bylo zmeneno i na 0, byla ze strany klienta ukoncena komunikace
			return EXIT_SUCCESS;
		/***** SEND ANSWER *****/
		if ( send(*in_soc, answer.c_str(), answer.length(), 0) < 0 )	// odeslani odpovedi klientovi
		{
			perror("Error on sending.\n");	// chyba pri odesilani zpravy
			exit(EXIT_FAILURE);
		}
		opts->search.clear();
		opts->parameters.clear();
	}
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce pro vytvoreni zpravy.
 ****************************************************************************************************
 * @param request	obdrzena zprava
 * @param opts		ukazatel na strukturu s prepinaci
 * @return			vytvorena zprava
 */
string createAnswer(char* request, tOptions* opts, int* i)
{
	string answer, s_request;
	s_request = request;
	processRequest(s_request, opts, i);	// zpracovani pozadavku klienta
	if ( *i == 0 )	// pokud bylo zmeneno i na 0, byla ze strany klienta ukoncena komunikace
		return "END\n";

	find(opts);	// hledani pozadovanych informaci
	if ( opts->goals.empty() )	// pokud nebyl nalezen vysledek
	{
		answer.append("N");	// zprava bude zacinat N jako "NOT FOUND"
		if ( opts->u_or_l == 1 )
			answer.append("L\n");
		else if ( opts->u_or_l == 2 )
			answer.append("U\n");
		answer.append(opts->search);
		answer.append("\n");	
	}
	else	// pokud odpovidajici informace byla nalezena
	{
		answer.append("F\n");	// zprava bude zacinat F jako "FOUND"
		while ( !opts->goals.empty() )	// zkopirovani vsech vysledku z vektoru
		{
			answer.append( opts->goals.front() );
			opts->goals.erase( opts->goals.begin() );
		}

	}
	return answer;
}

/****************************************************************************************************
 * Funkce pro hledani v souboru etc/passwd.
 ****************************************************************************************************
 * @param opts		...
 * @return			...
 */
int find(tOptions* opts)
{
	string line;
	string delimiter = ":";
	string new_line = "\n";
	ifstream etcPasswd("/etc/passwd");	// otevreni souboru /etc/passwd
	if(!etcPasswd)
	{
		cerr << "Error on open: /etc/passwd." << endl;
		exit(EXIT_FAILURE);
	}
	
	while( getline(etcPasswd, line) )	// nacitani soubotu po radcich
	{
		vector<string> items;	// vektor do ktereho se oddelene ulozi vsechny informace na radku
		size_t offset = 0;
		int match = 0;
		string item, goal;
		
		while ( ( offset = line.find(delimiter) ) != string::npos )	// prochazeni radku, nalezeni pozice dvojtecky
		{
			item = line.substr(0, offset);	// vytvoreni informace pomoci substringu
			items.push_back(item);
			line.erase( 0, offset+delimiter.length() );	// odmazani zpracovane informace z radku
		}
		items.push_back(line);	// ulozeni posledni iformace, tzn. default shellu
		if ( opts->u_or_l == 1 ) // case login
		{
			if ( opts->search.compare(items[0]) == 0 )	// pokud se ve vektoru na miste loginu nachazi hledany login, dochazi k matchi
				match = 1;
		}
		else if ( opts->u_or_l == 2 )	// case uid
		{
			if ( opts->search.compare(items[2]) == 0 )	// match uid
				match = 1;
		}		
		if ( match )
		{
			for (size_t s = 0; s < opts->parameters.length(); s++)	// prochazeni pozadovanych parametru v poradi v jakem prisli, pripadne vlozeni
			{
				switch(opts->parameters[s])
				{
					case 'L':
						goal.append(items[0]);
						goal.append(" ");
						break;
					case 'U':
						goal.append(items[2]);
						goal.append(" ");
						break;
					case 'G':
						goal.append(items[3]);
						goal.append(" ");
						break;
					case 'H':
						goal.append(items[5]);
						goal.append(" ");
						break;
					case 'S':
						goal.append(items[6]);
						goal.append(" ");
						break;
					case 'N':
						goal.append(items[4]);
						goal.append(" ");
						break;
				}
			}
			size_t dest;
			dest = goal.rfind(" ");
			goal.replace(dest, 1, new_line);	// zmena posledniho znaku z mezery na novy radek
			opts->goals.push_back(goal);
		}
	}
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce na zpracovani pozadavku.
 ****************************************************************************************************
 * @param request		zprava od klienta
 * @param opts			...
 * @return				...
 */
int processRequest(string request, tOptions* opts, int* i)
{
	string line;
	istringstream fStr(request);
	if ( request[0] == 'L' )	// pokud v prichozi zprave je prvni znak L, bude se hledat podle loginu
		opts->u_or_l = 1;
	else if ( request[0] == 'U' )	// pokud U, hledat se bude podle uid
		opts->u_or_l = 2;
	else if ( request[0] == 'E' )	// pokud na prvnim miste je E, klient konci komunikaci
	{
		*i = 0;
		return 0;
	}
	getline(fStr, line);
	opts->search.append(line, 2, line.length()-2 );	// ulozeni loginu/uid zacinajicich na 2 pozici
	getline(fStr, line);
	opts->parameters.append(line);	// ulozeni pozadovanych informaci
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce na inicializaci struktury s prepinaci.
 ****************************************************************************************************
 * @param opts - uk. na strukturu k inicializaci
 */
void initOptions(tOptions* opts)
{
	opts->port = -1;
	opts->u_or_l = 0;
	opts->parameters = "";
}