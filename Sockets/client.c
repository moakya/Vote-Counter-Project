/*login: anwar027
  date: 04/28/18
  name: Muhammad Anwar
*/

#include "client.h"

int main(int argc, char **argv){

  if (argc != 4){
    printf("Usage: %s <REQ FILE> <SERVER IP> <SERVER PORT>\n", argv[0]);
    exit (1);
  }  // end if

  char *mainDir = NULL;

  //get the root directory of the input files
  getMainDir(argv[1], &mainDir);

  //begin the process of creating a request and calling the server with it
  processInfo(argv[1], mainDir, argv[2], argv[3]);

  free(mainDir);
  return 0;
}

void processInfo(char *reqFile, char *dir, char *iPAddr, char *portStr){ 
  FILE *infile;
  infile = fopen(reqFile, "r");
  openFileCheck(infile);

  //get size of file
  fseek(infile, 0, SEEK_END);
  long fsize = ftell(infile);
  rewind(infile); //go back to beginning of file

  //to hold the file data
  char *buff = malloc(fsize + 1);
  mallocCheck(buff);
  buff[fsize] = '\0'; //end buff in NULL char

  //convert the port string into an int for use
  int portNum;
  sscanf(portStr, "%d", &portNum);
  
  int sock = socket(AF_INET , SOCK_STREAM , 0);
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(portNum);
  address.sin_addr.s_addr = inet_addr(iPAddr);
  
  //try to connect to the server
  if (connect(sock, (struct sockaddr *) &address, sizeof(address)) == 0) {
    printf("Initiated connection with server at %s:%d\n", iPAddr, portNum);
    while(fgets(buff, fsize, infile) != NULL){
      if(buff[0] != '\n') // only trimwhitespace if buff is not only a newline
        buff = trimwhitespace(buff);
      else
        continue;// go to next iteration if buff is a '\n'

      if(buff[strlen(buff) - 1] == '\n')
        buff[strlen(buff) - 1] = 0;

      //to contain the command and its list
      char *strArray[3];
      int count = 0;

      char *token = strtok(buff, " ");
      strArray[count] = (char *) malloc(strlen(token) * sizeof(char) + 1);
      mallocCheck(strArray[count]);
      memset(strArray[count], 0, strlen(token) + 1);
      copyStrToBuffer(strArray[count++], token);

      //ensure that the strArray is initialized with all elements
      while(token != NULL){
        token = strtok(NULL, " ");
        if(token != NULL){
          strArray[count] = (char *) malloc(strlen(token) * sizeof(char) + 1);
          mallocCheck(strArray[count]);
          memset(strArray[count], 0, strlen(token) + 1);
          copyStrToBuffer(strArray[count++], token);
          if(count > 3){
            printf("Error. Malformed command, exceeded maximum number of args.\n");
            exit(0);
          } //end inner if
        } //end outer if
      } //end while

      //check whether the command code is legitimate
      int flag = chkCmdReq(strArray[0], count);

      if (flag != 1){
        printf("Invalid Use of Command %s! Exiting Program!\n", strArray[0]);
        free(buff);
        for(int i = 0; i < count; i++){
          char *temp = strArray[i];
          free(temp);
        } //end for
        exit(3);
      } //end if

      //VALID COMMAND
      else{
        //create the request string
        char *req = createRequest(strArray, dir);
        
        int buffSize = strlen(req)+1;
        char buffer[buffSize];
        memset(buffer, 0, buffSize);
        
        //write all contents of request to static buffer
        for (int i = 0; i < strlen(req); i++)
          buffer[i] = req[i];

        //separate the request into multiple pieces for display
        char cmd[3];
        cmd[0] = req[0]; cmd[1] = req[1]; cmd[2] = 0;

        char region[15];
        memset(region, 0, 15);
        char data[strlen(req) - 18];
        memset(data, 0, strlen(req) - 18);

        if (req[3] != ' '){
          for (int i = 3, j = 0; i < 18; i++, j++)
            if (req[i] != ' ')
              region[j] = req[i];

          if (req[19] != 0)
            for (int i = 19, j = 0; i < strlen(req); i++, j++)
              data[j] = req[i];
          else{
            copyStrToBuffer(data, "(null)");  
          }
        } //end outer if

        else{
          copyStrToBuffer(region, "(null)");
          copyStrToBuffer(data, "(null)");
        }

        //print request process statement
        printf("Sending request to server: %s %s %s\n", cmd, region, data);

        //write to the socket
        write(sock, buffer, buffSize);
        
        //buffer to contain contents returned from server
        char retBuf[1024];
        memset(retBuf, 0, 1024);

        //get data from the server
        recv(sock, retBuf, 1024, 0);
        
        //separate the contents of the new buffer
        char retCode[3];
        retCode[0] = retBuf[0]; retCode[1] = retBuf[1]; retCode[2] = 0;

        char retData[strlen(retBuf)];
        memset(retData, 0, strlen(retBuf));

        if (retBuf[3] != 0)
          for (int i = 3, j = 0; i < strlen(retBuf); i++, j++)
            retData[j] = retBuf[i];
        else
          copyStrToBuffer(retData, "(null)");

        //print the received response correctly
        printf("Received response from server: %s %s\n", retCode, retData);

        for(int i = 0; i < count; i++){
          char *temp = strArray[i];
          free(temp);
        } //end for
      } //end else
    } //end while
    close(sock);
    printf("Closed connection with server at %s:%d\n", iPAddr, portNum);
  } //end connection if
  else 
    perror("Connection failed!");
  
  free(buff);
  fclose(infile); //close the input file
}

char * createRequest(char *strArray[], char *dir){
  //multiple strings to contain different parts of the request
  char *cmd = NULL;
  char *region = (char *)malloc(sizeof(char) * 15 + 1);
  mallocCheck(region);
  char *data = "\0";

  memset(region, ' ', 15);
  region[15] = 0;

  //will change if the region name needs to be added to the request
  int flag = 0;

  struct candNode *votes = NULL;
  if (strcmp(strArray[0], "Return_Winner") == 0){
    cmd = "RW";
  }
  else if (strcmp(strArray[0], "Count_Votes") == 0){
    cmd = "CV";
    flag = 1;
  }
  else if (strcmp(strArray[0], "Open_Polls") == 0){
    cmd = "OP";
    flag = 1;
  }
  else if (strcmp(strArray[0], "Add_Votes") == 0){
    cmd = "AV";
    flag = 1;
    char *filename = strConcat(dir, strArray[2]);
    parseLeafNodeInput(filename, &votes);
    data = getDataString(votes);
  }
  else if (strcmp(strArray[0], "Remove_Votes") == 0){
    cmd = "RV";
    flag = 1;
    char *filename = strConcat(dir, strArray[2]);
    parseLeafNodeInput(filename, &votes);
    data = getDataString(votes);
  }
  else if (strcmp(strArray[0], "Close_Polls") == 0){
    cmd = "CP";
    flag = 1;
  }
  else if (strcmp(strArray[0], "Add_Region") == 0){
    cmd = "AR";
    flag = 1;

    data =(char *)malloc(strlen(strArray[2]) + 1 * sizeof(char));
    mallocCheck(data);

    memset(data, 0, strlen(strArray[2]) + 1);
    for (int i = 0; i < strlen(strArray[2]); i++)
      data[i] = strArray[2][i];
  }
  else {
    printf("ERROR! CAN NOT FIND COMMAND! EXITING!\n");
    exit(7);
  } //massive failure case

  //add region name if needed
  if (flag == 1)
    for (int i = 0; i < strlen(strArray[1]); i++)
      region[i] = strArray[1][i];

  //string to contain the final request
  char *final = (char *)malloc(17+strlen(data)+2+1);
  mallocCheck(final);
  memset(final, 0, 17+strlen(data)+2+1);
  int count = 0;
  
  //add the command
  for (int i = 0; i < 2; i++)
    final[count++] = cmd[i]; 

  final[count++] = ';';

  //add the region
  for (int i = 0; i < 15; i++)
    final[count++] = region[i];

  final[count++] = ';';

  //add the data segment
  for (int i = 0; i < strlen(data); i++)
    final[count++] = data[i];
  final[count++] = 0;

  //free unneeded mallocs
  freeCandList(votes);
  free(region);
  if (data != "\0")
    free(data);

  //return the request
  return final;
}

char *getDataString(candnode_t *votes){
  int size = listSize(votes);

  //get max size possible for the list
  int mem = (size * 100) + (size * 5) + size + size;

  //initialize the string to the max possible length
  char *temp = (char *)malloc(mem * sizeof(char));
  mallocCheck(temp);
  memset(temp, 0, mem);

  struct candNode *n = votes;

  //ensure the formatting is correct in the new string
  while (n != NULL){
    appendStr(temp, n->name); //add the candidate name
    appendStr(temp, ":"); //add a ':'
    char *x = (char*)malloc(5);
    mallocCheck(x);
    memset(x, 0, 5);
    sprintf(x, "%d", n->count); //change the count to a string
    appendStr(temp, x); //add the count string to the string
    if (n->next != NULL)
      appendStr(temp, ","); //add a ',' as long is its not the last node
    n = n->next;
    free(x);  //free x, as its unneeded
  }

  return temp;  //return the data string

}

void parseLeafNodeInput(char *path, candnode_t **vote){
  FILE *infile;
  infile = fopen(path, "r");
  openFileCheck(infile);

  //get size of file
  fseek(infile, 0, SEEK_END);
  long fsize = ftell(infile);
  rewind(infile); //go back to beginning of file

  //to hold all the file data
  char *buff = malloc(fsize + 1);
  mallocCheck(buff);

  while (fgets(buff, fsize + 1, infile)){
    char *x = trimwhitespace(buff);

    char *ntok = (char *)malloc(260 * sizeof(char));
    mallocCheck(ntok);
    memset(ntok, 0, 260);
    
    for (int i = 0; i < strlen(x); i++)
      ntok[i] = x[i];
    //printf("ntok: %s\n", ntok);
    if(*vote == NULL){
      *vote = (struct candNode*)malloc(sizeof(struct candNode));
      mallocCheck(vote);

      memset((*vote)->name, 0, CANDIDATE_NAME_MAX_SIZE);
      appendStr((*vote)->name, ntok);

      (*vote)->count = 1;
      (*vote)->next = NULL;
    } //end if
    else{ 
      if (existCandNode(ntok, *vote) == 0){
        traverseCandLinkList(ntok, vote, 1);
      } //end inner if
      else{
        struct candNode *a = addrCandNode(ntok, *vote);
        a->count++;
      } //end inner else
    } //end else
  } //end while
  fclose(infile);
} // end parseInput 