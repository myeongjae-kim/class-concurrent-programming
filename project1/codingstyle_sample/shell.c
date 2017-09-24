/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : shell.c
 * Due date             : 2017-03-27
 * Compilation Standard : c11 */

/** Design Document
 *  If argc is 1, interactive mode, else batch mode.
 *
 *  Both interactive and batch mode are using same function. 
 *  The difference is that when interactive mode "prompt> " is printed 
 * but batch mode, an instruction is printed.
 *
 *  First, parse input string to statements.
 *  Next, parse statments to a program path and options.
 *  Make a child process and execute the program.
 *
 *  The shell will run children processes simultaneously and wait all of them.
 *
 *  When the shell meets "quit", it terminates.*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

/** #define DEBUGGING */

#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_RESET    "\x1b[0m"

#define ERROR_MSG( msg ) \
    fprintf( stderr, ANSI_COLOR_RED "\tERROR: "ANSI_COLOR_YELLOW"%s" ANSI_COLOR_RESET, ( msg ) )

static const int STR_BUFF_SIZE = 512;
static const char* const STDIN_FILEPATH = "/dev/fd/0";

void readInstruction(const char* const filePath);
void parseAndExecute(char* const instructionBuffer, int* const numberOfChildProcesses);

int main(const int argc, const char* const argv[])
{
    /** If argc is zero, interactive mode, else batch mode. */
    if (argc == 1) {
        /** interactive mode */
        readInstruction(STDIN_FILEPATH); 
    } else {
        /** batch mode for all command line arguments*/
        for (int i = 1; i < argc; ++i) {
            readInstruction(argv[i]);
        }
    }
    return 0;
}

void readInstruction(const char* const filePath){
    /** Read file to the end. */
    FILE* fp = fopen(filePath, "r");
    if (fp == NULL) {
        /** file open failed. do nothing. */
        ERROR_MSG("Batch mode\n");
        ERROR_MSG("File opening is failed. Check the arguments.\n");
        return;
    }
    
    bool interactiveMode = false;
    /** Turn on interactiveMode
      * when addresses of below pointers are same. */
    if(STDIN_FILEPATH == filePath) {
        interactiveMode = true;
    }

    /** this is a buffer for reading instructions. */
    char* instructionBuffer = (char*)calloc(STR_BUFF_SIZE, sizeof(char));
    assert(instructionBuffer != NULL);
    
    /** Parse strings and run the process using fork(). */
    while (!feof(fp)) {
        if (interactiveMode) {
            printf("prompt> ");
        }

        if ( fgets(instructionBuffer, STR_BUFF_SIZE, fp) != NULL ) {
            /** fgets catches '\n' */

            /** remove '\n'. If you cannot understand below, learn how strtok() works. */
            strtok(instructionBuffer, "\n");
            if (*instructionBuffer == '\n') {
                /** no input. do nothing */
                continue;
            }

            /** print instruction when batch mode. */
            if (interactiveMode == false) {
                /** batch mode */
                printf("%s\n", instructionBuffer);
            }

            /** Parse user inputs and run the process using fork(). */
            int numberOfChildProcesses = 0;
            parseAndExecute(instructionBuffer, &numberOfChildProcesses);

            /** run child processes simultaneously */
            pid_t childPid;
            int status = 0;
            for (int i = 0; i < numberOfChildProcesses; ++i) {
                childPid = wait(&status);

                /** error in wait function */
                if (childPid == -1) {
                    perror("waitpid");
                } else {
                    /** wait() has succeeded. Do nothing. */
                }

                /** print debugging information. */
#ifdef DEBUGGING
                printf("Process number %d: ", childPid);
                if (WIFEXITED(status)) {
                    printf("exited, status=%d\n", WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("killed by signal %d\n", WTERMSIG(status));
                } else if (WIFSTOPPED(status)) {
                    printf("stopped by signal %d\n", WSTOPSIG(status));
                } else if (WIFCONTINUED(status)) {
                    printf("continued\n");
                }
#endif
            }

            memset(instructionBuffer, 0, STR_BUFF_SIZE);
        } else {
            /* end of the file */
            if (interactiveMode) {
                /* Ctrl+D */
                putchar('\n');

            } else {
                /** do nothing.
                  * next while loop, loop ends. */
            }
        }
    }

    free(instructionBuffer);
    fclose(fp);
    fp = NULL;
}

void parseAndExecute(char* const inputString, int* const numberOfChildProcesses) {
    /** Parse user inputs and run the process using fork(). */

    /** statement is partitioned by ';'.
      * statement is composed of programPath and options. */

    /** statement tokenization start */
    int numberOfStatements = 1;
    int statementArgsIdx = 0;
    char** statementArgs = (char**)calloc(numberOfStatements, sizeof(char*));
    assert(statementArgs != NULL);

    char* statementArgsToken;
    const char delimitorOfStatement[2] = ";";

    /** get the first token */
    statementArgsToken = strtok(inputString, delimitorOfStatement);
    if (statementArgsToken == NULL) {
        /** no program path. */
        return;
    }

    /** first statement */
    statementArgs[statementArgsIdx++] = statementArgsToken; 

    while (true) {
        statementArgsToken = strtok(NULL, delimitorOfStatement);

        /** no token */
        if (statementArgsToken == NULL) {
            /** end of the loop. */
            break;
        }

        /** if the allocated memory is not enough, reallocate. */
        if (numberOfStatements == statementArgsIdx) {
            numberOfStatements *= 2;
            statementArgs = (char**)realloc(statementArgs, numberOfStatements * sizeof(char*));
            assert(statementArgs != NULL);
        }

        /** add a token to tokenArray */
        statementArgs[statementArgsIdx++] = statementArgsToken;
    }

#ifdef DEBUGGING
        for (int i = 0; i < statementArgsIdx; ++i) {
            printf("statement %d: %s\n", i, statementArgs[i]);
        }
#endif

    /** for every statment */
    for (int i = 0; i < statementArgsIdx; ++i) {
        /** remove spacebar before the programPath */
        while ( *(statementArgs[i]) == ' ') {
            statementArgs[i]++;
        }

        /** optionArgs intialization */
        int numberOfOptionArgs = 1;
        int optionArgsIdx = 0;
        char** optionArgs = (char**)calloc(numberOfOptionArgs, sizeof(char*));
        assert(optionArgs != NULL);

        /** options Tokenization start */
        char* optionArgsToken;
        const char delimitorOfOptions[2] = " ";

        /** get the first token */
        optionArgsToken = strtok(statementArgs[i], delimitorOfOptions);
        if (optionArgsToken == NULL) {
            /** no program path. */
            return;
        }

        /** optionArgs[0] is program path. */
        optionArgs[optionArgsIdx++] = optionArgsToken; 

        while (true) {
            optionArgsToken = strtok(NULL, delimitorOfOptions);

            /** no token */
            if (optionArgsToken == NULL) {
                /** end of the loop. */
                break;
            }

            /** if the allocated memory is not enough, reallocate. */
            if (numberOfOptionArgs == optionArgsIdx) {
                numberOfOptionArgs *= 2;
                optionArgs = (char**)realloc(optionArgs, numberOfOptionArgs * sizeof(char*));
                assert(optionArgs != NULL);
            }

            /** add a token to tokenArray */
            optionArgs[optionArgsIdx++] = optionArgsToken;
        }

#ifdef DEBUGGING
            printf("Token: %s", optionArgs[0]);
            for (int i = 1; i < optionArgsIdx; ++i) {
                printf(", %s", optionArgs[i]);
            }
            putchar('\n');
#endif

        if (optionArgs[0] == NULL) {
            /** no program path. do nothing. */
            continue;
        }

        /** if "quit" is inputted, exit the program. */
        if (strncmp(optionArgs[0], "quit", sizeof("quit")) == 0) {
            exit(0);
        }

        /** fork and execute */
        pid_t pid = fork();
        if (pid == 0) {
            /** child proess */
            if (execvp(optionArgs[0], optionArgs)) {
                /** invalid input */
                printf("%s: command not found\n", optionArgs[0]);

                /** terminate child process */
                exit(0); 
            }
        } else if (pid > 0){
            /** parent process */
            (*numberOfChildProcesses)++;
        } else {
            /** parent process, fork() error */
            ERROR_MSG("fork() is failed.");
        }
        free(optionArgs);
    }
    free(statementArgs);
}
