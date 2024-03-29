#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

//#define STDOUT 1
#define FREE(X) if(X) free((void*)X)

typedef struct variableList
{
    char* var;
    char* value;
    struct variableList* next;
} variableList;

void processSetCmd(char* words[], variableList* headVar);
void processPipeCmd(char* words[]);
int checkVarInCmd(char* command[], int count);
void addNewVar(variableList* head, char* newVar, char* varValue);
char* getVarValue(char* var, variableList* head);
int replaceVarInCmd(char* command[], int count, int num, const char* newValue);
static char* strCopy(const char* string1);
void printVariables(variableList* head);

int main(int argc, char *argv[])
{
    char buf[132];
    char *words[50] = {'\0'};
    int numwords;
    int status;
    int rc;
    char *cptr;
    char *path = "/bin/";
    char progpath[20];
    variableList* headVar = (variableList*) calloc(1, sizeof(variableList));
    int bigLoopBreak = 0;
    int printTermination = 1;
    //char* setCmdArgsPtr;
    //int varIndex;
    printf("\nWelcome to my shell.Welcome to my shell (press \"print\" to print all variables, \"load $filename\" to load a sequence of commands from a text file).\n\n-> ");
    while (gets(buf))
    {
        printf("BUF IS : \"%s\"\n", buf);
        // fork child to exec command, parent waits for
        if (fork()) {
            // parent executes here
            wait(&status); // just wait for the child
        }
        else
        {
            // child executes here
            // Split line into words using strtok()
            numwords=0;
            cptr=strtok(buf," ");
            while (cptr!=NULL)
            {
                words[numwords++]=strdup(cptr);
                cptr=strtok(NULL," ");
            } // end while

            strcpy(progpath, path);
            strcat(progpath, words[0]);
            strcpy(words[0], progpath);

            int index;
            int loopBreak = 0;

            int count;
            for (count = 0; count < 50; count++)
            {
                if (words[count] == NULL)
                    break;
            }
            if (loopBreak)
            {
                loopBreak = 0;
                continue;
            }

            //******* process "SET" command *******
            if (strcmp(words[0], "/bin/set") == 0)
            {
                processSetCmd(words, headVar);
                printf("-> ");
                continue;
                //} //end if $ sign
            } //end if "set"

            if (strcmp(words[0], "/bin/print") == 0)
            {
                printVariables(headVar);
            }
            // ******** process PRE-LOADING file ********
            if (strcmp(words[0], "/bin/load") == 0)
            {
                if (argc <= 1)
                    printf("wrong loading format\n");
                char const* const fileName = words[1]; /* should check that argc > 1 */
                printf("File name : %s\n", fileName);
                FILE* file = fopen(fileName, "r"); /* should check the result */
                char line[132];
                char *newWords[50] = {'\0'};
                char* line_ptr;
                char progpath[20];

                while (fgets(line, 132, file))
                {
                    char *pos;
                    line[strlen(line) - 1] = '\0';
                    printf("line is \"%s\"\n", line);
                    int wordPos=0;
                    line_ptr = strtok(line, " ");
                    while (line_ptr != NULL)
                    {
                        newWords[wordPos] = strdup(line_ptr);
                        printf("newWords %i is : %s\n", wordPos, newWords[wordPos]);
                        wordPos++;
                        line_ptr = strtok(NULL, " ");
                    } // end while
                    printf("####\n");
                    strcpy(progpath, path);
                    strcat(progpath, newWords[0]);
                    strcpy(newWords[0], progpath);

                    processSetCmd(newWords, headVar);
                } //end while

                fclose(file);
                printf("-> ");
                continue;
            } //end pre-loading

            //int count;
            for (count = 0; count < 50; count++)
            {
                if (words[count] == NULL)
                    break;
            }

            // ******** process commands with PIPE *********
            int k = 0;
            while (words[k] != NULL)
            {
                if (strcmp(words[k], "|") == 0)
                {
                    if (words[k+1] == NULL)
                        printf("Invalid pipe command!\n");
                    char *path = "/bin/";
                    char progpath[20];
                    strcpy(progpath, path);
                    strcat(progpath, words[k+1]);
                    strcpy(words[k+1], progpath);

                    processPipeCmd(words);
                    bigLoopBreak = 1;
                    break;
                }
                    //************ process OUTPUT REDIRECTION *********
                else if (strcmp(words[k], ">") == 0)
                {
                    if ((index = checkVarInCmd(words, count)) != 0)
                    {
                        char *var = words[index];
                        var++;
                        char *value = getVarValue(var, headVar);
                        if (value == NULL) {
                            loopBreak = 1;
                            char *var = words[index];
                            printf("No variable available: %s\n", var);
                            break;
                        } else if (!replaceVarInCmd(words, count, index, value)) {
                            loopBreak = 1;
                            printf("There is no such index: %u.\n", index);
                            break;
                        }
                    } //end process of variable in output direction

                    if (words[k+1] == NULL)
                        printf("Invalid pipe command!\n");
                    close(STDOUT_FILENO);
                    fopen(words[k+1], "w");

                    char *legalCmd[20];
                    int m = 0;
                    int n = 0;
                    while (words[m] != NULL)
                    {
                        if (strcmp(words[m], ">") == 0)
                            m++;
                        else
                            legalCmd[n] = words[m];
                        m++;
                        n++;
                    }
                    rc=execv(legalCmd[0],legalCmd);
                    bigLoopBreak = 1;
                    break;
                }
                else
                    k++;
            } //end while for PIPE & output direction

            if (bigLoopBreak)
            {
                //printf("-> ");
                printTermination = 0;
                break;
            }


            //********* process command with VARIABLE **********
            while ((strcmp(words[0], "set") != 0) && (index = checkVarInCmd(words, count)) != 0)
            {
                char *var = words[index];
                var++;
                char *value = getVarValue(var, headVar);
                if (value == NULL) {
                    loopBreak = 1;
                    char *var = words[index];
                    printf("No variable available: %s\n", var);
                    break;
                } else if (!replaceVarInCmd(words, count, index, value)) {
                    loopBreak = 1;
                    printf("There is no such index: %u.\n", index);
                    break;
                }
            } //while

            words[numwords]=NULL;

            rc=execv(words[0],words);
            // check to see if the exec failed & rechild ldport
            if (rc!=0)
            {
                printf("Invalid command.\n");
                // perror("Invalid command.");
            } // end if
            // no need for an else since if execv
            //succeeds we don't get here
            exit(0);
        } // end if
        printf("-> ");
    } // end while
    if (printTermination == 1)
        printf("\n\nShell Terminating.\n\n");
} // end main

void processSetCmd(char* words[], variableList* headVar)
{
    if (strpbrk(words[1], "$") != 0)
    {
        //printf("IN SET\n");
        int count;
        for (count = 0; count < 50; count++)
        {
            if (words[count] == NULL)
                break;
        } //end for

        if (count != 2)
        {
            printf("Error: Set command is in the wrong format!\n");
            int i;
            for (i = 0; i < count; i++)
                words[i] = NULL;
        }
        else
        {
            int rightFormat = 0;
            int k;
            //the case user didn't enter '='
            for (k = 0; k < strlen(words[1]); k++)
            {
                if ((words[1])[k] == '=')
                {
                    rightFormat = 1;
                    break;
                }

            } //end for

            if (rightFormat == 0)
                printf("Error: Set command is in the wrong format!\n");

            char* setCmdArgsPtr;
            char* setCmdArgs[2] = {'\0'};

            int varIndex = 0;

            setCmdArgsPtr = strtok(words[1], "=");
            while (setCmdArgsPtr!=NULL)
            {
                setCmdArgs[varIndex++] = strdup(setCmdArgsPtr);
                setCmdArgsPtr = strtok(NULL,"=");
            }
            setCmdArgs[0]++;

            char *key = (char *) calloc(1, strlen(setCmdArgs[0]) + 1);
            char *value = (char *) calloc(1, strlen(setCmdArgs[1]) + 1);
            strcpy(key, setCmdArgs[0]);
            strcpy(value, setCmdArgs[1]);
            addNewVar(headVar, key, value);
            int i;
            for (i = 0; i < count; i++)
            {
                printf("adsfa %i: %s\n", i, words[i]);
                words[i] = NULL;
            }
            setCmdArgs[0]--;
            varIndex = 0;
            free(setCmdArgsPtr);
        } //else
    } //end if
} //end processSetCmd

void processPipeCmd(char* words[])
{
    int fileDes_0[2]; //filedes
    int fileDes_1[2]; //filedes2
    int numCmds = 0;
    char* command[256];
    int pid;
    int error = -1;
    int stop = 0; //end
    //get the number of commands
    int i = 0; //instead of l
    while (words[i] != NULL)
    {
        if (strcmp(words[i], "|") == 0)
            numCmds++;
        i++;
    }
    numCmds++;

    //loop through each pari of commands
    //int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    while (words[j] != NULL && stop != 1)
    {
        k = 0;
        while (strcmp(words[j], "|") != 0)
        {
            command[k] = words[j];
            j++;
            if (words[j] == NULL)
            {
                stop == 1;
                k++;
                break;
            }
            k++;
        } //end while
        command[k] = NULL;
        j++;

        //int l = 0; //instead of i
        //set up the pipe to connect two different commands
        if (l % 2 != 0)
            pipe(fileDes_0);
        else
            pipe(fileDes_1);

        pid = fork();

        if (pid == -1)
        {
            if (l != numCmds - 1)
            {
                if (l % 2 != 0)
                    close(fileDes_0[1]);
                else
                    close(fileDes_1[1]);
            }
            printf("Child process could not be created!\n");
            return;
        } //end if pid == -1
        if (pid == 0)
        {
            //first comand
            if (l == 0)
                dup2(fileDes_1[1], STDOUT_FILENO);
                //check if we are in the last command
            else if (l == numCmds -1)
            {
                if (numCmds % 2 != 0)
                    dup2(fileDes_0[0], STDIN_FILENO);
                else
                    dup2(fileDes_1[0], STDIN_FILENO);
            }
            else //middle commands
            {
                if (l % 2 != 0)
                {
                    dup2(fileDes_1[0], STDIN_FILENO);
                    dup2(fileDes_0[1], STDOUT_FILENO);
                }
                else
                {
                    dup2(fileDes_0[0], STDIN_FILENO);
                    dup2(fileDes_1[1], STDOUT_FILENO);
                }
            } //end else

            if (execvp(command[0], command) == error)
                kill(getpid(), SIGTERM);
        } //end pid == 0

        //closing descriptors on parent
        if (l == 0)
            close(fileDes_1[1]);
        else if (l == numCmds - 1)
        {
            if (numCmds % 2 != 0)
                close(fileDes_0[0]);
            else
                close(fileDes_1[0]);
        }
        else
        {
            if (l % 2 != 0)
            {
                close(fileDes_1[0]);
                close(fileDes_0[1]);
            }
            else
            {
                close(fileDes_0[0]);
                close(fileDes_1[1]);
            }
        }
        waitpid(pid, NULL, 0);
        l++;
    } //end big while
} //end processPipeCmd

int checkVarInCmd(char* command[], int count)
{
    if (command == NULL)
        return 0;

    int index;
    for (index = 0; index < count; index++)
    {
        if (strpbrk(command[index], "$") != 0)
            return index;
    }
    return 0;
} //checkVarInCmd

void addNewVar(variableList* head, char* newVar, char* varValue)
{
    //if the variable list is empty, then we have new head
    if (head->var == NULL)
    {
        head->var = newVar;
        head->value = varValue;
        head->next = NULL;
        return;
    }
    //if the variable list is not empty, then find the last item or if the variable's already in the list
    while (strcmp(head->var, newVar) != 0 && head->next != NULL)
        head = head->next;
    if (head->next != NULL || (head->next == NULL && strcmp(head->var, newVar) == 0))
    {
        free(head->var);
        free(head->value);
        head->var = newVar;
        head->value = varValue;
    }
        //add the new var to the end if the it's not in the list already
    else
    {
        variableList* newTail = (variableList*) malloc(sizeof(variableList));
        newTail->var = newVar;
        newTail->value = varValue;
        newTail->next = NULL;
        variableList* curr = head;
        while (curr->next != NULL)
        {
            curr = curr->next;
        }
        curr->next = newTail;
    }
} //addNewVar

char* getVarValue(char* var, variableList* head)
{
    if (head == NULL)
        return NULL;
    variableList* curr = head;
    while (curr != NULL)
    {
        if (curr->var == NULL)
            return NULL;
        if (strcmp(var, curr->var) != 0)
            curr = curr->next;
        else
            break;
    }
    return curr->value;
} //getVarValue

int replaceVarInCmd(char* command[], int count, int num, const char* newValue)
{
    if (command == NULL)
        return 0;
    if (num >= count)
        return 0;
    FREE(command[num]);
    ((char**)command)[num] = strCopy(newValue);
    return 1;
}


/* Replaces the variable with the saved string value */
/* Returns 0 if num is out-of-range, otherwise - returns 1 */
static char* strCopy(const char* string1)
{
    char* string2 = (char*) malloc(strlen(string1) + 1);
    strcpy(string2, string1);
    return string2;
}

void printVariables(variableList* head)
{
    if(head == NULL)
    {
        printf("There is no saved variables!\n");
        return;
    }
    if(head->var == NULL)
    {
        printf("There is no such variable!\n");
        return;
    }
    while(head != NULL)
    {
        printf("%s=%s\n", head->var, head->value);
        head = head->next;
    }
}
