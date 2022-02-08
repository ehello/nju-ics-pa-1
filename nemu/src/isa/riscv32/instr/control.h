def_EHelper(jal) {
  rtl_j(s, s->pc + id_src1->simm);
  rtl_li(s, ddest, s->pc + 4);
}

def_EHelper(jalr) {
  vaddr_t target = (*(id_src1->preg) + id_src2->simm) & ((~0) << 1);
  rtl_j(s, target);
  rtl_li(s, ddest, s->pc + 4);
}