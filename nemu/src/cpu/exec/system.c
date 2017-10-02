#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  if (decoding.is_operand_size_16) {
	cpu.idtr.limit = vaddr_read(id_dest->addr, 2);
	cpu.idtr.base = vaddr_read(id_dest->addr + 2, 4) & 0x00ffffff;
  } else {
	cpu.idtr.limit = vaddr_read(id_dest->addr, 2);
	cpu.idtr.base = vaddr_read(id_dest->addr + 2, 4);
  }
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

extern void raise_intr(uint8_t, vaddr_t);

make_EHelper(int) {
  raise_intr(id_dest->val, decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
	printf("in iret!\n");
  if (decoding.is_operand_size_16) {
	Assert(0, "iret operand size is 16");
  } 
  rtl_pop((rtlreg_t *) &cpu.eip);
  rtl_pop((rtlreg_t *) &cpu.cs);
  rtl_pop((rtlreg_t *) &cpu.eflags_ini);

  decoding.jmp_eip = cpu.eip;
  decoding.is_jmp = true;

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  t0 = pio_read(id_src->val, id_src->width);
  operand_write(id_dest, &t0);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val, id_src->width, id_src->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
