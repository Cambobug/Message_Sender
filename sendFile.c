#include "network.h"

int main(int argc, char **argv)
{
    long bufferSize = BUFFER;

    // checks that the correct number or command line arguments are given
    if(argc < 3 || argc > 4) 
    {
        printf("Invalid number of arguments given! Exiting!\n");
        return -1;
    }

    //attempts to pull the IP and port out of the string
    char IPAddress[strlen(argv[2])];
    char port[strlen(argv[2])];
    
    //gets the IP before the :
    char * token = strtok(argv[2], ":"); 
    strcpy(IPAddress, token);

    //gets the port after the :
    token = strtok(NULL, ":");
    strcpy(port, token);

    //adds NULL terminator and converts the port to a number
    IPAddress[strlen(IPAddress)] = '\0';
    port[strlen(port)] = '\0';
    int portNum = atoi(port);

    //checks to make sure neither the IP or port are empty
    if(strlen(IPAddress) == 0 || strlen(port) == 0)
    {
        printf("Invalid IP:Port argument! Exiting!");
        return -1;
    }

    //checks if the optional buffer specified
    if(argc == 4 && atoi(argv[3]) != 0)
    {
        bufferSize = atoi(argv[3]);
    }

    //attempts to open the file specified in the command line
    FILE * fileToSend = fopen(argv[1], "r");
    if(fileToSend == NULL)
    {
        printf("File failed to open! Exiting!\n");
        return -1;
    }

    printf("IP: %s\nPORT: %d\n", IPAddress, portNum);

    //get size of the file
    fseek(fileToSend, 0, SEEK_END);
    long fileSize = ftell(fileToSend);
    fseek(fileToSend, 0, SEEK_SET);

    // fileSize + fileName length + 2 | characters + size of chunksToSend number
    int chunkNum = (fileSize + strlen(argv[1]) + 2) / bufferSize + 1; // gets the number of chunks the file will be split into
    int chunkNumLength = (chunkNum==0)?1:log10(chunkNum)+1; // finds te number of digits in chunkNum, needed to know how much space will be taken up by the number 

    long totalSize = fileSize + strlen(argv[1]) + 2 + chunkNumLength + 1; // gets the total size of the full message 

    long chunksToSend = (totalSize / bufferSize) + 1; //number of chunks that need to be sent
    char chunks[33];
    sprintf(chunks, "%d", chunksToSend); // converts chunksToSend to a string

    //END OF FILE PREPERATION
    int mySocket;
    int addressName = 0;

    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in destination;
    struct addrinfo hints, *result;
    memset(&destination, 0, sizeof(destination));

    // sets the family to TCP, converts IP to network language, and converts the port to network language
    destination.sin_family = AF_INET;
    if(inet_aton(IPAddress, &destination.sin_addr) == 0) // failed to convert number and dot notation to network IP
    {
        printf("Address Name Specified\n");
        memset (&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if(getaddrinfo(IPAddress, NULL, &hints, &result) != 0)
        {
            printf("Specified address cannot be connected to! Exiting!\n");
            fclose(fileToSend);
            close(mySocket);
            return -1;
        }
        addressName = 1;

        //freeaddrinfo(result);
    }
    destination.sin_port = htons(portNum);
    if(addressName == 1)
    {
        //printf("Address name specified!\n");
        int connected = 0;
        for(struct addrinfo *res = result; res != NULL; res = res->ai_next)
        {
            char str[100];
            struct sockaddr_in *p = (struct sockaddr_in *)res->ai_addr;
            inet_ntop(AF_INET, &p->sin_addr, str, 100);
            inet_aton(str, &destination.sin_addr);
            if(connect(mySocket, (struct sockaddr*)&destination, sizeof(struct sockaddr_in)) != -1) // successfully connected on a address
            {
                //printf("Connection successful!\n");
                connected = 1;
                freeaddrinfo(result);
                break;
            }   
        }
        if(connected == 0)
        {
            printf("Connection failed! Exiting!\n");
            freeaddrinfo(result);
            fclose(fileToSend);
            close(mySocket);
            return -1;
        }
    }
    else
    {
        freeaddrinfo(result);
        if(connect(mySocket, (struct sockaddr*)&destination, sizeof(struct sockaddr_in)) == -1) // failed to connect on an address
        {
            printf("Connection failed! Exiting!\n");
            fclose(fileToSend);
            close(mySocket);
            return -1;
        }
    }
    
    char * message = malloc(sizeof(char) * (totalSize + 1));
    //START OF MESSAGE PREPERATIONS
    if(chunksToSend == 1) // if the whole file plus filename and number of chunks is less than the buffer size
    {
        strcpy(message, chunks);
        strcat(message, "|");
        strcat(message, argv[1]);
        strcat(message, "|");

        int msgLen = strlen(message);
        char fileMsg[fileSize + 1];
        int bytesRead = fread(fileMsg, 1, fileSize, fileToSend);
        //fileMsg[bytesRead] = '\0';
        strcat(message, fileMsg);
        message[totalSize] = '\0';

        if(send(mySocket, message, strlen(message), 0) == -1)
        {
            printf("Send failed on message 1 with file: %s! Exiting!\n", argv[1]);
            fclose(fileToSend);
            close(mySocket);
            return -1;
        }
    }
    else
    {
        //creates entire file message
        strcpy(message, chunks);
        strcat(message, "|");
        strcat(message, argv[1]);
        strcat(message, "|");

        char fileMsg[fileSize + 1];
        int bytesRead = fread(fileMsg, 1, fileSize, fileToSend);
        fileMsg[bytesRead] = '\0';
        strcat(message, fileMsg);
        message[totalSize] = '\0';

        char * messagePointer = message; 
        
        for(int i = 1; i < chunksToSend; ++i)
        {
            if(send(mySocket, messagePointer, bufferSize, 0) == -1)
            {
                printf("Send failed on message %d with file: %s! Exiting!\n", i, argv[1]);
                fclose(fileToSend);
                close(mySocket);
                return -1;
            }
            messagePointer = messagePointer+bufferSize;
        }

        if(send(mySocket, messagePointer, strlen(messagePointer), 0) == -1)
        {
            printf("Send failed on message %d with file: %s! Exiting!\n", chunksToSend, argv[1]);
            fclose(fileToSend);
            close(mySocket);
            return -1;
        }
    }

    printf("Finished Sending File!\n");
    free(message);
    close(mySocket);
    fclose(fileToSend);

    return 0;
}