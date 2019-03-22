#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <stdbool.h>

int forknife_cd(char **args);
int forknife_path(char **args);
int forknife_exit(char **args);
char path[512] = "/bin";

//built-in commands are placed into the char array 'builtin_str[]'
//the pointer function 'builtin_func[]' contains the commands and corresponding functions
char *builtin_str[] ={ "cd", "path", "exit" };
int (*builtin_func[])(char **) = {
	&forknife_cd,
	&forknife_path,
	&forknife_exit
};
int forknife_cd(char **args){

if(args[1] == NULL){
fprintf(stderr, "forknife: expected argument to \"cd\"\n");
}

else{
if(chdir(args[1]) != 0){
perror("forknife");
}
}
return 1;
}
//this is a built-in command to change the path
int forknife_path(char **args){
	int counter = 1;
	char null = '\0';

	if (args[1] == NULL){
		path[0] = null;
	}
	else{
		path[0] = null;
		while(args[counter] != NULL){
			strcat(path, args[counter]);
			strcat(path, ":");
			counter = (counter + 1);
		}
	}
	return 1;
}
//this is a built-in command to exit by returning zero
int forknife_exit(char **args)
{
	return 0;
}

//function that will launch a program and wait for it to terminate
int forknife_launch(char **args)
{
	pid_t pid;

	char right_arrow[3] = ">", null = '\0', out_file[512];
	int  redir_output = 0, out_counter = 0;
	int i = 0;
	int status;
	int found = 0;
	char wholename[512];
	char temp[512];
	char *next_piece;
	int parallel_counter = 0;
	bool carrot_found = false;
	bool ampersand_found = false;
	char ampersand[3] ="&";
	char parallel_cmd[512];
	
	//check if there's any ampersand 
	while(args[parallel_counter] != NULL){
		//ampersand
		parallel_cmd[0] = null;
		printf("You want to parallel huh\n");
		strcat(parallel_cmd, args[(parallel_counter)]);
		printf("command entered: %s \n", parallel_cmd);
		ampersand_found = true;
		parallel_counter++;
	}
	
	printf("%s \n", parallel_cmd);

	//checking for right arrow (input)
	while(args[out_counter] != NULL){
		if(strcmp(args[out_counter], right_arrow) == 0) {	
			//found >
			out_file[0] = null;
			printf("You want to redirect output, huh\n");
			strcat(out_file, args[(out_counter+1)]); //put the file name into out_file
			printf("This is where you said you want output going to: %s\n", out_file);
			carrot_found = true;
			break;
		}
		out_counter = (out_counter + 1);
	}
	
	/*This general code just adds the command at the end of the path after tokening the command line. After that, it searches for an exectuable. If it doesn't at first, there
	will be a loop that makes sure that there is no executable to be found. But, if there is one -- we will fork it and exec it if fork == 0*/

	strncpy(temp, path, 511);

	printf("path variable currently contains: %s\n", path);
	next_piece = strtok(temp, ":");
	printf("next directory to check with 'access()': %s\n", next_piece);

	snprintf(wholename, 511, "%s/%s", next_piece, args[0]);
	printf("Looking for: %s\n", wholename);
	printf("%s\n", next_piece);

	if(access(wholename, X_OK) == 0) {
		printf("YAY, found executable in: %s\n", wholename);
		found = 1;//found executable
	}
	else {
		while(next_piece != NULL) {
			next_piece = strtok(NULL, ":");
			printf("next directory to check with 'access()': %s\n", next_piece);
			snprintf(wholename, 511, "%s/%s", next_piece, args[0]);
			printf("Looking for: %s\n", wholename);
			if(access(wholename, X_OK) == 0) {
				printf("YAY, found executable in: %s\n", wholename);
				found = 1;//found executable
				break;
			}
		}
	}
	if(found) {
		printf("Gonna fork and execv for: %s\n", wholename);
		pid = fork();
		printf("should be redircting \n");
		if (pid == 0) {
			//child process
			if(carrot_found == true){
				printf("found carrot\n");
				close(STDOUT_FILENO); 
				int fd = open(out_file, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
				printf("This printed into an output file");
				printf("%d \n", fd);
				int save_stdout = dup(1);
				int save_stderr = dup(2);
				dup2(fd, 1);
				dup2(fd, 2);
		
				if(fd < 0){
					printf("FILE ERROR");
				}
				char *myargs[2] = {wholename, NULL};
							
				execvp(myargs[0], myargs);
				
				dup2(save_stdout, fd);
				dup2(save_stderr, fd);
			}
			else if(execv(wholename, args) == -1) {
				perror("forknife");
			}
			exit(EXIT_FAILURE);
		} else if (pid < 0) {
			//error forking
			perror("forknife");
		} else {
			//parent process
			do {
				waitpid(pid, &status, WUNTRACED);
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		}
		return 1;
	}
	else {
		perror("forknife");
		return 1;
	}

}
int forknife_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
//execute built-in command or launch program
int forknife_execute(char **args)
{
	int i;
	if (args[0] == NULL) {
		//an empty command was entered
		return 1;
	}
	for(i = 0; i < forknife_num_builtins(); i++) {
		if(strcmp(args[0], builtin_str[i]) == 0) {//if the command is built-in
			return (*builtin_func[i])(args);
		}
	}
	return forknife_launch(args);//otherwise, execv
}

#define FORKNIFE_RL_BUFSIZE 1024
char *forknife_read_line(void)
{
	char *line = NULL;
	ssize_t bufsize = 0;//have getline allocate a buffer for us
	getline(&line, &bufsize, stdin);
	return line;
}

#define FORKNIFE_TOK_BUFSIZE 64
#define FORKNIFE_TOK_TOK_DELIM " \t\r\n\a"

char **forknife_split_line(char *line){
	int bufsize = FORKNIFE_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if(!token){
		fprintf(stderr, "allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, FORKNIFE_TOK_TOK_DELIM);
	while(token != NULL){
		tokens[position] = token;
		printf("%s \n", tokens[position]);
		position++;	

	if(position >= bufsize) {
		bufsize += FORKNIFE_TOK_BUFSIZE;
		tokens = realloc(tokens, bufsize * sizeof(char*));
		if(!tokens) {
			fprintf(stderr, "allocation error\n");
			exit(EXIT_FAILURE);
		}
	}

	token = strtok(NULL, FORKNIFE_TOK_TOK_DELIM);
	
	}//while
	tokens[position] = NULL;
	return tokens;
}//splitline

void forknife_loop(void) {
	char *line;
	char **args;
	int status;

	do{
		printf("forknife> ");
		line = forknife_read_line();
		args = forknife_split_line(line);
		status = forknife_execute(args);

		free(line);
		free(args);

	}while (status);

}//forknife_loop

int main(int argc, char **argv) {

	forknife_loop();

	return EXIT_SUCCESS;

}

