// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include <stdio.h>
#include <error.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */


typedef enum {
  WORD,
  AND,
  OR,
  PIPE,
  SEMICOLON,
  ONE_NEWLINE,
  TWO_NEWLINE,
  LEFT_BRAC,
  RIGHT_BRAC,
  LESS,
  GREATER,
  // COMMENT,
  NONE,
} tok_type;

const char* Names[] ={
  "WORD",
 "AND",
 "OR",
  "PIPE",
  "SEMICOLON",
  "ONE_NEWLINE",
  "TWO_NEWLINE",
  "LEFT_BRAC",
  "RIGHT_BRAC",
  "LESS",
  "GREATER",
  // "COMMENT",
  "NONE"
};

typedef struct token{
  char* content;
  tok_type type;
  int lineNo;
  int errNo;
}token;
  
int isWordType (char c){
  if (isalpha(c))
    return 1;
  if (isdigit(c))
    return 1;

  switch (c) {
  case '!':
  case '%':
  case '+':
  case ',':
  case '-':
  case '.':
  case '/':
  case ':':
  case '@':
  case '^':
  case '_':
    return 1;
  default:
    return 0;
  }
}
//Function to store all commands in a buffer
char* getBuffer(int (*get_next_byte) (void *),void *get_next_byte_argument, int* length)
{
  int size=128;
  char c;
  *length=0;
  char* entry = (char*) checked_malloc(sizeof(char)*size);
  while (1)
    {
      c=get_next_byte(get_next_byte_argument);
      if (c==EOF)
  break;
      entry[*length]=c;
      (*length)++;
      if (*length==size)
  {
    size=size*2;
    entry=(char*) checked_realloc(entry, size);
  }
    }
  if (*length==0)
    {
      error(1,1, "1:buffer is empty");
    }
    
  if (*length==size)
    {
      size=size*2;
      entry=(char*) checked_realloc(entry, size);
    }
  
  entry[*length]='\0'; //length does not include null
  return entry;
}

token* getToken(char* buff, int len, int* token_length )
{
  if (buff[0]== '\0')
    return NULL; //EMPTY
  int errCount=1;
  int size=256;
  token* arr=(token*)checked_malloc(sizeof(token)*size);
  char curr,next;
  int flag=0;
  int len_tok=0;
  int tree=1;
  int cnt;
  for (cnt=0; cnt<len; cnt++)
    {
      curr=buff[cnt];
      if (cnt+1!=len)
  next=buff[cnt+1];
      else
  flag=1;
      
      switch(curr)
  {  
  case '&':
    if (!flag && next=='&')
      {
        arr[len_tok].type=AND;
        arr[len_tok].content=NULL;
        arr[len_tok].lineNo=tree;
  arr[len_tok].errNo=errCount;
        (stderr, "AND %d %d, ",arr[len_tok].errNo,arr[len_tok].lineNo);
        len_tok++;
        cnt++;
      }
    else
      {
    error(1,1, "%d:unknown character", errCount);
        arr[len_tok].type=NONE;
        arr[len_tok].content=NULL;
        arr[len_tok].lineNo=tree;
  arr[len_tok].errNo=errCount;
        len_tok++;
      }
    break;

  case '|':
    if (!flag && next=='|')
      {
        arr[len_tok].type=OR;
        arr[len_tok].content=NULL;
        arr[len_tok].lineNo=tree;
  arr[len_tok].errNo=errCount;
        len_tok++;
        cnt++;
      }
    else
      {
        arr[len_tok].type=PIPE;
        arr[len_tok].content=NULL;
        arr[len_tok].lineNo=tree;
  arr[len_tok].errNo=errCount;
        len_tok++;
      }
    break;

  case '\n':
    if(!flag && next=='\n')
      {
        arr[len_tok].type=TWO_NEWLINE;
        arr[len_tok].content=NULL;
        arr[len_tok].lineNo=tree;
        tree++;
  arr[len_tok].errNo=errCount;
        int newl=0;
        while (buff[cnt+1+newl]=='\n')
    {
      newl++;
      errCount++;
    }
        errCount++;
        len_tok++;
        cnt=cnt+newl;       
      }
    else
      {
        arr[len_tok].type=ONE_NEWLINE;
        arr[len_tok].content=NULL;
        arr[len_tok].lineNo=tree;
  arr[len_tok].errNo=errCount;
        len_tok++;
  errCount++;
      }
  case ' ':
  case '\t':
    break;
    
  case ';':
    arr[len_tok].type=SEMICOLON;
    arr[len_tok].content=NULL;
    arr[len_tok].lineNo=tree;
    arr[len_tok].errNo=errCount;
    len_tok++;
    break;

  case '(':
    arr[len_tok].type=LEFT_BRAC;
    arr[len_tok].content=NULL;
    arr[len_tok].lineNo=tree;
    arr[len_tok].errNo=errCount;
    len_tok++;
    break;

  case ')':
    arr[len_tok].type=RIGHT_BRAC;
    arr[len_tok].content=NULL;
    arr[len_tok].lineNo=tree;
    arr[len_tok].errNo=errCount;
    len_tok++;
    break;

  case '<':
    arr[len_tok].type=LESS;
    arr[len_tok].content=NULL;
    arr[len_tok].lineNo=tree;
    arr[len_tok].errNo=errCount;
    len_tok++;
    break;

  case '>':
    arr[len_tok].type=GREATER;
    arr[len_tok].content=NULL;
    arr[len_tok].lineNo=tree;
    arr[len_tok].errNo=errCount;
    len_tok++;
    break;

  case '#':;
    // arr[len_tok].type=COMMENT;
    int n=0;
    while(buff[cnt+1+n]!='\n' && (cnt+1+n)<len)
      {
        n++;
      }
    char* comm=(char*)checked_malloc(sizeof(char)*(n+2));
    int q=0;
    for(;q<n+1;q++)
      {
        comm[q]=buff[cnt+q];
      }
    comm[q]='\0';
    cnt=cnt+n;
    break;
    
  default:
    if(isWordType(curr))
      {
        arr[len_tok].type=WORD;
        arr[len_tok].lineNo=tree;
  arr[len_tok].errNo=errCount;
        int newch=0;
        while (isWordType(buff[cnt+1+newch]))
    {
      newch++;
    }
        arr[len_tok].content=(char*)checked_malloc(sizeof(char)*(newch+2));
        int q=0;
        for(;q<newch+1;q++)
    {
      arr[len_tok].content[q]=buff[cnt+q];
    }
        arr[len_tok].content[q]='\0';
        len_tok++;
        cnt=cnt+newch;
        break;
      }
    else
      {
    error(1,1, "%d:unknown character", errCount);
  arr[len_tok].type=NONE;
        arr[len_tok].content=NULL;
        arr[len_tok].lineNo=tree;
  arr[len_tok].errNo=errCount;
        len_tok++;
        break;
      }
  }
    }
  *token_length=len_tok;
  return arr;
}



// Implementation of stack
// Source: https://www.cs.bu.edu/teaching/c/stack/array/
typedef struct stack stack;
struct stack{
  command_t commands[4];
  int top;
};

command_t stackPop(stack *s){
  if (stackIsEmpty(s)){
    exit(1); /*Exit, returning error code.*/
  }
  return s->commands[s->top--];
}

command_t stackPeep(stack *s){
  if (stackIsEmpty(s)){
    exit(1); /*Exit, returning error code.*/
  }
  return s->commands[s->top];
}


void stackPush(stack *s, command_t c){
  s->commands[++(s->top)] = c;
}

int stackIsEmpty(stack *s){
  return s->top < 0;
}
// End of implementation of stack



// Pop two operands and one operator, combine them, and push back onto operand
void combine(stack* operators, stack* operands){
  //Can't combine if less than two operands left
  if ((operands->top)+1 < 2){
    exit(1); /*Exit, returning error code.*/
  }

  command_t right = stackPop(operands);
  command_t left = stackPop(operands);

  //command_t tmp = (command_t) checked_malloc(sizeof(struct command));
  command_t tmp = stackPop(operators);

  //Set the new command's two operands
  tmp->u.command[0] = left;
  tmp->u.command[1] = right;
  tmp->input = NULL;
  tmp->output = NULL;

  //Push the new command back on the operand stack
  stackPush(operands, tmp);
}


// Returns the precedence of the operator, 0, 1, or 2
int getPrec(enum command_type com){
  int prec;
  if (com == SEQUENCE_COMMAND)
    prec=0;
  if (com == AND_COMMAND || com == OR_COMMAND)
    prec=1;
  if (com == PIPE_COMMAND)
    prec=2;

  return prec;
}


// Make a command tree from the token array, return a command_t when reach a different command tree
command_t make_command_tree(token* tok, int *start, int *tree_num, int tok_len){
  
  // Used to check if number of brackets are the same
  int bracFlag = 0;
  int bracLine = 1;

  // If there is only one token
  if (tok_len == 1 && tok[0].type == WORD){
    command_t cur_cmd = (command_t) checked_malloc(sizeof(struct command));
    cur_cmd->u.word = (char**) checked_malloc(2*sizeof(char*)); 
    cur_cmd->u.word[0] = tok[0].content;
    cur_cmd->type = SIMPLE_COMMAND;
    cur_cmd->u.word[1] = NULL;
    return cur_cmd;
  }
  else if(tok_len == 1){
    error(1, 1, "1:One input, not word");
  }

  // Allocating space for two stacks, operators and operands
  stack* operators = (stack*) checked_malloc(sizeof(stack));
  operators->top=-1;
  stack* operands = (stack*) checked_malloc(sizeof(stack));
  operands->top=-1;

  // The start position in the array for this command tree
  int i=*start;

  // Error checking for the first and last tokens
  if (tok[0].type == ONE_NEWLINE || tok[0].type == TWO_NEWLINE)
    {
      if(*tree_num==1){
	  {
	    i++;
      	  }
      if(tok[0].type == TWO_NEWLINE)
	{
	  (*tree_num)++;
        }
      }
    }
  else if(tok[0].type != WORD && tok[0].type != LEFT_BRAC){
    error(1, 1, "%d:Invalid first token of tree", tok[i].errNo);
  }

  if(tok[tok_len-1].type != RIGHT_BRAC && tok[tok_len-1].type != ONE_NEWLINE && tok[tok_len-1].type != TWO_NEWLINE && tok[tok_len-1].type != WORD)
  {
    error(1, 1, "%d:Invalid last token", tok[tok_len-1].errNo);
  }

  if(tok[tok_len-1].type == ONE_NEWLINE || tok[tok_len-1].type == TWO_NEWLINE){
    if(tok[tok_len-2].type != RIGHT_BRAC && tok[tok_len-2].type != WORD){
      error(1, 1, "%d:Invalid second last token", tok[tok_len-2].errNo);
    }
  }

  // The current command used throughout the loop
  command_t cur_cmd;
  while(i < tok_len && (tok[i].lineNo == *tree_num)){
    /* Process the current token by type, using the pseudo code:

    Maintain 2 stacks: operator and command
    1. If a simple command, push it onto command stack
    2. If "(", push onto operator stack
    3. If an operator and operator stack is empty, 
      push operator onto operator stack
    4. If an operator and operator stack NOT empty,
    4a. pop all operators with >= precedence off operator stack.
      for each operator, pop 2 commands off command stacks
      combine into new command, push it onto command stack
    4b. Stop when reach an operator with lower precedence or a "("
    4c. Push new operator onto operator stack
    5. If encounter ")", pop operators off stack like 4a until see a matching "("
      Create a subshell command by popping top command from command stack
      Push new command to command stack
    6. Back to 1
    7. Nothing left, pop remaining operators like 4a
    ** When seeing a redirection, just modify the input of the top command (on stack)    */
    switch(tok[i].type){
      case WORD:
        cur_cmd = (command_t) checked_malloc(sizeof(struct command));
        cur_cmd->type = SIMPLE_COMMAND;
        int num = 1; 

        // Check if the following tokens are words, allocate memory, and combine the words into a command
        while(i+num < tok_len && tok[i+num].type == WORD){
          num++;
        }
        cur_cmd->u.word = (char**) checked_malloc((num+1) * sizeof(char*));
        int j;
        for (j = 0; j < num; j++)
        {
          cur_cmd->u.word[j] = tok[i+j].content;
        }

        // Set the last element of u.word to be NULL, used for 1b and 1c
        cur_cmd->u.word[num] = NULL;
        stackPush(operands, cur_cmd);

        // Skip processing of next word tokens
        i+=num;
        break;

      case AND:
        if(tok[i-1].type != WORD && tok[i-1].type != RIGHT_BRAC){
          error(1, 1, "%d:Invalid token before AND", tok[i].errNo);
        }

        cur_cmd = (command_t) checked_malloc(sizeof(struct command));
        cur_cmd->type = AND_COMMAND;

        if(stackIsEmpty(operators)){
          stackPush(operators, cur_cmd);
        }
        else{
          while(getPrec(stackPeep(operators)->type) >= getPrec(cur_cmd->type) 
                       && stackPeep(operators)->type != LEFT_BRAC_COMMAND){
            combine(operators, operands);
            if(stackIsEmpty(operators))
              break;
          }
          stackPush(operators, cur_cmd);
        }
        i++;
        break;

      case OR:
        if(tok[i-1].type != WORD && tok[i-1].type != RIGHT_BRAC){
          error(1, 1, "%d:Invalid token before OR", tok[i].errNo);
        }
        
        cur_cmd = (command_t) checked_malloc(sizeof(struct command));
        cur_cmd->type = OR_COMMAND;

        if(stackIsEmpty(operators)){
          stackPush(operators, cur_cmd);
        }
        else{
          while(getPrec(stackPeep(operators)->type) >= getPrec(cur_cmd->type) 
            && stackPeep(operators)->type != LEFT_BRAC_COMMAND){
            combine(operators, operands);
            if(stackIsEmpty(operators))
              break;
          }
          stackPush(operators, cur_cmd);
        }
        i++;
        break;

      case PIPE:
        if(tok[i-1].type != WORD && tok[i-1].type != RIGHT_BRAC){
          error(1, 1, "%d:Invalid token before PIPE", tok[i].errNo);
        }


        cur_cmd = (command_t) checked_malloc(sizeof(struct command));
        cur_cmd->type = PIPE_COMMAND;

        if(stackIsEmpty(operators)){
          stackPush(operators, cur_cmd);
        }
        else{
          while(getPrec(stackPeep(operators)->type) >= getPrec(cur_cmd->type) 
            && stackPeep(operators)->type != LEFT_BRAC_COMMAND){
            combine(operators, operands);
            if(stackIsEmpty(operators))
              break;
          }
          stackPush(operators, cur_cmd);
        }
        i++;
        break;

      case SEMICOLON:
        if(tok[i-1].type != WORD && tok[i-1].type != RIGHT_BRAC){
          error(1, 1, "%d:Invalid token before SEMICOLON", tok[i].errNo);
        }

        if(tok[i+1].type == TWO_NEWLINE){
          i++;
          break;
        }

        cur_cmd = (command_t) checked_malloc(sizeof(struct command));
        cur_cmd->type = SEQUENCE_COMMAND;

        if(stackIsEmpty(operators)){
          stackPush(operators, cur_cmd);
        }
        else{
          while(getPrec(stackPeep(operators)->type) >= getPrec(cur_cmd->type) 
            && stackPeep(operators)->type != LEFT_BRAC_COMMAND){
            combine(operators, operands);
            if(stackIsEmpty(operators))
              break;
          }
          stackPush(operators, cur_cmd);
        }
        i++;
        break;


      case LESS:
        if (tok[i-1].type != WORD && tok[i-1].type != RIGHT_BRAC){
          error(1, 1, "%d:left of LESS isn't word or RIGHT_BRAC", tok[i].errNo);
        }

        if (tok[i+1].type != WORD){
          error(1, 1, "%d:right of LESS isn't word", tok[i].errNo);
        }

        cur_cmd = stackPop(operands);
        cur_cmd->input = tok[i+1].content;
        stackPush(operands, cur_cmd);
        i+=2;
        break;

      case GREATER:
        if (tok[i-1].type != WORD && tok[i-1].type != RIGHT_BRAC){
          error(1, 1, "%d:left of GREATER isn't word or RIGHT_BRAC", tok[i].errNo);
        }

        if (tok[i+1].type != WORD){
          error(1, 1, "%d:right of GREATER isn't word", tok[i].errNo);
        }

        cur_cmd = stackPop(operands);
        cur_cmd->output = tok[i+1].content;
        stackPush(operands, cur_cmd);
        i+=2;
        break;




      case LEFT_BRAC:
       if(i!=0 && tok[i-1].type == WORD)
         {
           error(1, 1, "%d:word followed by LEFT_BRAC", tok[i].errNo);
         }

       if(tok[i+1].type != WORD)
       {
         error(1, 1, "%d:LEFT_BRAC not followed by word", tok[i].errNo);
       }

        cur_cmd = (command_t) checked_malloc(sizeof(struct command));
        cur_cmd->type = LEFT_BRAC_COMMAND;
        stackPush(operators, cur_cmd);
        i++;
        bracFlag++;
        bracLine = tok[i].errNo;
        break;

      case RIGHT_BRAC:

        if(i!=0 && tok[i-1].type != WORD)
         {
          error(1, 1, "%d:not word followed by RIGHT_BRAC", tok[i].errNo);
         }

        if(i!=tok_len-1 && (tok[i+1].type == WORD || tok[i+1].type == LEFT_BRAC))
         {
            error(1, 1, "%d:word or LEFT_BRAC right after RIGHT_BRAC", tok[i].errNo);
         }
        if (bracFlag <= 0)
        {
          error(1,1,"%d:Number of brackets don't match", bracLine);
        }  
        cur_cmd = (command_t) checked_malloc(sizeof(struct command));
        while(stackPeep(operators)->type != LEFT_BRAC_COMMAND){
          combine(operators, operands);
        }
        stackPop(operators); //should be a left brac
        cur_cmd->type = SUBSHELL_COMMAND;
        cur_cmd->u.subshell_command = stackPop(operands);
        stackPush(operands, cur_cmd);
        i++;
        bracFlag--;
        break;

      case ONE_NEWLINE:
        if (i == 0){
          i++;
          break;
        }
        if (i == tok_len-1){
          i++;
          break;
        }

        // Treat ONE_NEWLINE as SEMICOLON
        if (tok[i-1].type == WORD || tok[i-1].type == RIGHT_BRAC){
          tok[i].type = SEMICOLON;
          break;
        }
        i++;
        break;

      case TWO_NEWLINE:
        if (i == 0) {
          i++;
          break;
        }
        if((tok[i-1].type == AND) || (tok[i-1].type == OR) || (tok[i-1].type == PIPE)){
          //doubleNewFlag = 1; 
          (*tree_num)++;
        }
        i++;
        break;

      default:
        i++;
        break;
    }
  }

  if(bracFlag != 0){
    error(1,1,"%d:Number of brackets don't match", bracLine);
  }

  int y;
  int ttt = operators->top;
  int tttt = ttt+1;
  for (y = 0; y < tttt; y++)
  {
    combine(operators, operands);
  }

  // Set the next tree's start to be the newest token 
  *start = i;
  return stackPeep(operands);
}



/*
// Defintion of a linked list of type command_stream)
struct commandNode{
  command_t cmd; //root of one command tree
  struct commandNode *next;
};


typedef struct command_stream command_stream;
struct command_stream{
  struct commandNode *head;
  struct commandNode *tail;
  struct commandNode *cursor;
};*/
// End of linked list definition

// Make the whole command stream, returns command_stream_t
command_stream_t make_command_woods(token* tok, int tok_len){
  command_stream_t cmd_str_t = (command_stream_t) checked_malloc(sizeof(command_stream)); 
  cmd_str_t->head = NULL;
  cmd_str_t->tail = NULL;
  cmd_str_t->cursor = NULL;

  int start = 0;
  int i;
  int total_trees = tok[tok_len-1].lineNo;

  // Add trees to the linked list
  for (i = 1; i <= total_trees; i++)
  {

    command_t tmp_t = make_command_tree(tok, &start, &i, tok_len);
    struct commandNode *tmpNode = (struct commandNode*) checked_malloc(sizeof(struct commandNode));
    tmpNode->cmd = tmp_t;
    tmpNode->next = NULL;
    
    if(cmd_str_t->head == NULL){ //tmp_t is the first tree
      cmd_str_t->head = tmpNode;
      cmd_str_t->tail = tmpNode;
      cmd_str_t->cursor = tmpNode; //Changing cursor to head 
    }                            
    else{                       //tmp_t isn't the first tree
      cmd_str_t->tail->next = tmpNode;
      cmd_str_t->tail = tmpNode;  
    }
  }

  return cmd_str_t;
}



// Read in the file, returns the command_stream_t
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
         void *get_next_byte_argument)
{
  int length;
  char* buffer=getBuffer(get_next_byte, get_next_byte_argument, &length);

  int cnt;

  int tok_len;
  token* tok_arr=getToken(buffer, length,&tok_len);

  /* int v=0;
  for (;v<tok_len;v++)
    {
      fprintf(stderr,"%s ",names[tok_arr[v].type]);
    }
  */
  command_stream_t cmd_str_t =  make_command_woods(tok_arr, tok_len);
  return cmd_str_t;
}



//reads the command stream tree by tree
command_t read_command_stream (command_stream_t s)
{
  //return NULL if finished reading all the trees
  if (s->cursor == NULL)
      return NULL;

  //move onto the next tree and return current tree
  command_t tmp = s->cursor->cmd;
  s->cursor = s->cursor->next;
  return tmp;
}

