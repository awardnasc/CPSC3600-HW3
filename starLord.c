#include "starLord.h"

int main(int argc, char *argv[]) {
	
	// Ensure that user ran with correct syntax and # of arguments. If not, exit
	if (argc != 3) {
		printf("Syntax: ./starLord -p <port>\n");
		exit(1);
	}

	// declare and initialize variables
	char *ipAddrs[MAXSTRINGLENGTH];	// array to hold unique client IPs
	int ipCounter = 0;					// number of unique IPs encountered	
	bool addMode = false;				// true if client sends with add mode
	bool viewMode = false;				// true if client sends with view mode
	char received[BUFSIZE];
	char *buffer;
	char *hostName;
	char *msgData;
	char *dataToAdd;
	char *msgToSend_header;
	char *msgToSend_body;
	char *msgToSend_entire;
	bool allowHeaderNeeded = false;
	char *httpNumResponse;
	char *connection, *date, *last_mod, *content_len, *content_type, *server;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	// Allocate memory space for strings
	buffer = malloc(BUFSIZE * 1000);
	httpNumResponse = malloc(sizeof("HTTP 40X ERROR "));
	connection = malloc(sizeof("Connection: close\n"));
	date = malloc(sizeof("Date: --- ::\n") + sizeof(tm));
	last_mod = malloc(sizeof("Last Modified: --- ::\n") + sizeof(tm));
	content_len = malloc(sizeof("Content-Length: \n") + sizeof(int));
	content_type = malloc(sizeof("Content-Type: text/plain\n"));
	server = malloc(sizeof("Server: Group8/1.0\n"));

	// Parse command line and initialize variables
	in_port_t servPort;
	int c;
	while ((c = getopt(argc, argv, "p:")) != -1) {
		switch (c) {
			case 'p':
				servPort = atoi(optarg);
				break;
			case '?':
				exitWithMsg("flags", "Only acceptable flag is -p.");	
			default:
				exit(1);
		}
	}

	// Create socket for incoming connections
	int servSock;
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		exitWithMsg("socket()", "it failed!");
  
	// Construct local address structure
	struct sockaddr_in servAddr;                  // Local address
	memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
	servAddr.sin_family = AF_INET;                // IPv4 address family
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
	servAddr.sin_port = htons(servPort);          // Local port

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
		exitWithMsg("bind()", "it failed!");

	// Mark the socket so it will listen for incoming connections
	if (listen(servSock, MAXPENDING) < 0)
		exitWithMsg("listen()", "it failed!");	
  
	// Print number of messages and the unique client ips after CTRL+C
	int msgs_recvd = 0;
	void ctrlHandle(int sig) {
		int i;
		signal(sig, SIG_IGN);
		printf("\n%d	", msgs_recvd-1);
		for(i = 0; i < ipCounter; i++)
			printf("%s, ",ipAddrs[i]);
		printf("\n");
		exit(1);
	}
	signal(SIGINT, ctrlHandle);

	// Actual receiving/sending portion of program. Run until CTRL+C
	while(true) { 
		msgs_recvd++;
		memset(buffer, 0, sizeof(buffer));
		// Declare and initialize variable used for receiving/inverting/sending
		int x, unique; // Unique IP checker and variable for loop
		struct sockaddr_in clntAddr; // Client address structure
		socklen_t clntAddrLen = sizeof(clntAddr); // Length of client address
	
		// Clear strings used for receiving and storing inverted message
		bzero(received, sizeof(received));
    
		// Wait for a client to connect
		int clntSock = accept(servSock,(struct sockaddr*)&clntAddr, &clntAddrLen);
		if (clntSock < 0)
			exitWithMsg("accept()", "it failed!");	

		// clntSock is connected to a client
		char clntName[INET_ADDRSTRLEN]; // String to contain client address
		if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
							sizeof(clntName)) != NULL)
			printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
		else
			printf("Unable to get client address\n");
    
		// Save unique client IPs in ipAddrs
		if (ipCounter == 0)
			ipAddrs[ipCounter++] = clntName;
		
		else {
			for(x=0; x<ipCounter; x++) {
				if (strcmp(ipAddrs[x], clntName) == 0)
					unique = 0;
				else
					unique = 1;
	   	}

			if(unique)
				ipAddrs[ipCounter++] = clntName;
		}
	
		// Receive message from client
		ssize_t numBytesRcvd = recv(clntSock, received, BUFSIZE, 0);
		if (numBytesRcvd < 0)
			exitWithMsg("recv()", "it failed!");
	
		// Parse received message
		sprintf(httpNumResponse, "null");
		char *token = strtok(received, " ");
		if (strcmp(token, "GET") != 0) {
			allowHeaderNeeded = true;
			sprintf(httpNumResponse, "HTTP 405 ERROR; ONLY 'GET' METHOD ALLOWED");
		}
		else {
			addMode = false;
			viewMode = false;
			
			token = strtok(NULL, " ");
			if (strstr(token, "/add?") != NULL) {
				addMode = true;
				msgData = malloc(sizeof(token) - sizeof("/add?"));
				char *temp = strstr(token, "/add?");
				strcpy(msgData, temp+5);
			}
			else if (strstr(token, "/view?") != NULL)	{
				viewMode = true;
				msgData = malloc(sizeof("\0"));
				sprintf(msgData, " ");
			}	
			else  {
				sprintf(httpNumResponse, "HTTP 404 ERROR; ACTION NOT FOUND");
			}

			token = strtok(NULL, "\n");
			if (strcmp(token, "HTTP/1.1") != 0)
				sprintf(httpNumResponse, "HTTP 400 ERROR; BAD REQUEST");

			token = strtok(NULL, " ");
			if (strcmp(token, "Host:") != 0)
				sprintf(httpNumResponse, "HTTP 400 ERROR; BAD REQUEST");
			else {
				if (addMode) {
					token = strtok(NULL, " ");
					hostName = malloc(sizeof(token));
					sprintf(hostName, "%s", token);
					dataToAdd=malloc(sizeof(hostName)+sizeof(msgData)+2);
					sprintf(dataToAdd, "%s", hostName);
					strcat(dataToAdd, " ");
					strcat(dataToAdd, msgData);
					strcat(dataToAdd, "\n");
					printf("buffer is %s\n", buffer);
					char *oldbuffer = malloc(sizeof(buffer));
					memset(oldbuffer, 0, sizeof(oldbuffer));
					sprintf(oldbuffer, "%s", buffer);
					printf("old buffer is %s\n", oldbuffer);
					buffer = malloc (sizeof(buffer) + sizeof(dataToAdd));
					printf("buffer after malloc is %s\n", buffer);
					sprintf(buffer, "%s", oldbuffer);
					printf("buffer after strcopy oldbuffer into buffer is %s\n", buffer);
					strcat(buffer, dataToAdd);
					printf("buffer after strcat dataToAdd into buffer is %s\n", buffer);
					sprintf(last_mod, " ");
					time_t last_mod_t = time(NULL);	
					struct tm lmt = *localtime(&t);
					memset(last_mod, 0, sizeof(last_mod));
					sprintf(last_mod, "Last Modified: %d-%d-%d %d:%d:%d\n",
												lmt.tm_mon+1, lmt.tm_mday, lmt.tm_year,
												lmt.tm_hour, lmt.tm_min, lmt.tm_sec);
				}
			}

			// if message was okay, http response is OK
			if (strcmp(httpNumResponse, "null") == 0) {
				sprintf(httpNumResponse, "HTTP OK 200");
			}

			// fill header strings with appropraite content
			sprintf(connection, "Connection: close\n");	
			time_t date_t = time(NULL);
			struct tm dt = *localtime(&t);
			sprintf(date, "Date: %d-%d-%d %d:%d:%d\n", 
										dt.tm_mon+1, dt.tm_mday, dt.tm_year,
										dt.tm_hour, dt.tm_min, dt.tm_sec);

			// fill body of message to be sent with appropriate content
			if (viewMode) {
				msgToSend_body = malloc(sizeof("Local Buffer:\n\n")+sizeof(buffer));
				memset(msgToSend_body, 0, sizeof(msgToSend_body));
				sprintf(msgToSend_body, "Local Buffer:\n%s\n", buffer);
			}
			else {
				msgToSend_body = malloc(sizeof("Msg Added:\n\nLocal Buffer:\n\n") +
												sizeof(dataToAdd) + sizeof(buffer));
				memset(msgToSend_body, 0, sizeof(msgToSend_body));
				sprintf(msgToSend_body, "Msg Added:\n%s\n", dataToAdd);
				strcat(msgToSend_body, "Local Buffer:\n");
				strcat(msgToSend_body, buffer);
				strcat(msgToSend_body, "\n");
			}

			// fill remaining header strings with appropriate content
			int content_length = strlen(msgToSend_body);
			sprintf(content_len, "Content-Length: %i\n", content_length);
			sprintf(content_type, "Content-Type: text/plain\n");
			sprintf(server, "Server: Group8/1.0\n");
		}

		// Create message to send back
		msgToSend_entire = malloc(sizeof("Allow: GET") + sizeof(connection) +
											sizeof(date) + sizeof(last_mod) +
											sizeof(content_len) + sizeof(content_type) +
											sizeof(server) + sizeof(msgToSend_body));
		
		sprintf(msgToSend_entire,"%s", connection);
		if (allowHeaderNeeded) strcat(msgToSend_entire, "Allow: GET");
		strcat(msgToSend_entire, date);
		strcat(msgToSend_entire, last_mod);
		strcat(msgToSend_entire, content_len);
		strcat(msgToSend_entire, content_type);
		strcat(msgToSend_entire, server);
		strcat(msgToSend_entire, "\n");
		strcat(msgToSend_entire, msgToSend_body);
		strcat(msgToSend_entire, "\n");
		printf("%s\n", msgToSend_entire);

/*
		// Send inverted message back to client
		ssize_t numBytesSent = send(clntSock, msgToSend, numBytesRcvd, 0);
		if (numBytesSent < 0)
			exitWithMsg("send()", "it failed!");
		else if (numBytesSent != numBytesRcvd)
			exitWithMsg("send()", "unexpected number of bytes!");
*/
	}	
}
