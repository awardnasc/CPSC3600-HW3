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
	time_t last_mod_t, date_t;
	struct tm lmt, dt;

	// Allocate memory space for strings
	buffer = malloc(10000);
	httpNumResponse = malloc(strlen("HTTP 40X ERROR "));
	connection = malloc(sizeof("Connection: close\n"));
	date = malloc(strlen("Date: --- ::\n") + sizeof(struct tm));
	last_mod = malloc(strlen("Last Modified: --- ::\n") + sizeof(struct tm));
	content_len = malloc(strlen("Content-Length: \n") + sizeof(int));
	content_type = malloc(strlen("Content-Type: text/plain\n"));
	server = malloc(strlen("Server: Group8/1.0\n"));

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
		// Declare and initialize variable used for receiving/inverting/sending
		int x, unique; // Unique IP checker and variable for loop
		struct sockaddr_in clntAddr; // Client address structure
		socklen_t clntAddrLen = sizeof(clntAddr); // Length of client address
	
		// Clear strings used for receiving and storing inverted message
		bzero(received, strlen(received));
    
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
				msgData = malloc(strlen(token) - strlen("/add?"));
				char *temp = strstr(token, "/add?");
				strcpy(msgData, temp+5);
			}
			else if (strstr(token, "/view?") != NULL)	{
				viewMode = true;
				msgData = malloc(strlen("\0"));
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
					hostName = malloc(strlen(token));
					sprintf(hostName, "%s", token);
					dataToAdd=malloc(strlen(hostName)+strlen(msgData)+2);
					sprintf(dataToAdd, "%s", hostName);
					strcat(dataToAdd, " ");
					strcat(dataToAdd, msgData);
					strcat(dataToAdd, "\n");
					if (msgs_recvd == 1) sprintf(buffer, "%s", dataToAdd);
					else strcat(buffer, dataToAdd);
					last_mod_t = time(NULL);	
					lmt = *localtime(&last_mod_t);
					sprintf(last_mod, "Last Modified: %d-%d-%d %d:%d:%d\n",
												lmt.tm_mon+1, lmt.tm_mday, lmt.tm_year-100,
												lmt.tm_hour, lmt.tm_min, lmt.tm_sec);
				}
			}

			// if message was okay, http response is OK
			if (strcmp(httpNumResponse, "null") == 0) 
				sprintf(httpNumResponse, "HTTP OK 200");

			// fill header strings with appropraite content
			sprintf(connection, "Connection: close\n");	
			date_t = time(NULL);
			dt = *localtime(&date_t);
			sprintf(date, "Date: %d-%d-%d %d:%d:%d\n", 
										dt.tm_mon+1, dt.tm_mday, dt.tm_year-100,
										dt.tm_hour, dt.tm_min, dt.tm_sec);

			// fill body of message to be sent with appropriate content
			if (viewMode) {
				msgToSend_body = malloc(strlen("Local Buffer:\n\n")+strlen(buffer));
				memset(msgToSend_body, 0, strlen(msgToSend_body));
				sprintf(msgToSend_body, "Local Buffer:\n%s\n", buffer);
			}
			else {
				msgToSend_body = malloc(strlen("Msg Added:\n\nLocal Buffer:\n\n") 
												+ strlen(dataToAdd) + strlen(buffer));
				sprintf(msgToSend_body, "Msg Added:\n%s\n", dataToAdd);
				strcat(msgToSend_body, "Local Buffer:\n");
				strcat(msgToSend_body, buffer);
				strcat(msgToSend_body, "\n");
			}

			// fill remaining header strings with appropriate content
			sprintf(content_len,"Content-Length:%li\n", strlen(msgToSend_body)-15);
			sprintf(content_type, "Content-Type: text/plain\n");
			sprintf(server, "Server: Group8/1.0\n");
		}

		// Create message to send back
		msgToSend_entire = malloc(strlen("Allow: GET") + strlen(connection) +
											strlen(date) + strlen(last_mod) +
											strlen(content_len) + strlen(content_type) +
											strlen(server) + strlen(msgToSend_body));
		
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
