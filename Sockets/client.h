#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

#ifndef CLIENT_H
#define CLIENT_H

#define CANDIDATE_NAME_MAX_SIZE 100

// struct that will be a linked list containing candidate name 
// and vote count
typedef struct candNode{
  char name[CANDIDATE_NAME_MAX_SIZE];
  int count;
  struct candNode *next;
}candnode_t;


/**Function : processInfo
 * Arguments: 'reqFile' - the main input file that holds the list of commands
 *            'dir' - the directory that contains all the input files
 *            'iPAddr' - the IP address where the server is running
 *            'portStr' - the port number as a string, to be converted to int later.
 *        
 * Output: -
 * 
 * Description - The heart of the client, where the reqFile, and the dir are used
 *   to generate a request that will be passed to the server. Then the IP and 
 *   the port numbers are used to connect to the server, and pass the request. 
 *   After which some data is recieved and displayed to the user in the terminal.
 *
 * Utilizes many different functions/methods that allow this function to 
 * process multiple parts of the request. 
 */
void processInfo(char *reqFile, char *dir, char *iPAddr, char *portStr);

/**Function : parseLeafNodeInput
 * Arguments: 'path' - path of the file to be processed
 *        'vote' - Pointer to linked list to hold all data,first node
 *          must have been allocated.
 *        
 * Output: - Changes vote to data read from the path file
 * 
 * Description - Gets all the data from the file, if there is a file
 *  named votes.txt. Then the data is processed utilizing other 
 *  functions, and then data is added to the linked list.
 *
 * It also makes use of functions existCandNode, addrCandNode, traverseCandLink 
 */
void parseLeafNodeInput(char *path, candnode_t **vote);

/**Function : createRequest
 * Arguments: 'strArray' - contains the main command and its arguments
 *        'dir' - a string to the directory to the input files
 *        
 * Output: - Returns the created request that is passed to the server
 * 
 * Description - Creates a string that will contain all the information
 *  required for the server to execute the command, and ensures that the
 *  correct arguments are in the correct spots in the string. 
 */
char * createRequest(char *strArray[], char *dir);

/**Function : getDataString
 * Arguments: 'votes' - linked list of the candidates and their counts
 *        
 * Output: - Creates the final string of the data.
 * 
 * Description - Uses the list and generates the final string of data
 *   that will be used in the string that will be passed as a request
 *   to the server. Ensures the formatting is correct.
 */
char *getDataString(candnode_t *votes);

void mallocCheck(void *x){
  if (x == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(4);
  }
}

void openFileCheck(FILE *x){
  if (x == NULL){
    perror("Error. Open file failed. \n------- Terminating -------");
    exit(8);
  }  // end if
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
/**Function : getMainDir
 * Arguments: 'filePath' - The path of the input req file
 *            'dirPath' - the string to contain the root path
 *     
 * Output: -
 *
 * determines the root directory where all the input files 
 * are located.
 */  
void getMainDir(char *filePath, char **dirPath){
  char *temp = (char *)malloc(strlen(filePath) * sizeof(char) + 1);
  mallocCheck(temp);
  memset(temp, 0, strlen(temp));

  for (int i = 0; i < strlen(filePath); i++)
    temp[i] = filePath[i];

  int flag = 1;
  for (int i = strlen(temp) - 1; i >= 0 && flag == 1; i--){
    if (temp[i] != '/')
      temp[i] = 0;
    else
      flag = 0;
  }

  if (strlen(temp) == 0){
    temp[0] = '.';
    temp[1] = '/';
  }

  *dirPath = (char *)malloc(strlen(temp) * sizeof(char) + 1);
  mallocCheck(*dirPath);

  memset(*dirPath, 0, strlen(temp));

  for (int i = 0; i < strlen(temp); i++)
    (*dirPath)[i] = temp[i];

  (*dirPath)[strlen(*dirPath)+1] = 0;
  
  free(temp);
}

/**************************************************************/
/**Function : chkCmdReq
 * Arguments: 'cmd' - The command that is being checked
 *            'numArgs' - The number of args the command expects
 *     
 * Output: returns an int as a flag
 *
 * checks whether the command is legitimate, and if so that it
 * has the correct number of arguments
 */  
int chkCmdReq(char *cmd, int numArgs){
  int num = numArgs - 1;
  
    if (strcmp(cmd, "Return_Winner") == 0){
      if (num == 0) return 1;
    }
    else if (strcmp(cmd, "Count_Votes") == 0){
      if (num == 1) return 1;
    }
    else if (strcmp(cmd, "Open_Polls") == 0){
      if (num == 1) return 1;
    }
    else if (strcmp(cmd, "Add_Votes") == 0){
      if (num == 2) return 1;
    }
    else if (strcmp(cmd, "Remove_Votes") == 0){
      if (num == 2) return 1;
    }
    else if (strcmp(cmd, "Close_Polls") == 0){
      if (num == 1) return 1;
    }
    else if (strcmp(cmd, "Add_Region") == 0){
      if (num == 2) return 1;
    }
    else{return 0;}
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
/**Function : strConcat
 * Arguments: 'dest' - the starting string to append src to
 *            for files
 *         'src' - char * to be appended to end of dest
 *
 * Output: executes appropriate cmd such as leafcounter, aggregate_votes,
 * and find_winner with correct arguments.
 * 
 * About strConcat: strConcat is supposed to
 * to concatenate 2 strings are return the final one.
 */
char * strConcat(char *dest, char *src){

  // plus 1 to account for NULL at end of string
  int size = strlen(dest) + strlen(src) + 1;

  // allocate string memory to a new string
  char *concat = (char *) malloc (size * sizeof (char));

  if(concat == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if

  // add the first strings contents
  for (int i = 0; i < strlen(dest); i++)
    concat[i] = dest[i];

  // add the second strings contents
  int i = strlen(dest); // overwrites first string NULL
  for (int j = 0; j < strlen(src); j++){
    concat[i] = src[j];
    i++;
  } // end for

  return concat;
} // end strConcat

/**Function : listSize
 * Arguments: 'votes' - linked list holding all the candidates and counts
 *        
 * Output: - Returns the total number of nodes in the linked list
 * 
 * Description - Calculates the total number of nodes in the linked list
 *   and returns it to be used for other reasons.  
 */
int listSize(candnode_t *votes){
  struct candNode *curr = votes;

  int size = 0;
  while(curr != NULL){
    size++;
    curr = curr->next;
  }

  return size;
}

#endif 