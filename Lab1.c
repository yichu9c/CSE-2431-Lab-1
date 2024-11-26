/* ****** Yihone Chu: ******** */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 40 /* 40 chars per line, limit for the shell */

/**
 * setup() reads in the next command line string stored in the input buffer,
 * separating it into distinct tokens using whitespace as delimiters. It modifies
 * the args parameter so that it holds pointers to the null-terminated strings
 * which are the tokens in the most recent user command line, as well as a NULL
 * pointer indicating the end of the argument list.
 */
void setup(char inputBuff[], char* args[], int* background) {
    int length, /* Number of characters in the command line */
        i, /* Index for inputBuff array */
        j, /* Index for args[] */
        start; /* Beginning of next command parameter */

    /* Read what the user enters */
    length = read(STDIN_FILENO, inputBuff, MAXLINE);
    j = 0;
    start = -1;

    if (length == 0)
        exit(0); /* Ctrl-d was entered, end of user command stream */

    if (length < 0) {
        perror("Error reading command");
        exit(-1); /* Terminate with error code -1 */
    }

    /* Examine every character in the input buffer */
    for (i = 0; i < length; i++) {
        switch (inputBuff[i]) {
        case ' ':
        case '\t': /* Argument separators */
            if (start != -1) {
                args[j] = &inputBuff[start]; /* Set up pointer */
                j++;
            }
            inputBuff[i] = '\0'; /* Add a null char; make a C string */
            start = -1;
            break;

        case '\n': /* Final char examined */
            if (start != -1) {
                args[j] = &inputBuff[start];
                j++;
            }
            inputBuff[i] = '\0';
            args[j] = NULL; /* No more arguments to this command */
            break;

        case '&':
            *background = 1; /* Set background flag */
            inputBuff[i] = '\0'; /* End command string */
            break;

        default: /* Some other character */
            if (start == -1)
                start = i;
        }
    }

    args[j] = NULL; /* Just in case the input line was too long */
}

int main(void) {
    char inputBuff[MAXLINE]; /* Input buffer to hold the command entered */
    char* args[MAXLINE / 2 + 1]; /* Command line arguments */
    int background; /* Equals 1 if a command is followed by '&', else 0 */
    pid_t ret_val; /* Value returned by fork() */

    while (1) { /* Infinite loop for the shell */
        background = 0; /* Reset background flag */

        printf("CSE2431Sh$ "); /* Print shell prompt */
        fflush(stdout);

        setup(inputBuff, args, &background); /* Get the command entered by the user */

        /* Fork a child process */
        ret_val = fork();

        if (ret_val < 0) { /* Error during fork */
            printf("Error: problem forking a child process.\n");
            exit(1);
        }
        else if (ret_val == 0) { /* Child process */
            if (execvp(*args, args) < 0) {
                printf("Error: problem with execvp.\n");
                exit(1);
            }
        }
        else { /* Parent process */
            if (background == 0) {
                waitpid(ret_val, NULL, 0); /* Parent waits for child */
                printf("Parent process done waiting.\n");
            }
            else {
                /* Parent returns to the top of the loop to print the prompt and call setup() again */
                continue;
            }
        }
    }

    return 0;
}
