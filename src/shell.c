// Custom Shell Program
#include "types.h"
#include "user.h"
#include "fcntl.h"

#define NULL 0

void shellInputHandler(char*);
int handlePipeCommand(char*);

void illegalCommandMessage() {
  printf(1, "Illegal command or arguments\n");
}

struct command {
  char cmdString[50]; // for testing purpose
  char *argv[10]; //at max 10 arguments are supported
  char inputfile[20];
  char outputfile[20];
  int argc;
};

// Separates each argument from command input string
// if I/O redirection exist, it takes care of that too
// and returns a new "command" structure.
struct command* parseCommand(char *userInput) {
  struct command *newCommand = malloc(sizeof(struct command));
  strcpy(newCommand->cmdString, userInput);
  strcpy(newCommand->inputfile, "");
  strcpy(newCommand->outputfile, "");
  int argCount = 0; // total count of arguments
  char *ch = userInput;
  char currWord[50];
  int currWordIndex = 0;
  char tempArgs[10][50];
  while(1) {
    if(*ch == '<' || *ch == '>') {
      int flag;
      if(*ch == '<') flag = 0; // inputfile
      else if(*ch == '>') flag = 1; // outputfile
      ch += 2;
      currWordIndex=0;
      while(*ch != ' ' && *ch != '\0' && *ch !='\n') {
        currWord[currWordIndex++] = *ch;
        ch++;
      }
      currWord[currWordIndex] = '\0';
      if(!flag) strcpy(newCommand->inputfile, currWord);
      else strcpy(newCommand->outputfile, currWord);
      currWordIndex=0;
      if(*ch == '\n' || *ch == '\0') break;
    } else if(*ch == ' ' || *ch == '\n' || *ch == '\0') {
      currWord[currWordIndex] = '\0';
      // printf(1, "currWord: %s\n", currWord);
      strcpy(tempArgs[argCount], currWord);
      newCommand->argv[argCount] = tempArgs[argCount];
      argCount++;
      currWordIndex=0;
      if(*ch == '\n' || *ch == '\0') break;
    } else {
      currWord[currWordIndex++] = *ch;
    }
    ch++;
  }
  newCommand->argc = argCount;
  return newCommand;
}

// Prints command struct : ONLY USED FOR TESTING PURPOSE
void printArgDetails(struct command* currCommand) { 
  printf(1,"User Input: %s\n", currCommand->cmdString);
  printf(1, "Number of args: %d\n", currCommand->argc);
  printf(1,"Inputfile: %s\n", currCommand->inputfile);
  printf(1,"Outputfile: %s\n", currCommand->outputfile);
  for(int i=0; i<currCommand->argc; i++) {
    printf(1, "%s, ", currCommand->argv[i]);
  }
  printf(1, "\n");
}

// Used to divide single input string into two commands divided by pipe or semicolon
void breakIntoTwo(char* inputCommand ,char divider, char* cmdOne, char* cmdTwo) {
  char *ptr = strchr(inputCommand, divider); // points to divider character
  char *secondCommand = ptr+2; // points to start of command 2
  ptr--; // goes back one character, now points to whitespace
  *ptr = '\0'; // to indicate end of  (for command 1)
  strcpy(cmdOne, inputCommand);
  strcpy(cmdTwo, secondCommand);
}

// Executes commands line by line from a given file
void executeCommands(char* filename) {
  int inputFile = open(filename , O_RDONLY);
  char buf [1000];
  int n;
  while ((n = read(inputFile, buf, sizeof(buf))) > 0) {
    char currLine[50];
    int currIndex = 0; // current index in current Line
    for(int i=0; i<n; i++) {
      if(buf[i] != '\n') {
        currLine[currIndex++] = buf[i];
        if(i == n-1) {
          // printf(1,"Line: %s\n",currLine);
          shellInputHandler(currLine);
        }
      } else {
        // printf(1,"Line: %s\n",currLine);
        currIndex=0;
        shellInputHandler(currLine);
      }
    }
  }
}

// all commands finally ends up here 
// essentially handles pipeless and semicolonless commands
// Executes commands (also handles I/O Redirection)
int runCommand(char* inputCommand) {
  struct command* currCommand = parseCommand(inputCommand);
  char *firstcmd = currCommand->argv[0];
  int inputFile, outputFile;
  int redirInputFlag = 0;
  int redirOutputFlag = 0;
  if(strchr(inputCommand, '<') != 0) redirInputFlag = 1;
  if(strchr(inputCommand, '>') != 0) redirOutputFlag = 1;

  if(strcmp(firstcmd, "ls")==0 || 
    strcmp(firstcmd, "cat")==0 || 
    strcmp(firstcmd, "grep")==0 || 
    strcmp(firstcmd, "echo")==0 || 
    strcmp(firstcmd, "wc")==0 ||
    strcmp(firstcmd, "procinfo") == 0) // procinfo is custom program
    {
      if(redirInputFlag) {
        inputFile = open(currCommand->inputfile, O_RDONLY);
        close(0); // close stdin
        dup(inputFile);
        close(inputFile);
      }
      if(redirOutputFlag) {
        outputFile = open(currCommand->outputfile, O_CREATE | O_RDWR);
        if(outputFile < 1) {
          printf(1, "open failed");
        }
        close(1); // close stdout
        dup(outputFile);
        close(outputFile);
      }
      int retval = exec(firstcmd, currCommand->argv);
      if(retval == -1) return -1;
    } else if(strcmp(firstcmd, "executeCommands") == 0) {
      if(currCommand->argc != 2) {
        illegalCommandMessage();
        return -1;
      }
      executeCommands(currCommand->argv[1]);
    } else {
      illegalCommandMessage();
      return -1;
    }
    return 0;
}

// handles command with semicolon
void handleParallelCommand(char* inputCommand) {
  if(strchr(inputCommand, ';') != 0) { // ; command (for parallel execution)
    char cmdOne[50];
    char cmdTwo[50];
    breakIntoTwo(inputCommand, ';', cmdOne, cmdTwo);
    int pid1 = fork();
    if(pid1 == 0) {
      int retval;
      if(strchr(cmdOne, '|') != 0) {
        retval = handlePipeCommand(cmdOne);
      } else {
        retval = runCommand(cmdOne);
      }
      if(retval == -1) exit(-1);
      else exit(0);
    }
    int pid2 = fork();
    if(pid2 == 0) {
      int retval;
      if(strchr(cmdTwo, '|') != 0) {
        retval = handlePipeCommand(cmdTwo);
      } else {
        retval = runCommand(cmdTwo);
      }
      if(retval == -1) exit(-1);
      else exit(0);
    }
    wait(0);
    wait(0);
  }
}

// Handles commands with pipe
int handlePipeCommand(char* inputCommand) {
  if(strchr(inputCommand, '|') != 0) {
    char cmdOne[50];
    char cmdTwo[50];
    breakIntoTwo(inputCommand, '|', cmdOne, cmdTwo);

    int pipefd[2];
    pipe(pipefd);
    if(fork() == 0) { // child 1
      close(1);
      dup(pipefd[1]);
      close(pipefd[0]);
      close(pipefd[1]);
      
      int retval = runCommand(cmdOne);
      
      if(retval == -1) exit(-1);
      else exit(0); // exit from child 1
    }

    if(fork() == 0) { // child 2
      close(0);
      dup(pipefd[0]);
      close(pipefd[0]);
      close(pipefd[1]);
      
      int retval = runCommand(cmdTwo);
      
      if(retval == -1) exit(-1);
      else exit(0); // exit from child 2
    }

    close(pipefd[0]); 
    close(pipefd[1]);
    int status1, status2;
    wait(&status1);
    wait(&status2);
    if(status1 == 0 && status2 == 0) return 0;
    else return -1;
  }
  return 0;
}

void handleSuccessiveExec(char* inputCommand) {
  char *ch = inputCommand;
  int andFlag = 0;
  int orFlag = 0;
  while(*ch != '\0') {
    if(*ch == '&' && *(ch+1) == '&') { // looks for &&
      andFlag = 1;
      break;
    } else if(*ch == '|' && *(ch+1) == '|') { // looks for ||
      orFlag = 1;
      break;
    }
    ch++;
  }
  char *secondCommand = ch+3;
  ch--;
  *ch = '\0';
  char cmdOne[50];
  char cmdTwo[50];
  strcpy(cmdOne, inputCommand);
  strcpy(cmdTwo, secondCommand);
  
  if(andFlag) {
    if(fork() == 0) {
      int retval = runCommand(cmdOne);
      if(retval == -1) exit(-1);
      else exit(0);
    }
    int status;
    wait(&status);
    if(status == 0) { // if first command is successful
      if(fork() == 0) {
        runCommand(cmdTwo);
        exit(0);
      }
      wait(0);
    }
  } else if(orFlag) {
    if(fork() == 0) {
      int retval = runCommand(cmdOne);
      if(retval == -1) exit(-1);
      else exit(0);
    }
    int status;
    wait(&status);
    if(status != 0) { // if first command is NOT successful
      if(fork() == 0) {
        runCommand(cmdTwo);
        exit(0);
      }
      wait(0);
    }
  }
}

// Root handler: All shell inputs starts here first.
void shellInputHandler(char* inputCommand) {
  struct command* currCommand = parseCommand(inputCommand);
  int succesiveFlag = 0;
  for(int i=0; i<currCommand->argc; i++) {
    if(strcmp(currCommand->argv[i], "&&") == 0 || strcmp(currCommand->argv[i], "||") == 0) {
      succesiveFlag = 1;
      break;
    }
  }
  if(succesiveFlag) {
    if(currCommand->argc >= 3) handleSuccessiveExec(inputCommand);
    else illegalCommandMessage();
  } else if(strchr(inputCommand, ';') != 0) { // pipe command
    handleParallelCommand(inputCommand);
  } else if(strchr(inputCommand, '|') != 0) { // ; command (for parallel execution)
    handlePipeCommand(inputCommand);
  } else if(strcmp(inputCommand, "ps") == 0) {
    psinfo();
  }else if(strcmp(inputCommand, "exit") == 0) {
    wait(0);
    exit(0);
  } else { // normal single command
    if(fork() == 0) {
      runCommand(inputCommand);
      exit(0);
    }
    wait(0);
  }
}

// Removes extra spaces from input string
void trimInput(char* inputCommand) {
  inputCommand[strlen(inputCommand)-1] = '\0'; // removing last newline character
  char tempFiltered[100];
  int tempIndex = 0;
  int firstSpaceFlag = 0;
  char *ch = inputCommand;
  while(*ch != '\0') {
    if(*ch == ' ') {
      if(firstSpaceFlag) {
        tempFiltered[tempIndex++] = ' ';
      } else {
        // skip these spaces
      }
      firstSpaceFlag = 0; // next consecutive spaces would not be allowed
    } else {
      firstSpaceFlag = 1; // allow next space to be considered
      tempFiltered[tempIndex++] = *ch;
    }
    ch++;
  }
  tempFiltered[tempIndex] = '\0';
  if(tempFiltered[tempIndex-1] == ' ') tempFiltered[tempIndex-1] = '\0'; // removing tailing space if exists
  strcpy(inputCommand, tempFiltered); 
}

int main(void) {
  while(1) { // infinite outer loop of shell
    char inputCommand[100];
    printf(1, "MyShell> ");
    gets(inputCommand, sizeof(inputCommand));

    // Filtering input string
    trimInput(inputCommand);

    // PLAY AREA - STARTS
    
    shellInputHandler(inputCommand);

    // PLAY AREA - ENDS
  }
  exit(0);
}