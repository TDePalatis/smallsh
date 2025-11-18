

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

int forgroundONLY = 0; // Toggle for SIGTSTP

/**
 * Command line structure.
 * Adapted from OSU CS 374 example.
 */
struct command_line
{
	// user inputted characters
	char *argv[MAX_ARGS + 1];
	// length of command
	int argc;
	// file marked for input within command
	char *input_file;
	// file marked for output within command
	char *output_file;
	// boolean for background processing
	bool is_bg;
};

/**
 * Background PID structure for linked list.
 */

struct bgPID {
	// the generated PID
	pid_t pid;
	// its exit status
	int exitStatus;
	// boolean for if it is finished
	int finished;
	// linked list functionality
	struct bgPID *next;
};

/**
 * Structure for creating a background PID for a linked list.
 */

struct bgPID* createBgPID(pid_t pid, int exitStatus, int finished){
        
    struct bgPID* currentBgPID = malloc(sizeof(struct bgPID));
        
    currentBgPID->pid = pid;
    
	currentBgPID->exitStatus = exitStatus;
        
    currentBgPID->finished = finished;

    return currentBgPID;
};

/**
 * Uses bgPID linked list to check the status of all background PID.
 * If any have finished, their exiting signal is checked and their status returned.
 */

int backgroundPidCheck(struct bgPID* bgPID) {
	
	while (bgPID != NULL) {

		int status;
		
		pid_t check = waitpid(bgPID->pid, &status, WNOHANG);

		if (check > 0) {
			bgPID->finished = 1;
			
			if (WIFEXITED(status)) {
				printf("background pid %d is done: exit value %d\n", bgPID->pid, WEXITSTATUS(status));
				return 0;
			}
			if (WIFSIGNALED(status)) {
				printf("background pid %d is done: terminated by signal %d\n", bgPID->pid, WTERMSIG(status));
				return status;
			}

		}
		bgPID = bgPID->next;
	}
	return 0;
}

int cdCheck (char * directory) {
	// enironment variable to set new directory 
	char * current = "PWD";
	// array to store the changed directory
	char directoryCheck[INPUT_LENGTH];

	// if the directory change is successful, it will be added to directoryCheck for the function of setenv
	if (chdir(directory) == 0) {

		getcwd(directoryCheck, sizeof(directoryCheck));
		setenv(current, directoryCheck, 1);
		return 0;
		}
		
	else {

	}
return 1;
}

/**
 * A sample program for parsing a command line.
 * Adapted from OSU CS 374 example.
 */

struct command_line *parse_input()
{
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	// Tokenize the input
	char *token = strtok(input, " \n");

	while(token){
		if(!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}

	return curr_command;
}

/**
 * A program for parsing the possible executable commands in a directory. 
 * Returns the full filepath for the inputted command if it exists.
 */

char* executeCheck (int argc, char *argv[]) {

	//list of options from tne PATH environment
	char * path = getenv("PATH");
	// a copy is made of path to prevent any error from alterations
	char * pathOptions = strdup(path);
	// Place saver for strtok_r tokens
	char* savePtr1;
	// comparision value for (null)
    int *ptr = '\0';
	// boolean value for if the path exisits and is executable
    int exist;
	// the inputted command
    char* userInput = argv[0];
	// for string combination
    char* dash = "/";
	// intial character value for path set as error - to be replaced by found path
	char* finalPath = "error";
	
	// Parses through pathOptions

    char* token = strtok_r(pathOptions, ":", &savePtr1);

    while (token) {

        if (token != (char*)ptr) {
			// combines the path with the command for execution testing
            char* fullPath = calloc(strlen(token) + strlen(dash) + strlen(userInput) + 1, sizeof(char));
    
            strcpy(fullPath, token);
            strcat(fullPath, dash);
            strcat(fullPath, userInput);
            
			// if successful, saved to final Path
            exist = access(fullPath, X_OK);
            if (exist == 0) {
				finalPath = fullPath;
				break;
            }
        
		    token = strtok_r(NULL, ":", &savePtr1);
        }
    }

	return finalPath;
	
}

/**
 * Handler for SIGNINT
 * Adapted from OSU CS 374 example.
 */
void handle_SIGINT(int signo){
  
	char message[25] = "terminated by signal ";
	// converts the signo to char and appends it to message
	snprintf(message + strlen(message), sizeof(message) - strlen(message), "%d", signo);
  
	write(STDOUT_FILENO, message, strlen(message));
   
  }


  /**
 * Handler for SIGTSTP
 * Adapted from OSU CS 374 example.
 */
void handle_SIGTSTP(int signo){
	
	if (forgroundONLY == 0 ) {
		char message[55] = "\nEntering foreground-only mode (& is now ignored)\n";
  
		write(STDOUT_FILENO, message, strlen(message));
		forgroundONLY = 1;
	}
	else {
		char messageExit[35] = "\nExiting foreground-only mode\n";
  
		write(STDOUT_FILENO, messageExit, strlen(messageExit));
		forgroundONLY = 0;
	}

	fflush(stdout);
	
	
	
  }

/**
 * A program for a small shell that will do the following: 
 * 1. Provide a prompt for running commands
 * 2. Handle blank lines and comments, which are lines beginning with the # character
 * 3. Execute 3 commands exit, cd, and status via code built into the shell
 * 4. Execute other commands by creating new processes using a function from the exec() family of functions
 * 5. Support input and output redirection
 * 6. Support running commands in foreground and background processes
 * 7. Implement custom handlers for 2 signals, SIGINT and SIGTSTP
 */

int main()
{
	struct command_line *curr_command;
	// variable for exit staus
	int exitStatus = 0;
	// value equal to (null)
	int *ptr = '\0';
	// structures to set up linked list of bgPID
	struct bgPID *bgPIDHead = NULL; 
    	struct bgPID *bgPIDTail = NULL;
    	struct bgPID *bgPIDNew = NULL;
	// structure for SIGNINT handler
	struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, ignore_action = {0};

	// Fill out the SIGINT_action struct
  
	// Register handle_SIGINT as the signal handler
	SIGINT_action.sa_handler = handle_SIGINT;
  
	// Block all catchable signals while handle_SIGINT is running
	sigfillset(&SIGINT_action.sa_mask);
  
	// No flags set
	SIGINT_action.sa_flags = 0;
  
	sigaction(SIGINT, &SIGINT_action, NULL);

	// Fill out the SIGTSTP_action struct
  
   	 // Register handle_SIGTSTP as the signal handler
    	SIGTSTP_action.sa_handler = handle_SIGTSTP;
  
   	 // Block all catchable signals while handle_SIGTSTP is running
   	 sigfillset(&SIGTSTP_action.sa_mask);
  
   	 // No flags set
   	 SIGTSTP_action.sa_flags = 0;
  
   	 sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  
	// The ignore_action struct as SIG_IGN as its signal handler
	ignore_action.sa_handler = SIG_IGN;

	// main loop for running commands
	while(true)
	{
		int checkBGSwitch = forgroundONLY;
		// check for background pid status
		if (bgPIDHead != NULL) {
			int checkStatus = backgroundPidCheck(bgPIDHead);
			if (checkStatus != 0) {
				exitStatus = checkStatus;
			} 
		}
		
		curr_command = parse_input();

		int isBgCopy;
		isBgCopy = curr_command->is_bg;


		// checks if SIGTSTP has just been activated
		if (checkBGSwitch != forgroundONLY) {
			
		}
		// checks if the input is an empty line
		else if (curr_command->argv[0] == (char*)ptr) { 
		
		}    
		// checks if the input is a comment
		else if (strncmp(curr_command->argv[0],"#", 1) == 0) {
		//	printf("comment");
		}
		// checks if the input is custom exit command
		else if (strcmp(curr_command->argv[0],"exit") == 0) {
			// kills all active signals
			kill(0, SIGTERM);
			// ends program
			exit(0);
		}
		// checks if the input is custom cd command
		else if (strcmp(curr_command->argv[0],"cd") == 0) {

			// if input is only cd, it sets the directory to the value of environment variable "HOME"
			if (curr_command->argv[1] == (char*)ptr) {

				char * change = getenv("HOME");

				chdir(change);
				exitStatus = 0;

				  }
			// if cd has an arguement, it checks its validity with cdCheck. Supports relative and absolute paths  
			else {
				exitStatus = cdCheck(curr_command->argv[1]);
			}
		}

		// checks if the input is custom status command
		else if (strcmp(*curr_command->argv,"status") == 0) {
			
			// returns exit value based on exitStatus
			if (WIFEXITED(exitStatus)) {
				printf("exit value %d\n", WEXITSTATUS(exitStatus));
			}
			// returns termination signal based on exitStatus
			else if (WIFSIGNALED(exitStatus)) {
				printf("terminated by signal %d\n",WTERMSIG(exitStatus));
			}
			
			
		}
		// instructions for all other commands
		else {
			// memory for final filepath
			char* filePath;
			// runs executeCheck to check if input is a proper command
			filePath = executeCheck(curr_command->argc, curr_command->argv);

			// continues if the result is not error
			// code for parent and child processing  Adapted from OSU CS 374 example.
			if (strcmp(filePath,"error") != 0) {
				// memory for child status 
				int childStatus;
				// parent allows for SIGTSTP command
				sigaction(SIGTSTP, &SIGTSTP_action, NULL);

				// ignores background commands if toggled
				if (forgroundONLY == 1) {
					curr_command->is_bg = 0;
					}
				else if (forgroundONLY == 0) {
					curr_command->is_bg = isBgCopy;
					}

				// parent ignores SIGINT in background and catches it in forground
				if (curr_command->is_bg == 1) {
					sigaction(SIGINT, &ignore_action, NULL);
				  }
				else {
					sigaction(SIGINT, &SIGINT_action, NULL);
					fflush(stdout);
				  }


				// Fork a new process
				pid_t spawnPid = fork();
  
				switch(spawnPid){
				// error if fork fails
				case -1:
				  perror("fork()\n");
				  exitStatus = 1;
				  exit(1); 
				  break;
				// The child process executes this branch
			  	case 0:
				// The child ignores the SIGTSTP command	
				  sigaction(SIGTSTP, &ignore_action, NULL);

				// ignores background commands if toggled
				if (forgroundONLY == 1) {
					curr_command->is_bg = 0;
					}
				else if (forgroundONLY == 0) {
					curr_command->is_bg = isBgCopy;
					}
		
				// The child ignores SIGINT in background and catches it in forground
				  if (curr_command->is_bg == 1) {
					sigaction(SIGINT, &ignore_action, NULL);
				  }
				  else {
					sigaction(SIGINT, &SIGINT_action, NULL);
					fflush(stdout);
				  }
				  
				 	// in the background, the child redirects unassigned input to /dev/null
				  	if (curr_command->is_bg == 1) {
						
						if (curr_command->input_file == (char*)ptr) {

							int devNullIn = open("/dev/null", O_RDONLY);
							if (devNullIn == -1) {
								perror("problem with open");
								exitStatus = 1;
								exit(1);
							}
							int resultIn = dup2(devNullIn, 0);
							if (resultIn == -1) { 
								perror("source dup2()"); 
								exitStatus = 1;
								exit(1);
							}
						}
						// in the background, the child redirects unassigned  output to /dev/null
						if (curr_command->output_file == (char*)ptr) {

							int devNullOut = open("/dev/null" , O_WRONLY | O_CREAT | O_TRUNC, 0640);
							if (devNullOut == -1) {
								printf("cannot open /dev/null for output\n"); 
								exitStatus = 1;
								exit(1);
							}
							  int resultOut = dup2(devNullOut, 1);
							  if (resultOut == -1) {
								exitStatus = 1;
								exit(1);
							}
						}
					}
				 	// if there is an assigned input file, the child redirects input to it. 
				  	if (curr_command->input_file != (char*)ptr) {

						int sourceFD = open(curr_command->input_file, O_RDONLY);
						if (sourceFD == -1) { 
						  printf("cannot open %s for input\n", curr_command->input_file); 
						  exitStatus = 1;
						  exit(1);
						}
					  
						// Redirect stdin to source file
						int result = dup2(sourceFD, 0);
						if (result == -1) { 
						  close(sourceFD); 
						  exitStatus = 1;
						  exit(1);
						}
					close(sourceFD); 
					}
					// if there is an assigned output file, the child redirects output to it. 
					if (curr_command->output_file != (char*)ptr)  {
						int targetFD = open(curr_command->output_file , O_WRONLY | O_CREAT | O_TRUNC, 0640);
						if (targetFD == -1) {
							printf("cannot open %s for output\n", curr_command->output_file); 
							exitStatus = 1;
							exit(1);
						}
						  int result = dup2(targetFD, 1);
						  if (result == -1) {
							close(targetFD);
							exitStatus = 1;
							exit(1);
						}
						close(targetFD);
					}

					// child exceutes the command
					execvp(filePath, curr_command->argv);
					// exec only returns if there is an error
					perror("error");
					exitStatus = 1;
					exit(1); 
					
				

				default:
			  	// The parent process executes this branch
					
					// if the child is in a background command, the parent prints the childs pid
			  		if (curr_command->is_bg == 1) {
						printf("background pid is %d\n", spawnPid);
						
						// a bgPID is created to track the child's progress
						bgPIDNew = createBgPID(spawnPid, 0, 0);
						
						// initializes the head of the linked list if it is the first one
						if (bgPIDHead == NULL) { 
							bgPIDHead = bgPIDNew;
							bgPIDTail = bgPIDNew;
						}
						// sets the rest of the linked list
						else{
							bgPIDTail->next = bgPIDNew;
							bgPIDTail = bgPIDNew;
						}
			  		} 
			  		else {
						// Wait for child's termination
				  		spawnPid = waitpid(spawnPid, &childStatus, 0);
						
						// checks the exit status of the child and records it
						if(WIFEXITED(childStatus)){
							exitStatus = childStatus;
						  }
					
					}
				}
			}
			// displays error message if executeCheck returns "error"
			else {
				exitStatus = 1;
				printf("%s: no such file or directory\n", curr_command->argv[0]);
				
			}
		}
	// cleans current command for next iteration
	free(curr_command);
	}
	

	return EXIT_SUCCESS;
}
