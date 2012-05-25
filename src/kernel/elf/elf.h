#ifndef __ELF_KLOADER_H__
#define __ELF_KLOADER_H__

/* This file is an ELF loader used by the kernel
 * to load programs into memory
 */

#include "types.h"
#include "pcbs.h"

/* Most of this documentation was transcribed from the ELF specification 
 *
 * These data structures and types were taken from the ELF specification
 */

typedef Uint32 Elf32_Addr;
typedef Uint16 Elf32_Half;
typedef Uint32 Elf32_Off;
typedef Int32  Elf32_Sword;
typedef Uint32 Elf32_Word;

/* For the Intel 32-bit architecture we want the following values
 * EI_CLASS - ELF_CLASS_32
 * EI_DATA  - ELF_DATA_2LSB
 *
 * In the header:
 * e_machine - EM_386
 * e_flags   - 0x0
 */

/* Note on ELF sections:
 *   -Every section has ONE section header describing it. Section headers
 *    may exist without a section
 *
 *   -Each section occupies one contiguous (possibly empty) sequence of bytes
 *    within a file
 *
 *   -Sections in a file may not overlap. No byte in a file resides in more than
 *    one section
 *
 *   -An object file may have inactive space. The various headers and sections
 *    might not "cover" every byte in an object file. The contents of the inactive
 *    data are unspecified
 */

/* Special sections:
 * Name     | Type         | Attributes
 * -----------------------------------------
 * .bss     | SHT_NOBITS   | SHF_ALLOC + SHF_WRITE
 * .comment | SHT_PROGBITS | None
 * .data    | SHT_PROGBITS | SHF_ALLOC + SHF_WRITE
 * .data1   | SHT_PROGBITS | SHF_ALLOC + SHF_WRITE
 * .debug   | SHT_PROGBITS | None
 * .dynamic | SHT_DYNAMIC  | See below
 * .dynstr  | SHT_STRTAB   | SHF_ALLOC
 * .dynsym  | SHT_DYNSYM   | SHF_ALLOC
 * .fini    | SHT_PROGBITS | SHF_ALLOC + SHF_EXECINSTR
 * .got     | SHT_PROGBITS | See below
 * .hash    | SHT_HASH     | SHF_ALLOC
 * .init    | SHT_PROGBITS | SHF_ALLOC + SHF_EXECINSTR
 * .interp  | SHT_PROGBITS | See below
 * .line    | SHT_PROGBITS | None
 * .note    | SHT_NOTE     | None
 * .plt     | SHT_PROGBITS | See below
 * .relname | SHT_REL      | See below
 * .relaname| SHT_RELA     | See below
 * .rodata  | SHT_PROGBITS | SHF_ALLOC
 * .rodata1 | SHT_PROGBITS | SHF_ALLOC
 * .shstrtab| SHT_STRTAB   | None
 * .strtab  | SHT_STRTAB   | See below
 * .symtab  | SHT_SYMTAB   | See below
 * .text    | SHT_PROGBITS | SHF_ALLOC + SHF_EXECINSTR
 * -----------------------------------------
 *
 * .bss - Holds uninitialized data that contributes to the program's memory image. By definition
 *        the system initializes the data with zeros when the program begins to run
 *
 * .comment - Holds version control information
 *
 * .data and .data1 - Hold initialized data that contributes to the programs memory image
 *
 * .debug - Section holds symbolic debug information
 *
 * .dynamic - Holds dynamic linking information
 *
 * .dynstr - Strings needed for dynamic linking, mnost commonly the strings that represent the names
 *           associated with symbol table entries.
 *
 * .dynsym - Holds the dynamic linking symbol table
 *
 * .fini - Holds executable instructions that contribute to the process termination code.
 *
 * .got - Section holds the global offset table
 *
 * .hash - Holds a symbol hash table
 *
 * .init - Executable instructions that contribute to the process initialization code.
 * 
 * .interp - Holds the path name of a program interpreter. If the file hash a loadable segment
 *           that include the section, the sections attribute will include SHF_ALLOC, otherwise
 *           the bit will be off
 *
 * .line - Holds line number information for symbolic debugging
 *
 * .note - Holds note information
 *
 * .plt - Procedure linkage table
 *
 * .relname and relaname - Holds relocation information. If the file has a loadable segment
 *                         that includes relocation, the sections attributes will include 
 *                         SHF_ALLOC. Conventionally name is supplied by the section to which
 *                         the relocations apply. Thus a relocation section for .text normally
 *                         would ahve the name .rel.text or .rela.text
 *
 * .shstrtab - Holds section names
 *
 * .strtab - Holds strings, most commonly the strings that represent the names associated with symbol
 *           table entries. If the file has a loadable segment that includes the symbol string table
 *           the sections attributes will contain SHF_ALLOC
 *
 * .symtab - Section holds a symbol table. If the file has a loadable segment that includes the symbol
 *           table the SHF_ALLOC bit will be set
 *
 * .text - The executable program
 */


/* Used for ELF identification */
#define EI_NIDENT 12 /* Size of e_ident */
/* Below are the indices of the different sections
 * of e_ident
 */
#define ELF_MAGIC_NUM 0x464C457F /* 0x7F 'E' 'L' 'F' (Little endian) */
#define EI_MAG0 0 /* Should be 0x7f */
#define EI_MAG1 1 /* Should be 'E' */
#define EI_MAG2 2 /* Should be 'L' */
#define EI_MAG3 3 /* Should be 'F' */

#define EI_CLASS 4 /* 0 - invalid, 1 - 32-bit objects, 2 - 64-bit objects */
#define ELF_CLASS_NONE 0
#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

#define EI_DATA 5 /* Data encoding */
#define ELF_DATA_NONE 0 /* Invalid encoding */
#define ELF_DATA_2LSB 1 /* Little endian */
#define ELF_DATA_2MSB 2 /* Big endian */

#define EI_VERSION 6 /* ELF header version number */
#define EI_PAD 7 /* Start of ignored bytes in the e_ident field */
/****/


/* ELF file types */
#define ET_NONE 0 /* No file type */
#define ET_REL  1 /* Relocatable file */
#define ET_EXEC 2 /* Executable file */
#define ET_DYN  3 /* Shared object file */
#define ET_CORE 4 /* Core file */
#define ET_LOPROC 0xFF00 /* Processor specific */
#define ET_HIPROC 0xFFFF /* Processor specific */
/****/

/* ELF machine types */
#define EM_NONE  0 /* No machine */
#define EM_M32   1 /* AT&T WE 32100 */
#define EM_SPARC 2 /* SPARC */
#define EM_386   3 /* Intel 80386 */
#define EM_68K   4 /* Motorola 68000 */
#define EM_88K   5 /* Motorola 88000 */
#define EM_860   7 /* Intel 80860 */ 
#define EM_MIPS  8 /* MIPS RS3000 */
/****/

/* ELF version numbers */
#define EV_NONE 0 /* Invalid version */
#define EV_CURRENT 1 /* Current version */
/****/

/* ELF section indexes */
#define SHN_UNDEF 0 /* Undefined, missing, or irrelevant section reference */
#define SHN_LORESERVE 0xFF00 /* Lower bound of the range of reserved indexes */
#define SHN_LOPROC 0xFF00 /* Values between LO_PROC and HI_PROC are for */
#define SHN_HIPROC 0xFF1F /* processor specific semantics */
#define SHN_ABS    0xFFF1 /* Absolute values not affected by relocation */
#define SHN_COMMON 0xFFF2 /* Common symbols, like unallocated C external variables */
#define SHN_HIRESERVE 0xFFFF /* Upper bound of the range of reserved references.
								Reserved indexes are between LORESERVE and 
								HIRESERVE inclusive. These values are not in the
								section header table
							  */
/****/

typedef struct {
	Elf32_Word e_magic;
	Uint8      e_ident[EI_NIDENT]; /* Marks the file as an object file, 
									* and provides machine-independent data
									*/
	Elf32_Half e_type;         /* Identifies the objects file type */
	Elf32_Half e_machine;      /* The architecture of the ELF file */
	Elf32_Word e_version;      /* File version, 1 is original (no extensions) */ 
	Elf32_Addr e_entry;        /* Virtual address to jump to, zero if none */
	Elf32_Off  e_phoff;        /* Program header table offset in bytes */
	Elf32_Off  e_shoff;        /* Section table header offset in bytes */
	Elf32_Word e_flags;        /* Processor specific flags */
	Elf32_Half e_ehsize;       /* ELF header size in bytes */
	Elf32_Half e_phentsize;    /* Program header size bytes */
	Elf32_Half e_phnum;        /* Number of program headers */
	Elf32_Half e_shentsize;    /* Section header size in bytes */
	Elf32_Half e_shnum;        /* Number of section headers */
	Elf32_Half e_shstrndx;     /* The section header table index of the entry
								  with the section name string table.
								  If there is no entry it has the value SHN_UNDEF
								*/
} Elf32_Ehdr;

/* ELF section header types */
#define SHT_NULL 0 /* Section header is inactive, no associated section */
#define SHT_PROGBITS 1 /* Holds information defined by the program whose format
						  and meaning are defined by the program */
#define SHT_SYMTAB 2 /* Holds a symbol table. Currently only one such section
						can exist. Symbols for link editing, but may be used
						for dynamic linking as well. Some symbols may not be
						used for linking. The DYNSYM section defines the minimal
						amount of symbols needed for dynamic linking */
#define SHT_STRTAB 3 /* Holds a string table, a file can have multiple string
						table sections */
#define SHT_RELA 4 /* Section holds relocation entries with explicit addedns, such
					  as ELF32_Rela, a file can have more than one of this section
					*/
#define SHT_HASH 5 /* Symbol hash table. All objects in dynamic linking must
					  contain a symbol hash table. Only one of these should exist
					  (may be relaxed in the future) */
#define SHT_DYNAMIC 6 /* Holds information for dynamic linking. Currently there
						 should only be one (But may be relaxed in the future) */
#define SHT_NOTE 7 /* Holds information that marks the file in some way */
#define SHT_NOBITS 8 /* Occupies no space in the file, but otherwise resembles
						SHT_PROGBITS */
#define SHT_REL 9 /* Holds the relocation entries without explicit addens, such
					 as type Elf32_Rel for the 32-bit class of objects. Can have
					 multiple sections. */
#define SHT_SHLIB 10 /* Reserved, programs that contain this section do not
						conform to the ABI
					  */
#define SHT_DYNSYM 11 /* Dynamic symbol table */
#define SHT_LOPROC 0x70000000 /* Values between LOPROC and HIPROC (inclusive) are
								 reserved for processor-specific semantics */
#define SHT_HIPROC 0x7FFFFFFF
#define SHT_LOUSER 0x80000000 /* Upper bound of the range of addresses reserved
								 for application programs */
#define SHT_HIUSER 0xFFFFFFFF
/****/

/* ELF header flag attributes
 * 
 * A set bit means the flag is 'on'
 */
#define SHF_WRITE 0x1 /* Section contains data that should be writable during
						 process execution */
#define SHF_ALLOC 0x2 /* Section occupies memory during process execution. Some
						 control sections do not reside in the memory image
						 of an object file; this attribute is off for those
						 sections */
#define SHF_EXECINSTR 0x4 /* This section contains executable machine instructions
						   */
#define SHF_MASKPROC 0xF0000000 /* All bits in this mask are reserved for processor
								   specific semantics */
/****/

/* Information about Section Header Table Entry Index 0 
 *
 * sh_name      - 0 - No name
 * sh_type      - SHT_NULL - Inactive
 * sh_flags     - 0 - No flags
 * sh_addr      - 0 - No address
 * sh_offset    - 0 - No file offset
 * sh_size      - 0 - No size
 * sh_link      - SHN_UNDEF - No link information
 * sh_info      - 0 - No auxilary information
 * sh_addralign - 0 - No alignment
 * sh_entsize   - 0 - No entries
 */

/* ELF section header */
typedef struct {
	Elf32_Word sh_name;      /* Name of the section. The value is the index 
								into the section header string table.*/
	Elf32_Word sh_type;      /* Defines the sections content and semantics */
	Elf32_Word sh_flags;     /* 1-bit flags */
	Elf32_Addr sh_addr;      /* The address at which the sections first byte
								should reside, or 0 if not specified */
	Elf32_Off  sh_offset;    /* Byte offset from the beginning of the file to
								the first byte in the section. SH_NOBITS occupies
								no space in the file, and sh_offset is the
								conceptual placement in the file */
	Elf32_Word sh_size;      /* The size of the section in bytes. If the section
								is SH_NOBITS it does not occupy any space in the
								file, but has space in memory */
	Elf32_Word sh_link;      /* Holds a section header table index link, whose
								interpretation depends on the section type */
	Elf32_Word sh_info;      /* This member holds extra information, whose 
								interpretation depends on the section type */
	Elf32_Word sh_addralign; /* Means sh_addr % sh_addralign == 0, values of 
								0 and 1 mean there are no alignment constraints */
	Elf32_Word sh_entsize;   /* Some sections hold a table of fixed-size entries,
								such as a symbol table. For such a section This
								member gives the size in bytes of each entry. 
								Contains 0 if the section does not have such a
								table */
} Elf32_Shdr;

/* ELF Symbol binding */
#define STB_LOCAL 0 /* Load symbols are not visible outside the object file containing their definition */
#define STB_GLOBAL 1 /* Global symbols are visible to all object files being combined */
#define STB_WEAK 2 /* Weak symbols resemble global symbols, but their definitions have lower precedence */
#define STB_LOPROC 13 /* Values in the range LOPROC to HIPROC (inclusive) are reserved for process-specific
						 semantics */
#define STB_HIPROC 15
/****/

/* ELF symbol table */
typedef struct {
	Elf32_Word st_name; /* Holds an index into the object file's symbol string table, 0 means no name */
	Elf32_Addr st_value; /* Gives the value of the associated symbol. May be an absolute value, address, etc */
	Elf32_Word st_size; /* Many symbols have associated sizes. For example a data object is the number of
						   bytes contained in the object. This member holds 0 if the symbol has no size or
						   an unknown size */
	Uint8      st_info; /* This member specifies the symbol's type and binding attributes */
	Uint8      st_other; /* Member currently holds 0 and has no defined meaning */
    Elf32_Half st_shndx; /* Every symbol entry is 'defined' in relation to some section. This member holds
							the relevant section header table index */
} Elf32_Sym;

typedef struct {
	Elf32_Addr r_offset; /* Gives the location at which to apply the relocation action.
							For a relocatable file, the value is the byte offset from
							the beginning of the section to the storage unit affected by
							the relocation. For an executable file or shared object, the
							value is the virtual address of the storage unit affected by
							the relocation */
	Elf32_Word r_info; /* This member gives both the symbol table index with respect to
						  which the relocation must be made, and the type of relocation
						  to apply. For example a call instruction's relocation entry would
						  hold the symbol table index of the function being called. If the
						  index is STN_UNDEF, the undefined symbol index, the relocation uses
						  0 as the 'symbol value'. Relocation types are processor-specific.
						  When the text refers to a relocation entry's relocation type or
						  symbol table index, it means the reault of applying ELF32_R_TYPE or
						  ELF32_R_SYM, respectively to the entry's r_info member */
} Elf32_Rel;

#define ELF32_R_SYM(i) ((i) >> 8)
#define ELF32_R_TYPE(i) ((Uint8)(i))
#define ELF32_R_INFO(s,t) (((s) << 8) + (Uint8)(t))

typedef struct {
	Elf32_Addr  r_offset; /* Section offset. The relocation section itself describes how to 
							 modify another section in the file. Relocation offsets designate
							 a storage unit within the second section */
	Elf32_Word  r_info;
	Elf32_Sword r_addend; /* This member specifies a constant addend used to compute the
							 value to be stored into the relocatable field */
} Elf32_Rela;

/* ELF program header */
typedef struct {
	Elf32_Word p_type; /* Tells what kind of segment this array element describes or how
						  to interpret the array of elements */
	Elf32_Off  p_offset; /* The offset from the beginning of the file at which the first
							byte of the segment resides */
	Elf32_Addr p_vaddr; /* The virtual address at which the first byte of the segment resides
						   in memory */
	Elf32_Addr p_paddr; /* Segments physical address, ignored */
	Elf32_Word p_filesz; /* The number of bytes in the file image of the segment, may be zero */
	Elf32_Word p_memsz; /* The number of bytes in the memory image of the segment, may be zero */
	Elf32_Word p_flags; /* Flags relevant to the segment */
	Elf32_Word p_align; /* 0 and 1 mean no alignment required. p_align should be positive, integral
						   power of 2, and p_vaddr should equal p_offset % p_align */
} Elf32_Phdr;
/****/

/* ELF program headers flags */
#define PF_WRITE 0x2
/****/

/* ELF program types */
#define PT_NULL 0 /* Array element is unused */
#define PT_LOAD 1 /* Specifies a loadable segment. described by p_filesz and p_memsz. The bytes
					 from the file are mapped to the beginning of the memory segment. If the
					 segments memory size (p_memsz) is larger than the file size (p_filesz) the
					 "extra" bytes are defined to hold the value 0 and to follow the segment's
					 initialized area. The file size my not be larger than the memory size. Loadable
					 segment entries in the program header table appear in ascending order, sorted
					 on the p_vaddr member */
#define PT_DYNAMIC 2 /* Dynamic linking information */
#define PT_INTERP 3 /* The location and size of a null terminated path name to invoke as an interpreter.
					   This segment type is meaningful only for executable files. It may not occur more
					   than once in a file. If it is present it must precede any loadable segment entry */
#define PT_NOTE 4 /* Auxilary information */
#define PT_SHLIB 5 /* Is reserved but has unspecified semantics. Programs with this do not conform to
					  the ABI */
#define PT_PHDR 6 /* Specifies the location and size of the program header table itself, both in the file
					 and in the memory image of the program. This segment type may not occur more than once
					 in a file. Moreover it may occur only if the program header table is part of the 
					 memory image of the program. If it is present, it must precede any loadable segment
					 entry */
#define PT_LOPROC 0x70000000 /* LOPROC - HIPROC (inclusive) is reserved for processor specific semantics */
#define PT_HIPROC 0x7fffffff
/****/

/* Determining the base address 
 * 
 * To compute the base address, one determines the memory address associated with the lowest p_vaddr value
 * for a PT_LOAD segment. One then obtains the base address by truncating the memory address to the nearest
 * multiple of the maxiumum page size. Depending on the type of file being loaded into memory, the memory
 * address might or might not match the p_vaddr values.
 *
 * The .bss section has the type SHT_NOBITS. Although it occupies no space in the file, it contributes to
 * the segment's memory image. Normally these uninitialized data reside at the end of the segment, making
 * p_memsz larger than p_filesz in the associated program header element
 */


/* Program loading
 *
 *                             ELF File         Virtual Address
 *                      .---------------------.
 *                    0 |     ELF Header      |
 *                      |---------------------|
 * Program header table |                     |
 *                      |---------------------|
 *                      |  Other information  |
 *                      |---------------------|
 *                0x100 |    Text segment     | 0x8048100
 *                      |        ...          |
 *                      |    0x2be00 bytes    | 0x8073eff
 *                      |---------------------|
 *              0x2bf00 |    Data Segment     | 0x8074f00
 *                      |        ...          |
 *                      |    0x4e00 bytes     | 0x8079cff
 *                      |---------------------|
 *              0x30d00 |  Ohter information  |
 *                      |        ....         |
 *                      '---------------------'
 *
 */

Status _elf_load_from_file(Pcb* pcb, const char* file_name);

#endif
