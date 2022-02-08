def_EHelper(jal) {
  rtl_j(s, s->pc + id_src1->simm);
  rtl_li(s, ddest, s->pc + 4);
}