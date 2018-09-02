#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h> 
#include <dirent.h>

#ifndef PA_2_UTIL_A
#define PA_2_UTIL_A
/*
* Struct to contain information about every candidate voted 
* for and the total count.
*/
typedef struct node{
  char *name;
  int count;
  struct node *next;  
}node_t;

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

/**************************************************************/
/**Function : outputFileContents
 * Arguments: 'vote' - Pointer to linked list to hold all data 
 *         'out' - holds the path for the output file
 *
 * Output: -
 * 
 * About: outputFileContents is supposed to use the file path
 *    out, and add the contents from the linked list vote to the
 *    file in the correct format. 
 */
void outputFileContents(node_t *vote, char *out){
  FILE *outfile;   
  outfile = fopen(out, "w"); //maybe w+
  struct node *curr = vote;
  while (curr != NULL){
    fprintf(outfile, "%s:%d", curr->name, curr->count);
    if(curr->next != NULL)
      fprintf(outfile, ",");
    curr = curr->next;
  } //end while

  fprintf(outfile, "\n");
  fclose(outfile);
} //end outputFileContents

/**************************************************************/
/**Function : freeList
 * Arguments: 'vote' - Pointer to linked list to hold all data 
 *
 * Output: -
 *
 * About: freeList is a destructor for the linked list, it 
 *    frees all the data that was malloced for the linked list.
 */
void freeList(node_t *vote){
  struct node* prev = NULL;
  while(vote != NULL){
    prev = vote;
    vote = vote->next;
    free(prev);
  } //end while
} //end freeList

/**************************************************************/
/**Function : disp
 * Arguments: 'vote' - Pointer to linked list to hold all data 
 *
 * Output: -
 *
 * About: displays the contents of the lined list into the console.
 */
void disp(node_t *vote){
  struct node *loc = vote;
  while (loc != NULL){
    printf("Name:  %s\n", loc->name);
    printf("Count: %d\n\n", loc->count);
    loc = loc->next;
  } //end while
} //end disp

/**************************************************************/
/**Function : addrNode
 * Arguments: 'name' - the string that contains the name of the 
                  candidate
 *         'vote' - Pointer to linked list to hold all data
 *
 * Output: returns a pointer to a specfic node in the linked list
 * 
 * About: addrNode is supposed to determine where in the linked list
 *    a certain node is, and return its address.
 */
struct node* addrNode(char *name, node_t *vote){
  struct node *loc = vote;

  while (loc != NULL){
    if (strcmp(name, loc->name) == 0)
      return loc;
    loc = loc->next;
  } // end while
  return NULL;
} // end addrNode

/**************************************************************/
/**Function : existNode
 * Arguments: 'name' - the string that contains the name of the 
                  candidate
 *         'vote' - Pointer to linked list to hold all data
 *
 * Output: returns an int value, to represent whether, the 
 *    candidate already exists in the linked list
 * 
 * About: existNode is supposed to determine whether a node
 *    with the same candidate name exists in the linked list.
 */
int existNode(char *name, node_t *vote){
  struct node *loc = vote; 
  while (loc != NULL){
    if (strcmp(name, loc->name) == 0)
      return 1;
    loc = loc->next;
  } // end while

  return 0;
} // existNode

/**************************************************************/
/**Function : subfolder
 * Arguments: 'nPath' - holds the path of the input file 
 *      Ex. Who_Won/Region_1
 * Note there cannot be a / at the end
 * Output: returns a char * for the name of the output file
 * 
 * About: subfolder goes through the name of the path, gets the 
 *    name of the last directory and returns it. 
 */
char * subfolder(char * nPath){
  char revPath[2048];
  char * reversePath = revPath;
  int size = strlen(nPath);
  // copy nPath into reversePath in reverse without the '\0'
  // i = size - 1 because -1 vs -0 so skip the '\0'
  for(int i = size - 1, j = 0; i >= 0; i--, j++)
    reversePath[j] = nPath[i];
  reversePath[size] = '\0';

  // get last subfolder in reverse
  char revSubFolder[2048];
  char * reverseSubfolder = revSubFolder;
  reverseSubfolder = strtok(reversePath, "/");

  char * subfolder = (char *) malloc(sizeof(char) * 2048);
  if (subfolder == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if
  
  size = strlen(reverseSubfolder);
  // unreverse the subfolder
  for(int i = size - 1, j = 0; i >= 0; i--, j++)
    subfolder[j] = reverseSubfolder[i];
  subfolder[size] = '\0';

  return subfolder;
}

/**************************************************************/
/**Function : isLeafNode
 * Arguments: 'path' - holds the path of the directory to check if
 *                     it is a leaf node
 *                     Ex. Who_Won/Region_1
 *
 * Output: returns a bool: true if leaf node false if not
 * 
 * About: opens dir given path and checks if there are any sub dirs. 
 */
bool isLeafNode(char * path){
  DIR* dir = opendir(path);
  struct dirent* entry;
  dir = opendir(path);
  bool isLeafNode = true;
  while ((entry = readdir(dir)) != NULL && isLeafNode) {

    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

    if (entry->d_type == DT_DIR) {
      isLeafNode = false;
    }
  }
  return isLeafNode;
}

#endif