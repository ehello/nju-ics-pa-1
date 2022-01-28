#include "sdb.h"
#include <string.h>
#include <stdlib.h>

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char *e;
  word_t old_val; 

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
static inline WP *alloc_wp()
{
  WP *ret = free_;
  if (free_ != NULL) {
    free_ = free_->next;
    ret->next = head;
    head = ret;
  }
  return ret;
}

static inline void free_wp(WP *wp)
{
  if (wp == NULL) {
    return;
  }
  free(wp->e);
  wp->next = free_;
  free_ = wp;
}

int del_wp(int wp_no)
{
  WP *wp, *pre = head;
  for (wp = head; wp != NULL; wp = wp->next) {
    if (wp->NO == wp_no) {
      break;
    }
    pre = wp;
  }
  if (wp == NULL) {
    return 1;
  }
  if (wp == head) {
    head = wp->next;
  } else {
    pre->next = wp->next;
  }
  free_wp(wp);
  return 0;
}

int add_wp(char* e)
{
  WP *wp;
  bool success = true;
  word_t old_val;

  old_val = expr(e, &success);
  if (success == false) {
    return 3;
  }

  if ((wp = alloc_wp()) == NULL) {
    return 2;
  }
  wp->e = strdup(e);
  wp->old_val = old_val;
  return 0;
}

bool check_wp(word_t pc)
{
  bool success = true, change = false;
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    word_t val = expr(wp->e, &success);
    if (val != wp->old_val) {
      printf("Hit watchpoint %d at 0x%08x.\n", wp->NO, pc);
      wp->old_val = val;
      change = true;
    }
  }
  return change;
}

void display_wp()
{
  if (head == NULL) {
    printf("There is no watchpoint set.\n");
    return;
  }
  printf("%8s\t%8s\n", "NO", "EXPR");
  for (WP *wp = head; wp != NULL; wp = wp->next) {
    printf("%8d\t%8s\n", wp->NO, wp->e);
  }
  return;
}
