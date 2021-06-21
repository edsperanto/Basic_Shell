#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>

// constants
// calculations take into account spaces between tokens and commands
const int MAX_CMD_NUM = 10;
const int MAX_TOKENS_PER_CMD = 15+1;
const int MAX_TOKEN_LEN = 20;
const int MAX_CMD_CHAR = MAX_TOKENS_PER_CMD*MAX_TOKEN_LEN+MAX_TOKENS_PER_CMD-1;
const int MAX_CHAR_COUNT = MAX_CMD_NUM*MAX_CMD_CHAR+3*(MAX_CMD_NUM-1);

// function declarations
void readLine(char str[]);
void tokenize(char str[], char tokens[][MAX_TOKENS_PER_CMD][MAX_TOKEN_LEN]);
void setUpPipes(int fd[][2], int cmdNum, int totalCmds, pid_t cPID);
void execCommand(char cmd[][MAX_TOKEN_LEN], int cmdNum, int totalCmds);
void waitForChild(pid_t cPID);
void runCommands(char cmdList[][MAX_TOKENS_PER_CMD][MAX_TOKEN_LEN]);

// main function
int main() {

    // variables
    char cmd[MAX_CHAR_COUNT];
    char tokens[MAX_CMD_NUM][MAX_TOKENS_PER_CMD][MAX_TOKEN_LEN];

    // run command(s)
    readLine(cmd);
    tokenize(cmd, tokens);
    runCommands(tokens);
        
    // success
    return 0;
    
}

// reads command line from user
void readLine(char str[]) {
    printf("myshell$");
    fgets(str, MAX_CHAR_COUNT, stdin);
}

// split user command into pipe-separated commands and space-separated tokens
void tokenize(char str[], char tokens[][MAX_TOKENS_PER_CMD][MAX_TOKEN_LEN]) {

    // initialize empty tokens
    int cmdNum = 0, tokenNum = 0, charNum = 0;
    for(int i = 0; i < MAX_CMD_NUM; i++)
        for(int j = 0; j < MAX_TOKENS_PER_CMD; j++)
            tokens[i][j][0] = '\0';
            
    // populate token array
    for(; *str != '\0'; str++) {
        
        // if character is not a space and is not the pipe character
        // add character to current word
        if(!isspace(*str) && *str != '|') {
            tokens[cmdNum][tokenNum][charNum++] = *str;
            
        // if character is a space and current token not empty
        // end current token and start populating next token
        } else if(isspace(*str) && charNum != 0) {
            tokens[cmdNum][tokenNum++][charNum] = '\0';
            charNum = 0;
            
        // if character is the pipe character
        // end current word and start populating next command
        } else if(*str == '|') {
            tokens[cmdNum++][tokenNum][charNum] = '\0';
            tokenNum = charNum = 0;
        } 
        
    }
    
}

// set up pipes
void setUpPipes(int fd[][2], int cmdNum, int totalCmds, pid_t cPID) {

    // first command in pipeline
    if(cmdNum == 0 && totalCmds > 1) {
        if(!cPID && dup2(fd[cmdNum][1], 1) < 0) {
            perror("could not duplicate file descriptor\n");
            exit(1);
        }
        close(fd[cmdNum][1]);
        
    // last command in pipeline
    } else if(cmdNum == totalCmds - 1 && totalCmds > 1) {
        if(!cPID && dup2(fd[cmdNum-1][0], 0) < 0) {
            perror("could not duplicate file descriptor\n");
            exit(1);
        }
        close(fd[cmdNum-1][0]);
        
    // commands in middle of pipeline
    } else if(totalCmds > 1) {
        if(!cPID && dup2(fd[cmdNum-1][0], 0) < 0) {
            perror("could not duplicate file descriptor\n");
            exit(1);
        }
        if(!cPID && dup2(fd[cmdNum][1], 1) < 0) {
            perror("could not duplicate file descriptor\n");
            exit(1);
        }
        close(fd[cmdNum-1][0]);
        close(fd[cmdNum][1]);
    }
    
}

// execute command with arguments
void execCommand(char cmd[][MAX_TOKEN_LEN], int cmdNum, int totalCmds) {

    // prepare arguments for command
    int tokenNum = 0;
    char *args[MAX_TOKENS_PER_CMD];
    for(; cmd[tokenNum][0] != '\0'; tokenNum++)
        args[tokenNum] = (char*)&cmd[tokenNum][0];
    args[tokenNum] = (char*)NULL;
    
    // execute command
    if(execvp(args[0], args)) {
        perror("child process failed\n");
        exit(1);
    }
    
}

// wait for previous command to finish
void waitForChild(pid_t cPID) {
    int status;
    wait(&status);
    printf("process %d exits with %d\n", cPID, status);
}

// run all commands 
void runCommands(char cmdList[][MAX_TOKENS_PER_CMD][MAX_TOKEN_LEN]) {
    
    // find out how many commands to run
    int totalCmds = 0;
    for(; cmdList[totalCmds][0][0] != '\0'; totalCmds++) ;
    
    // prepare pipes
    int fd[totalCmds-1][2];
    for(int i = 0; i < totalCmds - 1; i++) {
        if(pipe(fd[i])) {
            perror("could not create pipe\n");
            exit(1);
        }
    }
    
    // run commands
    pid_t cPID;
    for(int cmdNum = 0; cmdNum < totalCmds; cmdNum++) {
        if((cPID = fork()) < 0) {         // fail
            perror("could not fork process\n");
            exit(1);
        } else if(cPID == 0) {            // child
            setUpPipes(fd, cmdNum, totalCmds, cPID);
            execCommand(cmdList[cmdNum], cmdNum, totalCmds);
        } else {                          // parent
            setUpPipes(fd, cmdNum, totalCmds, cPID);
            waitForChild(cPID);
        }
    }
    
}