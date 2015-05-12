// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

void execute_command (command_t c, bool time_travel);

void executeSimple (command_t c){
	// creating child process
	pid_t p=fork();
	if (p == 0){ // if in the child process
		if (c->input != NULL){ 
			int fd = open(c->input,O_RDONLY,0666); //0666 file permission
			//error(1,0,"Got into input");
			if (fd<0)
				error(1, 0, "Input: opening file failed");
			if (dup2(fd,0) < 0)
				error(1, 0, "Input: dup2 failed");
		}
		if (c->output != NULL){
			int fd = open(c->output,O_CREAT|O_TRUNC|O_WRONLY,0666); //0666 file permission
			//	error(1,0,"Got into output");
			if (fd<0)
				error(1, 0, "Output: opening file failed");
			if (dup2(fd,1) < 0)
				error(1, 0, "Output: dup2 failed");
		}

		if (!strcmp(c->u.word[0],"exec"))
		  execvp(c->u.word[1], &(c->u.word[1]));
		else  
		  execvp(c->u.word[0], c->u.word);
		// executing the simple command
	}
	else { // if in the parent process
		int status; 
		waitpid(p, &status, 0);
		c->status = WEXITSTATUS(status);
	}
}

void executeExec (command_t c){
	// creating child process
	pid_t p=fork();
	if (p == 0){ // if in the child process
		if (c->input != NULL){ 
			int fd = open(c->input,O_CREAT|O_TRUNC|O_WRONLY,0666); //0666 file permission
			if (fd<0)
				error(1, 0, "Input: opening file failed");
			if (dup2(fd,0) < 0)
				error(1, 0, "Input: dup2 failed");
		}
		if (c->output != NULL){
			int fd = open(c->output,O_CREAT|O_TRUNC|O_WRONLY,0666); //0666 file permission
			if (fd<0)
				error(1, 0, "Output: opening file failed");
			if (dup2(fd,0) < 0)
				error(1, 0, "Output: dup2 failed");
		}

		// executing the simple command
		execvp(c->u.word[1], &c->u.word[1]);
	}
	else { // if in the parent process
		int status; 
		waitpid(p, &status, 0);
		c->status = WEXITSTATUS(status);
	}
}

void executeAnd (command_t c){
	//creating child process
	pid_t p=fork();
	if (p==0){ // if in the child process
	  execute_command(c->u.command[0],0); // execute the left side 
		c->status = c->u.command[0]->status; // store the status
		_exit(c->status);
	}
	else{ // if in the parent process 
		int status; //32-bit number, exit status is only the least significant 8 bit 
		waitpid(p, &status, 0); //0 specifies blocking wait
		int exitStatus = WEXITSTATUS(status); //the macro extracts the least significant 8 bit
		if(exitStatus == 0){ // if the left side succeeded 
		  execute_command(c->u.command[1],0); // execute the right side 
			c->status = c->u.command[1]->status;
		}
	}
}


void executeOr (command_t c){

	//creating child process
	pid_t p=fork();
	if (p==0){ // if in the child process
	  execute_command(c->u.command[0],0); // execute the left side 
		c->status = c->u.command[0]->status; // store the status
		_exit(c->status);
	}
	else{ // if in the parent process 
		int status; //32-bit number, exit status is only the least significant 8 bit 
		waitpid(p, &status, 0); //0 specifies blocking wait
		int exitStatus = WEXITSTATUS(status); //the macro extracts the least significant 8 bit
		if(exitStatus != 0){ // if the left side failed
		  execute_command(c->u.command[1],0); // execute the right side 
			c->status = c->u.command[1]->status;
		}
	}
}

void executeSequence (command_t c){

	//creating child process
	pid_t p=fork();
	if (p==0){ // if in the child process
	  execute_command(c->u.command[0],0); // execute the left side 
		c->status = c->u.command[0]->status; // store the status
		_exit(c->status);
	}
	else{
		int status; 
		waitpid(p, &status, 0); // wait for the left side to finish, but don't care about the status
		execute_command(c->u.command[1],0); // execute the right side
		c->status = c->u.command[1]->status;
	}
}


void executePipe(command_t c){
	int fd[2];
	if(pipe(fd) < 0)
	  error(1, errno, "pipe fail");

	pid_t firstPid = fork();
	if (firstPid < 0)
	  error(1, errno, "fork fail");

	if (firstPid == 0){ // execute right side of pipe, in child process
		close(fd[1]);
		if (dup2(fd[0],0) < 0) // if success, it will return the second arg. if fail, it will return a negative num
		  error(1, errno, "dup2 fail");
		execute_command(c->u.command[1],0);
		_exit(c->u.command[1]->status); // will perform kernel clean up
	}
	else { // now in parent process
		pid_t secondPid = fork();
		if (secondPid < 0)
		  error(1, errno, "fork fail");

		if (secondPid == 0){ // execute left side of pipe, in child process
			close(fd[0]);
			if (dup2(fd[1],1) < 0){
			  error(1, errno, "dup2 fail");
			}
				execute_command(c->u.command[0],0);
				_exit(c->u.command[0]->status);
		}
		else{ // back in parent process
			close(fd[0]);
			close(fd[1]);
			int status;
			pid_t returnPid = waitpid(-1, &status, 0); // -1 means any process
			if (secondPid == returnPid)
				waitpid(firstPid, &status, 0);
			if (firstPid == returnPid)
				waitpid(secondPid, &status, 0);
			c->status = WEXITSTATUS(status);
		}
	}
}


// The only two test cases: (echo abc def), (cat <a.txt)>b.txt
void executeSubshell (command_t c){
  //Add a check here
	c->u.subshell_command->output = c->output; // for case 2
	execute_command(c->u.subshell_command,0);
}


int command_status (command_t c)
{
  return c->status;
}

void execute_command (command_t c, bool time_travel)
{
	switch (c->type){
		case SIMPLE_COMMAND:
			executeSimple(c);
			break;
			/*	case EXEC_COMMAND;
			executeExec(c);
			break;
			*/
		case AND_COMMAND:
			executeAnd(c);
			break;
		case OR_COMMAND:
			executeOr(c);
			break;
		case SEQUENCE_COMMAND:
			executeSequence(c);
			break;
		case PIPE_COMMAND:
			executePipe(c);
			break;
		case SUBSHELL_COMMAND:
			executeSubshell(c);
			break;
	default:
	  break;
	}
}

//============================================================================
/*typedef struct GraphNode GraphNode;
typedef struct DependencyGraph DependencyGraph;
typedef struct LLNode LLNode;
 
struct GraphNode{
    command_t cmd;
    GraphNode** before;
    int len_before;
    pid_t pid;
};
 
struct DependencyGraph{
    GraphNode** no_dep;
    GraphNode** dep;
    int len_no_dep;
    int len_dep;
};
 
struct LLNode{
    GraphNode* gn;
    char** write_list;
    char** read_list;
    LLNode* next;
    int len_read_list;
    int len_write_list;
};

*/

int haveDependency(LLNode* head, LLNode* cursor){
	//for RAW, head's RL and cursor's WL
  int i;
  for(i = 0; i < head->len_read_list; i++){
    int j;
    for(j = 0; j < cursor->len_write_list; j++){
      if(strcmp(head->read_list[i],cursor->write_list[j])==0)
			  {
			    return 1;
			  }
		}
	}
  
	//for WAR, head's WL and cursor's RL
	for(i = 0; i < head->len_write_list; i++){
	  int j;
	  for(j = 0; j < cursor->len_read_list; j++){
            if(strcmp(head->write_list[i],cursor->read_list[j])==0)
			  {
        			return 1;
			  }
	  }
	}

	//for WAW, head and cursor's WL
	for(i = 0; i < head->len_write_list; i++){
	  int j;
	  for(j = 0; j < cursor->len_write_list; j++){
            if(strcmp(head->write_list[i],cursor->write_list[j])==0)
			  {
        		    return 1;
			  }
		}
	}

	return 0;
}

void process_cmd_tree(command_t i, LLNode* newNode)
{
    if (i->type==SIMPLE_COMMAND)
    {
    	if(i->input != NULL){
    		newNode->read_list[newNode->len_read_list]=i->input;
       		newNode->len_read_list++;
    	}

	int cnt=1;
        while (i->u.word[cnt]!=NULL)
        {
            if (i->u.word[cnt][0]!='-')
            {
                newNode->read_list[newNode->len_read_list]=i->u.word[cnt];
                newNode->len_read_list++;
            }
            cnt++;
        }
       
		if(i->output != NULL){
			newNode->write_list[newNode->len_write_list]=i->output;
        	newNode->len_write_list++;
		}       
    }
    else if(i->type==SUBSHELL_COMMAND)
    {
    	if(i->input != NULL){
    		newNode->read_list[newNode->len_read_list]=i->input;
       		newNode->len_read_list++;
    	}
    	if(i->output != NULL){
			newNode->write_list[newNode->len_write_list]=i->output;
        	newNode->len_write_list++;
		}    
        process_cmd_tree(i->u.subshell_command, newNode);
    }
    else
    {
        process_cmd_tree(i->u.command[0], newNode);
        process_cmd_tree(i->u.command[1], newNode);
    }
}

DependencyGraph* createGraph(command_stream_t cmd_str_t){

	int numNodes = 0;
	while(cmd_str_t->cursor != NULL){
		numNodes++;
		cmd_str_t->cursor = cmd_str_t->cursor->next;
	}
	cmd_str_t->cursor = cmd_str_t->head;

	DependencyGraph* depGraph = (DependencyGraph*)checked_malloc(sizeof(DependencyGraph));
	depGraph->no_dep=(GraphNode**)checked_malloc(sizeof(GraphNode*)*numNodes);
	depGraph->dep=(GraphNode**)checked_malloc(sizeof(GraphNode*)*numNodes);
	depGraph->len_no_dep = 0;
	depGraph->len_dep = 0;



	LLNode* head = NULL;
	while(cmd_str_t->cursor != NULL){ //for each tree
		GraphNode* cur_gn=(GraphNode*)checked_malloc(sizeof(GraphNode));
		cur_gn->cmd= cmd_str_t->cursor->cmd;
		cur_gn->before=(GraphNode**)checked_malloc(sizeof(GraphNode*)*numNodes);
		cur_gn->pid=-1;
		cur_gn->len_before=0;
		 
		LLNode* newNode=(LLNode*)checked_malloc(sizeof(LLNode));
		newNode->gn = cur_gn;
		newNode->write_list=(char**)checked_malloc(5*sizeof(char*));
		newNode->read_list=(char**)checked_malloc(5*sizeof(char*));
		newNode->next=NULL;
		newNode->len_read_list=0;
		newNode->len_write_list=0;

		process_cmd_tree(newNode->gn->cmd, newNode);


		//inserting the newNode 
		newNode->next = head; 
		head = newNode;
        	//foreach j LLNode after head, check dependencies
		LLNode* cursor = head->next;
		while(cursor != NULL){

		  int k;
	          if(haveDependency(head, cursor)){
	        		head->gn->before[head->gn->len_before] = cursor->gn;
				head->gn->len_before++;
			}
			cursor = cursor->next;
		}

		if (head->gn->len_before == 0){
	        	depGraph->no_dep[depGraph->len_no_dep] = head->gn;
			depGraph->len_no_dep++;
		}
		else{
	        	depGraph->dep[depGraph->len_dep] = head->gn;
			depGraph->len_dep++;
		}

		cmd_str_t->cursor = cmd_str_t->cursor->next;
	}
	cmd_str_t->cursor = cmd_str_t->head;
	return depGraph;
}

void executeNoDependencies(DependencyGraph* graph){
  int i;
  for (i = 0; i < graph->len_no_dep; i++)
	{
		pid_t pid = fork();
		if(pid == 0){
			execute_command(graph->no_dep[i]->cmd, true);
			exit(0);
		}
		else{
			graph->no_dep[i]->pid = pid;
		}
	}
}

void executeDependencies(DependencyGraph* graph){
  int i;
  for (i = 0; i < graph->len_dep; i++)
	{	
		//may not be necessary, used for guarentee that there is already a spread spawned for it
	  int j;
	  	loop_label:
		for (j = 0; j < graph->dep[i]->len_before; j++){
			if(graph->dep[i]->before[j]->pid == -1)
			  {
			    fprintf(stderr,"THE GOTO");
			    goto loop_label;
			  }
				}

		int status;
		for (j = 0; j < graph->dep[i]->len_before; j++){
  	 		waitpid(graph->dep[i]->before[j]->pid, &status, 0);
		}
		pid_t pid = fork();
		if (pid == 0){
			execute_command(graph->dep[i]->cmd, true);
			exit(0);
		}
		else{
			graph->dep[i]->pid = pid;
		}
	}
}




int executeGraph(DependencyGraph* graph){
	executeNoDependencies(graph);
	executeDependencies(graph);
	return 1;
}
