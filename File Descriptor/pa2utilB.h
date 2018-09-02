#include "pa2utilA.h"

#define AGGREGATE_VOTES "Aggregate_Votes"
#define LEAF_COUNTER "Leaf_Counter"
#define SENTINEL_COUNT -9999
#define ARGC_SIZE 3

#ifndef PA_2_UTIL_B
#define PA_2_UTIL_B

/**Function : traverseLink
 * Arguments: 'ntoken' - the string that contains the name of the 
                  candidate
 *         'vote' - Pointer to linked list to hold all data
 *         'count' - Count to initilize the node to
 *
 * Output: -
 * 
 * About: traverseLink goes to the end of the linked list, and
 *    creates a new node and adds it to the list, and then updates 
 *    the data held within with ntoken.
 */
void traverseLink(char *ntoken, node_t *vote, int count){
  struct node *loc = vote;
  struct node *temp = (struct node*)malloc(sizeof(struct node));
  if (temp == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if

  temp->name = ntoken;
  temp->count = count;
  temp->next = NULL;

  while (loc != NULL){
    if (loc->next == NULL){
      loc->next = temp;
      return;
    } //end if
    else
      loc = loc->next;
  } //end while
} //end traverseLink


/**Function : parseInput
 * Arguments: 'path' - path of the file to be processed
 *        'vote' - Pointer to linked list to hold all data, which 
 *                 the first node's count is initilized to SENTINEL_COUNT,
 *                 and the next = NULL
 *
 * Output: - Initilizes vote with the contents of file (path)
 * 
 * Description - Checks if first node of vote is uninitialized by checking
 * if count == SENTINEL_COUNT. If so then it tokenizes first token of infile.
 * If not then it adds count of first token to the appropriate node.
 * Then it makes a char * array of tokens. After this it processes each token
 * into the linked list (vote).
 *
 * It also makes use of functions existNode, addrNode, traverseLink 
 */
void parseInput(char path[], node_t *vote){
  FILE *infile;   
  infile = fopen(path, "r");  

  fseek(infile, 0, SEEK_END);
  long fsize = ftell(infile);
  rewind(infile);

  char *buff = malloc(fsize + 1);
  if (buff == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if

  fread(buff, fsize, 1, infile);
  buff[fsize] = '\0';

  char *tokenArr[2048];
  int i = 0; // maximum number of tokens
  char *token = strtok(buff, ",");
  tokenArr[i++] = token;

  //fclose(infile);
  while (token != NULL){
    token = strtok(NULL, ",");
    tokenArr[i++] = token;
  } //end while
  // i - 1 because last token is null
  char * name = strtok(tokenArr[0], ":");
  int count = atoi(strtok(NULL, ":"));

  int exist;

  if(vote->count == SENTINEL_COUNT){//if null allocate a node
    vote->next = NULL;
    vote->name = name;
    vote->count = count;
  } else { // if the vote linked list already exists then
    exist = existNode(name, vote);
    if(exist == 1) { // if node already in program
      struct node *temp = addrNode(name, vote);
      temp->count = temp->count + count;
    } else {
      traverseLink(name, vote, count);
    }
  }

  for(int j = 1; j < i - 1; j++){
    name = strtok(tokenArr[j], ":");
    count = atoi(strtok(NULL, ":"));

    exist = existNode(name, vote);
    if(exist == 1) { // if node already in program
      struct node *temp = addrNode(name, vote);
      temp->count = temp->count + count;
    }
    else {
      traverseLink(name, vote, count);
    }
  }
//fclose(infile);
} // end parseInput

/**Function : getOutputFilePath
 * Arguments: 'dirName' - directory of which to get the input file to
 * search for. 
 * Ex. dirName is Who_Won/. It returns a ptr to Who_Won/Who_Won.txt
 *
 * Output: - ptr to dirName/inputFileName.txt as shown above
 * 
 * Description - Gets the rightmost folder from the directory first,
 * by removing the / and then tokenizing it. Then it takes it and appends
 * the subdirectory name.txt to the original dirName.
 * It also makes use of functions strConcat and subfolder 
 */
char * getOutputFilePath(char *dirName){
  char *outputFilePath = "\0";
  outputFilePath = strConcat(dirName, outputFilePath);
  if (outputFilePath[strlen(outputFilePath) - 1] == '/')
    outputFilePath[strlen(outputFilePath) - 1] = '\0';

  char *subfldr = subfolder(outputFilePath);
  char *slash = "/";
  char *extension = ".txt";
  outputFilePath = strConcat(outputFilePath, slash);
  outputFilePath = strConcat(outputFilePath, subfldr);
  outputFilePath = strConcat(outputFilePath, extension);

  return outputFilePath;
}

/**Function : execAggregateVotes
 * Arguments: 'path' - path of the directory LeafCounter should be
 *                     run in
 *
 * Output: - Execs AggregateVotes, does not return
 * 
 * Description - Uses execv with the parameters being: AggregateVotes, 
 * path, and null.
 */
void execAggregateVotes(char *path){
  char *arr[ARGC_SIZE];  //arr of arguments to execute
  int i = 0;      //counter

  arr[i++] = AGGREGATE_VOTES;
  arr[i++] = path;
  arr[i++] = (char *) NULL;

  if (execv(arr[0], arr) == -1) {
    perror("Error. execv failed. \n------- Terminating -------");
    exit(4);
  } //end if
}

/**Function : execLeafCounter
 * Arguments: 'path' - path of the directory LeafCounter should be
 *                     run in
 *
 * Output: - Execs LeafCounter, does not return
 * 
 * Description - Uses execv with the parameters being: LeafCounter, 
 * path, and null.
 */
void execLeafCounter(char *path){
    char *arr[ARGC_SIZE];  //arr of arguments to execute
    int i = 0;      //counter

    arr[i++] = LEAF_COUNTER;
    arr[i++] = path;
    arr[i++] = (char *) NULL;

    if(execv(arr[0], arr) == -1){
      perror("Error. execv failed. \n------- Terminating -------");
      exit(4);
    } //end if
}

/**Function : maxAddrNode
 * Arguments: 'name' - the string that contains the name of the 
                  candidate
 *         'vote' - Pointer to linked list to hold all data
 *
 * Output: returns a pointer to a specfic node in the linked list
 * 
 * About: addrNode is supposed to determine where in the linked list
 *    a certain node is, and return its address.
 */
struct node* maxAddrNode(node_t *vote){
  struct node *loc = vote;
  struct node *max = vote;
  
  int num = loc->count;

  while (loc != NULL){
    if (loc->count > num){
      num = loc->count;
      max = loc;
    }
    loc = loc->next;
  } // end while
  return max;
} // end addrNode

/**Function : addFinOut
 * Arguments: 'path' - holds the path for the final output file
 *         'name' - holds the name of the winner
 *
 * Output: -
 * 
 * About: opens the file in path, and adds the information
 *  about winner to it, and then closes the file.
 */
void addFinOut(char *path, char *name){
  FILE *outfile;   
  outfile = fopen(path, "a+");

  fprintf(outfile, "Winner:%s\n", name);
  fclose(outfile); 
} //end addFinOut

#endif