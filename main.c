#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
    char buf[132];
    char *words[50];
    int numwords;
    int status;
    int rc;
    char *cptr;
    printf("\nWelcome to my shell.\n\n-> ");
    while (gets(buf)) {
        // fork child to exec command, parent waits for
        if (fork()) {
            // parent executes here
            wait(&status); // just wait for the chi
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
