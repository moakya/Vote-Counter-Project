#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <pthread.h>

#ifndef SERVER_H
#define SERVER_H

#define CANDIDATE_NAME_MAX_SIZE 100
#define REGION_NAME_MAX_SIZE 15
#define MAX_CHILDREN 5
#define MAX_NUMBER_NODES 100 // no more than 100 regions allowed
#define NULL_INDEX -1 // if for example node does not have a parent, parent is set to this

// ---------- BEGIN ERROR CODES ---------- //
#define SUCCESS 0             // SC
#define UNHANDLED_ERROR 1     // UE
#define UNHANDLED_COMMAND 2   // UC
#define NO_REGION 3           // NR
#define REGION_OPEN 4         // RO
#define REGION_CLOSED 5       // RC
#define REGION_REOPENED 6     // RR
#define POLL_FAILURE 7        // PF
#define NON_LEAF 8            // NL
#define ILLEGAL_SUBTRACTION 9 // IS
#define NO_DATA 10            // CV if candList is empty
// ----------  END ERROR CODES  ---------- //

// struct that will be a linked list containing candidate name and vote info
typedef struct candNode{
  char name[CANDIDATE_NAME_MAX_SIZE]; // will need to change function to reflect static array
  int count;
  struct candNode *next;
}candnode_t;

// struct that will contain the DAG/tree structure
typedef struct node {
  char name[REGION_NAME_MAX_SIZE]; // region name

  int parent; // index of parent node
  int children[MAX_CHILDREN]; // max children possible is 5
  int numChildren;
// all nodes have point to 1 mutex lock for entire structure
  pthread_mutex_t* nodeMutex; 

  int open; // 0 means node is closed, 1 means node is open
  int closedFlag; // initially set to 0, 1 means it is closed and cannot be reopened

  struct candNode * candList;// function that initilaizes it should take in **
}node_t;

// struct that contains necessary info for thread function
typedef struct threadStruct{
  char *clientMessage;  // format: "cmd;region;data"
  int socket;           // socket to write to
  node_t *dag;          // node structure
  int *numNodes;         // number of nodes in tree structure
  struct sockaddr_in client; // needed for printf statements
}thrdstr_t;

/*************************************************************
  _    _        _ 
 | |  | |      | |                        
 | |__| |  ___ | | _ __    ___  _ __  ___ 
 |  __  | / _ \| || '_ \  / _ \| '__|/ __|
 | |  | ||  __/| || |_) ||  __/| |   \__ \
 |_|  |_| \___||_|| .__/  \___||_|   |___/
                  | |                     
                  |_|                     
/**************************************************************/
void mallocCheck(void *x){
  if (x == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(4);
  }
}

void lockCheck(int x){
  if (x != 0){
    perror("Error. Lock failed. \n------- Terminating -------");
    exit(5);
  }
}

void unlockCheck(int x){
  if (x != 0){
    perror("Error. Unlock failed. \n------- Terminating -------");
    exit(6);
  }
}

void openFileCheck(FILE *x){
  if (x == NULL){
    perror("Error. Open file failed. \n------- Terminating -------");
    exit(8);
  }  // end if
}

//Function defined by the 4061 staff
char *trimwhitespace(char *str){
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;

  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

/**************************************************************/
/**Function : appendStr
 * Arguments: 'str1' - The first string
 *            'str2' - The second string
 *     
 * Output: -
 *
 * appends str2 to the end of str1 based on 
 * strlen of str1. str1 must already have been allocated.
 */  
void appendStr(char *str1, char *str2){
  for(int i = strlen(str1), j = 0; j < strlen(str2); i++, j++)
    str1[i] = str2[j];
}

/**************************************************************/
/**Function : copyStrToBuffer
 * Arguments: 'buff' - buffer to have the str copied into, 
 *            function assumes null terminated.
 *            'str' - The string to copy into buff, must be
 *            null terminated
 *     
 * Output: - buff is now initilized to str
 *
 * appends str2 to the end of str1 based on 
 * strlen of str1. str1 must already have been allocated.
 */  
void copyStrToBuffer(char *buff, char * str){
  for(int i = 0; i < strlen(str); i++)
    buff[i] = str[i];
  buff[strlen(str)] = 0; // null terminate buff
}

/**************************************************************/
/**Function : existCandNode
 * Arguments: 'name' - the string that contains the candidate
                  to search for
 *         'list' - Pointer to linked list to hold all data
 *
 * Output: returns an int value, to represent whether, the 
 *    candidate already exists in the linked list
 * 
 * About: existNode is supposed to determine whether a node
 *    with the same candidate name exists in the linked list.
 */
int existCandNode(char *name, candnode_t *list){
  struct candNode *loc = list; 
  while (loc != NULL){
    if (strcmp(name, loc->name) == 0)
      return 1;
    loc = loc->next;
  } // end while

  return 0;
} // existNode

/**************************************************************/
/**Function : addrCandNode
 * Arguments: 'name' - the string that contains the name of the 
                  candidate to search for
 *         'list' - Pointer to linked list to hold all data
 *
 * Output: returns a pointer to a specfic node in the linked list
 * 
 * About: addrCandNode is supposed to determine where in the linked list
 *    a certain node is, and return its address.
 */
struct candNode* addrCandNode(char *name, candnode_t *list){
  struct candNode *loc = list;

  while (loc != NULL){
    if (strcmp(name, loc->name) == 0)
      return loc;
    loc = loc->next;
  } // end while
  return NULL;
} // end addrNode

/**************************************************************/
/**Function : dispCandList
 * Arguments: 'list' - Pointer to linked list to hold all data 
 *
 * Output: -
 *
 * About: displays the contents of the linked list into the console.
 */
void dispCandList(candnode_t *list){
  struct candNode *loc = list;
  while (loc != NULL){
    printf("Name:  %s\n", loc->name);
    printf("Count: %d\n", loc->count);
    loc = loc->next;
  } //end while
} //end disp

/**************************************************************/
/**Function : freeCandList
 * Arguments: 'list' - Pointer to linked list to hold all data 
 *
 * Output: -
 *
 * About: freeList is a destructor for the linked list, it 
 *    frees all the data that was malloced for the linked list.
 */
void freeCandList(candnode_t *list){
  struct candNode* prev = NULL;
  while(list != NULL){
    prev = list;
    list = list->next;
    free(prev);
  } //end while
} //end freeList

/**************************************************************/
/**Function : traverseCandLinkList
 * Arguments: 'ntoken' - The name of the new node to add
 *         'list' - The linked list we want to add a new node to
 *            'num' - The count of the candidate name
 * 
 * creates a new node in the list and adds the name from ntoken
 * and the count
 */
void traverseCandLinkList(char *ntoken, candnode_t **list, int num){
  struct candNode *loc = *list;  //ptr to beginning of list

  //create a new node
  struct candNode *temp = (struct candNode*)malloc(sizeof(struct candNode));
  mallocCheck(temp);
  //change the members of the node
  temp->count = 0;
  memset(temp->name, 0, CANDIDATE_NAME_MAX_SIZE);
  appendStr(temp->name, ntoken);
  //temp->name = ntoken;
  temp->count += num;
  temp->next = NULL;

  if(*list == NULL) // if list is empty it now points to temp
    *list = temp;
  else 
    while (loc != NULL){
      if (loc->next == NULL){ //if the node is the last in the list
        loc->next = temp; //reassign node ptr to the new node
        return;
      } //end if
      else
        loc = loc->next;  //increment the pointer
    } //end while
} //end traverseLink

/**************************************************************/
/**Function : mergeLists
 * Arguments: 'l1' - The first linked list we want to merge
 *             'l2' - The second linked list we want to merge
 * 
 * Output: -
 *
 * merges l1 and l2 into l2 without changing l1
 */
void mergeLists(candnode_t *l1, candnode_t **l2){
  struct candNode *curr = l1;

  while(curr != NULL){
    if(existCandNode(curr->name, *l2)){
      struct candNode *temp = addrCandNode(curr->name, *l2);
      temp->count = temp->count + curr->count;
    } 
    else {
      traverseCandLinkList(curr->name, l2, curr->count);
    }
    curr = curr->next;
  }
}

/**************************************************************/
/**Function : subtractLists
 * Arguments: 'l1' - The first linked list we want to merge
 *             'l2' - The second linked list we want to merge
 * 
 * Output: -
 *
 * subtractLists l1 from l2 meaning if a node l1 exists in l2
 * l2's node's count is substracted according to node l1's count.
 */
void subtractLists(candnode_t *l1, candnode_t *l2){
  struct candNode *curr = l1;

  while(curr != NULL){
    if(existCandNode(curr->name, l2)){
      struct candNode *temp = addrCandNode(curr->name, l2);
      temp->count = temp->count - curr->count;
    } 
    curr = curr->next;
  }
}

/**************************************************************/
/**Function : findMaxCandNode
 * Arguments: 'l1' - The linked list we want to find max node of
 * 
 * Output: - addr of max node with maximum count
 *
 * implements linear search to compare the counts. 
 */
struct candNode* findMaxCandNode(candnode_t *l1){
  struct candNode *curr = l1;
  struct candNode *maxCandNode = curr;// points to max node

  while(curr != NULL){
    if(curr->count > maxCandNode->count)
      maxCandNode = curr;

    curr = curr->next;
  }
  return maxCandNode;
}


/**Function : initializeNodeArray
 * Arguments: 'n' - array of nodes we are initializing mutex lock
 *
 * Output: 
 * 
 * About initializeNodeArray: Initializes every element in n to
 * proper default values: int indices are 0, ints are 0s,
 * buffers are zeroed, pointers are NULL, etc.
 */
void initializeNodeArray(struct node *n){
  for(int i = 0; i < MAX_NUMBER_NODES; i++){
    memset(n[i].name, 0, REGION_NAME_MAX_SIZE);
    n[i].parent = NULL_INDEX;
    n[i].numChildren = 0;

    n[i].closedFlag = 0;
    n[i].open = 0; // default to close, not sure if default should be open
    n[i].candList = NULL;
  }
}

/**Function : initializeNodeMutex
 * Arguments: 'n' - array of nodes we are initializing mutex lock
 *        'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About initializeNodeMutex: Initializes a 1 mutex lock for tree and sets
 * every element in n between indicies 0 - (numNodes - 1) to point to it.
 */
void initializeNodeMutex(struct node *n, int numNodes){
  if (numNodes > 0){ // only make a lock if there are nodes to point to it
    pthread_mutex_t* treeMutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    mallocCheck(treeMutex);

    pthread_mutex_init(treeMutex, NULL);

    for(int i = 0; i < numNodes; i++){
      n[i].nodeMutex = treeMutex;
    }
  }
}

/**Function : freeNodeMutex
 * Arguments: 'n' - array of nodes in we are destroying mutex lock
 *        'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About freeNodeMutex: Destroyes the mutex lock for the tree structure
 * by destroying the one any of them point to since there is 1
 * lock for the tree structure. 
 */
void freeNodeMutex(struct node *n, int numNodes){
  if(numNodes > 0){ // only delete lock if there is a lock to begin with
    int x = pthread_mutex_destroy(n[0].nodeMutex); // all nodes point to same lock anyway
    if (x != 0){
      perror("Error. Failed to destroy lock. \n------- Terminating -------");
      exit(9);
    }
  }
}

// Testing Purposes
void displayTree(struct node *n, int numNodes){
  for(int i = 0; i < numNodes; i++){
    printf("\nIndex: %d\n", i);
    printf("Name: %s\n", n[i].name);
    printf("parent id: %d\n", n[i].parent);
    printf("Num Children: %d\n", n[i].numChildren);
    printf("Closed Flag: %d\n", n[i].closedFlag);
    printf("Open(1) Close (0): %d\n", n[i].open);
    printf("Cand List: \n");
    dispCandList(n[i].candList);
  }
}

/**Function : findIndex
 * Arguments: 'nodename' - node name we are searching for
 *         'n' - array of nodes we are searching nodename in
 *        'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About findIndex: findIndex is supposed to
 * looks for specific index of name we are searching for in node array 
 * and returns -1 if node not found or index if found.
 */
int findIndex (char *nodeName, node_t *n, int numNodes){
  for (int j = 0; j < numNodes; j++)
    if (strcmp(nodeName, n[j].name)==0)
      return j; //match found

  return NULL_INDEX;    //If match not found
} //end findIndex

/**Function : findParentIndex
 * Arguments: 'childName' - child node name we are searching for the parent node of
 *         'n' - array of nodes we are searching nodename in
 *        'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About findParentIndex: It goes through the array n and goes through 
 * each nodes children and checks what node element the children indices 
 * correspond to and compares if the names match then returns index if found.
 * If not found it returns NULL_INDEX (-1).
 */
int findParentIndex(char *childName, node_t *n, int numNodes){
  // search through all relevant nodes
  for(int i = 0; i < numNodes; i++)
    for(int j = 0; j < n[i].numChildren; j++) // search through all children of all relavant nodes
      // if the children's name equals the name we are searching for, 
      //the node under which the children index lies in the parent Node, so return i
      if(strcmp(n[ n[i].children[j] ].name, childName) == 0)
        return i; 

  return NULL_INDEX; // if not found
}

/**Function : existNode
 * Arguments: 'nodename' - node name we are searching for
 *         'n' - array of nodes we are searching nodename in
 *        'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About existNode: Function sees if a node with the name
 * nodeName exists within n. If it does, it returns 1,
 * if not it returns 0.
 */
int existNode(char *nodeName, node_t *n, int numNodes){
  int existFlag = 0;
  for(int i = 0; i < numNodes && !existFlag; i++)
    if(strcmp(n[i].name, nodeName) == 0)
      existFlag = 1;

  return existFlag;
}

/**Function : findRootIndex
 * Arguments: 
 *         'n' - array of nodes we are searching nodename in
 *        'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About findRootIndex: Finds and returns index of the root
 * node by checking the parent is the NULL_INDEX.
 */
int findRootIndex(node_t *n, int numNodes){
  int rootIndex = NULL_INDEX;

  for(int i = 0; i < numNodes && rootIndex == NULL_INDEX; i++)
    if(n[i].parent == NULL_INDEX)
      rootIndex = i;

  return rootIndex;
}

/**Function : checkTreeClosed
 * Arguments:  'regionName' - node name to check closed, intially root node name
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n,
 *          'openNodeIndex' - index of node that is not closed, 
 *            must be set to -1 (NULL_INDEX) before calling function
 *
 * Output: 
 * 
 * About checkTreeClosed: Recursively checks tree to make sure
 * all nodes are closed. If node is not closed it sets
 */
void checkTreeClosed(char *regionName, node_t *n, int numNodes, int *openNodeIndex){
  int nodeIndex = findIndex(regionName, n, numNodes);
  if(!n[nodeIndex].closedFlag) // if not permanently closed then
    *openNodeIndex = nodeIndex; // set the index of open node

  if(*openNodeIndex == NULL_INDEX) // only proceed if no errors
    // call function recursively with names of all children nodes
    // so they are checked if they are open as well
    for(int i = 0; i < n[nodeIndex].numChildren; i++)
      checkTreeClosed(n[ n[nodeIndex].children[i] ].name, n, numNodes, openNodeIndex);
}

/**Function : createDAGStructure
 * Arguments: 
 *         'n' - array of nodes to be initialized
 *        'DAGPath' - path to file that has DAG structure
 *
 * Output: 
 * 
 * About createDAGStructure: Function opens the file, then
 * line by line tokenizes it, first token is parent node
 * then rest are children. 
 * It initializes the node name, numChildren, children array, and
 * parent index.
 */
int createDAGStructure(node_t *n, char *DAGPath) {
    FILE *infile;   
  infile = fopen(DAGPath, "r");  
  openFileCheck(infile);

  //get size of file
  fseek(infile, 0, SEEK_END);
  long fsize = ftell(infile);
  rewind(infile); //go back to beginning of file

  //to hold all the file data
  char *buff = malloc(fsize + 1);
  mallocCheck(buff);

  int numNodes = 0; // number of nodes n will contain after processing file
                    // and creating DAG structure
  int extraCredit = 0; // whether DAG file is extra credit case
  // meaning it is just "Who_Won"

  while (fgets(buff, fsize, infile)){
    char *token = strtok(buff, ":"); // get the parent node name

    int parentIndex; // holds current parentIndex of line in DAG file
    if(numNodes != 0) // if not on root node find the parentIndex of current node
      parentIndex = findIndex(token, n, numNodes); // index of parent node for children
    else // if root node we know the parent index is 0, start of n
      parentIndex = 0;

    if(!existNode(token, n, numNodes)){ // only add token into node structure if it doesn't exist
      copyStrToBuffer(n[numNodes].name, token); // initialize parent node name

      numNodes++; // ready for children nodes
    }

    int numChildren = 0;

    token = strtok(NULL, ":");

    if(token != NULL)// only trimwhitespace if token is not null
      token = trimwhitespace(token);

    // if only 1 node name then it is extra credit case, even if it isn't,
    // it'll skip the rest of the children processing which it should
    if(token == NULL) 
      extraCredit = true;

    if(!extraCredit){ // only proceed if it isn't the extra credit case
      while(token != NULL){
        n[parentIndex].children[numChildren++] = numNodes; // initialize children element within children to next child node
        copyStrToBuffer(n[numNodes].name, token); // copy token into child node
        n[numNodes++].parent = parentIndex; // increment numNodes and initialize parentIndex, only rootNode won't be initialized

        token = strtok(NULL, ":");
        if(token != NULL){ // only trimwhitespace if token is not null
          token = trimwhitespace(token);
        }
      }
      n[parentIndex].numChildren = numChildren; // set number of children when completed
    }
  }
  
  fclose(infile); //close the input file
  free(buff); //deallocate buffer

  return numNodes;
}// end createDAGStructure

/**Function : initializeCandList
 * Arguments: 
 *         'str' - format: "candName:count,candName:count,...\0"
 *        'candList' - address of ptr to candList to be initialized
 *                    based upon the str
 *
 * Output: 
 * 
 * About initializeCandList: Function tokenizes string based on "," to 
 * get info about each node then it gets the name and count and puts
 * them into their own buffer and then checks if a node by the candName
 * already exists. If so it gets the node and adds the count to the node's
 * count. If not, it adds the node to the candList.
 */
void initializeCandList(char *str, candnode_t **candList){
  char * strCopy = (char *) malloc(sizeof(char) * strlen(str) + 1);
  mallocCheck(strCopy);

  memset(strCopy, 0, strlen(str) + 1);
  copyStrToBuffer(strCopy, str);

  char *nodeStr = strtok(strCopy, ","); // nodeStr has "candName:count"

  while(nodeStr != NULL){
    char candName[CANDIDATE_NAME_MAX_SIZE]; // holds name of candidate
    memset(candName, 0, CANDIDATE_NAME_MAX_SIZE);

    char countStr[10]; // holds number of votes candName has
    memset(countStr, 0, 10);

    // first copy candName from nodeStr to candName buff
    int i;
    for(i = 0; nodeStr[i] != ':'; i++)
      candName[i] = nodeStr[i];

    // nodeStr[i] points to ':', want it to start to point to count so increment
    // copy count into count buffer
    int j; // for loop errors out if putting in in the for loop itself
    for(++i, j = 0; i < strlen(nodeStr); i++, j++)
      countStr[j] = nodeStr[i];

    int count = atoi(countStr); // convert string count to int

    if(existCandNode(candName, *candList)){ // if already exists need to add votes to it
      struct candNode* node = addrCandNode(candName, *candList); // get node
      node->count += count; // add count to it
    }// end if
    else
      traverseCandLinkList(candName, candList, count); // add node to linked list

    nodeStr = strtok(NULL, ","); // retokenize to get next nodeStr
  }// end while

  free(strCopy);
}// end initializeCandList()

int listSize(candnode_t *votes){
  struct candNode *curr = votes;

  int size = 0;
  while(curr != NULL){
    size++;
    curr = curr->next;
  }

  return size;
}

char *getDataString(candnode_t *votes){
  int size = listSize(votes);

  int mem = (size * 100) + (size * 5) + size + size;

  char *temp = (char *)malloc(mem * sizeof(char));
  mallocCheck(temp);

  memset(temp, 0, mem);

  struct candNode *n = votes;

  while (n != NULL){
    appendStr(temp, n->name);
    appendStr(temp, ":");
    char *x = (char*)malloc(5);
    mallocCheck(x);

    memset(x, 0, 5);
    sprintf(x, "%d", n->count);
    appendStr(temp, x);
    if (n->next != NULL)
      appendStr(temp, ",");
    n = n->next;
    free(x);
  }

  return temp;
}

/**Function : aggregateNodes
 * Arguments: 'regionName' - node name to start aggregating going
 *                        down the tree structure
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About aggregateNodes: This is a recursive function. The steps this
 * function takes in general is first destroy the current candList of the 
 * node if not a leaf node, then recursively call itself, then merge
 * all of its children's candLists into itself. 
 */
void aggregateNodes(char * nodeName, node_t *n, int numNodes){
  int nodeIndex = findIndex(nodeName, n, numNodes);

  if(n[nodeIndex].numChildren > 0){ // no need to delete and merge for leaf nodes
    freeCandList(n[nodeIndex].candList); // destroy aggregator node's cand list
    n[nodeIndex].candList = NULL;

    // call function recursively with names of all children nodes
    for(int i = 0; i < n[nodeIndex].numChildren; i++)
      aggregateNodes(n[ n[nodeIndex].children[i] ].name, n, numNodes);

    // now that function has been called for all children, we need to aggregate
    // assuming all children nodes have already been aggregated (leaf nodes are
    // by default aggregated)
    for(int i = 0; i < n[nodeIndex].numChildren; i++)
      mergeLists(n[ n[nodeIndex].children[i] ].candList, &(n[nodeIndex].candList));
  }
}

// --- BEGINNING SERVER RESPONSE COMMAND HELPER FUNCTION --- //
/*
The overall design of these function is that for every command
such as Count_Votes, there are 3 functions. 
Here is a specific example in the case of Count_Votes (CV) cmd:

countVotesErrorCheck: checks if there will be any errors in counting
the votes and sets status flags and data char *'s accordingly.

countVotesAction: carries out actual action of countingVotes 
without any error checking.

countVotes: first call countVotesErrorCheck and checks status flags.
If no errors then it calls countVotes. Then it returns a string
containing the server's reponse string which will also have the right
2 character response depending on if it was a success or errored.
Also appends data as needed.
*/

// ------------------ Count_Votes (CV) --------------------- // 

/**Function : countVotesErrorCheck
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *          'status' - int to determine errors
 *          'data' - holds relavant information if there is an error 
 *
 * Output: 
 * 
 * About countVotesErrorCheck: Must have been aggregated if possible.
 * There are 2 possible errors possible: in running Count_Votes cmd: 
 * regionName does not exist, or that
 * there is no data. 
 */
void countVotesErrorCheck(char *regionName, node_t *n, int numNodes, 
        int *status, char **data){

  int nodeIndex = findIndex(regionName, n, numNodes);

  // if nodeIndex is invalid set status and data accordingly
  if(nodeIndex == NULL_INDEX){
    *status = NO_REGION;
    *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(regionName) + 1);
    copyStrToBuffer(*data, regionName);
  } else if(n[nodeIndex].candList == NULL) { // other condition is there is no data
    *status = NO_DATA;
    char *noVotes = "No votes.";// msg data should have if region has no votes

    *data = (char *) malloc(sizeof(char) * strlen(noVotes) + 1);
    memset(*data, 0, strlen(noVotes) + 1);
    mallocCheck(*data);

    copyStrToBuffer(*data, noVotes);
  }
}

/**Function : countVotesAction
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *          'data' - holds vote information in format: "candList:count,.."
 *
 * Output: 
 * 
 * About countVotesAction: Must have been aggregated and does not error check.
 * data is set to a string in format: "candList:count,..". Uses helper function
 * getDataString to create this string.
 */
void countVotesAction(char *regionName, node_t *n, int numNodes, char **data){
  int nodeIndex = findIndex(regionName, n, numNodes);

  *data = getDataString(n[nodeIndex].candList);
}

/**Function : countVotes
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About countVotes:First tries to aggregate, then calls error checking 
 * function. If no errors proceeds with countVotes. Then constructs
 * server response string with the cmd depending on the error and the data.
 */
char * countVotes(char *regionName, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);

  // if possible aggregate so that there are no false "No votes."
  // because it was not aggregated before error checking
  if(nodeIndex != NULL_INDEX)
    aggregateNodes(regionName, n, numNodes);

  char *data = "\0";// will hold data portion of server response string
  int status = SUCCESS; // assume there are no errors

  countVotesErrorCheck(regionName, n, numNodes, &status, &data); // verify no errors

  if(status == SUCCESS) // if no errors get the vote data
    countVotesAction(regionName, n, numNodes, &data);

  // At this point data is set either with error data or correct data or
  // even if it isn't we can append data since by default it is a null character
  int size = 3 + strlen(data) + 1; // 3 for 2 char cmd and ;, and 1 for null
  char *responseStr = (char *) malloc(sizeof(char) * size);// will contain server's response
  mallocCheck(responseStr);

  memset(responseStr, 0, size);

  switch(status) {
    case SUCCESS:

    case NO_DATA: // No voting data is considered a success
      copyStrToBuffer(responseStr, "SC;");
      break;

    case NO_REGION: 
      copyStrToBuffer(responseStr, "NR;");
      break;

    default : // should not reach this, if so it is an unhandled error
      if(strlen(data) > 0) // if needed free data
        free(data);

      copyStrToBuffer(responseStr, "UE;");
      return responseStr;
  }

  appendStr(responseStr, data); // append the data to complete the str

  if(strlen(data) > 0) // if needed free data
    free(data);

  return responseStr;
}

// ------------------ Open_Polls (OP) --------------------- // 

/**Function : openPollsErrorCheck
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *          'status' - int to determine errors
 *          'data' - holds relavant information if there is an error 
 *
 * Output: 
 * 
 * About openPollsErrorCheck: 
 * There are 2 possible errors possible: in running Open_Polls cmd: 
 * regionName does not exist, or that
 * poll is already open (PF). 
 */
void openPollsErrorCheck(char *regionName, node_t *n, int numNodes, 
        int *status, char **data){

  int nodeIndex = findIndex(regionName, n, numNodes);

  if(nodeIndex != NULL_INDEX){
    // if open or cannot open it then set status and data accordingly
    if(n[nodeIndex].open || n[nodeIndex].closedFlag){ 
      *status = POLL_FAILURE;

      char *appendPollFailureString;// to append after region name

      if (n[nodeIndex].open) // if already open append open after region name
        appendPollFailureString = " open.";
      else // if permanently closed append closed after region name
        appendPollFailureString = " closed.";

      *data = (char *) malloc(sizeof(char) * strlen(regionName) + 
            strlen(appendPollFailureString) + 1);
      mallocCheck(*data);

      memset(*data, 0, strlen(regionName) + strlen(appendPollFailureString) + 1);
      copyStrToBuffer(*data, regionName); // append regionName
      appendStr(*data, appendPollFailureString); // append " open." after regionName 
    }// end inner if
  } else { // if nodeIndex is NULL_INDEX, and status/data accordingly
    *status = NO_REGION;

    *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(regionName) + 1);
    copyStrToBuffer(*data, regionName);
  }// end outer if-else
}

/**Function : openPollsAction
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About openPollsAction: Assumes all polls are closed in the region
 * and all its children. Opens all nodes by chaning open member to 1
 * recursively.
 */
void openPollsAction(char *regionName, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);
  if(!n[nodeIndex ].closedFlag) // if not permanently closed then
    n[nodeIndex].open = 1; // open parent node

  // call function recursively with names of all children nodes
  // so they are opened as well
  for(int i = 0; i < n[nodeIndex].numChildren; i++)
    openPollsAction(n[ n[nodeIndex].children[i] ].name, n, numNodes);
}

/**Function : openPolls
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *         'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About openPolls: First calls error checking function. If no errors
 * proceeds with openPollsAction. Then constructs server response string 
 * with the cmd depending on the error and the data.
 */
char * openPolls(char *regionName, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);

  char *data = "\0";// will hold data portion of server response string
  int status = SUCCESS; // assume there are no errors

  openPollsErrorCheck(regionName, n, numNodes, &status, &data); // verify no errors

  if(status == SUCCESS) // if no errors then open the polls
    openPollsAction(regionName, n, numNodes);

  // At this point data is set either with error data or correct data or
  // even if it isn't we can append data since by default it is a null character
  int size = 3 + strlen(data) + 1; // 3 for 2 char cmd and ;, and 1 for null
  char *responseStr = (char *) malloc(sizeof(char) * size);// will contain server's response
  mallocCheck(responseStr);

  memset(responseStr, 0, size);

  switch(status) {
    case SUCCESS:
      copyStrToBuffer(responseStr, "SC;");
      break;

    case NO_REGION: 
      copyStrToBuffer(responseStr, "NR;");
      break;

    case POLL_FAILURE: 
      copyStrToBuffer(responseStr, "PF;");
      break;    

    default : // should not reach this, if so it is an unhandled error
      if(strlen(data) > 0) // if needed free data
        free(data);

      copyStrToBuffer(responseStr, "UE;");
      return responseStr;
  }

  appendStr(responseStr, data); // append the data to complete the str

  if(strlen(data) > 0) // if needed free data
    free(data);

  return responseStr;
}

// ------------------ Close_Polls (CP) --------------------- // 

/**Function : closePollsErrorCheck
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *          'status' - int to determine errors
 *          'data' - holds relavant information if there is an error 
 *
 * Output: 
 * 
 * About closePollsErrorCheck: 
 * There are 2 possible errors possible: in running Close_Polls cmd: 
 * regionName does not exist, or that
 * poll is already closed (PF). 
 */
void closePollsErrorCheck(char *regionName, node_t *n, int numNodes, 
        int *status, char **data){

  int nodeIndex = findIndex(regionName, n, numNodes);

  if(nodeIndex != NULL_INDEX){
    // if already closed set status and data accordingly
    if(n[nodeIndex].open == 0){ 
      *status = POLL_FAILURE;

      char *appendPollFailureString = " closed."; // to append after region name

      // if not open and never been closed (initial state)
      if(n[nodeIndex].closedFlag == 0)
        appendPollFailureString = " initial.";

      *data = (char *) malloc(sizeof(char) * strlen(regionName) + 
            strlen(appendPollFailureString) + 1);
      memset(*data, 0, strlen(regionName) + strlen(appendPollFailureString) + 1);
      copyStrToBuffer(*data, regionName); // append regionName
      appendStr(*data, appendPollFailureString); // append " closed." after regionName 
    }// end inner if
  } else { // if nodeIndex is NULL_INDEX, and status/data accordingly
    *status = NO_REGION;

    *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(regionName) + 1);
    copyStrToBuffer(*data, regionName);
  }// end outer if-else
}

/**Function : closePollsAction
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About closePollsAction: Assumes all polls are closed in the region
 * and all its children. Closes all nodes by chaning close member to 1
 * recursively.
 */
void closePollsAction(char *regionName, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);

  if(n[nodeIndex].open)// only permenantly close if already open
      n[nodeIndex].closedFlag = 1;// flag represents when a node has been permanently

  n[nodeIndex].open = 0; // close parent node
  // closed. Initially nodes are unopened, then are opened, then are closed 
  // represented by closedFlag = 1

  // call function recursively with names of all children nodes
  // so they are closeed as well
  for(int i = 0; i < n[nodeIndex].numChildren; i++)
    closePollsAction(n[ n[nodeIndex].children[i] ].name, n, numNodes);
}

/**Function : closePolls
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *         'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About closePolls: First calls error checking function. If no errors
 * proceeds with closePollsAction. Then constructs server response string 
 * with the cmd depending on the error and the data.
 */
char * closePolls(char *regionName, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);

  char *data = "\0";// will hold data portion of server response string
  int status = SUCCESS; // assume there are no errors

  closePollsErrorCheck(regionName, n, numNodes, &status, &data); // verify no errors

  if(status == SUCCESS) // if no errors then close the polls
    closePollsAction(regionName, n, numNodes);

  // At this point data is set either with error data or correct data or
  // even if it isn't we can append data since by default it is a null character
  int size = 3 + strlen(data) + 1; // 3 for 2 char cmd and ;, and 1 for null
  char *responseStr = (char *) malloc(sizeof(char) * size);// will contain server's response
  mallocCheck(responseStr);

  memset(responseStr, 0, size);

  switch(status) {
    case SUCCESS:
      copyStrToBuffer(responseStr, "SC;");
      break;

    case NO_REGION: 
      copyStrToBuffer(responseStr, "NR;");
      break;

    case POLL_FAILURE: 
      copyStrToBuffer(responseStr, "PF;");
      break;    

    default : // should not reach this, if so it is an unhandled error
      if(strlen(data) > 0) // if needed free data
        free(data);

      copyStrToBuffer(responseStr, "UE;");
      return responseStr;
  }

  appendStr(responseStr, data); // append the data to complete the str

  if(strlen(data) > 0) // if needed free data
    free(data);

  return responseStr;
}


// ------------------ Add_Votes (AV) --------------------- // 

/**Function : addVotesErrorCheck
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *          'status' - int to determine errors
 *          'data' - holds relavant information if there is an error 
 *
 * Output: 
 * 
 * About addVotesErrorCheck: 
 * There are 3 possible errors possible: in running Add_Votes cmd: 
 * regionName does not exist(NR), regionName is not a leafnode(NL), or
 * poll is closed (RC). 
 */
void addVotesErrorCheck(char *regionName, node_t *n, int numNodes, 
        int *status, char **data){

  int nodeIndex = findIndex(regionName, n, numNodes);

  if(nodeIndex != NULL_INDEX){
    // if already closed set status and data accordingly
    if(n[nodeIndex].open == 0){ 
      *status = REGION_CLOSED;

      *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
      mallocCheck(*data);

      memset(*data, 0, strlen(regionName) + 1);
      copyStrToBuffer(*data, regionName);
    } else if(n[nodeIndex].numChildren != 0){ // if it isn't a leaf node set status and data
      *status = NON_LEAF;

      *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
      mallocCheck(*data);

      memset(*data, 0, strlen(regionName) + 1);
      copyStrToBuffer(*data, regionName);
    } 
  } else { // if nodeIndex is NULL_INDEX, and status/data accordingly
    *status = NO_REGION;

    *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(regionName) + 1);
    copyStrToBuffer(*data, regionName);
  }// end outer if-else
}

/**Function : addVotesAction
 * Arguments: 'regionName' - node name to check count votes cmd
 *          'candListStr' - format: "candName:count,..."
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About addVotesAction: Makes a candList from the passed in
 * string and then merges that list with the region's node.
 * Assumes regionName exists. 
 */
void addVotesAction(char *regionName, char * candListStr, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);
  struct candNode * addVotesCandList = NULL; // contains candList specified by candList

  initializeCandList(candListStr, &addVotesCandList); // get list to add to node's candList
  mergeLists(addVotesCandList, &(n[nodeIndex].candList));// change the linked list in node

  freeCandList(addVotesCandList); // free list that is no longer needed
}

/**Function : addVotes
 * Arguments: 'regionName' - node name to check count votes cmd
*          'candListStr' - format: "candName:count,..."
 *         'n' - tree structure
 *         'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About addVotes: First calls error checking function. If no errors
 * proceeds with addVotesAction. Then constructs server response string 
 * with the cmd depending on the error and the data.
 */
char * addVotes(char *regionName, char * candListStr, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);

  char *data = "\0";// will hold data portion of server response string
  int status = SUCCESS; // assume there are no errors

  addVotesErrorCheck(regionName, n, numNodes, &status, &data); // verify no errors

  if(status == SUCCESS) // if no errors then add the votes
    addVotesAction(regionName, candListStr, n, numNodes);

  // At this point data is set either with error data or correct data or
  // even if it isn't we can append data since by default it is a null character
  int size = 3 + strlen(data) + 1; // 3 for 2 char cmd and ;, and 1 for null
  char *responseStr = (char *) malloc(sizeof(char) * size);// will contain server's response
  mallocCheck(responseStr);

  memset(responseStr, 0, size);

  switch(status) {
    case SUCCESS:
      copyStrToBuffer(responseStr, "SC;");
      break;

    case NO_REGION: 
      copyStrToBuffer(responseStr, "NR;");
      break;

    case REGION_CLOSED: 
      copyStrToBuffer(responseStr, "RC;");
      break;

    case NON_LEAF:
      copyStrToBuffer(responseStr, "NL;");
      break;    

    default : // should not reach this, if so it is an unhandled error
      if(strlen(data) > 0) // if needed free data
        free(data);

      copyStrToBuffer(responseStr, "UE;");
      return responseStr;
  }

  appendStr(responseStr, data); // append the data to complete the str

  if(strlen(data) > 0) // if needed free data
    free(data);

  return responseStr;
}

// ------------------ Remove_Votes (RV) --------------------- // 

/**Function : removeVotesErrorCheck
 * Arguments: 'regionName' - node name to check count votes cmd
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *          'status' - int to determine errors
 *          'data' - holds relavant information if there is an error 
 *
 * Output: 
 * 
 * About removeVotesErrorCheck: 
 * There are 4 possible errors possible: in running Add_Votes cmd: 
 * regionName does not exist(NR), regionName is not a leafnode(NL), 
 * poll is closed (RC), or illegal substraction (IS).
 */
void removeVotesErrorCheck(char *regionName, char * candListStr, node_t *n, 
        int numNodes, int *status, char **data){

  int nodeIndex = findIndex(regionName, n, numNodes);

  if(nodeIndex != NULL_INDEX){
    // if already closed set status and data accordingly
    if(n[nodeIndex].open == 0){ 
      *status = REGION_CLOSED;

      *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
      mallocCheck(*data);

      memset(*data, 0, strlen(regionName) + 1);
      copyStrToBuffer(*data, regionName);
    } else if(n[nodeIndex].numChildren != 0){ // if it isn't a leaf node set status and data
      *status = NON_LEAF;

      *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
      mallocCheck(*data);

      memset(*data, 0, strlen(regionName) + 1);
      copyStrToBuffer(*data, regionName);
    } else { // check for any illegal subtractions
      struct candNode *candList = NULL;// list to check for illegal substractions
      initializeCandList(candListStr, &candList);

      // if any problem candidates will contain "candName,candName,..."
      char * illegalSubstractionCandidates = (char *) malloc(sizeof(char) * 256);
      mallocCheck(illegalSubstractionCandidates);

      memset(illegalSubstractionCandidates, 0, 256);

      struct candNode *curr = candList;
      // continue processing candList until fully processed or
      // we have found an error
      while(curr != NULL){
        // if the candidate to substract doesn't exist or cannot substract
        // the amount then set status and data accordingly
        if(existCandNode(curr->name, n[nodeIndex].candList) == 0){
          *status = ILLEGAL_SUBTRACTION;

          appendStr(illegalSubstractionCandidates, curr->name);
          appendStr(illegalSubstractionCandidates, ",");// append candName,
        } else if ( curr->count > // we know node exists so see if substraction becomes < 0
                addrCandNode(curr->name, n[nodeIndex].candList)->count ){ 
          *status = ILLEGAL_SUBTRACTION;

          appendStr(illegalSubstractionCandidates, curr->name);
          appendStr(illegalSubstractionCandidates, ",");// append candName,
        }
        curr = curr->next;
      }// end while

      if(*status == ILLEGAL_SUBTRACTION){ // if there was an error copy string over to data
        *data = (char *) malloc(sizeof(char) * strlen(illegalSubstractionCandidates) + 1);
        mallocCheck(*data);
        memset(*data, 0, strlen(illegalSubstractionCandidates) + 1);

        char *token = strtok(illegalSubstractionCandidates, ",");

        while(token != NULL){
          appendStr(*data, token);

          token = strtok(NULL, ",");

          if(token != NULL)
            appendStr(*data, ",");
        }
      }

      free(illegalSubstractionCandidates);
      freeCandList(candList);
    }// end inner if-else
  } else { // if nodeIndex is NULL_INDEX, and status/data accordingly
    *status = NO_REGION;

    *data = (char *) malloc(sizeof(char) * strlen(regionName) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(regionName) + 1);
    copyStrToBuffer(*data, regionName);
  }// end outer if-else
}

/**Function : removeVotesAction
 * Arguments: 'regionName' - node name to check count votes cmd
 *          'candListStr' - format: "candName:count,..."
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About removeVotesAction: Makes a candList from the passed in
 * string and then merges that list with the region's node.
 * Assumes all substractions exists.
 */
void removeVotesAction(char *regionName, char * candListStr, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);
  struct candNode * removeVotesCandList = NULL; // contains candList specified by candList

  initializeCandList(candListStr, &removeVotesCandList); // get list to remove to node's candList
  subtractLists(removeVotesCandList, n[nodeIndex].candList);// do the substraction

  freeCandList(removeVotesCandList);
}

/**Function : removeVotes
 * Arguments: 'regionName' - node name to check count votes cmd
*          'candListStr' - format: "candName:count,..."
 *         'n' - tree structure
 *         'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About removeVotes: First calls error checking function. If no errors
 * proceeds with removeVotesAction. Then constructs server response string 
 * with the cmd depending on the error and the data.
 */
char * removeVotes(char *regionName, char * candListStr, node_t *n, int numNodes){
  int nodeIndex = findIndex(regionName, n, numNodes);

  char *data = "\0";// will hold data portion of server response string
  int status = SUCCESS; // assume there are no errors

  removeVotesErrorCheck(regionName, candListStr, n, numNodes, &status, &data); // verify no errors

  if(status == SUCCESS) // if no errors then remove the votes
    removeVotesAction(regionName, candListStr, n, numNodes);

  // At this point data is set either with error data or correct data or
  // even if it isn't we can append data since by default it is a null character
  int size = 3 + strlen(data) + 1; // 3 for 2 char cmd and ;, and 1 for null
  char *responseStr = (char *) malloc(sizeof(char) * size);// will contain server's response
  mallocCheck(responseStr);
  memset(responseStr, 0, size);

  switch(status) {
    case SUCCESS:
      copyStrToBuffer(responseStr, "SC;");
      break;

    case NO_REGION: 
      copyStrToBuffer(responseStr, "NR;");
      break;

    case REGION_CLOSED: 
      copyStrToBuffer(responseStr, "RC;");
      break;

    case NON_LEAF:
      copyStrToBuffer(responseStr, "NL;");
      break;    

    case ILLEGAL_SUBTRACTION:
      copyStrToBuffer(responseStr, "IS;");
      break;

    default : // should not reach this, if so it is an unhandled error
      if(strlen(data) > 0) // if needed free data
        free(data);

      copyStrToBuffer(responseStr, "UE;");
      return responseStr;
  }

  appendStr(responseStr, data); // append the data to complete the str

  if(strlen(data) > 0) // if needed free data
    free(data);

  return responseStr;
}


// ------------------ Return_Winner (RW) --------------------- // 

/**Function : returnWinnerErrorCheck
 * Arguments:
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *          'status' - int to determine errors
 *          'data' - holds relavant information if there is an error 
 *
 * Output: 
 * 
 * About returnWinnerErrorCheck: 
 * There are 1 possible errors possible: in running Return_Winner cmd: 
 * Region is not closed (RO).
 */
void returnWinnerErrorCheck(node_t *n, int numNodes, int *status, char **data){

  int rootNodeIndex = findRootIndex(n, numNodes);
  // will contain a valid node index if a node is not closed
  int openNodeIndex = NULL_INDEX; // index of openNode
  checkTreeClosed(n[rootNodeIndex].name, n, numNodes, &openNodeIndex);

  if(openNodeIndex != NULL_INDEX){ // if there is an error then set status and data
    *status = REGION_OPEN;

    *data = (char *) malloc(sizeof(char) * strlen(n[openNodeIndex].name) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(n[openNodeIndex].name) + 1);
    copyStrToBuffer(*data, n[openNodeIndex].name);
  }
}

/**Function : returnWinnerAction
 * Arguments: 
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About returnWinnerAction: Makes a candList from the passed in
 * string and then merges that list with the region's node.
 * Assumes all substractions exists.
 */
void returnWinnerAction(node_t *n, int numNodes, char **data){
  int rootNodeIndex = findRootIndex(n, numNodes);

  aggregateNodes(n[rootNodeIndex].name, n, numNodes);// first aggregate

  if(n[rootNodeIndex].candList != NULL){
    struct candNode *winnerNode = findMaxCandNode(n[rootNodeIndex].candList);
    char *winnerStr = "Winner:";

    *data = (char *) malloc(sizeof(char) * strlen(winnerNode->name) +
        strlen(winnerStr) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(winnerNode->name) + strlen(winnerStr) + 1);

    copyStrToBuffer(*data, winnerStr);

    appendStr(*data, winnerNode->name);
  } else { // if candList is uninitialized then data should be "no votes."
    char *noVotesStr = "No votes.";

    *data = (char *) malloc(sizeof(char) * strlen(noVotesStr) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(noVotesStr) + 1);
    copyStrToBuffer(*data, noVotesStr);
  }

}

/**Function : returnWinner
 * Arguments: 
 *         'n' - tree structure
 *         'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About returnWinner: First calls error checking function. If no errors
 * proceeds with returnWinnerAction. Then constructs server response string 
 * with the cmd depending on the error and the data.
 */
char * returnWinner(node_t *n, int numNodes){

  char *data = "\0";// will hold data portion of server response string
  int status = SUCCESS; // assume there are no errors

  returnWinnerErrorCheck(n, numNodes, &status, &data); // verify no errors

  if(status == SUCCESS) // if no errors then remove the votes
    returnWinnerAction(n, numNodes, &data);
  
  // At this point data is set either with error data or correct data or
  // even if it isn't we can append data since by default it is a null character
  int size = 3 + strlen(data) + 1; // 3 for 2 char cmd and ;, and 1 for null
  char *responseStr = (char *) malloc(sizeof(char) * size);// will contain server's response
  mallocCheck(responseStr);

  memset(responseStr, 0, size);

  switch(status) {
    case SUCCESS:
      copyStrToBuffer(responseStr, "SC;");
      break;

    case REGION_OPEN: 
      copyStrToBuffer(responseStr, "RO;");
      break;

    default : // should not reach this, if so it is an unhandled error
      if(strlen(data) > 0) // if needed free data
        free(data);

      copyStrToBuffer(responseStr, "UE;");
      return responseStr;
  }

  appendStr(responseStr, data); // append the data to complete the str

  if(strlen(data) > 0) // if needed free data
    free(data);

  return responseStr;
}

// ------------------ Add_Region (AR) --------------------- // 

/**Function : addRegionErrorCheck
 * Arguments: 'parentRegion' - parent region to add child region to
 *            'childRegion' - child region to add under parent region
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *          'status' - int to determine errors
 *          'data' - holds relavant information if there is an error 
 *
 * Output: 
 * 
 * About removeVotesErrorCheck: 
 * There are 1 possible error: in running Add_Regions cmd: 
 * regionName does not exist(NR).
 */
void addRegionErrorCheck(char *parentRegion, char *childRegion, node_t *n, 
        int numNodes, int *status, char **data){

  int nodeIndex = findIndex(parentRegion, n, numNodes);

  // if nodeIndex is NULL_INDEX, and status/data accordingly
  if(nodeIndex == NULL_INDEX){
    *status = NO_REGION;

    *data = (char *) malloc(sizeof(char) * strlen(parentRegion) + 1);
    mallocCheck(*data);

    memset(*data, 0, strlen(parentRegion) + 1);
    copyStrToBuffer(*data, parentRegion);
  }// end if
}

/**Function : addRegionAction
 * Arguments: 'regionName' - node name to check count votes cmd
 *          'candListStr' - format: "candName:count,..."
 *         'n' - tree structure
 *          'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About removeVotesAction: Makes a candList from the passed in
 * string and then merges that list with the region's node.
 * Assumes all substractions exists.
 */
void addRegionAction(char *parentRegion, char *childRegion, node_t *n, 
        int *numNodes, char **data){
  char parentRegionCleansed[REGION_NAME_MAX_SIZE];
  memset(parentRegionCleansed, 0, REGION_NAME_MAX_SIZE);

  for(int i = 0; i < REGION_NAME_MAX_SIZE; i++)
    if(parentRegion[i] != ' ')
      parentRegionCleansed[i] = parentRegion[i];


  int parentIndex = findIndex(parentRegionCleansed, n, *numNodes);

  // need to add childRegion to n, set up parent for childRegion,name
  // parentRegion needs childRegion as a child
  copyStrToBuffer(n[*numNodes].name, childRegion);
  n[*numNodes].parent = parentIndex;

  n[parentIndex].children[ n[parentIndex].numChildren++ ] = *numNodes;

  (*numNodes)++; // added new node

  char *regionAddedMsg = "Region added successfully.";

  *data = (char *) malloc(sizeof(char) * strlen(regionAddedMsg) + 1);
  mallocCheck(*data);

  memset(*data, 0, strlen(regionAddedMsg) + 1);
  copyStrToBuffer(*data, regionAddedMsg);
}

/**Function : addRegion
 * Arguments: 'regionName' - node name to check count votes cmd
*          'candListStr' - format: "candName:count,..."
 *         'n' - tree structure
 *         'numNodes' - number of nodes in n
 *
 * Output: 
 * 
 * About addRegion: First calls error checking function. If no errors
 * proceeds with addRegionAction. Then constructs server response string 
 * with the cmd depending on the error and the data.
 */
char * addRegion(char *parentRegion, char *childRegion, node_t *n, int *numNodes){

  char *data = "\0";// will hold data portion of server response string
  int status = SUCCESS; // assume there are no errors

  addRegionErrorCheck(parentRegion, childRegion, n, *numNodes, &status, &data); // verify no errors

  if(status == SUCCESS) // if no errors then remove the votes
    addRegionAction(parentRegion, childRegion, n, numNodes, &data);

  // At this point data is set either with error data or correct data or
  // even if it isn't we can append data since by default it is a null character
  int size = 3 + strlen(data) + 1; // 3 for 2 char cmd and ;, and 1 for null
  char *responseStr = (char *) malloc(sizeof(char) * size);// will contain server's response
  
  mallocCheck(responseStr);
  memset(responseStr, 0, size);

  switch(status) {
    case SUCCESS:
      copyStrToBuffer(responseStr, "SC;");
      break;

    case NO_REGION: 
      copyStrToBuffer(responseStr, "NR;");
      break;

    default : // should not reach this, if so it is an unhandled error
      if(strlen(data) > 0) // if needed free data
        free(data);

      copyStrToBuffer(responseStr, "UE;");
      return responseStr;
  }

  appendStr(responseStr, data); // append the data to complete the str

  if(strlen(data) > 0) // if needed free data
    free(data);

  return responseStr;
}

#endif