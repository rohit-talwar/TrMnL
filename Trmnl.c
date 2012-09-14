/*

Name Rohit Talwar
Assumptions : I am listing all the processes/commands input into the terminal

If a command was not able to execute then i am setting/printing the pid value of that process as = 0
If u make a mistake in using a special function like pid hist while using pipe or redirection then no error regarding them is printed

Action of quit- since quit can also be a filename for input or output hence it is difficult to identify its action
-- my assumptions with quit ---
1. if u give command like - quit < abc > asd then it will quit the program  as here the 'command' is quit
2. but if u give a command like sort < quit here quit will be treated as a file


 */

#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<signal.h>
#include<fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int errno;

typedef struct process {
	char name[100];
	int pid;
	int wait;  // all processes are by default forground process
	struct process *next;
} process ;
char home[100],pre[100];  // to store the prev working dir and the initial home directory
int mainid;  // to store pid of ./a.out
int badavala=0;  // 1 if errors are seen anywhere
int quit;  // will be -1 if quit was encountered
process *copy;
void handler(int signum){
	int status;
	int pid = waitpid(WAIT_ANY,&status,WNOHANG);
	process *temp;
	temp = copy;
	while(temp->next){
		temp=temp->next;
		if(temp->pid == pid){
			if(temp->wait == 1){    // refers to the backgrnd proc
				temp->wait = 0;
				if(status == 0)
					printf("%s with pid %d exited normally \n",temp->name,temp->pid);
				else
					printf("%s with pid %d exited abnormally \n",temp->name,temp->pid);
			}
			break;
		}
	}
}

int display(){
	char pwd[100],host[100],user[100];
	getcwd(pwd,100);
	strcpy(user,getlogin());
	gethostname(host,100);
	int a = strlen(home),i=0;
	while(a>=i){
		if(pwd[i]==home[i])
			i++;
		else
			break;
	}
	if(i>=a){
		printf("<%s@%s:~",user,host);
		while(strlen(pwd)>=i){
			printf("%c",pwd[i]);
			i++;
		}
		printf(">");
	}
	else{
		printf("<%s@%s:%s>",user,host,pwd);
	}
}

int pid_all(process *head){
	int a=0;
	while(head->next){
		a++;
		head = head->next;
		if(strcmp(head->name,"pid")==0 || strcmp(head->name,"pid all")==0 || strcmp(head->name,"pid current")==0 || strcmp(head->name,"cd")==0 || strstr(head->name,"hist") ){
			printf("command name: %s process id: %d\n",head->name,mainid);
		}else{
			printf("command name: %s process id: %d\n",head->name,head->pid);
		}
	}
	if(a==0)
		printf("No process executed so far\n");
}
int pid_current(process *head){
	int check=0;
	while(head->next){
		head=head->next;
		if(head->wait == 1){
			check =1;
			printf("currently running %s with pid- %d\n",head->name,head->pid);
		}
	}
	if(check == 0){
		printf("No background processes running right now!!\n please give a white space and then '&'\n ");
	}
}



//Action of quit- since quit can also be a filename for input or output hence it is difficult to identify its action
// therfore no quit will be checked inside the redirect function and execute2 fn <- as it will be checked before!
int fd,fd2;  // did this for debugging purpose can be placed inside the function 

int redirect(process *new,process *head){
	int i = 0;
	char *pointer,*arg[100];
	char statement[100];       // stores the initial command statement
	strcpy(statement,new->name);
	for(i=0;i<100;i++)
		arg[i] = (char *)malloc(100*sizeof(char));
	pointer = strtok(statement," \t,<,>");    // also removing the < and > 
	i=0;
	while(pointer != NULL){
		strcpy(arg[i],pointer);
		pointer = strtok(NULL," \t,<,>");
		i++;
	}
	int india=0;  // india is the index!!!!! God this is frustrating
	process *tem;
	tem = (process *)malloc(sizeof(process));
	arg[i] = NULL;  // i contains the max number of args minus the < and >
	// start implementing
	int j=0;  // index for the arguments
	if(strstr(new->name,">")){
		fd=open(arg[i-1],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		if(fd == -1){
			char *asdf = strerror(errno);  // printing the error
			printf("%s\n",asdf);
			return ;
		}
		if(strstr(new->name,"<")){     // searching for the other < 
			fd2=open(arg[i-2],O_RDONLY,S_IRUSR|S_IWUSR);
			if(fd2 == -1){
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
				return ;
			}
			strcpy(tem->name,arg[0]);
			for(india=1;india < i-2 ;india++){
				strcat(tem->name," ");
				strcat(tem->name,arg[india]);
			}
			int pid = vfork();
			if(pid == -1 ){
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
			}else if(pid == 0){
				if(dup2(fd,1) == -1){
					char *asdf = strerror(errno);  // printing the error
					printf("%s\n",asdf);
					return ;

				}
				if(dup2(fd2,0) == -1){
					char *asdf = strerror(errno);  // printing the error
					printf("%s\n",asdf);
					return ;
				}
				execute2(tem,head);
				close(fd);
				close(fd2);
				exit(1);
			}else{
				wait(NULL);
				if(quit == -1){
					exit(1);
				}
				close(fd);
				close(fd2);
				new->pid = pid;
			}
			close(fd2);
		}else{
			strcpy(tem->name,arg[0]);
			for(india=1;india < i-1 ;india++){
				strcat(tem->name," ");
				strcat(tem->name,arg[india]);
			}
			int pid = vfork();
			if(pid == -1 ){
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
			}else if(pid == 0){
				if(dup2(fd,1) == -1){
					char *asdf = strerror(errno);  // printing the error
					printf("%s\n",asdf);
					return ;
				}
				execute2(tem,head);
				close(fd);
				exit(1);
			}else{
				wait(NULL);
				if(quit == -1){
					exit(1);
				}
				close(fd);
				new->pid = pid;
			}
		}
		close(fd);
	}else if(strstr(new->name,"<")){
		fd=open(arg[i-1],O_RDONLY,S_IRUSR|S_IWUSR);
		if(fd == -1){
			char *asdf = strerror(errno);  // printing the error
			printf("%s\n",asdf);
			return ;
		}
		strcpy(tem->name,arg[0]);
		for(india=1;india < i-1 ;india++){
			strcat(tem->name," ");
			strcat(tem->name,arg[india]);
		}
		int pid = vfork();
		if(pid == -1 ){
			char *asdf = strerror(errno);  // printing the error
			printf("%s\n",asdf);
		}else if(pid == 0){
			if(dup2(fd,0) == -1){
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
				return ;
			}
			execute2(tem,head);
			close(fd);
			exit(1);
		}else{
			wait(NULL);
			if(quit == -1){
				exit(1);
			}
			close(fd);
			new->pid = pid;
		}
		close(fd);
	}
}
/*
   Note on the inclusion of > and < along with the pipe command - 
   1. < can come only in the first statement ie before the first pipe
   2. > can come only in the last statement ie after the last pipe
 */
int pipes(process *new,process *head){   
	int i = 0;
	int luffy = 0,fd;  // the file descriptor incase a > or < comes and luffy to check wether this happnd
	char *pointer,*arg[100];
	char statement[100];       // stores the initial command statement
	strcpy(statement,new->name);
	for(i=0;i<100;i++)
		arg[i] = (char *)malloc(100*sizeof(char));
	pointer = strtok(statement,"|");    // also removing the | spaces are not removed
	i=0;
	while(pointer != NULL){
		strcpy(arg[i],pointer);
		pointer = strtok(NULL,"|");  // dont wanna remove the spaces as they will be needed
		i++;
	}
	int india=0;  // india is the index!!!!! God this is frustrating
	process *tem;
	tem = (process *)malloc(sizeof(process));
	arg[i] = NULL;  // i contains the max number of args minus the |
	// start implementing
	int status;
	int pipefd[100][2];
	for(india = 0; india<i-1; india++){  // max value of india is i - 2 ie i -1 pipes for i process
		pipe(pipefd[india]);
	}
	int a=0;
	int id = vfork();
	if(id == -1 ){
		char *asdf = strerror(errno);  // printing the error
		printf("%s\n",asdf);
	} else if(id == 0){
		for(a=0;a<i-1;a++){
			if(a==0)
				continue;
			close(pipefd[a][0]);
			close(pipefd[a][1]);
		}
		if(dup2(pipefd[0][1],1) == -1){
			char *asdf = strerror(errno);  // printing the error
			printf("%s\n",asdf);
			return ;
		}
		close(pipefd[0][0]);
		process *abc;
		abc = (process *)malloc(sizeof(process));
		strcpy(abc->name,arg[0]);
		if(strstr(arg[0],"<")){
			redirect(abc,head);
			exit(1);
		}else{
			execute2(abc,head);
			close(pipefd[0][1]);
			exit(1);
		}
	}else{
		wait(NULL);
		if(quit == -1){
			exit(1);
		}
		close(pipefd[0][1]);
		new->pid = id;
	}
	for( india = 1 ; india<i-1; india++){
		id = vfork();
		if(id == -1 ){
			char *asdf = strerror(errno);  // printing the error
			printf("%s\n",asdf);
		} else if(id == 0){
			for(a=0;a<i-1;a++){
				if(a==india || a==(india-1))
					continue;
				close(pipefd[a][0]);
				close(pipefd[a][1]);
			}

			if(dup2(pipefd[india][1],1) == -1){
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
				return ;
			}
			close(pipefd[india][0]);
			if(dup2(pipefd[india-1][0],0) == -1){
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
				return ;
			}
			close(pipefd[india-1][1]);
			process *abc;
			abc = (process *)malloc(sizeof(process));
			strcpy(abc->name,arg[india]);
			execute2(abc,head);
			exit(1);
		}else{
			wait(NULL);
			if(quit == -1){
				exit(1);
			}
			close(pipefd[india][1]);
			close(pipefd[india-1][0]);
		}
	}
	id = vfork();
	if(id == -1 ){
		char *asdf = strerror(errno);  // printing the error
		printf("%s\n",asdf);
	}else if(id == 0){
		for(a=0;a<i-1;a++){
			if(a==i-2)
				continue;
			close(pipefd[a][0]);
			close(pipefd[a][1]);
		}
		if(dup2(pipefd[i-2][0],0) == -1){
			char *asdf = strerror(errno);  // printing the error
			printf("%s\n",asdf);
			return ;
		}
		close(pipefd[i-2][1]);
		process *abc;
		abc = (process *)malloc(sizeof(process));
		char sec[2][100],*p,stmt[100];
		strcpy(abc->name,arg[i-1]);
		if(strstr(abc->name,">")){
			redirect(abc,head);	
			exit(1);
		}else{
			execute2(abc,head);
			exit(1);
		}
	}else{
		wait(NULL);
		if(quit == -1){
			exit(1);
		}
		close(pipefd[i-2][0]);
	}
}

int execute2(process *new,process *head){   //  here no child are created for the exec commands but still cant use for redirect 
	signal(SIGCHLD,handler);            //  as that creates a new child for executing the dup command
	int i = 0;			    // i can also change the redirect fun to accomodate the no child creation as dup will still be done in child process
	char naruto[100];		    // also here no comments about the usage of special fns like pid hist and cd will be printed as they are now called from other processes
	strcpy(naruto,new->name);
	char *pointer,*arg[100];
	for(i=0;i<100;i++)
		arg[i] = (char *)malloc(100*sizeof(char));
	pointer = strtok(new->name," \t");
	i=0;
	while(pointer != NULL){
		strcpy(arg[i],pointer);
		pointer = strtok(NULL," \t");
		i++;
	}
	badavala = 0;
	arg[i] = NULL;  // i contains the max number of args
	strcpy(new->name,arg[0]);

	// now strcating the full command u can 

	int str;
	for(str = 1; str<i; str++){
		strcat(new->name," ");
		strcat(new->name,arg[str]);
	}
	// done strcating u can also comment this part to maintain the workablily of getting only the first name

	// checking if &, is there or not
	// value of status will be 0 if no & or | or > < is there
	// status == 1 if & present   ASSUMPTION that & will come standalone
	// status == 2 if | present
	// status == 3 if > or < present
	// status == 4 if | and > < present
	int status=0;
	if(strstr(naruto,">") || (strstr(naruto,"<"))){
		if(strstr(naruto,"|"))
			status = 4;
		else
			status = 3;
	}else if(strstr(naruto,"|")){
		status = 2;
	}else if(strstr(naruto,"&")){
		status = 1;
	}

	//  CHECK IF QUIT IS GIVEN AS A COMMAND AND NOT AS AN ARGUMENT
	if(strcmp(arg[0],"quit")==0){
		quit = -1;
		return;
	}
	// CHECK COMPLETE

	if(status == 3){
		redirect(new,head);
	}
	if(status == 4 || status == 2)
		pipes(new,head);
	if(strstr(naruto,"cd") || strstr(naruto,"hist") || strstr(naruto,"pid")){
		int err;
		if(strcmp(arg[0],"cd")==0){
			if(i==1){
				getcwd(pre,100);
				err=chdir(home);
			}else{
				if(strcmp(arg[1],"-")==0){
					char old[100];
					getcwd(old,100);
					err=chdir(pre);
					strcpy(pre,old);
				}else if(strcmp(arg[1],"~")==0){
					getcwd(pre,100);
					err=chdir(home);
				}else if(strcmp(arg[1],".")==0){
					char new[100];
					strcpy(new,home);
					int target,letter=1;
					target=strlen(new);
					while(letter<=strlen(arg[1])){
						new[target++]=arg[1][letter++];
					}
					getcwd(pre,100);
					err=chdir(new);
				}else{
					getcwd(pre,100);
					err=chdir(arg[1]);
				}
			}
			if(err==-1){
				badavala = 1;
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
			}
		}else if(strcmp(arg[0],"pid")==0){
			if(arg[1]==NULL){
				printf("Command name:./a.out process id:%d\n",mainid);
			}else if(strcmp(arg[1],"current")==0){
				pid_current(head);
			} else 	if(strcmp(arg[1],"all")==0){
				pid_all(head);
			}
		}else if(strcmp(arg[0],"hist")==0){
			if(arg[1]==NULL){
				hist(head);
			}
		}else if(strstr(arg[0],"hist")){
			if(arg[0][0]=='!'){
				// determining the number
				int hist=5,number=0;
				while(arg[0][hist] != '\0')
					number = number*10 + new->name[hist++] - '0';
				banghist(head,number);
			}else if(arg[0][4]=='1' || arg[0][4] == '2' ||  arg[0][4] == '3' || arg[0][4] == '4' ||  arg[0][4] == '5' ||  arg[0][4] == '6'|| arg[0][4] == '7' || arg[0][4] == '8' || arg[0][4] == '9') {
				int hist=4,number=0;
				while(arg[0][hist] != '\0')
					number = number*10 + new->name[hist++] -'0';
				histn(head,number);
			}
		}
	} else {
		if(status<=1){
			if(status == 1)
				arg[i-1] = NULL;
			int a = execvp(arg[0],arg);
			if(a==-1){
				badavala = 1;
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
				exit(0);
			}
		}
	}
}


int execute(process *new,process *head){
	signal(SIGCHLD,handler);
	int i = 0;
	char naruto[100];
	strcpy(naruto,new->name);
	char *pointer,*arg[100];
	for(i=0;i<100;i++)
		arg[i] = (char *)malloc(100*sizeof(char));
	pointer = strtok(new->name," \t");
	i=0;
	while(pointer != NULL){
		strcpy(arg[i],pointer);
		pointer = strtok(NULL," \t");
		i++;
	}
	//  CHECK IF QUIT IS GIVEN AS A COMMAND AND NOT AS AN ARGUMENT
	if(strcmp(arg[0],"quit")==0){
		quit = -1;
		exit(1);
	}
	// check complete
	badavala = 0;
	arg[i] = NULL;  // i contains the max number of args
	strcpy(new->name,arg[0]);

	// now strcating the full command u can 

	int str;
	for(str = 1; str<i; str++){
		strcat(new->name," ");
		strcat(new->name,arg[str]);
	}
	// done strcating u can also comment this part to maintain the workablily of getting only the first name

	// checking if &, is there or not
	// value of status will be 0 if no & or | or > < is there
	// status == 1 if & present   ASSUMPTION that & will come standalone
	// status == 2 if | present
	// status == 3 if > or < present
	// status == 4 if | and > < present
	int status=0;
	if(strstr(naruto,">") || (strstr(naruto,"<"))){
		if(strstr(naruto,"|"))
			status = 4;
		else
			status = 3;
	}else if(strstr(naruto,"|")){
		status = 2;
	}else if(strstr(naruto,"&")){
		status = 1;
	}

	if(status == 3){
		redirect(new,head);
		return 2;  // assuming they still have a valid output
	}
	if(status == 4 || status == 2){
		pipes(new,head);
		return 2;
	}
	if(strstr(naruto,"cd") || strstr(naruto,"hist") || strstr(naruto,"pid")){
		int err;
		if(strcmp(arg[0],"cd")==0){
			if(i==1){
				getcwd(pre,100);
				err=chdir(home);
			}else{
				if(strcmp(arg[1],"-")==0){
					char old[100];
					getcwd(old,100);
					err=chdir(pre);
					strcpy(pre,old);
				}else if(strcmp(arg[1],"~")==0){
					getcwd(pre,100);
					err=chdir(home);
				}else if(strcmp(arg[1],".")==0){
					char new[100];
					strcpy(new,home);
					int target,letter=1;
					target=strlen(new);
					while(letter<=strlen(arg[1])){
						new[target++]=arg[1][letter++];
					}
					getcwd(pre,100);
					err=chdir(new);
				}else{
					getcwd(pre,100);
					err=chdir(arg[1]);
				}
			}
			if(err==-1){
				badavala = 1;
				char *asdf = strerror(errno);  // printing the error
				printf("%s\n",asdf);
			}
		}else if(strcmp(arg[0],"pid")==0){
			if(arg[1]==NULL){
				printf("Command name:./a.out process id:%d\n",mainid);
			}else if(strcmp(arg[1],"current")==0){
				pid_current(head);
			} else 	if(strcmp(arg[1],"all")==0){
				pid_all(head);
			}else {
				badavala = 1;
				printf("try pid or pid all or pid current\n");
			}
		}else if(strcmp(arg[0],"hist")==0){
			if(arg[1]==NULL){
				hist(head);
			}else{
				badavala = 1;  // temp var for an error
				printf("please try !histn or histn or hist\n");
			}
		}else if(strstr(arg[0],"hist")){
			if(arg[0][0]=='!'){
				// determining the number
				int hist=5,number=0;
				while(arg[0][hist] != '\0')
					number = number*10 + new->name[hist++] - '0';
				banghist(head,number);
			}else if(arg[0][4]=='1' || arg[0][4] == '2' ||  arg[0][4] == '3' || arg[0][4] == '4' ||  arg[0][4] == '5' ||  arg[0][4] == '6'|| arg[0][4] == '7' || arg[0][4] == '8' || arg[0][4] == '9') {
				int hist=4,number=0;
				while(arg[0][hist] != '\0')
					number = number*10 + new->name[hist++] -'0';
				histn(head,number);
			}else{
				badavala = 1;
				printf("please try !histn or histn or hist\n");
			}
		}
	} else {
		int child = vfork();
		if(child == -1 ){
			char *asdf = strerror(errno);  // printing the error
			printf("%s\n",asdf);
		}else if(child == 0){
			if(status<=1){
				if(status == 1)
					arg[i-1] = NULL;
				int a = execvp(arg[0],arg);
				if(a==-1){
					badavala = 1;
					char *asdf = strerror(errno);  // printing the error
					printf("%s\n",asdf);
					exit(0);
				}
			}
		}else{    /// parent process
			new->pid=child;
			if(status ==1){
				new->wait =1;
			}else{
				new->wait = 0;
				wait(NULL);
			}
			return badavala;
		}
	}
}


int hist(process *head){
	int i=1;
	while(head->next){
		head=head->next;
		printf("%d. %s\n",i++,head->name);
	}
}

int banghist(process *head, int num){
	int i=0,check=0;
	process *temp;
	temp = head;
	while(temp->next){
		temp = temp->next;
		i++;
		if(i==num){
			check=1;
			printf("%d. %s\n",i,temp->name);
		}
	}
	if(check==0)
		printf("Please give a number(>0) lesser than number of processes spawned from this shell\n");
}
int histn(process *head, int num){
	int i=0;
	process *temp;
	temp = head;
	while(temp->next){
		temp = temp->next;
		i++;
	}
	if(i<=num){
		hist(head);
	}
	else{
		int j=0;
		while(j!=(i-num)){
			head=head->next;
			j++;
		}
		hist(head);
	}
}
void handler1(int a){
}
int main(int argc, char** argv) {
	signal(SIGCHLD,handler);
	signal(SIGINT,handler1);
	signal(SIGTSTP,handler1);
	char host[100],user[100];
	getcwd(home,100);
	getcwd(pre,100);
	int a=1;
	mainid=getpid();
	char temp;
	process *head;
	process	*curr,*prev;  // current is the present node and prev is the prev node //revision number 1
	head = (process *)malloc(sizeof(process ));
	copy = head;
	curr = head;
	head->next=NULL;
	while(a==1){
		int listing;  // to decide if a failed process should enter the linked list or not
		display();
		process *new;
		new = (process *)malloc(sizeof(process));
		scanf("%[^\n]",new->name);
		if(new->name[0]!='\0'){
			new->next = NULL;
			prev = curr;
			curr->next = new;
			curr = new;
			listing = execute(new,head);
			if(listing == 1){
				new->pid = 0;
			}
		}
		scanf("%c",&temp);
	}
	return (EXIT_SUCCESS);
}
