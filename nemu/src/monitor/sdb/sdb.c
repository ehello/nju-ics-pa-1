#include <isa.h>
#include <cpu/cpu.h>
#include <memory/vaddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  // readline() will allocate the memory space of 
  // the read string via malloc(), so first check
  // if 'line_read' is allocated.
  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  // if string 'line_read' is not empty, store it.
  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  // the state of NEMU should be changed to NEMU_QUIT
  // while command 'q' is executed.
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args)
{
  char *arg = strtok(NULL, " ");
  int step = 0;
  if (arg == NULL) {
      step = 1;
  } else {
      sscanf(arg, "%d", &step);
      if (step < 0) {
          printf("Please input a positive integer.\n");
          return 0;
      }
  }
  cpu_exec(step);
  return 0;
}

static int cmd_info(char *args)
{
  char *arg = strtok(args, " ");
  if (arg == NULL) {
      printf("Please input [r] for registers or [w] for watchpoints.\n");
      return 0;
  }

  switch (*arg)
  {
  case 'r':
    // isa_reg_display() is in ./src/isa/#YOUR_ISA/reg.c
    isa_reg_display();
    break;
  
  case 'w':
    display_wp();
    break;
  
  default:
    printf("Please input [r] for registers or [w] for watchpoints.\n");
    break;
  }
  return 0;
}

static int cmd_x(char *args)
{
  char *arg[2];
  arg[0] = strtok(NULL, " ");
  arg[1] = strtok(NULL, " ");
  if (arg[0] == NULL) {
    printf("Please input [N] for the number of words.\n");
    return 0;
  } else if (arg[1] == NULL) {
    printf("Please input [EXPR] for the beginning address.\n");
    return 0;
  }

  word_t N, vaddr;
  bool success;

  N = atoi(arg[0]);
  // sscanf(arg[1], "%x", &vaddr);
  vaddr = expr(arg[1], &success);

  while (N > 0) {
    printf("0x%08x: ", vaddr);
    for (int i = 4; i > 0 && N > 0; i--, N--, vaddr += 4) {
      printf("%08x ", vaddr_read(vaddr, 4));
    }
      putchar('\n');
  }
  return 0;
}

static int cmd_p(char *args)
{
  bool success;
  if (args == NULL) {
    printf("Please input [EXPR] to evaluate its value.\n");
    return 0;
  }
  int val = expr(args, &success);
  if (success == false) {
    printf("Illegal expression, please retry.\n");
  } else {
    printf("%u\n", val);
  }
  return 0;
}

static int cmd_w(char *args)
{
  int add_wp_state = add_wp(args);
  switch (add_wp_state) {
  case 1:
    printf("[EXPR] is too long.\n");
    break;
  case 2:
    printf("There is no available watchpoint in the pool.\n");
    break;
  case 3:
    printf("Illegal expression, please retry.\n");
    break;
  case 0:
    printf("Successfully set a watchpoint with expression %s.\n", args);
    break;
  default:
    break;
  }
  return 0;
}

static int cmd_d(char *args)
{
  int NO = atoi(args);
  if (del_wp(NO) == 1) {
    printf("No such watchpoint set.\n");
  } else {
    printf("Successfully delete watchpoint with number %d.\n", NO);
  }
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Run [N] instructions", cmd_si },
  { "info", "Print the information of [r]egiters or [w]atchpoints", cmd_info },
  { "x", "Scan the memory for [N] words begins with the value of [EXPR]", cmd_x },
  { "p", "Print the value of [EXPR]", cmd_p},
  { "w", "Set a watchpoint with [EXPR]", cmd_w},
  { "d", "Delete a watchpoint with number [N]", cmd_d},

  // TODO: Add more commands

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      // find the command matches to string 'cmd'
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        // execute the command
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
