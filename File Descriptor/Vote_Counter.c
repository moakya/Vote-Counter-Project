/*login: anwar027
  date: 03/09/18
  name: Muhammad Anwar
*/
#include "pa2utilB.h"

void parseInput(char *path, node_t *vote);

int main(int argc, char **argv){
  if (argc != 2){
    printf("Usage: %s Program\n", argv[0]);
    return -1;
  }  // end if

  char *arr[ARGC_SIZE];
  arr[0] = AGGREGATE_VOTES;
  arr[1] = argv[1];
  arr[2] = (char *) NULL;

  struct node* vote = (struct node*)malloc(sizeof(struct node));
  if (vote == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if
  
  vote->name = "";
  vote->count = SENTINEL_COUNT;
  vote->next = NULL;

  char *path = getOutputFilePath(argv[1]);

  pid_t pid = fork();

  if (pid == 0){
    /* child process */

    /* open /dev/null for writing */
    int fd = open("/dev/null", O_WRONLY);

    dup2(fd, 1);    /* make stdout a copy of fd (> /dev/null) */
    close(fd);      /* close fd */

    /* stdout and stderr now write to /dev/null */
    /* ready to call exec */
    execv(arr[0], arr);
    perror("Error. Exec failed. \n------- Terminating -------");
    exit(4);
  }

  else if (pid > 0){
    wait(NULL);
    parseInput(path, vote);
    struct node *temp = maxAddrNode(vote);
    char *str = temp->name;
    addFinOut(path, str);
    printf("%s\n", path);
  } //end else

  else{
    perror("Error. Fork failed. \n------- Terminating -------");
    exit(3);
  }  //error forking

  freeList(vote);

return 0;
} //end main