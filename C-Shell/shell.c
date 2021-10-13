#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<time.h>
#include<grp.h>
#include<pwd.h>
#include <fcntl.h>

#define max_inp 20
#define f(start,end,i) for(int i = start;i<=end;i++)
char* fileName;
char* stored_command[max_inp];
int stored = 0;

int powa(int a,int b)
{
    int result = 1;
    for(int i =0 ;i<b;i++)
    {
        result = result*a;
    }
    return result;
}

char* removing_quotes(char* input) 
{
	int l = strlen(input);

	char *out = malloc(sizeof(char) * (l+1));
	int pre = 0;
	for(int i = 0; i < l; i++) {
		if(input[i] != '"') out[pre++] = input[i];
	}
	out[pre] = NULL;
	return out;
}

char** parse(char input[]){
    int b = 0;
    for(int i = 0 ; i < strlen(input) ; i++){
        if(input[i] == '"') {
            b = b^1;
            input[i] = '\n';
        }
        else{
            if(b == 0 && input[i] == ' '){
                input[i] = '\n';
            }
        }
    }
    int index = 0;
    int buffersize = 32;
    char **tokens = malloc(buffersize * sizeof(char*));
	char *token = strtok(input, "\n");

	while(token != NULL) 
	{
		if(index == buffersize) {
			// Resize
			buffersize *= 2;
			tokens = realloc(tokens, buffersize*sizeof(char*));
		}

		tokens[index++] = token;

		token = strtok(NULL, "\n");
	}

	tokens[index] = NULL;
	return tokens;
}


void current_dir();

int processstring(char* input);

void history_save(char* input);

void history_show();

void excep_c(char** input,int l);

void retrieve_Stored();

void history_save(char* cmd);

void write_history();

void history_display(char* input);

void history_Setup();

int takeinput(char* str)
{
	fgets(str, 1023, stdin);
	int len = strlen(str);
	str[len - 1] = NULL;
	if(strlen(str) != 0)
		return 0;
	else
		return 1;		
	
}

char* initdir;
int pos = 0;
char* currdir;

int main()
{
	initdir = malloc(1023*sizeof(char));
	getcwd(initdir,1023*sizeof(char));
	char input[1023];
    while(1)
    {
	    current_dir();
        if(takeinput(input))
		  	continue;
		else{	
			history_Setup();
			 history_save(input);
			if(processstring(input))
				break;		
		}
    }
	return 0;
}

void current_dir()
{
    printf("MTL458:");
    char* currdir;
    currdir = (char*)malloc(1023*sizeof(char));
    getcwd(currdir,1023);
    int f = 0;
    if(strstr(currdir,initdir) == NULL){
        f = 1;
    }
    if(f){
        printf("%s",currdir);
    }
    else{
        printf("~%s",currdir+strlen(initdir));
    }
    printf("$ ");
    free(currdir);
}

void history_Setup() {
 
    fileName = (char*)calloc(1000, 1);
    char* path = initdir;

    strcpy(fileName, path);
    strcat(fileName, "/history.txt");
    retrieve_Stored();
}

void retrieve_Stored() {
    
    int fd = open(fileName, O_CREAT | O_RDONLY, S_IRWXU);
    char* buf;
    buf = (char*)calloc(1000, 1);
    read(fd, buf, 1000);

    if (!(fd >= 0)) {
        perror("open_history() ");
        return;
    }

    stored = 0;

    char* ptr;
    ptr = strtok(buf, "\n");
    while (ptr) {
        
        if (!(strlen(ptr) == 0)) {
            stored_command[stored] = ptr;
            stored = stored + 1;
        }
        ptr = strtok(NULL, "\n");
    }

    close(fd);
}

void history_save(char* cmd) {
    if (stored > 0)
    {
        if(strcmp(stored_command[stored - 1], cmd) == 0) 
        return;
    }

    if (!(stored >= max_inp)) {
        stored_command[stored] = cmd;
        stored = stored + 1;
    } else {
        f(0,max_inp - 1,i)
         {
            stored_command[i] = stored_command[i + 1];
        }

        stored_command[max_inp - 1] = cmd;
    }
    write_history();
}

void write_history() {

    int fd = open(fileName, O_TRUNC | O_WRONLY, S_IRWXU);

    if (!(fd >= 0)) {
        perror("Couldn't open history file while writing");
        return;
    }
    f(0,stored-1,i)
    {
        int len = strlen(stored_command[i]);
        int y = 1;
        write(fd, stored_command[i], len);
        write(fd, "\n", y);
    }
    close(fd);
}

int processstring(char input[])
{
    char temp[1023];
    char* temp1 = malloc(1023*sizeof(char));
    strcpy(temp1,input);
    char* par[1023];
    char* token;
    char oristring[1023];
    strcpy(oristring, input);
    strcpy(temp,input);
    //int len =  strlen(input);
    //temp[len] = '\n'; 
    char** args = parse(temp);
    
    int i = 0;
    par[i] = strtok(input, " ");
    
    while(par[i]!=NULL)
    {
        i++;
        par[i] = strtok(NULL, " ");
    }

    int k = 0;
    char* listofcommands[2];
    
    listofcommands[0] = "cd";
    listofcommands[1] = "history";

    for (size_t j = 0; j < 2; j++)
    {
        if(strcmp(par[0], listofcommands[j]) == 0){
           k = j + 1; 
           break;
        }
    }
    switch (k)
    {
        case 1:{
            cd_c(temp1+3);
            return 0;
        }
        case 2:{
            history_display(par[1]);
            return 0;
        }

        default:{
            pid_t pid = fork();

	if(pid < 0) 
	{
		perror("fork error");
		exit(0);
	}
	else if(pid == 0)
	{
		if(execvp(args[0], args) == -1) 
		{	
			printf("%s: command not found\n", args[0]);
		}
		
	}
	else
	{	int status;
		waitpid(pid, &status, 0);
	}
            //execvp_c(par,i);
            return 0;
        }
    } 
        
}

void cd_c(char* input)
{

    char* homedir = getenv("HOME");
    int r;
    
    if(input == NULL){
        r = chdir(initdir);
        return;
    }
    else if(!strcmp(input, "~")){
        r = chdir(initdir);
        return;
    }
        
    /*char* a;
    a = (char*)malloc(1023*sizeof(char));
    strcpy(a,"");
    if(par[1] == NULL || !strcmp(par[1], "~"))
        r = chdir(initdir);
    else
    {
        int i = 1;
        while(par[i]!=NULL)
        {
            int len = strlen(par[i]);
            if(par[i][len-1] == 92)
            {
                par[i][len-1] = NULL;
                strcat(a, par[i]);
                strcat(a, " ");
            }
            
            else
            {
                strcat(a, par[i]);
            }
            i++;
        }
        */
       char* a = removing_quotes(input);
        r = chdir(a);
    
    if(r<0){
        perror("chdir()");
    }
    free(a);
}

void history_display(char* input) {

    int n = 0;
    if(input == NULL )
    {
        n = 5;
    }    
    else
    {
        for(int i=0;i<strlen(input);i++)
        {
            n+= (input[i]-48)*powa(5,strlen(input) - 1 - i);
        }
    }

    if(n>5)
    {
        n = 5;
    }    
    
    if(stored - n > 0)
    {
        f(stored - n,stored - 1,i){
            printf("%s\n", stored_command[i]);
        }
    }
    else
    {
        f(0,stored - 1,i) {
            printf("%s\n", stored_command[i]);
        }
    }
}

void execvp_c(char** par,int l){
    char **command = malloc(10*sizeof(char*));
    
    if(command == NULL)
    {
        perror("malloc failed");
        exit(1);
    }

    char* separator = " ";
    int i = 0,stat_loc;
    for(i = 0;i < l;i++)
    {
        command[i] = par[i];
    }
    command[i] = NULL;
    int child_pid = fork();
    if(child_pid < 0)
    {
        perror("Fork failed");
    }
    if(child_pid == 0)
    {
        if(execvp(command[0],command) < 0)
        {
                perror(command[0]);
                exit(1);
        }
    }
    else
    {
        waitpid(child_pid,&stat_loc,WUNTRACED);
    }
    
    free(command);
    return;
}


