#ifndef _H_BC
#define _H_BC

typedef unsigned char u8;
typedef char s8;
typedef unsigned short int u16;
typedef short int s16;
typedef unsigned int u32;
typedef int s32;
typedef unsigned long long int u64;
typedef long long int s64;
typedef float f32;
typedef double f64;


typedef union reg {
	u8  u8;
	s8  s8;
	u16 u16;
	s16 s16;
	u32 u32;
	s32 s32;
	u64 u64;
	s64 s64;
	f32 f32;
	f64 f64;
} reg;

typedef u32 op;
const op ADD32 = 0;

typedef struct bytecode {
	op op;
	reg r0;
	reg r1;
	reg r2;
	reg r3;
} bytecode;

typedef struct bc_code{
	bytecode *code;
	unsigned int capacity;
	unsigned int length;
} bc_code;

bc_code make_bc_code();
bc_code join(bc_code *a, bc_code *b);

bytecode make_bytecode(op op, s32 r0, s32 r1, s32 r2, s32 r3);
void add_bytecode(bc_code *bcode, bytecode bc);

#endif
