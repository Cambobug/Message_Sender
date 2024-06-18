Cameron Fraser

In order to run the progams first type  "make all" which will create executables called "server" and "sendFile". To start the server type "./server [PORT] [OPTIONAL_BUFFER_SIZE]" where the "PORT" is the port you wish to open the server on and the "OPTIONAL_BUFFER_SIZE" is the size of the buffer you wish use. In order to run the client type "./sendFile [TEXT_FILE] [IP/ADDRESS_NAME]:[PORT] [OPTIONAL_BUFFER_SIZE]" where "TEXT_FILE" is the name of the text file you wish to send, "IP/ADDRESS_NAME" is the IP or address name of ther host the server is running on, "PORT" is the port the server is listening on and "OPTIONAL_BUFFER_SIZE" is the size of the buffer you wish use.

Simultaneous Script:

In order to run the script that starts 4 clients at once you must type "./multiClient.bat". This will run a batch file which will call the client 4 times with 4 different files to a local port 50652. If you are given a "permission denied" error, type "chmod u+x multiClient.bat" into the terminal and try again. The text files required to run the script are included in the "TXTFILES" folder.

Known Issue:

When trying to connect to a host using the external IP, if the specified IP does not have the open port that the client specified the client seems to freeze for a minute or so before it gives a connection failed message.