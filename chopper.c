#include "starLord.h"

int main(int argc, char *argv[]) {

	// Ensure that user ran with correct syntax and # of arguments. If not, exit
	if (argc < 8 || argc > 10) {
		printf("Syntax: ./chopper -s <server addr> -p <port> -h <hostHeader>\n");
		printf("[-a <message>] and/or [-v] should also be included, \n");
		printf("depending on which mode you want to use.\n");
		exit(1);
	}

	// Declare variables
	char *servIP;	 	   	// server IP address
	char *servPort;			// server port number
	char *hostHeader; 		// host header
	char *msgArg;				// message sent in on command line
	char *msgWithMode;		// message argument with mode prepended
	char *msgToSend;			// entire message that will be sent to server
	bool addMode = false;	// will be true if -a flag is sent	
	bool viewMode = false;	// will be true if -v flag is sent

	// Parse command line arguments with flags and initialize variables
	int c;
	opterr = 0;
	while ((c = getopt(argc, argv, "vs:p:h:a:")) != -1) {
		switch (c) {
			case 's':
				servIP = optarg;
				break;
			case 'p':
				servPort = optarg;
				break;
			case 'h':
				hostHeader = optarg;
				break;
			case 'a':
				if (strlen(optarg) > 80)
					exitWithMsg("message length", "max message length is 80 chars.");
				msgArg = optarg;
				addMode = true;
				break;
			case 'v':
				viewMode = true;
				break;
			case '?':
				exitWithMsg("flags", "Acceptable flags are -s, -p, -h, -a, -v.");
			default:
				exit(1);
		}
	}

	// Exit if neither view flag or add flag were sent in
	if (!viewMode && !addMode)
		exitWithMsg("flags", "Neither view or add mode were chosen.");

	// Tell the system what kind of address info we want
	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC;
	addrCriteria.ai_socktype = SOCK_STREAM;
	addrCriteria.ai_protocol = IPPROTO_TCP;

	// Get address
	struct addrinfo *servAddr;
	int rtnVal = getaddrinfo(servIP, servPort, &addrCriteria, &servAddr);
	if (rtnVal != 0)
		exitWithMsg("getaddrinfo()", gai_strerror(rtnVal));

	// Create a reliable stream socket using TCP
	int sock = socket(servAddr->ai_family,servAddr->ai_socktype,
							servAddr->ai_protocol);
	if (sock < 0) 
		exitWithMsg("socket()", "it failed!");

	// Establish the connection to the server
	if (connect(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
		exitWithMsg("connect()", "it failed!");

	// Declare new variables for sending/receiving and clock time
	bool send_success = false, recv_success = false;
	int attempts = 0;
	clock_t begin,end;
	begin = clock();

	// Build message to send to server
	if (viewMode) {
		msgWithMode = malloc(sizeof("/view?")); 
		sprintf(msgWithMode, "/view?");
	}
	else if (addMode) {
		msgWithMode = malloc(sizeof("/add?") + sizeof(msgArg));
		sprintf(msgWithMode, "/add?%s", msgArg);
	}
	msgToSend = malloc(21 + sizeof(msgWithMode) + sizeof(hostHeader));
	sprintf(msgToSend, "GET %s HTTP/1.1\nHost: %s", msgWithMode, hostHeader);

	// Send the message to the server
	size_t messageLen = strlen(msgToSend);
	ssize_t numBytes = send(sock, msgToSend, messageLen, 0);
	if (numBytes < 0)
		exitWithMsg("send()", "it failed!");
	else if (numBytes != messageLen)
		exitWithMsg("send()", "unexpected number of bytes!");

/*
	// Receive the same string back from the server
  	struct sockaddr_storage fromAddr;	
	socklen_t fromAddrLen = sizeof(fromAddr);
	char received[MAXSTRINGLENGTH + 1];
	while (!recv_success) {
		numBytes = recv(sock, received, BUFSIZE - 1, 0);
		attempts++;
		if (numBytes == messageLen) {
			end = clock();
			recv_success = true;
		}
		else if (numBytes != messageLen) {
			printf("recv() error: received unexpected number of bytes; \n");
			printf("attempting to receive again...\n");
		}
		else if (!SockAddrsEqual(servAddr->ai_addr, (struct sockaddr*)&fromAddr))
			printf("recv() error: received a packet from unknown source\n");
		else
			printf("recv() failed; attempting to receive again...\n");
	}
  
	// Null-terminate received message, end clock, and print communication stats
	received[messageLen] = '\0';
	end = clock();
	double time_spent = ((double)(end-begin)) / CLOCKS_PER_SEC;
	printf(" %d %.6f %s %s\n", attempts, time_spent, msgArg, received);

*/

	// Close socket and free addrinfo allocated in getaddrinfo()
	close(sock);
	freeaddrinfo(servAddr);
	return 0;
}
