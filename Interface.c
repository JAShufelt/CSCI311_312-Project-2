#include "Interface.h"

int main()
{
    int parentToChild[2];
    int childToParent[2];

    if(pipe(parentToChild) == -1)
    {
        printf("An error occurred establishing a pipe");
        exit(1);
    }
    if(pipe(childToParent) == -1)
    {
        printf("An error occurred establishing a pipe");
        exit(2);
    }

    int pid = fork();   //Fork process into parent and child.
    if(pid < 0)
    {
        printf("Parent process failed to fork.\n");
        exit(3);
    }

    if(pid == 0)        //In the child process
    {
        //Convert File Descriptors into char[] to pass as command line arguments.
        char PToC0[11];
        char PToC1[11];
        char CToP0[11];
        char CToP1[11];
        char BUFFER_SIZE_CHAR[11];
        snprintf(PToC0,10,"%d",parentToChild[0]);
        snprintf(PToC1,10,"%d",parentToChild[1]);
        snprintf(CToP0,10,"%d",childToParent[0]);
        snprintf(CToP1,10,"%d",childToParent[1]);
        snprintf(BUFFER_SIZE_CHAR,10,"%d",BUFFER_SIZE);

        execlp("./Server.out","./Server.out",PToC0,PToC1,CToP0,CToP1, BUFFER_SIZE_CHAR, NULL);  //UNCOMMENT OUT

    }
    else    //In the parent process
    {
        close(parentToChild[0]);
        close(childToParent[1]);
        char input[BUFFER_SIZE];
        char output[BUFFER_SIZE];
        int child_pid;
        int status;
        int w;

        //Wait for initialization completion signal from Server before allowing input.
        read(childToParent[0], output, BUFFER_SIZE);

        //Definite looping of
        while(1)
        {
            printf("\nInput Command\n>");
            fgets(input,BUFFER_SIZE,stdin);
            input[strlen(input)-1] = '\0';
            write(parentToChild[1], input, BUFFER_SIZE);

            if(strcmp(input, "exit") == 0)  //If client typed "exit" wait for server to exit.
            {
                waitpid(-1, &status, 0);
                if(WIFEXITED(status))
                {
                    int exit_status = WEXITSTATUS(status);
                    printf("Exit status of the child was %d", exit_status);
                }
                return 0;
            }

            //Loop reading and printing until "STOP READING" has been read,
            while(1)
            {
                //Before reading, check if child has exited (would not be code 0).
                w = waitpid(-1, &status, WNOHANG);
                if(w != 0)
                {
                    if(WIFEXITED(status))
                    {
                        printf("Child process exited status %d", WEXITSTATUS(status));
                        exit(5);    //Exit code 5 indicating child exited from error.
                    }
                }

                read(childToParent[0], output, BUFFER_SIZE);
                if(strcmp(output,"STOP READING") != 0)
                {
                    printf("%s\n", output);
                }
                else
                {
                    strcpy(output, "ERASE OUTPUT");
                    break;
                }
            }
        }
    }
}

