/*date: 02/21/18
* name: Muhammad Osamah Anwar
* x500: anwar027*/

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

#define MAX_NODES 100
#define OUTPUT "Output_"
#define ROOT_NODE_NAME "Who_Won"
#define LEAFCOUNTER "leafcounter"
#define AGGREGATE_VOTES "aggregate_votes"
#define FIND_WINNER "find_winner"

/*
* Struct to contain information about every node 
* including name, tree structure, i/o, and for running
* and executing processes.
*/
typedef struct node{
	char *name;
	char *input[50];
	char *numInputs;
	char *output;
	int children[10];
	int num_children;
	pid_t pid;
	bool visited;	//for checking if there is a cycle in tree
}node_t;

/**Function : trimwhitespace
 * Arguments: 'str' - name of the input file
 *
 * Output: new char * that is str without the white space
 *
 * About trimwhitespace: trimwhitespace is supposed to
 * remove all whitespace from str and return a new char *.
 */
char *trimwhitespace(char *str);

/**Function : parseInput
 * Arguments: 'filename' - name of the input file
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * 				'totCandidates' - total candidates as a char *
 * 				'candNames' - names of candidates as a array
 *
 * Output: Number of Total Allocated Nodes
 * Alters n to contain all the nodes initilized for name, input, numInputs,
 * output, children, and num_children. Also totCandidates and candNames are
 * initialized as well.
 *
 * About parseInput: parseInput is supposed to
 * 1) Open the Input File 
 * 2) Read it line by line : Ignore the empty lines 
 * It achieves this by a while loop getting each line so that it ignores
 * empty and commented out lines.
 * It also makes use of helper functions trimwhitespace() and strConcat(). 
 */
int parseInput(char *filename, node_t *n, char **totCandidates, char **candNames);

/**Function : detectCycle
 * Arguments: 
 *  		   'n' - array of node structs which hold all info
 * 				'numNodes' - total elements in n
 * 				 of char *s.
 * 				'index' - index of array n which is the node
 * 						 that is index of rootIndex initially
 * 				'hasCycle' - pointer to a bool value that indicates
 * 				if there is a cycle
 *
 * Output: Changes hasCycle to true if there is a cycle.
 * 
 * About detectCycle: detectCycle is supposed to
 * Detect if there is cycle within the node structure n, and change hasCycle 
 * variable accordingly by recursively traversing tree.
 */
void detectCycle(node_t *n, int numNodes, int index, bool *hasCycle);

 /**Function : execNodes
 * Arguments:
 *  		   'n' - array of nodes we are searching nodename in
 * 				'totCandidates' - total candidates as a char *
 * 				'namesArray' - names of candidates as a array
 * 				 of char *s. 
 *  			'index' - index of node to run
 *
 * Output: executes appropriate cmd such as leafcounter, aggregate_votes,
 * and find_winner with correct arguments.
 * 
 * About execNodes: execNodes is supposed to
 * execute the node passed in but first wait for any children to run first.
 * it does this by recursively calling itself for children node and waiting 
 * for all children to finish running before the parent runs.
 * This means main simply needs to call execNodes with the root node.
 */
void execNodes(node_t *n, char *totCandidates, char *namesArray[1024], int index);

/**Function : findIndex
 * Arguments: 'nodename' - node name we are searching for
 *  		   'n' - array of nodes we are searching nodename in
 * 				'numNodes' - number of nodes in n
 *
 * Output: executes appropriate cmd such as leafcounter, aggregate_votes,
 * and find_winner with correct arguments.
 * 
 * About findIndex: findIndex is supposed to
 * looks for specific index of name we are searching for in node array 
 * and returns -1 if node not found or index if found.
 */
int findIndex (char *nodeName, node_t *n, int numNodes);

/**Function : strConcat
 * Arguments: 'dest' - the starting string to append src to
 * 						for files
 *  		   'src' - char * to be appended to end of dest
 *
 * Output: executes appropriate cmd such as leafcounter, aggregate_votes,
 * and find_winner with correct arguments.
 * 
 * About strConcat: strConcat is supposed to
 * to concatenate 2 strings are return the final one.
 */
char * strConcat(char *dest, char *src);

/**Function : execCmd
 * Arguments: 'index' - index of array n which is the node that has the info 
 * 						for files
 *  		   'n' - array of node structs which hold all info
 * 			    'totCandidates' - total candidates as a char *
 * 				'namesArray' - names of candidates as a array
 * 				 of char *s.
 *
 * Output: executes appropriate cmd such as leafcounter, aggregate_votes,
 * and find_winner with correct arguments.
 * 
 * About execCmd: execCmd is supposed to
 * To call execv with proper arguments given what executable file to run.
 */
void execCmd(int index, node_t *n, char *totCandidates, char *namesArray[]);

int main(int argc, char **argv){

	//Allocate space for MAX_NODES to node pointer
	struct node* n=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

	if(n == NULL){
		perror("Error. Malloc call failed. \n------- Terminating -------");
		exit(7);
	}  //end if

	if (argc != 2){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}  //end if

	char *totCandidates;	//to hold number of total Candidates as a string
	char *candNames;	//to hold all the candidates in one array
	candNames = (char*) malloc(sizeof(char) * 1024);

	if(candNames == NULL){
		perror("Error. Malloc call failed. \n------- Terminating -------");
		exit(7);
	}  //end if

	//call parseInput
	int numNodes = parseInput(argv[1], n, &totCandidates, &candNames);

	int rootIndex = findIndex(ROOT_NODE_NAME, n, numNodes);

	if(rootIndex == -1){
		perror("Error. Could not find root node. ");
		exit(2);
	}  //end if

	//call detectCycle to see if there's a cycle
	bool hasCycle = false;//assume there is not a cycle
	detectCycle(n, numNodes, rootIndex, &hasCycle);

	if(hasCycle){
		perror("Error. Cycle detected within the inputFile. \n------- Terminating -------");
		exit(1);
	}  //end if

	//tokenize candidate names into a char* [] format for execv
	char *namesArray[1024];
	int nameSize = atoi(totCandidates);
	char *token = strtok(candNames, " ");

	for (int i = 0; i < nameSize; i++){
		namesArray[i] = token;
		token = strtok(NULL, " ");
	}	//end for

	//Call execNodes on the root node
	execNodes(n, totCandidates, namesArray, rootIndex);

	//free up all the malloc[ed] memory
    for (int i = 0; i < numNodes; i++){
    	free(n[i].numInputs);
    	n[i].numInputs = NULL;

    	free(n[i].output);
    	n[i].output = NULL;
    	for (int j = 0; j < n[i].num_children; j++){
    		free(n[i].input[j]);
    		n[i].input[j] = NULL;
    	}	//end inner for
    }	//end for

    free(candNames);
    candNames = NULL;
    free(n);
    n = NULL;
    
	return 0;
}	//end main

int parseInput(char *filename, node_t *n, char **totCandidates, char **candNames){
	//get input file
	FILE *infile;		
	infile = fopen(filename, "r");	

	if(infile == NULL){
		perror("Cannot open the input file. \n------- Terminating -------");
		exit(4);
	}  //end if

	//create string, and get string from file
	char line[1024];

	fgets(line, sizeof(line), infile);


	while((line[0] == '#' || line[0] == '\n' || line[0] == ' ')  && !feof(infile))
		fgets(line, sizeof(line), infile);


	//get first token
	char *token = strtok(line, " ");

	//total number of candidates
	*totCandidates = token;

	//check if totCandidates is a number	
	for(int i = 0; (*totCandidates)[i] != '\0'; i++)
		if(!(isdigit((*totCandidates)[i]))){
			perror("Error. First line must start with a number. \n------- Terminating -------");
			exit(10);
		} //end if

	*candNames = strtok(NULL, "\n");

	if(candNames == NULL){
		perror("Error. . \n------- Terminating -------");
		exit(10);
	}  //end if

	char line2[1024];   //get next line

	fgets(line2, sizeof(line2), infile);

	while(line2[0] == '#' || line2[0] == '\n' || line2[0] == ' ')
		fgets(line2, sizeof(line2), infile);

	//tokenize next line
	token = strtok(line2, " ");

	int numNodes = 0;	//to hold total number of nodes
	for (; token != NULL; numNodes++){
		n[numNodes].name = trimwhitespace(token);
		token = strtok(NULL, " ");
	}	//end for

	//check if name contains one alphabetical character, and all alpha numeric char's
	for(int i = 0; i < numNodes; i++){
		bool hasAlphaChar = false;//assume there is no alpha character in name

		for(int j = 0; n[i].name[j] != '\0'; j++){
			if(isalpha(n[i].name[j]))//check if name has alpha numeric character
				hasAlphaChar = true;
			else if( !isalnum(n[i].name[j]) && n[i].name[j] != '_'){
				perror("Error. Node name must contain only alphanumeric character. \n------- Terminating -------");
				exit(6);
			}    //end else if`
		} //end inner for

		if(hasAlphaChar == false){
			perror("Error. Node name must at least one alpha character. \n------- Terminating -------");
			exit(5);
		} //end if
	}  //end outer for

	//initialize the children count to 0 in each node
	for (int i = 0; i < numNodes; i++)
		n[i].num_children = 0;

	//now process the rest of the lines till EOF
	char line3[1024];		// create new buffer

	while (fgets(line3, sizeof(line3),infile) != NULL){
		
		while((line3[0] == '#' || line3[0] == '\n' || line3[0] == ' ') && !feof(infile))
			fgets(line3, sizeof(line3), infile);

		if(!feof(infile)){ // if at end of file there is nothing to parse
			char *depend [2048];	//dependency array
			char *par;			//will contain parent

			token = strtok(line3, " ");

			par = token;	//store parent

			//ignoring the ':'	
			token = strtok(NULL, " ");

			//get first child
			token = strtok(NULL, " ");
			
			int depCount = 0;	//hold total number of dependencies

			//process children into array depend
			for (int i = 0; token != NULL; i++){
				depend[i] = trimwhitespace(token);	//add to array
				depCount++;	//increment num of dependencies
				token = strtok(NULL, " ");	//get next token
			}	//end for	

			//find the index of the parent in the array
			int parent = findIndex(par, n, numNodes);

			if(parent == -1){
				perror("Error. Cannot find node. Malformed tree. \n------- Terminating -------");
				exit(3);
			} //end if

			n[parent].num_children = depCount;	//assign parent member to depCount

			char *temp;
			temp = (char *) malloc (sizeof(char) * 128);

			if(temp == NULL){
				perror("Error. Malloc call failed. \n------- Terminating -------");
				exit(7);
			} //end if

			sprintf(temp, "%d", depCount);
			n[parent].numInputs = temp;

			//now we have to add ints to the child array in the parent
			for (int k = 0; k < depCount; k++){	//go through depend array
				n[parent].children[k] = findIndex(depend[k], n, numNodes);

				if(n[parent].children[k] == -1){
					perror("Error. Cannot find node. Malformed tree. \n------- Terminating -------");
					exit(3);
				}    //end if
			} //end for
		}
	}	//end while 
	
	for(int i = 0; i < numNodes; i++){
		if(n[i].num_children == 0) //leaf nodes's inputfile is its ownname
			n[i].input[0] = n[i].name;

		else 		//initialize name of inputfiles with names of children
			for(int j = 0; j < n[i].num_children; j++){
				char *temp = strConcat(OUTPUT, n[n[i].children[j]].name);
				n[i].input[j] = temp;
			}	//end inner for
			
		//initialize name of outputfile with name of node prepended with "Output_"
		char *temp = strConcat(OUTPUT, n[i].name);
		n[i].output = temp;
	}	//end outer for
		
	//initilaize pid's so that they do not throw an error when compared against something 
	for(int i = 0; i < numNodes; i++){
		n[i].pid = 9999;//sintinel value
		n[i].visited = false; 	// set all nodes to initially unvisited to check for cycles
	}	//end for

	fclose(infile);
	
	return numNodes;
}	//end parseInput

char *trimwhitespace(char *str){
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;

  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}	//end trimwhitespace

int findIndex (char *nodeName, node_t *n, int numNodes){
	for (int j = 0; j < numNodes; j++)
		if (strcmp(nodeName, n[j].name)==0)
			return j;	//match found

	return -1;		//If match not found
}	//end findIndex

char * strConcat(char *dest, char *src){

	//plus 1 to account for NULL at end of string
	int size = strlen(dest) + strlen(src) + 1;

	//allocate string memory to a new string
	char *concat = (char *) malloc (size * sizeof (char));

	if(concat == NULL){
		perror("Error. Malloc call failed. \n------- Terminating -------");
		exit(7);
	}	//end if

	//add the first strings contents
	for (int i = 0; i < strlen(dest); i++)
		concat[i] = dest[i];

	//add the second strings contents
	int i = strlen(dest);	//overwrites first string NULL
	for (int j = 0; j < strlen(src); j++){
		concat[i] = src[j];
		i++;
	}	//end for

	return concat;
}	//end strConcat

void execCmd(int index, node_t *n, char *totCandidates, char *namesArray[]){
	char *arr[1024];	//arr of arguments to execute
	int i = 0; 			//counter

	if (n[index].num_children == 0){//if leaf node set an input file
		arr[i++] = LEAFCOUNTER;
		arr[i++] = n[index].input[0];
	}	//end if
	
	else {
		if (strcmp(n[index].name, ROOT_NODE_NAME) == 0)
			arr[i++] = FIND_WINNER;
		else
			arr[i++] = AGGREGATE_VOTES;

		arr[i++] = n[index].numInputs;
		for (int j = 0; j < n[index].num_children; j++)
			arr[i++] = n[index].input[j];
	}	//end outer else

	arr[i++] = n[index].output; //assign output file
	arr[i++] = totCandidates; 	//total number of candidates

	//assign all canidate names
	for (int j = 0; j < atoi(totCandidates); j++)
		arr[i++] = namesArray[j];

	arr[i++] = (char *) NULL;

	if(execv(arr[0], arr) == -1){
		perror("Error. execv failed. \n------- Terminating -------");
		exit(8);
	}	//end if
}	//end execCmd

void execNodes(node_t *n, char *totCandidates, char *namesArray[1024], int index){
	if(strcmp(n[index].name, ROOT_NODE_NAME) == 0){ //root node name

		for(int i = 0; i < n[index].num_children && n[index].pid > 0; i++){//make sure only parent runs this loop
			n[index].pid = fork();//spawn new process for each child

			if(n[index].pid < 0){
				perror("Error. Fork failed. \n------- Terminating -------");
				exit(9);
			}
			else if(n[index].pid == 0)//if child then it needs to run node
				execNodes(n, totCandidates, namesArray, n[index].children[i]);
		}	//end for

		if(n[index].pid > 0){
			for(int i = 0; i < n[index].num_children; i++)
				if(wait(NULL) == -1){	//if parent process then wait for child nodes to execute
					perror("Error. Wait failed. \n------- Terminating -------");
					exit(11);
				}	//end inner if
			execCmd(index, n, totCandidates, namesArray);//get winner
		}	//end inner if
	}//end if - root node

	else if(n[index].num_children > 0){
		for(int i = 0; i < n[index].num_children && n[index].pid > 0; i++){//make sure only parent runs this loop
			n[index].pid = fork();//spawn new process for each child

			if(n[index].pid < 0){
				perror("Error. Fork failed. \n------- Terminating -------");
				exit(9);
			}
			else if(n[index].pid == 0)//if child then it needs to run node
				execNodes(n, totCandidates, namesArray, n[index].children[i]);
		}	//end for

		if(n[index].pid > 0){
			for(int i = 0; i < n[index].num_children; i++)
				if(wait(NULL) == -1){	//if parent process then wait for child nodes to execute
					perror("Error. Wait failed. \n------- Terminating -------");
					exit(11);
				}
			execCmd(index, n, totCandidates, namesArray);//aggregate votes
		}	//end if
	}//end else if non-leaf non-root nodes
	else if(n[index].num_children == 0)
		execCmd(index, n, totCandidates, namesArray);//if leaf node count votes

}	//end execNodes


void detectCycle(node_t *n, int numNodes, int index, bool *hasCycle){
	//try to visit node
	if(n[index].visited){//if node is already visited there is a cycle
		*hasCycle = true;
		return;
	}	//end if

	n[index].visited = true;//visit node

	//traverse all children
	for(int i = 0; i < n[index].num_children; i++)
		detectCycle(n, numNodes, n[index].children[i], hasCycle);
}	//end detectCycle