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
#define LOGIN	1
#define UID		2
#define BUFF_SIZE 100
// pomocny ladici define
#define DEBUG	1

/*
 * TO DO LIST:
 * - dokumentace
 * - SLEEP?
 */

using namespace std;
 
/****************************************************************************************************
 * Deklarace pomocnych funkci a promennych.
 ****************************************************************************************************
 *	- 
 */

typedef struct
{
	int u_or_l;	// prepinac - slouzi pro rozliseni, zda se hleda podle loginu nebo uid
	// pomocne vektory
	vector<string> login_vect;
	vector<string> uid_vect;
	string prepinace;
	// pomocne flagy
	int L_flag;
	int U_flag;
	int G_flag;
	int N_flag;
	int H_flag;
	int S_flag;
	int hostname_flag;
	int port_flag;
	int login_flag;
	int uid_flag;
	string hostname;
	int port;
} tOptions;

/***** DEKLARACE FUNKCI *****/	
void initOptions(tOptions*);
int processArg(int, char* [], tOptions*);
int createSocket();
int setNetwork(struct sockaddr_in*, tOptions*, int*);
string createMessage(tOptions* opts);
int communication(string, int*);
int connectToServer(int* soc, struct sockaddr_in* sin);

/****************************************************************************************************
 * Main.
 ****************************************************************************************************
 */
int main (int argc, char* argv[])
{
	int soc;
	struct sockaddr_in sin;
	string message;
	tOptions options;
	memset(&sin, 0, sizeof(sin));
	
	initOptions(&options);	// incializace hodnot ve pomocne strukture
	processArg(argc, argv, &options);	// zpracovani argumentu - ulozeni prepinacu
	setNetwork(&sin, &options, &soc);	// vytvoreni socketu, ziskani adresy
	connectToServer(&soc, &sin);	// pripojeni k serveru
	
	int i = 0;
	if( ( options.u_or_l == LOGIN && !(options.login_vect.empty()) ) || ( options.u_or_l == UID && !(options.uid_vect.empty()) ) )
		i = 1;
	while( i )
	{
		message = createMessage(&options);
		communication(message, &soc);		
		message.clear();
		
		if( ( options.u_or_l == LOGIN && !(options.login_vect.empty()) ) || ( options.u_or_l == UID && !(options.uid_vect.empty()) ) )
			i = 1;
		else
			i = 0;
		/*
		if( ( options.u_or_l == LOGIN && options.login_vect.empty() ) || ( options.u_or_l == UID && options.uid_vect.empty() ) )
			i = 0;
		*/
	}
	
	/***** UZAVRENI SOCKETU *****/
	if ( close(soc) < 0 )
	{
		cerr << "Error closing socket." << endl;
		return(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce zpracovavajici argumenty.
 ****************************************************************************************************
 * @param argc, argv - ...
 * @param tOptions* - pointer na moji pomocnou strukturu
 * @return - se stejne zahodi
 */
int processArg(int argc, char* argv[], tOptions* opts)
{
	int option;
	while ((option = getopt (argc, argv, "h:p:l:u:LUGNHS")) != -1)	// pouziti getoptu
	{
		switch(option)
		{
			case 'h':
				opts->hostname_flag = 1;
				// pridat nejaky podminky, co nesmi obsahovat?
				opts->hostname.append(optarg);
				break;
				
			case 'p':
				opts->port_flag = 1;
				opts->port = atoi(optarg);
				break;
				
			case 'l':
				opts->login_flag = 1;
				opts->u_or_l = 1;
				optind--;
				while( optind < argc && argv[optind][0] != '-' )
					opts->login_vect.push_back(argv[optind++]);	// pushnuti loginu do vektoru
				break;
			
			case 'u':
				opts->u_or_l = 2;
				opts->uid_flag = 1;
				optind--;
				while( optind < argc && argv[optind][0] != '-' )
					opts->uid_vect.push_back(argv[optind++]);	// pushnuti uid do vektoru
				break;
				
			case 'L':
				if ( opts->L_flag == 0 )
					opts->prepinace.append("L");
				opts->L_flag = 1;
				break;
				
			case 'U':
				if ( opts->U_flag == 0 )
					opts->prepinace.append("U");
				opts->U_flag = 1;
				break;
				
			case 'G':
				if ( opts->G_flag == 0 )
					opts->prepinace.append("G");
				opts->G_flag = 1;
				break;

			case 'N':
				if ( opts->N_flag == 0 )
					opts->prepinace.append("N");
				opts->N_flag = 1;
				break;
			
			case 'S':
				if ( opts->S_flag == 0 )
					opts->prepinace.append("S");
				opts->S_flag = 1;
				break;
				
			default:
				cerr << "Wrong argument." << endl;
				exit(EXIT_FAILURE);
				break;
		}
	}
	if ( argc == 1 || opts->hostname_flag == 0 || opts->port_flag == 0) // ( login_f == 0 && uid_f == 0 )?
	{
		cerr << "Client needs hostname and port." << endl;
		exit(EXIT_FAILURE);
	}
	
	return 0;
}

/****************************************************************************************************
 * Funkce slouzici k samotne komunikaci.
 ****************************************************************************************************
 * @param message - string se zpravou sestavenou podle protokolu
 * @param sock - socket
 * @return - 
 */
int communication(string message, int* soc)
{
	char recieved_message[BUFF_SIZE];	// ulozeni zpravy do potrebneho datoveho typu
	int n;

	/***** ODESLANI A OBDRZENI SOCKETU *****/
	if ( send(*soc, message.c_str(), message.length(), 0) < 0 )	// odeslani zpravy na server
	{
		perror("Error on sending.\n");	// chyba pri odesilani zpravy
		exit(EXIT_FAILURE);
	}
	if(DEBUG){cout << "Message for server:" << endl << message << endl;}
		
	// uspat?
	//sleep(1);
	if ( ( n = read(*soc, recieved_message, sizeof(recieved_message)) ) < 0 )	// obdrzeni zpravy
	{
		perror("Error on read.\n");	// read error
		exit(EXIT_FAILURE);
	}
	
	// projit recieved message, jestli bude na zacatku F cout, jestli N cerr;
	
	/***** TISK OBDRZENE ZPRAVY *****/
	if(DEBUG){cout << "Answer from server: " << endl;}
	cout << recieved_message;
	
	return 0;
}
 
/****************************************************************************************************
 * Funkce.
 ****************************************************************************************************
 * @param sin - pointer na strukturu
 * @param soc_p - socket
 * @return - se stejne zahodi
 * - z prikladu k predmetu, jen vlozeno do funkce
 */
int setNetwork(struct sockaddr_in* sin, tOptions* opts, int *soc_p)
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

/****************************************************************************************************
 * Funkce pro vytvoreni socketu.
 ****************************************************************************************************
 * @return - vytvoreny socket
 */
int connectToServer(int* soc, struct sockaddr_in* sin)
{
	if ( connect (*soc, (struct sockaddr *)sin, sizeof(*sin) ) < 0 )
	{
		perror("Error on connect.\n");	// chyba pripojeni
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

/****************************************************************************************************
 * Funkce pro sestaveni zpravy.
 ****************************************************************************************************
 * @param opts_p - ukazatel na moji pomocnou strukturu
 * @return - vysledna zprava
 */
string createMessage(tOptions* opts)
{
	string message;
	if ( opts->u_or_l == LOGIN )	// case: login
	{
		message.append("L ");
		message.append(opts->login_vect.front());
		opts->login_vect.erase (opts->login_vect.begin());
	}
	else if ( opts->u_or_l == UID )	// case: uid
	{
		message.append("U ");
		message.append(opts->uid_vect.front());
		opts->uid_vect.erase (opts->uid_vect.begin());
	}
	message.append("\n");
	message.append(opts->prepinace);
	message.append("\n");
	return message;
}

/****************************************************************************************************
 * Funkce na inicializaci struktury s prepinaci.
 ****************************************************************************************************
 * @param tOptions* - uk. na strukturu k inicializaci
 */
void initOptions(tOptions* opts)
{
	opts->u_or_l = 0;
	opts->L_flag = 0;
	opts->U_flag = 0;
	opts->G_flag = 0;
	opts->N_flag = 0;
	opts->H_flag = 0;
	opts->S_flag = 0;
	opts->hostname_flag 	= 0;
	opts->port_flag 		= 0;
	opts->login_flag 		= 0;
	opts->uid_flag 			= 0;
	opts->port				= 0;
	opts->hostname			= "";
	opts->prepinace 		= "";
	return;
}