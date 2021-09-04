#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues
// function will return true if there is the target in array
int isThere(char arry[], char target, int length){

	int i;
	for(i=0; i < length; i++){
		if(arry[i] == target){
			return 1;
		}
	}
	return 0;
}

// function will return target match at index - use for encrypt and decrypt
// function will return target match at index
int isWhere(char arry[], char target, int length){

	int i;
	for(i=1; i < length; i++){
		if(arry[i] == target){
			return i;
		}
	}
	return 0;
}

// This function will encrypt passed message.
// not the reason we start at 1 is because of issues from reseting and encrypting the right values
// space and A gets the same value somehow. 
// A AND SPACE ADDS THE SAME VALUE;
char *encryptMsg(char msg[], int msgSize, char keyCode[]){
	//printf("SERVER: HI I AM GOING TO RETURN A ENCRYPT MSG\n");
	//printf("SERVER: MSG IS %s, with size of %d\n", msg, msgSize);
	//printf("SERVER: key IS %s, with size of %d\n", keyCode);
	int i;
    char dict[27];
    //load up our dictionary
    int k;
    k = 65;
    dict[0] = '\0';
    for(i=1; i < 27; i++){
        dict[i] = k;
        k++;
    }
    dict[27] = 32;

    // basically have to malloc() to return variable
    char *buffer = malloc(msgSize);
    memset(buffer, '\0', sizeof(buffer));
	// note we will only go up to our msgSize

    int holder, m, y;
    for(i=0; i < msgSize; i++){
        m = isWhere(dict, msg[i], 28);
        y = isWhere(dict, keyCode[i], 28);
        //printf("m is at %d, and y is at %d\n", m, y);
        holder = m + y;
        //printf("1. HOLDER at I = %d, with %d\n", i, holder);
        while(holder > 27){
            holder -= 27;
        }
        //printf("2. HOLDER at I = %d, with %d\n", i, holder);
        //holder += 65;
		buffer[i] = dict[holder];
        //buffer[i] += 64;
        /*
        printf("3. HOLDER at I = %d, with %d\n", i, holder);
        printf("MSG at i=%d, == %d, with %c\n", i, msg[i], msg[i]);
        printf("KEYCODE at i=%d, == %d, with %c\n", i, keyCode[i], keyCode[i]);
        printf("buffer at i=%d, == %d, with %c\n", i, buffer[i], buffer[i]);
        */
	}
	//printf("THE FINAL ENCRYPT BUFFER STRING IS.. \"%s\"\n", buffer);
    return buffer;
}

// this function wil be used to automatically read and return a string from a connectionFD.
char *recMsg(int establishedConnectionFD, int msgSize){

    // set up inital variables
    char *msgBuffer = malloc(msgSize);
    memset(msgBuffer, '\0', sizeof(msgBuffer));
    char buffer[256];
    int charsRead;
    int curMsgSize = msgSize;
	memset(buffer, '\0', 256);	

	// 257 is the perfect number to read....
	while(1){
		charsRead = recv(establishedConnectionFD, buffer, 257, 0); // Read the client's message from the socket
		if (charsRead < 0) error("ERROR reading from socket");
		//printf("SERVER: I received this from the client: \"%s\"\n", buffer);
		strcat(msgBuffer, buffer);
		// logic, each each message we will minus the buffer size, then... check if we need to break.	
		//printf("%s\n", buffer);
		curMsgSize = curMsgSize - strlen(buffer);
		//printf("the current message size is %d\n", curMsgSize);
		if(curMsgSize <= 0 || isThere(buffer, '\0', 256)){
			break;
		}
		memset(buffer, '\0', 256);	
	}

    return msgBuffer;
}

// THIS IS A SPAWN OFF FUNCTION THAT WILL HANDLE THE EXCHANGE OF CHILD PROCESSES.
void forkOffMsg(int establishedConnectionFD){
        int charsRead;
	    char buffer[256];
	// check if the right client is trying to connect to the correct server.
	// server will send its name, we will return with either success or no.
		// Get return message from server
		// check to make sure the right function is connecting. 
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		charsRead = recv(establishedConnectionFD, buffer, 256, 0); // Read the client's message from the socket
		if (charsRead < 0) error("ERROR reading from socket");

	// VERY IMPORTANT, NEED TO RUN WITH RIGHT COMMAND OR ELSE it wouldn't work
		if(strcmp(buffer, "otp_enc") == 0){
			memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
			strcpy(buffer, "Success");
		}

		// if statement here to check the correctness.
		if(strcmp(buffer, "Success") == 0){
			charsRead = send(establishedConnectionFD, "Success", 7, 0); // Send success back
			if (charsRead < 0) error("ERROR writing to socket");
		}
		else{
			charsRead = send(establishedConnectionFD, "fail", 4, 0); // Send success back
			if (charsRead < 0) error("ERROR writing to socket");
			//printf("FAILED CONNECTION TO BE MADE.\n");
			// skip everything and stop.
			return;
		}
		/* LOGIC,
		1. get the size of message first, then send the confirmation internally
		2. loop till we get all the messages and strcpy into master buffer list.
		3. done.
		// failed trying to break from charRead, and buffer. note to self.
		*/
		// Get the message from the client and display it
		memset(buffer, '\0', 256);
		int message_size = 0;
		int curMsgSize = 0;
		int returnMessageSize;
		returnMessageSize = read(establishedConnectionFD, &message_size, sizeof(message_size));
		if(returnMessageSize > 0){
			//printf("SERVER1: the message size is %d\n", ntohl(message_size));
			curMsgSize = ntohl(message_size);
			// temp - 1
			//curMsgSize--;
			//printf("Server1: the current message size is %d\n", curMsgSize);
		}
		else{
			error("ERROR reading from socket");
		}

		int f1msgsize = curMsgSize;
		char *msgBuffer = recMsg(establishedConnectionFD, curMsgSize);
		//printf("the msg buffer is \"%s\"\n", msgBuffer);
		// POTENTIALLY REMVOE THE BOTTOM BECAUSE UNNESSARY
		charsRead = send(establishedConnectionFD, "Success", 7, 0); // Send success back
		if (charsRead < 0) error("ERROR writing to socket");

		//printf("SERVER: first argument I received this from the client: \"%s\"\n", msgBuffer);
	// ***********************
		//printf("MSG RECV was %s\n", msgBuffer);
		//printf("SERVER ABOUT TO DO STUFF second time\n");
		//int f2msgsize = curMsgSize;
		//char keyBuffer[curMsgSize];
		//memset(keyBuffer, '\0', curMsgSize);
		//memset(buffer, '\0', 256);

		//printf("MADE IT HERE - msg size is %d - with file size %d\n", curMsgSize, f1msgsize);
		char *keyBuffer = recMsg(establishedConnectionFD, f1msgsize);
		//printf("MADE IT HERE\n");
		//printf("THE KEYBUFFER MSG is \"%s\"\n", keyBuffer);
		//printf("SERVER: second argument - I received this from the client: \"%s\"\n", keyBuffer);
	// ******************

		/* LOGIC,
		// Send a Success message back to the client
		charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
		if (charsRead < 0) error("ERROR writing to socket");
		*/

		//3. send back final encrypt message.
		char* secret = encryptMsg(msgBuffer, f1msgsize, keyBuffer);
		//printf("THE SECRET MSG is %s\n", secret);
		// send back the encrypt message.
		//printf("NOW SENDING BACK ENCRYPT MESSAGE\n");
		charsRead = send(establishedConnectionFD, secret, f1msgsize, 0); // Send success back
		if (charsRead < 0) error("ERROR writing to socket");
		free(secret);
		free(msgBuffer);
		free(keyBuffer);
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
}

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections - trying to increase it just in case

    pid_t spawnpid = -5;
//****************************
// WHILE LOOP FOR CONTINUOUS PORT CONNECTION WILL GO HERE.
	while(1){
		spawnpid = -5;
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

        /*** 
        FORK GOES HERE - from lecture 
        ****/
        spawnpid = fork();
        switch(spawnpid){
            case -1:
                perror("ERROR HULL BREACH!\n");
                fflush(stdout);
                close(establishedConnectionFD); // Close the existing socket which is connected to the client
                break; // NOT EXIT; because we want our loop to keep going.
            case 0:
                // pass off our back and forth into a fork and let it handle its own message.
                forkOffMsg(establishedConnectionFD);
                //close(establishedConnectionFD); // Close the existing socket which is connected to the client
                exit(0);
				break; // to continue on to the next one. 
            default:
                close(establishedConnectionFD); // Close the existing socket which is connected to the client
				break;
		}

	//END OF WHILE LOOP FOR CONNECTION GOES HERE.
	}
	close(listenSocketFD); // Close the listening socket
	return 0; 
}
