// Copyright 2019-2020 - Omar Sandoval
// SPDX-License-Identifier: GPL-3.0+

#ifndef DRGN_PLATFORM_H
#define DRGN_PLATFORM_H

#include <elfutils/libdwfl.h>

#include "drgn.h"

struct drgn_register {
	const char *name;
	enum drgn_register_number number;
};

/* Register in NT_PRSTATUS note or struct pt_regs used for stack unwinding. */
struct drgn_frame_register {
	enum drgn_register_number number;
	size_t size;
	size_t prstatus_offset;
	/* Name used in the kernel. */
	const char *pt_regs_name;
	/* Name used for the UAPI, if different from above. */
	const char *pt_regs_name2;
};

struct pgtable_iterator {
	struct drgn_program *prog;
	uint64_t pgtable;
	uint64_t virt_addr;
};

typedef struct drgn_error *
(pgtable_iterator_next_fn)(struct pgtable_iterator *it, uint64_t *phys_page_ret,
			   uint64_t *page_offset_ret, uint64_t *page_size_ret);

struct drgn_architecture_info {
	const char *name;
	enum drgn_architecture arch;
	enum drgn_platform_flags default_flags;
	const struct drgn_register *registers;
	size_t num_registers;
	const struct drgn_register *(*register_by_name)(const char *name);
	const struct drgn_frame_register *frame_registers;
	size_t num_frame_registers;
	struct drgn_error *(*linux_kernel_set_initial_registers)(Dwfl_Thread *,
								 const struct drgn_object *);
	struct drgn_error *(*linux_kernel_get_page_offset)(struct drgn_program *,
							   uint64_t *);
	struct drgn_error *(*linux_kernel_get_vmemmap)(struct drgn_program *,
						       uint64_t *);
	struct drgn_error *(*linux_kernel_live_direct_mapping_fallback)(struct drgn_program *,
									uint64_t *,
									uint64_t *);
	size_t pgtable_iterator_size;
	void (*pgtable_iterator_init)(struct pgtable_iterator *it);
	pgtable_iterator_next_fn *pgtable_iterator_next;
};

static inline const struct drgn_register *
drgn_architecture_register_by_name(const struct drgn_architecture_info *arch,
				   const char *name)
{
	if (!arch->register_by_name)
		return NULL;
	return arch->register_by_name(name);
}

extern const struct drgn_architecture_info arch_info_unknown;
extern const struct drgn_architecture_info arch_info_x86_64;

struct drgn_platform {
	const struct drgn_architecture_info *arch;
	enum drgn_platform_flags flags;
};

/**
 * Initialize a @ref drgn_platform from an architecture, word size, and
 * endianness.
 *
 * The default flags for the architecture are used other than the word size and
 * endianness.
 */
void drgn_platform_from_arch(const struct drgn_architecture_info *arch,
			     bool is_64_bit, bool is_little_endian,
			     struct drgn_platform *ret);

/** Initialize a @ref drgn_platform from an ELF header. */
void drgn_platform_from_elf(GElf_Ehdr *ehdr, struct drgn_platform *ret);

#endif /* DRGN_PLATFORM_H */
