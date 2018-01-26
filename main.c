#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FREE(X) if(X) free((void*)X)

typedef struct variableList
{
    char* var;
    char* value;
    struct variableList* next;
} variableList;

int checkVarInCmd(char* command[]);
void addNewVar(variableList* head, char* newVar, char* varValue);
char* getVarValue(char* var, variableList* head);
int replaceVarInCmd(char* command[], int num, const char* newValue);
static char* strCopy(const char* string1);

int main(int argc, char *argv[])
{
    char buf[132];
    char *words[50];
    int numwords;
    int status;
    int rc;
    char *cptr;
    variableList* headVar = (variableList*) calloc(1, sizeof(variableList));
    printf("\nWelcome to my shell.\n\n-> ");
    while (gets(buf))
    {
        printf("HEAD AGAIN: %s\n", headVar->value);
        // fork child to exec command, parent waits for
        if (fork()) {
            // parent executes here
            wait(&status); // just wait for the child
            //printf("*******%s\n", words[0]);
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
            // *****************
            //if there is a variable in the user's command
            printf("*******123%s\n", words[0]);
            //SOMETHING"S WRONG WITH checkVarInCmd
            int index;
            int loopBreak = 0;
            //printf("Number: %i\n", checkVarInCmd(words));
            printf("UMABAFA\n");

            //if ((index = checkVarInCmd(words)) != 0) {
            while ((strcmp(words[0], "set") != 0) && (index = checkVarInCmd(words)) != 0) {
                printf("adfafawefr %i\n", index);
                char *var = words[index];
                var++;
                char *value = getVarValue(var, headVar);
                if (value == NULL) {
                    loopBreak = 1;
                    char *var = words[index];
                    printf("No variable available: %s\n", var);
                    break;
                } else if (!replaceVarInCmd(words, index, value)) {
                    loopBreak = 1;
                    printf("There is no such index: %u.\n", index);
                    break;
                }
                else
                {
                    printf("@@@@%s\n", value);
                }
            } //while
            //}
            printf("UMABAFA1\n");
            //printf("*******123%s\n", words[0]);
            //in case the variable in the command does not exist, quit the loop
            if (loopBreak)
            {
                printf("CON CAC\n");
                loopBreak = 0;
                continue;
            }
            //process set command
            printf("WTF: %s\n", words[0]);
            if (strcmp(words[0], "set") == 0)
            {
                if (strpbrk(words[1], "$") != 0) {
                    printf("IN SET\n");
                    int count = 0;
                    while (words[count] != NULL)
                        count++;
                    if (count != 2) {
                        printf("Error: Set command is in the wrong format!\n");
                        for (int i = 0; i < count; i++)
                            words[i] = NULL;
                    } else {
                        char* setCmdArgsPtr;
                        char* setCmdArgs[2];
                        int varIndex;
                        setCmdArgsPtr = strtok(words[1],"=");
                        while (setCmdArgsPtr!=NULL)
                        {
                            setCmdArgs[varIndex++]=strdup(setCmdArgsPtr);
                            setCmdArgsPtr=strtok(NULL," ");
                        }
                        setCmdArgs[0]++;
                        printf("SET1: %s\n", setCmdArgs[0]);
                        printf("SET2: %s\n", setCmdArgs[1]);
                        printf("IN SET ELSE\n");

                        char *key = (char *) calloc(1, strlen(setCmdArgs[0]) + 1);
                        char *value = (char *) calloc(1, strlen(setCmdArgs[1]) + 1);
                        strcpy(key, setCmdArgs[0]);
                        strcpy(value, setCmdArgs[1]);
                        addNewVar(headVar, key, value);
                        printf("HEAD: %s\n", headVar->value);
                        for (int i = 0; i < count; i++)
                            words[i] = NULL;
                    }
                    printf("-> ");
                    continue;
                }
            }

//*************
            //printf("*******123%s\n", words[0]);
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
    printf("\n\nShell Terminating.\n\n");
} // end main

int checkVarInCmd(char* command[])
{
    int count = 0;
    if (command != NULL)
    {
        while (command[count] != NULL)
            count++;
    }
    else
    {
        return 0;
    }
    int index = 0;
    for (; index < count; index++)
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
        head->next = (variableList*) malloc(sizeof(variableList));
        head = head->next;
        head->next = NULL;
        head->value = newVar;
        head->value = varValue;
    }
} //addNewVar

char* getVarValue(char* var, variableList* head)
{
    while (head != NULL)
    {
        if (head->var == NULL)
            break;
        if (strcmp(var, head->var) != 0)
            head = head->next;
        else
            break;
    }
    if (head== NULL)
        return NULL;
    return head->value;
} //getVarValue

int replaceVarInCmd(char* command[], int num, const char* newValue)
{
    int count = 0;
    if (command != NULL) {
        while (command[count] != NULL)
            count++;
    }
    else
    {
        return 0;
    }
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
