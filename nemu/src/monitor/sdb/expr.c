#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ,
  TK_DEC,
  TK_HEX,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

//   {"[\\+-]?[[:digit:]]+", TK_DEC},  // decimal number
  {"0x[[:digit:]a-fA-F]+", TK_HEX}, // hexadecimal number
  {"[[:digit:]]+", TK_DEC},         // decimal number
  {" +", TK_NOTYPE},                // spaces
  {"\\(", '('},                     // left parenthese
  {"\\)", ')'},                     // right parenthese
  {"\\+", '+'},                     // plus
  {"-", '-'},                       // minus
  {"\\*", '*'},                     // multiple
  {"/", '/'},                       // divide
  {"==", TK_EQ},                    // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_DEC: case TK_HEX:
          if (substr_len > 31) {
            // token is too long.
            return false;
            break;
          }
          memcpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
        case '+': case '-': case '*': case '/':
        case '(': case ')':
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;
        case TK_NOTYPE:
          break;
        default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

// check if the sub-expression is wrapped by a pair of parentheses
static bool check_parentheses(int begin, int end)
{
  if (tokens[begin].type != '(' || tokens[end].type != ')') {
    return false;
  }

  // check the remaining parentheses, use an integer 'match'
  // to simulate a stack
  int match = 0;
  for (begin = begin + 1; begin < end; begin++) {
    if (match < 0) {
      return false;
    }
    if (tokens[begin].type == '(') {
      match++;
    } else if (tokens[begin].type == ')') {
      match--;
    }
  }
  if (match == 0) {
    return true;
  } else {
    return false;
  }
}

// calculate the value of tokens begin with 'begin' and end with 'end'
static word_t eval(int begin, int end, bool *success)
{
  if (*success == false) {
    return 0;
  }
  if (begin > end) {
    *success = false;
    return 0;
  } else if (begin == end) {
    uint32_t ret;
    switch (tokens[begin].type) {
    case TK_DEC:
      ret = atoi(tokens[begin].str);
      return ret;
      break;
    case TK_HEX:
      sscanf(tokens[begin].str, "%x", &ret);
      return ret;
      break;
    default:
      *success = false;
      return 0;
    }
  } else if (check_parentheses(begin, end)) {
    return eval(begin + 1, end - 1, success);
  } else {
    // find the main operator
    int op = 0, type = 0, match = 0;
    for (int i = begin; i <= end; i++) {
      if (match < 0) {
        *success = false;
        return 0;
      }
      switch (tokens[i].type) {
      case '(':
        match++;
        break;
      case ')':
        match--;
        break;
      case '+': case '-': case '*': case '/':
        if (match == 0 && (((type == 0 || type == '*' || type == '/') && (tokens[i].type == '+' || tokens[i].type == '-')) || 
                           ((type == 0)                               && (tokens[i].type == '*' || tokens[i].type == '/')))) {
          type = tokens[i].type;
          op = i;
        }
        break;
      default:
        break;
      }
    }
    if (type == 0) {
      *success = false;
      return 0;
    }

    int rval = eval(begin, op - 1, success);
    int lval = eval(op + 1, end, success);

    switch (type) {
      case '+': return rval + lval;
      case '-': return rval - lval;
      case '*': return rval * lval;
      case '/': return rval / lval;
      default: assert(0);
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */

  // Initialize '*success' to true, if there is something wrong
  // in eval, it will be set to false.
  *success = true;
  return eval(0, nr_token - 1, success);
}
