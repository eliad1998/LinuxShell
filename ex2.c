//
// Eliad Arzuan 206482622
//

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#define MAXARGS 10
#define MAXINPUTSIZE 1000
#define JOBSSIZE 20
#define DELETEDJOB (-5)


//*************************************************************************
//* Job functions
//****************************************************
typedef struct {
    pid_t pid; //The id of the process
    char ** args;//Arguments each one is string
    int numArgs;//Number of arguments.
}job;

/**
 * addJob
 * Adding a job to array of jobs
 * @param jobs the array of jobs.
 * @param add  the job we want to add.
 * @param numJobs number of jobs in the array.
 * @param size the size of the array.
 */
void addJob(job * jobs,job add, int * numJobs, int *size) {
    //The array is full so we need to realloc it
    if (*numJobs >= *size) {
        jobs = (job*) realloc(jobs, *size * 2 * sizeof(job));
        //Increasing the size of the array
        *size = *size *2;
    }
    //Adding the job
    jobs[*numJobs] = add;
    //Increasing the current number of jobs
    *numJobs= *numJobs + 1;
}

/**
 * removeJob.
 * Removes a job from the jobs array.
 * @param jobs the jobs array
 * @param del the job we want to delete
 * @param numJobs the number of current jobs in the array.
 */
void removeJob(job * jobs,job del, int * numJobs) {
    //First find the job
    int i;
    int found = 0;
    int len = *numJobs; //A little optimization
    for (i = 0; i < len; i++) {
        //We found the job
        if (jobs[i].pid == del.pid) {
            found = 1;
            break;
        }
    }

    //We found the job
    if (found) {
        //First free the memmory allocated for args
        int sizeArguments = jobs[i].numArgs;
        char ** argv = jobs[i].args;
        //Remove the job from the array by promote each job in 1
        for (int j = i; j < len; j++) {
            jobs[j] = jobs[j + 1];
        }
        //Decreasing the number of the jobs
        *numJobs = *numJobs - 1;
        job prob;
        prob.pid = DELETEDJOB;
        jobs[*numJobs] = prob;
    }
}


/**
 * printJobs.
 * Prints our jobs that did not finished if finished we will remove them from jobs array.
 * @param jobs our job array.
 * @param numJobs the number of the jobs.
 */
void printJobs(job * jobs, int *numJobs) {
    int status;
    //Moving on all jobs.
    for (int i =  0; i < *numJobs; i++) {
        //The process finished
        if (jobs[i].pid != DELETEDJOB && waitpid(jobs[i].pid, &status, WNOHANG) == jobs[i].pid) {
            if (*numJobs > 0) {
                removeJob(jobs, jobs[i], numJobs);
                //After removing the job return
                 i--;
            }
        }
            //The process did not finished.
        else if (jobs[i].pid !=DELETEDJOB && waitpid(jobs[i].pid, &status, WNOHANG) == 0 ) {
            //Printing it's id and args.
            printf("%d ", jobs[i].pid);
            for (int j = 0; j < jobs[i].numArgs; j++) {
                printf("%s ", jobs[i].args[j]);
            }
            printf("\n");
        }
    }

}
/**
 * isJob.
 * Checks if the command is a job if yes remove the & from the arguments array.
 * @param argv out arguments
 * @param sizeArguments the number of arguments we have,
 * @return 1 if the command is a job, 0 otherwise
 */
int isJob(char ** argv, int * sizeArguments) {
    if (strcmp(argv[*sizeArguments - 1], "&") == 0) {
        //Remove the & to excecute with the arguments.
        argv[*sizeArguments - 1] = NULL;
        *sizeArguments = *sizeArguments - 1;
        return 1;
    }
    return 0;

}

//*************************************************************************
//Other functions
//****************************************************

/**
 * readIntoArray.
 * Reads from the console until enter and organize it into array by spaces.
 * @param size pointer to int which the size of the string words will be stored in.
 * @return array of strings which contain our commands and arguments.
 */
char ** readIntoArray(int * size) {
    //The buffer we read into it.
    char * buffer = (char*)malloc(MAXINPUTSIZE * sizeof(char));
    //Our string.
    char * myStr;

    //Reading from the user until enter and get the length of the string.
    fgets(buffer, MAXINPUTSIZE, stdin);
    myStr = (char*)malloc(strlen(buffer) * sizeof(char));

    strcpy(myStr, buffer);
    //The array of strings.
    char ** argv = (char**)malloc(MAXARGS * sizeof(char));
    //Seperates the main string by spaces.
    char *token = strtok(myStr, " ");
    char * current = token;
    int i = 0;
    //walk through other tokens
    while( current != NULL ) {
       argv[i] = (char*) malloc(strlen(current) * sizeof(char));
       strcpy(argv[i], current);
        //Moving to the next.
       current = strtok(NULL, " ");
        i++;
    }

    //free the buffer because we don't use it anymore.
    free(buffer);
    //Storing the size of the array in size.
    *size = i;

    int s= *size;
    //Remove the new line character from each string to prevent problems
    for (i = 0; i < s; i++) {
        strtok(argv[i], "\n");
    }
    //Giving null to the last argument
    if (s < MAXARGS) {
        argv[s] = NULL;
    }


    return argv;
}
/**
 * cdCommand
 * Implements the original cd command from bash.
 * @param argv our arguments.
 */
void cdCommand(char **argv) {
    //The directory we want to get into.
    char *directory;
    //The argument of directory isn't empty.
    if ((directory= argv[1]) != NULL) {
        //The directory does not exists.
        if (chdir(directory) == -1) {
            printf("cd: %s : No such file of directory\n",directory);
        };

    } else{ //We do not have argument after cd so we will navigate to home directory.
        //Getting the environment variable of home directory.
        chdir(getenv("HOME"));
    }
}

/**
 * runCommand.
 * Running the command that in argv[0] with the arguments.
 * @param argv the arguments and the command.
 */
void runCommand(char **argv, job * jobs, int *numJobs) {
    //We need to implement cd ourselves
   if (strcmp(argv[0], "cd") == 0) {
      cdCommand(argv);
       return;
   }

    //The user only typed enter
    if (strcmp(argv[0], "\n") == 0 || strlen(argv[0]) == 0) {
        printf("\n");
        return;
    }
    //Regular commands.
    //   execvp works with the directories that specified in the PATH directories

    execvp(argv[0], argv);
    //If we got here the command was wrong.
    fprintf(stderr, "Wrong command\n");

}
int main() {
    //The size of argv array.
    int sizeArguments = 0;
    pid_t pid = 1;
    int stat;
    int numJobs = 0;
    int sizeJobs = JOBSSIZE;
    job * jobs = (job*) calloc(JOBSSIZE ,sizeof(jobs));


    int commandIsJob = 0;


    while(pid > -1) {

        printf("prompt> ");

        char ** argv = readIntoArray(&sizeArguments);

        commandIsJob = isJob(argv, &sizeArguments);

        //If we just insert enter we won't do anything.
        if (strcmp(argv[0], "\n") == 0) {
            //Showing the next iteration
            continue;
        }
        //We won't create process for jobs we just will show the jobs.
        if (strcmp(argv[0], "jobs") == 0) {
            printJobs(jobs, &numJobs);
            continue;

        }
        //The child will run the command

        if ((pid = fork()) == 0) {
            runCommand(argv, jobs, &numJobs);
        }
        //The father will wait until the children end. we do it w'שith process because excecvp open new task
        else if (pid > 0) {
            //Printing the process id
            printf("%d\n", pid);

            if(!commandIsJob) {
                //Free argv after usage
                wait(&stat);
            }

            else {
                //We won't add cd to jobs because we handle it by chdir
                if (strcmp(argv[0], "cd") != 0) {
                    job j;
                    j.args = argv;
                    //The child pid is the one returns from fork
                    j.pid = pid;
                    j.numArgs = sizeArguments;
                    addJob(jobs, j, &numJobs, &sizeJobs);
                }
            }

            printf("\n");
        }
        else { //pid <0
            fprintf(stderr,"Error with forking");
        }

    }
    //Free the jobs
    free(jobs);

    return 0;
}
