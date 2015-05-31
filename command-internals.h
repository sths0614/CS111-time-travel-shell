// UCLA CS 111 Lab 1 command internals
#include <sys/wait.h>
enum command_type
  {
    AND_COMMAND,         // A && B
    SEQUENCE_COMMAND,    // A ; B
    OR_COMMAND,          // A || B
    PIPE_COMMAND,        // A | B
    SIMPLE_COMMAND,      // a simple command
    SUBSHELL_COMMAND,    // ( A )
    LEFT_BRAC_COMMAND,
    RIGHT_BRAC_COMMAND,
  };

// Data associated with a command.
struct command
{
  enum command_type type;

  // Exit status, or -1 if not known (e.g., because it has not exited yet).
  int status;

  // I/O redirections, or null if none.
  char *input;
  char *output;
  char *output2;
  char *output_and; // a bool set to 1 if it's >&
  char *input_output;
  char *input_and;
  char *output_pipe;

  union
  {
    // for AND_COMMAND, SEQUENCE_COMMAND, OR_COMMAND, PIPE_COMMAND:
    struct command *command[2];

    // for SIMPLE_COMMAND:
    char **word;

    // for SUBSHELL_COMMAND:
    struct command *subshell_command;
  } u;
};

typedef struct command *command_t;
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
};

typedef struct command_stream *command_stream_t;

typedef struct GraphNode GraphNode;
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
