
	if ( (s = socket(PF_INET, SOCK_STREAM, 0 ) ) < 0)		// create socket
	{
		cerr << "Error on socket" << endl;  							// socket error
		return EXIT_FAILURE;
	}
	
	
	sin.sin_family = PF_INET;              								// set protocol family to Internet
	sin.sin_port = url.port;
	// sin.sin_port = htons(url.port);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	host = url.hostname.c_str();
	
	if ( ( hptr = gethostbyname(host) ) == NULL)
	{
		cerr << "Gethostname error: " << endl;
		return EXIT_FAILURE;
	}
	memcpy( &sin.sin_addr, hptr->h_addr, hptr->h_length);
	
// ********************************************************************************************************


	if ( recv(sock, char_message, BUFF_SIZE, 0) < 0 )	// odeslani zpravy na server
	{
		perror("Error on recieving answer.\n");	// chyba pri odesilani zpravy
		exit(EXIT_FAILURE);
	}
	
	if ( send(soc, message.c_str(), message.length(), 0) < 0 )	// odeslani zpravy na server
	{
		perror("Error on sending.\n");	// chyba pri odesilani zpravy
		exit(EXIT_FAILURE);
	}
	
// ********************************************************************************************************

	char char_message[BUFF_SIZE] = message.c_str();	// ulozeni zpravy do potrebneho datoveho typu
	int n;
	
	/***** PRIPOJENI, ODESLANI A OBDRZENI SOCKETU *****/
	if ( connect (soc, (struct sockaddr *)&sin, sizeof(sin) ) < 0 )
	{
		perror("Error on connect.\n");	// chyba pripojeni
		exit(EXIT_FAILURE);
	}
	if(DEBUG){cout << "Client: Sending message: " << message << endl;}
	
	
	
	if ( write(soc, char_message, strlen(char_message) +1) < 0 )	// odeslani zpravy na server
	{
		perror("Error on write.\n");	// chyba pri odesilani zpravy
		exit(EXIT_FAILURE);
	}
	if(DEBUG){cout << "Client: Waiting for response."<< endl;}
	
	// uspat?
	//sleep(1);
	
	if ( ( n = read(soc, char_message, sizeof(char_message)) ) < 0 )	// obdrzeni zpravy
	{
		perror("Error on read.\n");	// read error
		exit(EXIT_FAILURE);
	}
	if(DEBUG){cout << "Client: Received " << n << "bytes: " << char_message << endl;}
	
	/***** TISK OBDRZENE ZPRAVY *****/
	cout << char_message;
	
// ********************************************************************************************************
	
for( vector<string>::const_iterator i = options.login_vect.begin(); i != options.login_vect.end(); i++)
	{
		cout << *i << endl;
	}
	
// ********************************************************************************************************

	sin_len = sizeof(*sin);
	if ( (*in_soc = accept(*welcome_soc, (struct sockaddr *)sin, (socklen_t*)&sin_len)) < 0 )
	{
		cerr << "Error on accept." << endl;					// Accept error
		exit(EXIT_FAILURE);
	}

// ********************************************************************************************************

	if ( write(*in_soc, recived_message, strlen(recived_message) ) < 0 )
	{
		cerr << "Error on write." << endl;
		exit(EXIT_SUCCESS);	// write error
	}

// ********************************************************************************************************

		if( ( options.u_or_l == LOGIN && !(options.login_vect.empty()) ) || ( options.u_or_l == UID && !(options.uid_vect.empty()) ) )
			i = 1;
		else
			i = 0;
			
// ********************************************************************************************************

if ( find(opts) )
	{
		answer.append("F\n");
		/*
		for(int i = 0; i < length; i++)
		{
			switch([i])
			{
				case 'L':
					answer.append(opts->LOGIN);
					break;
				case 'U':
					answer.append(opts->UID);
					break;
				case 'G':
					answer.append(opts->GID);
					break;
				case 'H':
					answer.append(opts->HOME);
					break;
				case 'S':
					answer.append(opts->SHELL);
					break;
				case 'N':
					answer.append(opts->NAME);
					break;
			}
		}
		*/
	}
	else
	{
		/*
		if ( request[0] == 'L' )
		{
			answer.append("NL\n");
			answer.append(opts->search_login);
			answer.append("\n");
		}
		else if ( request[0] == 'U' )
		{
			answer.append("NU\n");
			answer.append(opts->search_uid);
			answer.append("\n");
		}
		*/
		answer.append("NL\n");
	}

// ********************************************************************************************************

if(DEBUG)
	{
		hostname = inet_ntoa(sin->sin_addr);
		port = htons(sin->sin_port);
		cout << "From " << port << ": " << hostname << "." << endl;
	}
	
