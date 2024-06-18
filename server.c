#include "network.h"

int mySocket; 
int connectedSocket;

static void sigintCatcher()
{
    printf("\n\n************** Caught SIG_INT: shutting down the server ********************\n");

	close(connectedSocket);
	close(mySocket);
	exit(0);
}

int main(int argc, char **argv)
{
    int portNumber = PORTNUM;
    int bufferSize = BUFFER;

    if(argc == 2 || argc == 3) // user may have specified a port number
    {
        if(atoi(argv[1]) != 0) // checks that the command line argument for server is a number
        {
            portNumber = atoi(argv[1]);
        }
        if(argc == 3 && atoi(argv[2]) != 0) // check if a buffer size was specified
        {
            bufferSize = atoi(argv[2]);
        }
    }
    else
    {
        printf("Invalid number of arguments given! Exiting!\n");
        return -1;
    }

    struct sockaddr_in connection; // socket information for client connection
    struct sockaddr_in server; // socket information for server

    signal(SIGINT, sigintCatcher);

    socklen_t sockSize = sizeof(struct sockaddr_in);

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET; // server will use IPV4 addressing
    server.sin_addr.s_addr = htonl(INADDR_ANY); // server will set its address to any interface
    server.sin_port = htons(portNumber);

    /* Create a socket.
   	   The arguments indicate that this is an IPv4, TCP socket
	*/
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mySocket == -1)
    {
        printf("Failed to create a socket.\n");
        return -1;
    }

    // checks whether the a socket was able to be opened on the specified port number
    if(bind(mySocket, (struct sockaddr *)&server, sizeof(struct sockaddr)) != 0) 
    {
        printf("Failed to opening a TCP socket on localhost: %d\n", portNumber);
        printf("%s\n", strerror(errno));

        close(mySocket);
        return -1;
    }

    printf("Server Activated: Listening on PORT: %d\n", portNumber);
    if(listen(mySocket, 10) == -1) // listens on socket, allowing a queue of 10 connections
    {
        printf("listen() command failed! Exiting!");
        printf("%s\n", strerror(errno));

        close(mySocket);
        return -1;
    } 

    connectedSocket = accept(mySocket, (struct sockaddr *)&connection, &sockSize);
    if(connectedSocket == -1)
    {
        printf("accept() command failed! Exiting!");
        printf("%s\n", strerror(errno));

        close(mySocket);
        return -1;
    }
    
    printf("Server accepted send request!\n");

    int currChunk = 1;
    int numChunks = 1;
    
    while(connectedSocket != -1) // loops until connect call fails
    {
        int numBytes = 0;
        char fileName[100];
        char * buffer = malloc(sizeof(char) * (bufferSize+1));
        char * head = buffer;
        printf("Incoming connection from %s on port %d\n", inet_ntoa(connection.sin_addr), ntohs(connection.sin_port));

        // creates a buffer with a length of BUFFER + 1 to hold each transmission
        //the recv signal will block until the full request has been satisfied
        //numBytes += recv(connectedSocket, buffer, BUFFER, MSG_WAITALL);
        FILE * sentFile = NULL;

        for(currChunk = 1; currChunk <= numChunks; currChunk++)
        {   
            long bytesSent = recv(connectedSocket, buffer, bufferSize, 0);
            if(bytesSent == -1)
            {
                printf("Receive failed on IP: %s! Exiting!\n", inet_ntoa(connection.sin_addr));
                printf("%s\n", strerror(errno));
                close(connectedSocket);
                close(mySocket);
                return -1;
            }
            numBytes += bytesSent; // counts up the total amount of bytes received by the server

            buffer[bufferSize] = '\0';
            int textLen = bytesSent;
            if(currChunk == 1) // only executes on first transmission
            {
                char * token = strtok(buffer, "|");
                textLen = strlen(token);
                numChunks = atoi(token);
                token = strtok(NULL, "|");
                textLen += strlen(token) + 2;
                //strcpy(fileName, "A1ServerOutput/"); //CHANGE THIS BACK TO ITS ROOT FOLDER
                strcpy(fileName, token);
                token = strtok(NULL, "|");
                buffer = token;

                sentFile = fopen(fileName, "w");
                if(sentFile == NULL)
                {
                    printf("Failed to open file! Exiting!\n");
                    close(connectedSocket);
                    close(mySocket);
                    return -1;
                }
            }

            if(currChunk == 1)
            {
                fwrite(buffer, 1, bytesSent - textLen, sentFile);
            }
            else
            {
                fwrite(buffer, 1, textLen, sentFile);
            }
            memset(head, 0, bufferSize);
            buffer = head;

        }

        if(sentFile != NULL)
        {
            fclose(sentFile);
        }

        printf("\n\n-----------File Received!-----------\n");
        printf("File Name: %s\n", fileName);
        printf("Total Size of File: %d bytes\n", numBytes);
        printf("IP Address of Sender: %s\n", inet_ntoa(connection.sin_addr));
        printf("Number of Sections Sent: %d\n", numChunks);

        numBytes = 0;
        numChunks = 1;
        currChunk = 1;
        memset(fileName, 0, sizeof(fileName));
        free(buffer);

        close(connectedSocket);

        //listen(mySocket, 0); // listens on socket, allowing a queue of 1 connection
        connectedSocket = accept(mySocket, (struct sockaddr *)&connection, &sockSize);
        if(connectedSocket == -1)
        {
            printf("accept() command failed! Exiting!");
            printf("%s\n", strerror(errno));
            
            close(mySocket);
            return -1;
        }
    }
    
    
    close(mySocket);
    return 0;

}