#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

int del_wp(int wp_no);
int add_wp(char* e);
void display_wp();
word_t expr(char *e, bool *success);

#endif
