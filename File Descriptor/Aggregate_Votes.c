/*login: anwar027
  date: 03/09/18
  name: Muhammad Anwar
*/
#include "pa2utilB.h"

int main(int argc, char **argv){
  if (argc != 2){
    printf("Usage: %s Program\n", argv[0]);
    return -1;
  }  // end if

  DIR* dir = opendir(argv[1]);
  struct dirent* entry;

  pid_t pid;// for forks

  // if not leaf node then search though directories to exec Aggregate_Votes again
  if(!isLeafNode(argv[1])){

    while ((entry = readdir(dir)) != NULL) {
      if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

      //If Directory, then get the Subdirectory Path, fork, and exec
      if (entry->d_type == DT_DIR) {
        char subDirPath[strlen(argv[1]) + strlen(entry->d_name) + 2];
        subDirPath[0] = '\0';

        strcat(subDirPath, argv[1]); // subDirPath contains parentDirectory
        if(argv[1][strlen(argv[1]) - 1] != '/'){ // add the / if it isn't there already
          strcat(subDirPath, "/");
        }
        strcat(subDirPath, entry->d_name); // add subdirectory name
        strcat(subDirPath, "/"); 
        // subDirPath is now ParentDir/subDirName/

        pid = fork();
        if (pid == 0){ // if child then exec Aggregate_Votes
          /* child process */

          /* open /dev/null for writing */
          int fd = open("/dev/null", O_WRONLY);

          dup2(fd, 1);    /* make stdout a copy of fd (> /dev/null) */
          close(fd);      /* close fd */

          /* stdout and stderr now write to /dev/null */
          /* ready to call exec */

          execAggregateVotes(subDirPath);
        }
        else if (pid < 0){
          perror("Error. Fork failed. \n------- Terminating -------");
          exit(3);
        }  //error forking
      }  //end while
    } //end out if
    //closedir(dir);
    // reopen parentDir, and wait for all the children, which is the 
    // number of subdirectories
    dir = opendir(argv[1]);
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        // wait the same number of subdirectories 
        if (entry->d_type == DT_DIR)
          wait(NULL);
    } //end while
    //closedir(dir);
  } //end outer if
  else { // if leaf node execute Leaf Counter and do not fork
    execLeafCounter(argv[1]); 
  }
// --- Now Aggregate all txt files into a linked list and then output it ---
  // make a 1 node linked list that is uninitilized with sintinel data
  struct node* vote = (struct node*)malloc(sizeof(struct node));
  
  if (vote == NULL){
    perror("Error. Malloc call failed. \n------- Terminating -------");
    exit(2);
  } // end if

  vote->name = "";
  vote->count = SENTINEL_COUNT;
  vote->next = NULL;

  dir = opendir(argv[1]); // opening the directory of the passed in path
  while ((entry = readdir(dir)) != NULL) {

    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

    // if it is a directory then sumup all the files in the subdirectories
    if (entry->d_type == DT_DIR) { 
      char subDirPath[strlen(argv[1]) + strlen(entry->d_name) + 2];
      subDirPath[0] = '\0';

      strcat(subDirPath, argv[1]);// subDirectoryPath now contains parentDir
      if(argv[1][strlen(argv[1]) - 1] != '/'){
        strcat(subDirPath, "/");
      } // puts a / at the end of the parentDir
      strcat(subDirPath, entry->d_name); 

      // now need to get the actual filename to search for
      char * inputFileName = subfolder(subDirPath);
      char *postfix_c = ".txt";
      inputFileName = strConcat(inputFileName, postfix_c);

      strcat(subDirPath, "/");// now have parentDir/subDir/ (full path)

      // now search each subdirectory for the input files
        //closedir(dir);
        DIR* dirPtr = opendir(subDirPath);
        struct dirent* dEntry;

        while((dEntry = readdir(dirPtr)) != NULL){
          if(strcmp(dEntry->d_name, inputFileName) == 0){
            
            char fullFilePath[strlen(subDirPath) + strlen(inputFileName) + 1];
            int i; // counter for position for fullFilePath
            for (i = 0; i < strlen(subDirPath); i++)
              fullFilePath[i] = subDirPath[i];
            for(int j = 0; j < strlen(inputFileName); j++)
              fullFilePath[i++] = inputFileName[j];
            fullFilePath[i++] = '\0';

            parseInput(fullFilePath, vote); // parse every input file with proper name
          }// processing relavent file for sub directory
        }// end while for subdirecotyr

      }// end if if directory entry is a directory
    }// end while for recursing trough main directory 
    //closedir(dir);

  char *outputFilePath = getOutputFilePath(argv[1]);
  outputFileContents(vote, outputFilePath);
  if (pid != 0) // only parent process outputs to console
    printf("%s\n", outputFilePath);// print created output file

  freeList(vote);
  return 0;
} // end main