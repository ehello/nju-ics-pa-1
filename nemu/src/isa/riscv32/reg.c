#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
    for (int i = 0; i < 32; i++) {
        printf("%s \t0x%08x \t%d\n", reg_name(i, 32), gpr(i), gpr(i));
    }
    printf("pc \t0x%08x \t%d\n", cpu.pc, cpu.pc);
}

word_t isa_reg_str2val(const char *s, bool *success) {
  if (strcmp(s, reg_name(0, 32)) == 0) {
    return 0;
  }
  if (strcmp(s, "$pc") == 0) {
    return cpu.pc;
  }
  s++;
  for (int i = 1; i < 32; i++) {
    if (strcmp(s, reg_name(i, 32)) == 0) {
      return gpr(i);
    }
  }
  *success = false;
  return 0;
}
