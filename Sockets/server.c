/*login: anwar027
  date: 04/28/18
  name: Muhammad Anwar
*/

#include "server.h"

#define _BSD_SOURCE
#define NUM_ARGS 3

#define MAX_CONNECTIONS 20

void * threadFunction(void * arg); // prototype

int main(int argc, char** argv) {

  if (argc != NUM_ARGS) {
    printf("Usage: %s <DAG FILE> <SERVER PORT>\n", argv[0]);
    exit(1);
  }

  struct node tree[MAX_NUMBER_NODES]; // declare internal tree data structure

  initializeNodeArray(tree);
  int numNodes = createDAGStructure(tree, argv[1]); // pass in tree and path of DAG file
  initializeNodeMutex(tree, numNodes);
  
  int portNum;
  sscanf(argv[2], "%d", &portNum);

  printf("Server listening on port %d\n", portNum);
  // Create a TCP socket.
  int sock = socket(AF_INET , SOCK_STREAM , 0),
    length;
  
  // Bind it to a local address.
  struct sockaddr_in servAddress, client;
  servAddress.sin_family = AF_INET;
  servAddress.sin_port = htons(portNum);
  servAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sock, (struct sockaddr *) &servAddress, sizeof(servAddress));

  // TODO: Listen on this socket.
  listen(sock, MAX_CONNECTIONS);
  // A server typically runs infinitely, with some boolean flag to terminate.
  while (1) {    
    socklen_t size = sizeof(struct sockaddr_in);
    
    // Accept a connection.
    int client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t*)&length);
    
    if (client_sock < 0){
      perror("Accept Failed");
      exit(2);
    }

    //initialize client IP
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(client_sock, (struct sockaddr *)&client, &addr_size);

    printf("Connection initiated from client at %s:%d\n", inet_ntoa(client.sin_addr), 
        (int) ntohs(client.sin_port));

    int readBytes = 1;// initialize beforeloop

    while(readBytes > 0){ // stop reading from socket when client has stopped writing to it
      // Buffer for data.
      char buffer[256];
      for (int i = 0; i < 256; i++)
        buffer[i] = 0;
      
      // Read from the socket and print the contents.
      readBytes = recv(client_sock, buffer, 256, 0);

      if(readBytes > 0){
        struct threadStruct *tStruct = (struct threadStruct*) malloc(sizeof(struct threadStruct));
        mallocCheck(tStruct);
        // initialize struct with all needed information
        tStruct->clientMessage = buffer;
        tStruct->socket = client_sock;
        tStruct->dag = tree;
        tStruct->numNodes = &numNodes;
        tStruct->client = client;

        pthread_t helperThread;// make a thread for every client request

        int lockVar; // used for error checking for locks

        lockVar = pthread_mutex_lock(tStruct->dag[0].nodeMutex);// every node points to same lock
        lockCheck(lockVar);

        pthread_create(&helperThread, NULL, threadFunction, (void *) tStruct);
        
        lockVar = pthread_mutex_unlock(tStruct->dag[0].nodeMutex);// every node points to same lock
        unlockCheck(lockVar);

        pthread_join(helperThread, NULL); // wait for thread to finish
      }// end if
    }// end while
    // Close the connection.

    printf("Closed connection with client at %s:%d\n", inet_ntoa(client.sin_addr), 
        (int) ntohs(client.sin_port));
    close(client_sock);
  }
  
  // Close the server socket.
  close(sock);

  freeNodeMutex(tree, numNodes); 

  return 0;
}

/**************************************************************/
/**Function : threadFunction
 * Arguments: 'arg' - struct threadStruct * that needs to be 
 *                casted back
 *
 * Output: Processes client request, writes to sockets, and printfs
 *          appropriate messages.
 * 
 * About: threadFunction tokenizes client request to get cmd, region,
 * and data. Then depending on command it calls a helper function,
 * then writes server response to socket.
 */
void * threadFunction(void * arg){
  struct threadStruct *tStruct = (struct threadStruct *) arg;

  // tStruct->clientMessage is in the form "cmd;regionName;data"
  char *cmd = strtok(tStruct->clientMessage, ";");// get two character cmd
  char *serverResponse = "\0";
  char *regionName = (char *)malloc(sizeof(char)*15);
  mallocCheck(regionName);

  memset(regionName, 0, 15);
  char * temp = strtok(NULL, ";");

  for (int i = 0, j = 0; i < 15; i++)
    if (temp[i] != ' ')
      regionName[j++] = temp[i];
  
  char *data = strtok(NULL, ";");
 
  printf("Request received from client at %s:%d, ", inet_ntoa(tStruct->client.sin_addr), 
      (int) ntohs(tStruct->client.sin_port));// output first part of message then second part in thread fcn

  if(strcmp(cmd, "RW") == 0)// if success there is no data
    printf("%s (null) (null)\n", cmd);
  else
    printf("%s %s %s\n", cmd, regionName, data);// complete print statements

  // match cmd to function call
  if(strcmp(cmd, "RW") == 0){
    serverResponse = returnWinner(tStruct->dag, *(tStruct->numNodes));
  } else if(strcmp(cmd, "CV") == 0){
    serverResponse = countVotes(regionName, tStruct->dag, *(tStruct->numNodes));
  } else if(strcmp(cmd, "OP") == 0){
    serverResponse = openPolls(regionName, tStruct->dag, *(tStruct->numNodes));
  } else if(strcmp(cmd, "AV") == 0){
    serverResponse = addVotes(regionName, data, tStruct->dag, *(tStruct->numNodes));
  } else if(strcmp(cmd, "RV") == 0){
    serverResponse = removeVotes(regionName, data, tStruct->dag, *(tStruct->numNodes));
  } else if(strcmp(cmd, "CP") == 0){
    serverResponse = closePolls(regionName, tStruct->dag, *(tStruct->numNodes));
  } else if(strcmp(cmd, "AR") == 0){
    serverResponse = addRegion(regionName, data, tStruct->dag, tStruct->numNodes);
  } else { // unhandled command "UC"
    serverResponse = (char *) malloc(sizeof(char) * 4); // 2 cmd 1 ; 1 \0
    mallocCheck(serverResponse);

    memset(serverResponse, 0, 4);
    copyStrToBuffer(serverResponse, "UC;");
  }

  char responseCode[3]; // will hold 2 letter command and null
  memset(responseCode, 0, 3);
  for(int i = 0; i < 2; i++)
    responseCode[i] = serverResponse[i];

  char responseData[256];
  memset(responseData, 0, 256);
  int j = 3; // index to start copying skipping "SC;"
  for(int i = 0; serverResponse[j] != 0; i++, j++)
    responseData[i] = serverResponse[j];

  printf("Sending response to client at %s:%d, ", inet_ntoa(tStruct->client.sin_addr), 
      (int) ntohs(tStruct->client.sin_port));

  // if success there is no data, unless RW or CV
  if(strcmp(responseCode, "SC") == 0 && strcmp(cmd, "RW") != 0 && 
        strcmp(cmd, "CV") != 0 && strcmp(cmd, "AR") != 0)
    printf("%s (null)\n", responseCode);
  else 
    printf("%s %s\n", responseCode, responseData);

  write(tStruct->socket, serverResponse, strlen(serverResponse));

  free(serverResponse);
}