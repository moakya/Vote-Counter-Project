/*login: anwar027
  date: 03/09/18
  name: Muhammad Anwar
*/
#include "pa2utilA.h"
 
/**Function : parseInput
 * Arguments: 'path' - path of the file to be processed
 *        'vote' - Pointer to linked list to hold all data
          'nPath' - path to the output file
 *        
 * Output: -
 * 
 * Description - Gets all the data from the file, if there is a file
 *  named votes.txt. Then the data is processed utilizing other 
 *  functions, and then data is added to the linked list, it then
 *  calls the output function to add data to output file
 *
 * It also makes use of functions existNode, addrNode, traverseLink 
 */
void parseInput(char *path, node_t *vote, char *nPath);

/**Function : traverseLink
 * Arguments: 'ntoken' - the string that contains the name of the 
                  candidate
 *         'vote' - Pointer to linked list to hold all data
 *
 * Output: -
 * 
 * About: traverseLink goes to the end of the linked list, and
 *    creates a new node and adds it to the list, and then updates 
 *    the data held within with ntoken.
 */
void traverseLink(char *ntoken, node_t *vote);

int main(int argc, char **argv){
  if (argc != 2){
    printf("Usage: %s Program\n", argv[0]);
    return -1;
  }  // end if

  //general postfixes for the paths used in the program
  char *postfix_a = "/votes.txt";
  char *postfix_b = "votes.txt";
  char *postfix_c = ".txt";

  //getting the path of the input file
  char *fname = argv[1];
  if ((argv[1])[strlen(argv[1])- 1] == '/')
    fname = strConcat(fname, postfix_b);
  else 
    fname = strConcat(fname, postfix_a);

  //to hold the path of the path for the output file
  char *nPath = (char *)malloc(sizeof(char) * 2048);
  if (nPath == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if

  nPath = argv[1];
  
  if ((nPath)[strlen(argv[1])- 1] == '/')
    nPath[strlen(argv[1]) - 1] = '\0'; 
  
  //hold the name of the directory the file will be in
  char * subfldr = subfolder(nPath);
  
  //operations to fix the path
  nPath = strConcat(nPath, "/");

  //add the name of the file
  nPath = strConcat(nPath, subfldr);

  //add the extension of the file
  nPath = strConcat(nPath, postfix_c);

  printf("%s\n", nPath);  // output file path

  //linked list to hold all the data
  struct node* vote = (struct node*)malloc(sizeof(struct node));
  if (vote == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if
  vote->count = 0;  //initializing the count 
  vote->next = NULL;  //initializing the next ptr

  parseInput(fname, vote, nPath);

  //processes to free all the allocated memory
  freeList(vote);
  free(vote);
  free(subfldr);
  free(nPath);
  return 0;
} // end main

void parseInput(char *path, node_t *vote, char *nPath){
  FILE *infile;   
  infile = fopen(path, "r");  

  if(infile == NULL){
    perror("Not a leaf node.\n");
    exit(1);
  }  // end if

  //get size of file
  fseek(infile, 0, SEEK_END);
  long fsize = ftell(infile);
  rewind(infile); //go back to beginning of file

  //to hold all the file data
  char *buff = malloc(fsize + 1);
  if(buff == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if

  fread(buff, fsize, 1, infile);  //read all the data
  buff[fsize] = '\0'; //end buff in NULL char

  fclose(infile); //close the input file

  if (strlen(buff) == 0){
    perror("Error. Empty Input File. \n------- Terminating -------");
    exit(5);
  }

  //tokenize the first name
  char *token = strtok(buff, "\n");
  vote->name = token; //change the name in the list

  while (token != NULL){
    //check if the name exists in the linked list
    int flag = existNode (token, vote);
      if (flag == 1) { // if true
        struct node *temp = addrNode(token, vote);
        temp->count++;  //increment count
      } // end if
      else
        traverseLink(token, vote);
    token = strtok(NULL, "\n");
  } //end while

  outputFileContents(vote, nPath);
  
  free(buff); //deallocate buffer
} // end parseInput 

void traverseLink(char *ntoken, node_t *vote){
  struct node *loc = vote;  //ptr to beginning of list

  //create a new node
  struct node *temp = (struct node*)malloc(sizeof(struct node));
  if (temp == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if
  //change the members of the node
  temp->name = ntoken;
  temp->count = 1;
  temp->next = NULL;

  while (loc != NULL){
    if (loc->next == NULL){ //if the node is the last in the list
      loc->next = temp; //reassign node ptr to the new node
      return;
    } //end if
    else
      loc = loc->next;  //increment the pointer
  } //end while
} //end traverseLink