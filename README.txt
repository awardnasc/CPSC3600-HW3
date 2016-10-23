Kevin Corcoran, Nick Barrs, Al Ward, David McElveen
CPSC 3600
Homework #3
24 October 2016


All 3 programs were based on the TCP Server/Client files from Project 2.

A makefile was also created to help with compiling/executing, and instructions on that are provided below.

Chopper:

 - The first client has 2 different modes of operation. 
 - The first mode sends a string, provided as a parameter at the command line, to the server. This string is encoded in the payload of a HTTP/1.1 GET request. Once it receives a response from the server, it displays the HTTP payload of the message.
 - The second mode allows it to make another (different) HTTP GET request and then, again, displays the HTTP payload of the message received.

Ahsoka:

 - This client verified whether the server performed the task that was asked of it.
 - This program sends sequences of add requests and view requests, which then receives the responses in the same order.
 - It will then compare the end message with the inital, and if the same will display a "verified" message.

starLord:

 - The server, when receiving a message, parses it to determine if it has received a valid request.
 - Once parsed, it determines whether is was requested to add or view.
 - It then prepends the contents of the Host Header to the received (buffer) data.
 - The content of the received data is added to the server's local buffer with a newline added.
 - The data is also sent as payload in the HTTP response.
 - Finally, it returns the contents of the entire local buffer as payload in the HTTP response.

	starLord Notes:

		- Implements a subset of the HTTP/1.1 spec
		- Supports the GET method.
		- Will provide one of the following responses:
			a. "HTTP OK 200" - for successful requests.
			b. "HTTP 400 ERROR" - for malformed HTTP requests.
			c. "HTTP 405 ERROR" - for any requests not using the GET method.


Instructions to compile and run the executables:

1. Type "make" and press "Enter" to run the makefile and compile the 6 files and create the executables.


2. Secondly, you need to choose which executable to run. There will be 3 options. The executables for the two clients: chopper and ahsoka.
	As well as the server executable: starLord. The first step in running any of these programs is ./exe.
	
	So, for example: ./chopper, ./ahsoka, or ./starLord. However, there is more that needs to be added before the program can actually be run.


3. a. For chopper the requirements to properly run the client are: ./chopper -s <serverIP> -p <serverPort> -h <hostHeader> [-a <message>] [-v]
 
  	Note: The "-v" is optional. If the "-v" is added to the end, the program will create a view request, and the contents of the message will not be sent
		as an add request.

   b. For ahsoka, the second client program, the requirements to properly run the client are: ./ahsoka -s <serverIP> -p <serverPort> -h <hostHeader> -a1 <message>... -aN <messageN>
	
	Note: Instead of just one message, this client has the ability to send multiple messages. It will send them one at a time, and in the order they are entered in.

   Important: If either of these programs is run first, without the server program already running - will receive an error. 
		Again, the server must be running for these programs to function properly.

   c. For starLord, the server program, the requirements to properly run the server are: ./starLord -p <serverPort>


4. Executing this line will run the program and display the results.

Notes:

For the client and server, it is easiest run on one computer, 
    with the <serverIP> entry being localhost, and the server being run in a separate terminal with an advised port from 5k-10k range.


