// This file contains definitions for the
// x86 memory management unit (MMU).

// Eflags register
#define FL_IF           0x00000200      // Interrupt Enable

// Control Register flags
#define CR0_PE          0x00000001      // Protection Enable
#define CR0_WP          0x00010000      // Write Protect
#define CR0_PG          0x80000000      // Paging

#define CR4_PSE         0x00000010      // Page size extension

// various segment selectors.
#define SEG_KCODE 1  // kernel code
#define SEG_KDATA 2  // kernel data+stack
#define SEG_UCODE 3  // user code
#define SEG_UDATA 4  // user data+stack
#define SEG_TSS   5  // this process's task state

// cpu->gdt[NSEGS] holds the above segments.
#define NSEGS     6

#ifndef __ASSEMBLER__

// Set up a segment descriptor
#define SEG(type, base, lim, dpl) (                     \
	(((uint64)(uint)(lim) >> 12) & 0xffff)          \
	| ((uint64)(uint)(base) & 0xffff) << 16         \
	| (((uint64)(uint)(base) >> 16) & 0xff) << 32   \
	| ((uint64)(type)) << 40                        \
	| 1ULL << 44                                    \
	| (uint64)(dpl) << 45                           \
	| 1ULL << 47                                    \
	| ((uint64)(uint)(lim) >> 28) << 48             \
	/* skip 2 zeroes */                             \
	| 1ULL << 54                                    \
	| 1ULL << 55                                    \
	| (((uint64)(uint)(base)) >> 24) << 56          \
)

#define SEG16(type, base, lim, dpl) (                   \
	((uint64)(uint)(lim) & 0xffff)                  \
	| ((uint64)(uint)(base) & 0xffff) << 16         \
	| (((uint64)(uint)(base) >> 16) & 0xff) << 32   \
	| ((uint64)(type)) << 40                        \
	| 1ULL << 44                                    \
	| (uint64)(dpl) << 45                           \
	| 1ULL << 47                                    \
	| ((uint64)(uint)(lim) >> 16) << 48             \
	/* skip 2 zeroes */                             \
	| 1ULL << 54                                    \
	/* skip 1 zero */                               \
	| (((uint64)(uint)(base)) >> 24) << 56          \
)

// Set system bit to 0.
#define SEG_CLS(seg) { (seg) &= ~(1ULL << 44); }
#endif

#define DPL_USER    0x3     // User DPL

// Application segment type bits
#define STA_X       0x8     // Executable segment
#define STA_W       0x2     // Writeable (non-executable segments)
#define STA_R       0x2     // Readable (executable segments)

// System segment type bits
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

// A virtual address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/

// page directory index
#define PDX(va)         (((uint)(va) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(va)         (((uint)(va) >> PTXSHIFT) & 0x3FF)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

#define PTXSHIFT        12      // offset of PTX in a linear address
#define PDXSHIFT        22      // offset of PDX in a linear address

#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PS          0x080   // Page Size

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint)(pte) & ~0xFFF)
#define PTE_FLAGS(pte)  ((uint)(pte) &  0xFFF)

#ifndef __ASSEMBLER__

// Task state segment format
struct taskstate {
	uint link;         // Old ts selector
	uint esp0;         // Stack pointers and segment selectors
	ushort ss0;        //   after an increase in privilege level
	ushort padding1;
	uint *esp1;
	ushort ss1;
	ushort padding2;
	uint *esp2;
	ushort ss2;
	ushort padding3;
	void *cr3;         // Page directory base
	uint *eip;         // Saved state from last task switch
	uint eflags;
	uint eax;          // More saved state (registers)
	uint ecx;
	uint edx;
	uint ebx;
	uint *esp;
	uint *ebp;
	uint esi;
	uint edi;
	ushort es;         // Even more saved state (segment selectors)
	ushort padding4;
	ushort cs;
	ushort padding5;
	ushort ss;
	ushort padding6;
	ushort ds;
	ushort padding7;
	ushort fs;
	ushort padding8;
	ushort gs;
	ushort padding9;
	ushort ldt;
	ushort padding10;
	ushort t;          // Trap on task switch
	ushort iomb;       // I/O map base address
};

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
//   interrupt gate clears FL_IF, trap gate leaves FL_IF alone
// - sel: Code segment selector for interrupt/trap handler
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//        the privilege level required for software to invoke
//        this interrupt/trap gate explicitly using an int instruction.
#define SETGATE(gate, istrap, sel, off, d)                        \
{                                                                 \
	(gate) = 0;                                               \
	(gate) |= (uint)(off) & 0xffff;                           \
	(gate) |= ((sel) & 0xffff) << 16;                         \
	(gate) |= (uint64)((istrap) ? STS_TG32 : STS_IG32) << 40; \
	(gate) |= (uint64)(d) << 45;                              \
	(gate) |= 1ULL << 47;                                     \
	(gate) |= ((uint64)(off) >> 16) << 48;                    \
}

#endif
