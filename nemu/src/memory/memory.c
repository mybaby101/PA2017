#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int NO = is_mmio(addr);
  if (NO == -1) {
	return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  } else {
	return mmio_read(addr, len, NO);
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int NO = is_mmio(addr);
  if (NO == -1) {
	memcpy(guest_to_host(addr), &data, len);
  } else {
	mmio_write(addr, len, data, NO);
  }
}

paddr_t page_translate(vaddr_t addr, bool is_write) {
  if (cpu.PG == 1) {
	paddr_t pde_base = cpu.cr3;
	paddr_t pde_addr = pde_base + ((addr >> 22) << 2);
	uint32_t pde = paddr_read(pde_addr, 4);
	if ((pde & 0x1) == 0)
	  Assert(0, "The present bit of pde is wrong!!!");
	paddr_t pte_base = pde & 0xfffff000;
	paddr_t pte_addr = pte_base + ((addr & 0x003ff000) >> 10);
	uint32_t pte = paddr_read(pte_addr, 4);
	if ((pte & 0x1) == 0)
	  Assert(0, "The present bit of pte is wrong!!!");
	paddr_t page_base = pte & 0xfffff000;
	pte |= 0x20;
	if (is_write)
		pte |= 0x40;
	paddr_write(pte_addr, 4, pte);
	return page_base + (addr & 0x00000fff);
  } else {
	return addr;
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_write(addr, len, data);
}
