#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

// function will return target match at index - use for encrypt and decrypt
int isWhere(char arry[], char target, int length){

	int i;
	for(i=1; i < length; i++){
		if(arry[i] == target){
			return i;
		}
	}
	return 0;
}


// this will check to make sure msg and key file are good and valid
// this function will compare both files, and will return 1, if the key is shorter than msg.
int isKeyTooShort(char msg[], char key[]){
   //printf("GOINNG TO TEST two things... 1. bad file and correct size.\n");
   //printf("THe message file is \"%s\"\n", msg);
   //printf("The Key file is \'%s\"\n", key);

    // variables
    long msgSize, keySize;
    FILE *msgFp, *keyFp;
    
    // open msg file
    msgFp = fopen(msg, "r");
    if(msgFp == NULL){
        perror("MSG FILE NOT FOUND\n");
        exit(1);
    }
    // Moving pointer to end 
    fseek(msgFp, 0, SEEK_END); 
    // Printing position of pointer 
    //printf("CLIENT: %d\n", ftell(fp));
    msgSize = ftell(msgFp);
    fclose(msgFp);

    // open key file.
    keyFp = fopen(key, "r");
    if(keyFp == NULL){
        perror("KEY FILE NOT FOUND\n");
        exit(1);
    }
    // Moving pointer to end 
    fseek(keyFp, 0, SEEK_END); 
    // Printing position of pointer 
    //printf("CLIENT: %d\n", ftell(fp));
    keySize = ftell(keyFp);
    fclose(keyFp);

    //printf("THE MSG SIZE IS %d, and the Key size is %d\n", msgSize, keySize);
    if(msgSize <= keySize){
        return 0;
    }
    else{
        return 1;
    }
}

// this function will be used to check for bad files
// function will iterate through file, to determine if it contain bad characters.
int isBadFile(char msg[]){

    // variables
    long msgSize;
    int flag = 0;
    int i;
    char ch;
    FILE *msgFp;
    // open msg file
    msgFp = fopen(msg, "r");
    if(msgFp == NULL){
        perror("FILE NOT FOUND\n");
        return 1;
    }

    fseek(msgFp, 0, SEEK_END); 
    // Printing position of pointer 
    //printf("CLIENT: %d\n", ftell(fp));
    msgSize = ftell(msgFp);
    fseek(msgFp, 0, SEEK_SET); 

    //https://www.geeksforgeeks.org/c-program-to-count-the-number-of-characters-in-a-file/
    for(i = 0; i < msgSize - 1; i++){
        ch = getc(msgFp);
        //printf("%d - %c\n", ch);
        if((ch < 91 && ch > 64) || ch == 32){
            continue;
        }
        else{
            flag = 1;
        }

    }
    //printf("FLAG is at %d\n", flag);
    fclose(msgFp);
    return flag;

}

// this function will read the content of a file and return will a string buffer.
char *readFile(char fileName[]){

    // variables;
	long fileSize;
    FILE *fp;
    fp = fopen(fileName, "r");
    if(fp == NULL){
        perror("FILE NOT FOUND\n");
        //return 1;
    }
    fseek(fp, 0, SEEK_END); 
    fileSize = ftell(fp);	
    fseek(fp, 0, SEEK_SET);

    // make read and make the buffer size.
    char *fileBuffer = malloc(fileSize);
    fgets(fileBuffer, fileSize, fp);
    fclose(fp);
    //printf("WHAT WAS READ WAS %s\n", fileBuffer);
    return fileBuffer;
}

// this function will read the content of a file and return the size of the file.
// this will return a long integers
long getFileSize(char fileName[]){

    // variables;
	long fileSize;
    FILE *fp;
    fp = fopen(fileName, "r");
    if(fp == NULL){
        perror("FILE NOT FOUND\n");
        //return 0;
    }
    fseek(fp, 0, SEEK_END); 
    fileSize = ftell(fp);	
    fclose(fp);
    //printf("THE FILE SIZE IS %d\n", fileSize);
    return fileSize;
}

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
    
    // note that there is 4 <<<< otp_enc plaintext1 mykey 57171 > ciphertext1
    // client localhost port
	if (argc < 4) { fprintf(stderr,"USAGE: %s - INVALID NUMBER OF ARGUMENTS\n", argv[0]); exit(0); } // Check usage & args

    // CHECK FOR SHORT KEY AND BAD FILES
    //printf("GOINNG TO TEST two things... 1. bad file and correct size.")
    if(isKeyTooShort(argv[1], argv[2])){
        //printf("ERROR: key \'%s\' is too short\n", argv[2]);
	    fprintf(stderr, "ERROR: key \'%s\' is too short\n", argv[2]);
		exit(1);
    }

    if(isBadFile(argv[1])){
        fprintf(stderr, "%s error: input contains bad Characters\n", argv[0]);
		exit(1);
    }


	// Set up the server address struct 
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	//serverHostInfo = gethostbyname(argv[1]); // Convert the machine name into a special form of address
	// **** LOCALHOST????
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

    //check if correct client server interaction.
    // send name to server.
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	strcpy(buffer, argv[0]);
	// Send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	//printf("********* CHECKING IF CORRECT NAME IS GIVEN AND SUCESS IS REACHED\n");
	//printf("MESSAGE AND BUFFER RECIEVED IS \"%s\"", buffer);
	if(strcmp(buffer, "Success") != 0){
		printf("ERROR: could not contact opt_dec_d on port %s\n", argv[3]);
		close(socketFD); // Close the socket because its the wrong one before exiting
		exit(2);
	}
	else{
	// *******************************************************
	// logic.
	// send size, then plaintext msg first.
	// send keySize, then key second.
	// recieved encrypt key back from enc_d.
		// open the file.
		//printf("CLIENT: THE name of first argument is %s\n", argv[1]);
		long fileSize = getFileSize(argv[1]);
		int convert_length = htonl(fileSize);
		//write to socket the size of the message
		// note to self, we will only send one size, because... the only thing
		// that matter is the plaintext size.
		write(socketFD, &convert_length, sizeof(convert_length));
		char *fileBuffer = readFile(argv[1]);
		//printf("CLIENT: THIS IS FILE BUFFER STRING %s\n", fileBuffer);
		// Send file message to server. 
		charsWritten = send(socketFD, fileBuffer, strlen(fileBuffer), 0); // Write to the server
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(fileBuffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
		//fclose(fp);

		// Get return message from server THIS IS TO ENSURE SUCCESS.
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
		if(strcmp(buffer, "Success") != 0){
			printf("failure for buffer is %s\n", buffer);
			//printf("ERROR: FAILURE UPON plaintext FILE TRANSFER");
			exit(1);
		}

/* - REMOVING SENDING FILE SIZE, because well use msg size.
####################################
		fileSize = getFileSize(argv[2]);
		convert_length = htonl(fileSize);
		//write to socket the size of the message
		// note to self, we will only send one size, because... the only thing
		// that matter is the plaintext size.
		write(socketFD, &convert_length, sizeof(convert_length));
*/

		char *fileBuffer2 = readFile(argv[2]);
		//printf("CLIENT: THIS IS FILE BUFFER STRING \"%s\"\n", fileBuffer2);
		// Send message to server
		// only send up to the expected amount.

    
		charsWritten = send(socketFD, fileBuffer2, strlen(fileBuffer), 0); // Write to the server
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(fileBuffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
		//fclose(fp2);
		// get back the encrypt message from encrypter.
		// we use the inital file buffer because thats the exact same size
		// Get return message from server
		memset(fileBuffer, '\0', sizeof(fileBuffer)); // Clear out the buffer again for reuse
		// because somehow i always add 2 characters to the end of the coded msgs.
		
		//charsRead = recv(socketFD, fileBuffer, sizeof(fileBuffer)-1, 0); 
		charsRead = recv(socketFD, fileBuffer, fileSize, 0);
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		//printf("CLIENT: I received this from the server: \"%s\"\n", fileBuffer);
		printf("%s\n", fileBuffer);
	/*
		// for your own purposes decrypt the message just in case 
		char* secret = decryptMsg(fileBuffer, sizeof(fileBuffer), fileBuffer2);
		printf("the original message was\n");
		printf("%s\n", secret);
		free(secret);
	*/
	}

	close(socketFD); // Close the socket
	return 0;
}
