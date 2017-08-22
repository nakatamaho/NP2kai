#include	"compiler.h"
#include	"cpucore.h"
#include	"i286x.h"
#include	"i286xadr.h"
#include	"i286xs.h"
#include	"i286xrep.h"
#include	"i286xcts.h"
#include	"pccore.h"
#include	"bios/bios.h"
#include	"iocore.h"
#include	"i286x.mcr"
#include	"i286xea.mcr"
#include	"dmax86.h"
#if defined(ENABLE_TRAP)
#include "trap/steptrap.h"
#endif


typedef struct {
	UINT	opnum;
	I286TBL	v30opcode;
} V30PATCH;

static	I286TBL v30op[256];
static	I286TBL v30op_repne[256];
static	I286TBL	v30op_repe[256];
static	I286TBL	v30ope0xf6_xtable[8];
static	I286TBL v30ope0xf7_xtable[8];

#if defined(SUPPORT_V30ORIGINAL)
static	I286TBL	v30op_repc[256];
#endif

static const UINT8 rotatebase16[256] =
				{0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15};

static const UINT8 rotatebase09[256] =
				{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6,
				 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4,
				 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2,
				 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9,
				 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7,
				 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5,
				 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3,
				 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1,
				 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8,
				 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6,
				 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4,
				 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2,
				 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9,
				 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7,
				 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3, 4, 5,
				 6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 1, 2, 3};

static const UINT8 rotatebase17[256] =
				{0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16,17, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
				15,16,17, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,
				14,15,16,17, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,
				13,14,15,16,17, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,
				12,13,14,15,16,17, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,
				11,12,13,14,15,16,17, 1, 2, 3, 4, 5, 6, 7, 8, 9,
				10,11,12,13,14,15,16,17, 1, 2, 3, 4, 5, 6, 7, 8,
				 9,10,11,12,13,14,15,16,17, 1, 2, 3, 4, 5, 6, 7,
				 8, 9,10,11,12,13,14,15,16,17, 1, 2, 3, 4, 5, 6,
				 7, 8, 9,10,11,12,13,14,15,16,17, 1, 2, 3, 4, 5,
				 6, 7, 8, 9,10,11,12,13,14,15,16,17, 1, 2, 3, 4,
				 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17, 1, 2, 3,
				 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17, 1, 2,
				 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17, 1,
				 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17};

static const UINT8 shiftbase[256] =
				{0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
				16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
				17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17};



I286 v30_reserved(void) {

		__asm {
				I286CLOCK(2)
				GET_NEXTPRE1
				ret
		}
}

I286 v30pop_ss(void) {							// 17: pop ss

		__asm {
				I286CLOCK(5)
				REGPOP(I286_SS)
				and		eax, 00ffffh
				shl		eax, 4					// make segreg
				mov		SS_BASE, eax
				mov		SS_FIX, eax
				cmp		i286core.s.prefix, 0	// 00/06/24
				je		noprefix
				call	removeprefix
				pop		eax
		noprefix:
				movzx	ebp, bh
				GET_NEXTPRE1
				jmp		v30op[ebp*4]
		}
}

I286 v30segprefix_es(void) {					// 26: es:

		__asm {
				mov		eax, ES_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				I286PREFIX(v30op)
		}
}

I286 v30segprefix_cs(void) {					// 2E: cs:

		__asm {
				mov		eax, CS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				I286PREFIX(v30op)
		}
}

I286 v30segprefix_ss(void) {					// 36: ss:

		__asm {
				mov		eax, SS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				I286PREFIX(v30op)
		}
}

I286 v30segprefix_ds(void) {					// 3E: ds:

		__asm {
				mov		eax, DS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				I286PREFIX(v30op)
		}
}

I286 v30push_sp(void) {							// 54: push sp

		__asm {
				I286CLOCK(3)
				GET_NEXTPRE1
				sub		I286_SP, 2
				movzx	ecx, I286_SP
				mov		edx, ecx
				add		ecx, SS_BASE
				jmp		i286_memorywrite_w
		}
}

I286 v30pop_sp(void) {							// 5C: pop sp

		__asm {
				I286CLOCK(5)
				REGPOP(I286_SP)
				GET_NEXTPRE1
				ret
		}
}

#if defined(SUPPORT_V30ORIGINAL)
I286 v30_repc(void) {							// 65: repc

		__asm {
				I286PREFIX(v30op_repc)
		}
}
#endif

I286 v30mov_seg_ea(void) {						// 8E: mov segrem, EA

		__asm {
				movzx	eax, bh
				mov		ebp, eax
				shr		ebp, 3-1
				and		ebp, 3*2
				cmp		al, 0c0h
				jc		src_memory
				I286CLOCK(2)
				and		eax, 7
				mov		edi, eax
				GET_NEXTPRE2
				mov		ax, word ptr I286_REG[edi*2]
				jmp		segset
				align	4
		src_memory:
				I286CLOCK(5)
				call	p_ea_dst[eax*4]
				call	i286_memoryread_w
		segset:
#if 0
				cmp		ebp, 1*2				// prefixed cs?
				je		segsetr
#endif
				mov		word ptr I286_SEGREG[ebp], ax
				and		eax, 0000ffffh
				shl		eax, 4					// make segreg
				mov		SEG_BASE[ebp*2], eax
				sub		ebp, 2*2
				jc		segsetr
				mov		SS_FIX[ebp*2], eax
				je		setss
		segsetr:ret

				align	4
		setss:	cmp		i286core.s.prefix, 0	// 00/05/13
				je		noprefix
				pop		eax
				call	eax						// eax<-offset removeprefix
		noprefix:
				movzx	eax, bl
				jmp		v30op[eax*4]
		}
}

I286 v30_pushf(void) {							// 9C: pushf

		__asm {
				GET_NEXTPRE1
				I286CLOCK(3)
				mov		dx, I286_FLAG
				or		dx, 0f000h
				sub		I286_SP, 2
				movzx	ecx, I286_SP
				add		ecx, SS_BASE
				jmp		i286_memorywrite_w
		}
}

I286 v30_popf(void) {							// 9D: popf

		__asm {
				GET_NEXTPRE1
				I286CLOCK(5)
				movzx	ecx, I286_SP
				add		ecx, SS_BASE
				call	i286_memoryread_w
				add		I286_SP, 2
#if defined(VAEG_FIX)
				or		ax, 0f002h
#else
				or		ah, 0f0h
#endif
				mov		I286_FLAG, ax
#if defined(VAEG_FIX)
				and		ah, 1
				cmp		ah, 1
#else
				and		ah, 3
				cmp		ah, 3
#endif
				sete	I286_TRAP
				I286IRQCHECKTERM
		}
}


// ----- reg8

I286 rol_r8_v30(void) {

		__asm {
				mov		cl, rotatebase16[ecx]
				rol		byte ptr [edx], cl
				FLAG_STORE_OC
				ret
		}
}

I286 ror_r8_v30(void) {

		__asm {
				mov		cl, rotatebase16[ecx]
				ror		byte ptr [edx], cl
				FLAG_STORE_OC
				ret
		}
}

I286 rcl_r8_v30(void) {

		__asm {
				mov		cl, rotatebase09[ecx]
				CFLAG_LOAD
				rcl		byte ptr [edx], cl
				FLAG_STORE_OC
				ret
		}
}

I286 rcr_r8_v30(void) {

		__asm {
				mov		cl, rotatebase09[ecx]
				CFLAG_LOAD
				rcr		byte ptr [edx], cl
				FLAG_STORE_OC
				ret
		}
}

I286 shl_r8_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				shl		byte ptr [edx], cl
				FLAG_STORE_OF
				ret
		}
}

I286 shr_r8_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				shr		byte ptr [edx], cl
				FLAG_STORE_OF
				ret
		}
}

I286 sar_r8_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				sar		byte ptr [edx], cl
				FLAG_STORE0
				ret
		}
}

static void (*sftreg8v30_table[])(void) = {
		rol_r8_v30,		ror_r8_v30,		rcl_r8_v30,		rcr_r8_v30,
		shl_r8_v30,		shr_r8_v30,		shl_r8_v30,		sar_r8_v30};

I286 rol_ext8_v30(void) {

		__asm {
				mov		cl, rotatebase16[ecx]
				rol		dl, cl
				FLAG_STORE_OC
				mov		ecx, ebp
				jmp		i286_memorywrite
		}
}

I286 ror_ext8_v30(void) {

		__asm {
				mov		cl, rotatebase16[ecx]
				ror		dl, cl
				FLAG_STORE_OC
				mov		ecx, ebp
				jmp		i286_memorywrite
		}
}

I286 rcl_ext8_v30(void) {

		__asm {
				mov		cl, rotatebase09[ecx]
				CFLAG_LOAD
				rcl		dl, cl
				FLAG_STORE_OC
				mov		ecx, ebp
				jmp		i286_memorywrite
		}
}

I286 rcr_ext8_v30(void) {

		__asm {
				mov		cl, rotatebase09[ecx]
				CFLAG_LOAD
				rcr		dl, cl
				FLAG_STORE_OC
				mov		ecx, ebp
				jmp		i286_memorywrite
		}
}

I286 shl_ext8_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				shl		dl, cl
				FLAG_STORE_OF
				mov		ecx, ebp
				jmp		i286_memorywrite
		}
}

I286 shr_ext8_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				shr		dl, cl
				FLAG_STORE_OF
				mov		ecx, ebp
				jmp		i286_memorywrite
		}
}

I286 sar_ext8_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				sar		dl, cl
				FLAG_STORE0
				mov		ecx, ebp
				jmp		i286_memorywrite
		}
}

static void (*sftext8v30_table[])(void) = {
		rol_ext8_v30,	ror_ext8_v30,	rcl_ext8_v30,	rcr_ext8_v30,
		shl_ext8_v30,	shr_ext8_v30,	shl_ext8_v30,	sar_ext8_v30};

I286 v30shift_ea8_data8(void) {					// C0: shift EA8, DATA8

		__asm {
				movzx	eax, bh
				mov		edi, eax
				shr		edi, 3-2
				and		edi, 7*4						// opcode
				cmp		al, 0c0h
				jc		memory_eareg8
				I286CLOCK(5)
				bt		ax, 2
				rcl		eax, 1
				and		eax, 7
				lea		edx, I286_REG[eax]
				mov		ecx, ebx
				shr		ecx, 16
				and		ecx, 255
				I286CLOCK(ecx)
				push	ecx
				GET_NEXTPRE3
				pop		ecx
				jmp		sftreg8v30_table[edi]
				align	4
		memory_eareg8:
				I286CLOCK(8)
				call	p_ea_dst[eax*4]
				cmp		ecx, I286_MEMWRITEMAX
				jnc		extmem_eareg8
				lea		edx, I286_MEM[ecx]
				movzx	ecx, bl
				I286CLOCK(ecx)
				push	ecx
				GET_NEXTPRE1
				pop		ecx
				jmp		sftreg8v30_table[edi]
				align	4
		extmem_eareg8:
				call	i286_memoryread
				mov		ebp, ecx
				mov		edx, eax
				movzx	ecx, bl
				I286CLOCK(ecx)
				push	ecx
				GET_NEXTPRE1
				pop		ecx
				jmp		sftext8v30_table[edi]
	}
}

I286 rol_r16_v30(void) {

		__asm {
				mov		cl, rotatebase16[ecx]
				rol		word ptr [edx], cl
				FLAG_STORE_OC
				ret
		}
}

I286 ror_r16_v30(void) {

		__asm {
				mov		cl, rotatebase16[ecx]
				ror		word ptr [edx], cl
				FLAG_STORE_OC
				ret
		}
}

I286 rcl_r16_v30(void) {

		__asm {
				mov		cl, rotatebase17[ecx]
				CFLAG_LOAD
				rcl		word ptr [edx], cl
				FLAG_STORE_OC
				ret
		}
}

I286 rcr_r16_v30(void) {

		__asm {
				mov		cl, rotatebase17[ecx]
				CFLAG_LOAD
				rcr		word ptr [edx], cl
				FLAG_STORE_OC
				ret
		}
}

I286 shl_r16_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				shl		word ptr [edx], cl
				FLAG_STORE_OF
				ret
		}
}

I286 shr_r16_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				shr		word ptr [edx], cl
				FLAG_STORE_OF
				ret
		}
}

I286 sar_r16_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				sar		word ptr [edx], cl
				FLAG_STORE0
				ret
		}
}

static void (*sftreg16v30_table[])(void) = {
		rol_r16_v30,	ror_r16_v30,	rcl_r16_v30,	rcr_r16_v30,
		shl_r16_v30,	shr_r16_v30,	shl_r16_v30,	sar_r16_v30};

I286 rol_ext16_v30(void) {

		__asm {
				mov		cl, rotatebase16[ecx]
				rol		dx, cl
				FLAG_STORE_OC
				mov		ecx, ebp
				jmp		i286_memorywrite_w
		}
}

I286 ror_ext16_v30(void) {

		__asm {
				mov		cl, rotatebase16[ecx]
				ror		dx, cl
				FLAG_STORE_OC
				mov		ecx, ebp
				jmp		i286_memorywrite_w
		}
}

I286 rcl_ext16_v30(void) {

		__asm {
				mov		cl, rotatebase17[ecx]
				CFLAG_LOAD
				rcl		dx, cl
				FLAG_STORE_OC
				mov		ecx, ebp
				jmp		i286_memorywrite_w
		}
}

I286 rcr_ext16_v30(void) {

		__asm {
				mov		cl, rotatebase17[ecx]
				CFLAG_LOAD
				rcr		dx, cl
				FLAG_STORE_OC
				mov		ecx, ebp
				jmp		i286_memorywrite_w
		}
}

I286 shl_ext16_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				shl		dx, cl
				FLAG_STORE_OF
				mov		ecx, ebp
				jmp		i286_memorywrite_w
		}
}

I286 shr_ext16_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				shr		dx, cl
				FLAG_STORE_OF
				mov		ecx, ebp
				jmp		i286_memorywrite_w
		}
}

I286 sar_ext16_v30(void) {

		__asm {
				mov		cl, shiftbase[ecx]
				sar		dx, cl
				FLAG_STORE0
				mov		ecx, ebp
				jmp		i286_memorywrite_w
		}
}

static void (*sftext16v30_table[])(void) = {
		rol_ext16_v30,	ror_ext16_v30,	rcl_ext16_v30,	rcr_ext16_v30,
		shl_ext16_v30,	shr_ext16_v30,	shl_ext16_v30,	sar_ext16_v30};

I286 v30shift_ea16_data8(void) {				// C1: shift EA16, DATA8

		__asm {
				movzx	eax, bh
				mov		edi, eax
				shr		edi, 3-2
				and		edi, 7*4						// opcode
				cmp		al, 0c0h
				jc		memory_eareg16
				and		eax, 7
				I286CLOCK(5)
				lea		edx, I286_REG[eax*2]
				mov		ecx, ebx
				shr		ecx, 16
				and		ecx, 255
				I286CLOCK(ecx)
				push	ecx
				GET_NEXTPRE3
				pop		ecx
				jmp		sftreg16v30_table[edi]
				align	4
		memory_eareg16:
				I286CLOCK(8)
				call	p_ea_dst[eax*4]
				cmp		ecx, (I286_MEMWRITEMAX-1)
				jnc		extmem_eareg16
				lea		edx, I286_MEM[ecx]
				movzx	ecx, bl
				I286CLOCK(ecx)
				push	ecx
				GET_NEXTPRE1
				pop		ecx
				jmp		sftreg16v30_table[edi]
				align	4
		extmem_eareg16:
				call	i286_memoryread_w
				mov		ebp, ecx
				mov		edx, eax
				movzx	ecx, bl
				I286CLOCK(ecx)
				push	ecx
				GET_NEXTPRE1
				pop		ecx
				jmp		sftext16v30_table[edi]
	}
}

#if defined(VAEG_FIX)
I286 v30_iret(void) {								// CF: iret

		__asm {
				I286CLOCK(31)
				mov		edi, SS_BASE
				movzx	ebx, I286_SP
				lea		ecx, [edi + ebx]
				add		bx, 2
				call	i286_memoryread_w
				mov		si, ax
				lea		ecx, [edi + ebx]
				add		bx, 2
				call	i286_memoryread_w
				mov		I286_CS, ax
				and		eax, 0000ffffh
				shl		eax, 4					// make segreg
				mov		CS_BASE, eax
				lea		ecx, [edi + ebx]
				add		bx, 2
				call	i286_memoryread_w
				mov		I286_SP, bx
				and		ah, 0fh
				or		ax, 0f002h				// V30\82\C6286\82̗B\88\EA\82̍\B7\95\AA
				mov		I286_FLAG, ax

				and		ah, 1
				cmp		ah, 1
				//and		ah, 3
				//cmp		ah, 3

				sete	I286_TRAP
				RESET_XPREFETCH

				cmp		I286_TRAP, 0			// fast_intr
				jne		irqcheck
				test	I286_FLAG, I_FLAG
				je		nextop
				mov		al, pic.pi[0 * (type _PICITEM)].imr
				mov		ah, pic.pi[1 * (type _PICITEM)].imr
				not		ax
				test	al, pic.pi[0 * (type _PICITEM)].irr
				jne		irqcheck
				test	ah, pic.pi[1 * (type _PICITEM)].irr
				jne		irqcheck
nextop:			ret

irqcheck:		I286IRQCHECKTERM
		}
}
#endif

I286 v30shift_ea8_cl(void) {					// D2: shift EA8, cl

		__asm {
				movzx	eax, bh
				mov		edi, eax
				shr		edi, 3-2
				and		edi, 7*4						// opcode
				cmp		al, 0c0h
				jc		memory_eareg8
				I286CLOCK(5)
				bt		ax, 2
				rcl		eax, 1
				and		eax, 7
				lea		edx, I286_REG[eax]
				GET_NEXTPRE2
				movzx	ecx, I286_CL
				I286CLOCK(ecx)
				jmp		sftreg8v30_table[edi]
				align	4
		memory_eareg8:
				I286CLOCK(8)
				call	p_ea_dst[eax*4]
				cmp		ecx, I286_MEMWRITEMAX
				jnc		extmem_eareg8
				lea		edx, I286_MEM[ecx]
				movzx	ecx, I286_CL
				I286CLOCK(ecx)
				jmp		sftreg8v30_table[edi]
				align	4
		extmem_eareg8:
				call	i286_memoryread
				mov		edx, eax
				mov		ebp, ecx
				movzx	ecx, I286_CL
				I286CLOCK(ecx)
				jmp		sftext8v30_table[edi]
	}
}

I286 v30shift_ea16_cl(void) {					// D3: shift EA16, cl

		__asm {
				movzx	eax, bh
				mov		edi, eax
				shr		edi, 3-2
				and		edi, 7*4						// opcode
				cmp		al, 0c0h
				jc		memory_eareg16
				I286CLOCK(5)
				and		eax, 7
				lea		edx, I286_REG[eax*2]
				GET_NEXTPRE2
				movzx	ecx, I286_CL
				I286CLOCK(ecx)
				jmp		sftreg16v30_table[edi]
				align	4
		memory_eareg16:
				I286CLOCK(8)
				call	p_ea_dst[eax*4]
				cmp		ecx, (I286_MEMWRITEMAX-1)
				jnc		extmem_eareg16
				lea		edx, I286_MEM[ecx]
				movzx	ecx, I286_CL
				I286CLOCK(ecx)
				jmp		sftreg16v30_table[edi]
				align	4
		extmem_eareg16:
				call	i286_memoryread_w
				mov		edx, eax
				mov		ebp, ecx
				movzx	ecx, I286_CL
				I286CLOCK(ecx)
				jmp		sftext16v30_table[edi]
	}
}

I286 v30_aam(void) {							// D4: AAM

		__asm {
				I286CLOCK(16)
				mov		ax, I286_AX
				aam
				mov		I286_AX, ax
				FLAG_STORE
				GET_NEXTPRE2
				ret
		}
}

I286 v30_aad(void) {							// D5: AAD

		__asm {
				I286CLOCK(14)
				mov		ax, I286_AX
				aad
				mov		I286_AX, ax
				FLAG_STORE
				GET_NEXTPRE2
				ret
		}
}

I286 v30_xlat(void) {							// D6: XLAT

		__asm {
				I286CLOCK(5)
				movzx	ecx, I286_AL
				add		cx, I286_BX
				add		ecx, DS_FIX
				call	i286_memoryread
				mov		I286_AL, al
				GET_NEXTPRE1
				ret
		}
}

I286 v30_repne(void) {							// F2: repne

		__asm {
				I286PREFIX(v30op_repne)
		}
}

I286 v30_repe(void) {							// F3: repe

		__asm {
				I286PREFIX(v30op_repe)
		}
}

I286 v30div_ea8(void) {							// F6-6: div ea8

		__asm {
				PREPART_EA8(14)
					movzx	ebp, byte ptr I286_REG[eax]
					GET_NEXTPRE2
					jmp		divcheck
				MEMORY_EA8(17)
					movzx	ebp, byte ptr I286_MEM[ecx]
					jmp		divcheck
				EXTMEM_EA8
					movzx	ebp, al

				align	4
	divcheck:	test	ebp, ebp
				je		divovf
				mov		ax, I286_AX
				xor		dx, dx
				div		bp
				mov		I286_AL, al
				mov		I286_AH, dl
				mov		dx, ax
				FLAG_STORE_OF
				test	dh, dh
				jne		divovf
				ret

				align	4
	divovf:		INT_NUM(0)
		}
}

I286 v30idiv_ea8(void) {						// F6-7 idiv ea8

		__asm {
				PREPART_EA8(17)
					movsx	ebp, byte ptr I286_REG[eax]
					GET_NEXTPRE2
					jmp		idivcheck
				MEMORY_EA8(20)
					movsx	ebp, byte ptr I286_MEM[ecx]
					jmp		idivcheck
				EXTMEM_EA8
					movsx	ebp, al

				align	4
	idivcheck:	test	ebp, ebp
				je		idivovf
				mov		ax, I286_AX
				cwd
				idiv	bp
				mov		I286_AL, al
				mov		I286_AH, dl
				mov		dx, ax
				FLAG_STORE_OF
				bt		dx, 7
				adc		dh, 0
				jne		idivovf
				ret

				align	4
	idivovf:	INT_NUM(0)
		}
}

I286 v30_ope0xf6(void) {						// F6: 

		__asm {
				movzx	eax, bh
				mov		edi, eax
				shr		edi, 3-2
				and		edi, 7*4
				jmp		v30ope0xf6_xtable[edi]
		}
}

I286 v30div_ea16(void) {						// F7-6: div ea16

		__asm {
				PREPART_EA16(22)
					movzx	ebp, word ptr I286_REG[eax*2]
					GET_NEXTPRE2
					jmp		divcheck
				MEMORY_EA16(25)
					movzx	ebp, word ptr I286_MEM[ecx]
					jmp		divcheck
				EXTMEM_EA16
					movzx	ebp, ax

				align	4
	divcheck:	test	ebp, ebp
				je		divovf
				movzx	eax, I286_DX
				shl		eax, 16
				mov		ax, I286_AX
				xor		edx, edx
				div		ebp
				mov		I286_AX, ax
				mov		I286_DX, dx
				FLAG_STORE_OF
				cmp		eax, 10000h
				jae		divovf
				ret

				align	4
	divovf:		INT_NUM(0)
		}
}

I286 v30idiv_ea16(void) {						// F7-7: idiv ea16

		__asm {
				PREPART_EA16(25)
					movsx	ebp, word ptr I286_REG[eax*2]
					GET_NEXTPRE2
					jmp		idivcheck
				MEMORY_EA16(28)
					movsx	ebp, word ptr I286_MEM[ecx]
					jmp		idivcheck
				EXTMEM_EA16
					cwde
					mov		ebp, eax

				align	4
	idivcheck:	test	ebp, ebp
				je		idivovf
				movzx	eax, I286_DX
				shl		eax, 16
				mov		ax, I286_AX
				cdq
				idiv	ebp
				mov		I286_AX, ax
				mov		I286_DX, dx
				mov		edx, eax
				FLAG_STORE_OF
				shr		edx, 16
				adc		dx, 0
				jne		idivovf
				ret

				align	4
	idivovf:	INT_NUM(0)
		}
}

I286 v30_ope0xf7(void) {						// F7: 

		__asm {
				movzx	eax, bh
				mov		edi, eax
				shr		edi, 3-2
				and		edi, 7*4
				jmp		v30ope0xf7_xtable[edi]
		}
}

#if defined(SUPPORT_V30ORIGINAL)



// ----------------------------------------------------------------- 0F

/*
	\88ȉ\BA\82͔j\89󂵂Ă͂\A2\82\AF\82Ȃ\A2\81B
	esi		IP
	ebx		\83v\83\8A\83t\83F\83b\83`\82\B3\82ꂽ\83f\81[\83^
*/

I286 v30_reserved_0x0f(void) {

		__asm {
				I286CLOCK(2)
				//GET_NEXTPRE1		// \83f\83o\83b\83O\82̖ړI\82̂\BD\82߁AIP\82\F0\90i\82߂\B8\82ɒ\E2\8E~\82\B3\82\B9\82\E9
				ret
		}
}

I286 v30_add4s(void) {							// 0F 20
		__asm {
				push	ebx
				push	esi
				push	edi

				I286CLOCK((7+19))

				xor		esi, esi
				mov		si, I286_SI
				add		esi, DS_FIX
				xor		edi, edi
				mov		di, I286_DI
				add		edi, ES_BASE


				//----\82\B1\82\B1\82\A9\82\E71\89\F1\96\DA
				push	esi
				push	edi

				mov		ecx, esi

				push	edi
				call	i286_memoryread
				pop		edi

				push	ax

				mov		ecx, edi
				call	i286_memoryread

				pop		dx

				add		al, dl
				daa
				lahf
				mov		bh, ah				//bh=\83t\83\89\83O
				mov		bl, bh
				or		bl, ~40h			//\88ȍ~\81Adaa\82ň\EA\93x\82ł\E0ZF=0\82ƂȂ\E9\82ƁAbl=0bfh\82ƂȂ\E9

				mov		dl, al
				call	i286_memorywrite

				pop		edi
				pop		esi
				//---- \82\B1\82\B1\82܂\C51\89\F1\96\DA

				mov		cl, I286_CL
				inc		cl
				shr		cl,1
				dec		cl
				and		ecx,007fh
				jcxz	add4s_endloop

			add4s_loop:
				I286CLOCK(19)
				push	ecx
				inc		esi
				inc		edi
				push	esi
				push	edi

				mov		ecx, esi

				push	edi
				call	i286_memoryread
				pop		edi

				push	ax

				mov		ecx, edi
				call	i286_memoryread

				pop		dx

				mov		ah, bh
				sahf	
				adc		al, dl
				daa
				lahf
				and		ah, bl
				mov		bh, ah
				mov		bl, bh
				or		bl, ~40h

				mov		dl, al
				call	i286_memorywrite

				pop		edi
				pop		esi
				pop		ecx
				loop	add4s_loop

			add4s_endloop:

				test	bh, 1		//CF
				jz		cf_zero
	
				//CF=1
				mov		bh, 93h		//SF,AF,CF=1 \82ق\A9\82\CD0
				jmp		add4s_end

			cf_zero:
				//CF=0
				test	bh, 40h		//ZF
				jz		zf_zero
	
				//ZF=1
				mov		bh, 46h		//ZF,PF=1 \82ق\A9\82\CD0
				jmp		add4s_end

			zf_zero:
				//CF=0, ZF=0
				mov		bh, 02h		//\82\B7\82ׂ\C40

			add4s_end:

				mov		I286_FLAGL, bh
				and		I286_FLAGH, 0f7h		;OF=0

				pop		edi
				pop		esi
				pop		ebx

				GET_NEXTPRE2

				ret
		}
}


I286 v30_sub4s(void) {							// 0F 22
		__asm {
				push	ebx
				push	esi
				push	edi

				I286CLOCK((7+19))

				xor		esi, esi
				mov		si, I286_SI
				add		esi, DS_FIX
				xor		edi, edi
				mov		di, I286_DI
				add		edi, ES_BASE


				//----\82\B1\82\B1\82\A9\82\E71\89\F1\96\DA
				push	esi
				push	edi

				mov		ecx, esi

				push	edi
				call	i286_memoryread
				pop		edi

				push	ax

				mov		ecx, edi
				call	i286_memoryread

				pop		dx

				sub		al, dl
				das
				lahf
				mov		bh, ah				//bh=\83t\83\89\83O
				mov		bl, bh
				or		bl, ~40h			//\88ȍ~\81Adaa\82ň\EA\93x\82ł\E0ZF=0\82ƂȂ\E9\82ƁAbl=0bfh\82ƂȂ\E9

				mov		dl, al
				call	i286_memorywrite

				pop		edi
				pop		esi
				//---- \82\B1\82\B1\82܂\C51\89\F1\96\DA

				mov		cl, I286_CL
				inc		cl
				shr		cl,1
				dec		cl
				and		ecx,007fh
				jcxz	add4s_endloop

			add4s_loop:
				I286CLOCK(19)
				push	ecx
				inc		esi
				inc		edi
				push	esi
				push	edi

				mov		ecx, esi

				push	edi
				call	i286_memoryread
				pop		edi

				push	ax

				mov		ecx, edi
				call	i286_memoryread

				pop		dx

				mov		ah, bh
				sahf	
				sbb		al, dl
				das
				lahf
				and		ah, bl
				mov		bh, ah
				mov		bl, bh
				or		bl, ~40h

				mov		dl, al
				call	i286_memorywrite

				pop		edi
				pop		esi
				pop		ecx
				loop	add4s_loop

			add4s_endloop:

				test	bh, 1		//CF
				jz		cf_zero
	
				//CF=1
				mov		bh, 93h		//SF,AF,CF=1 \82ق\A9\82\CD0
				jmp		add4s_end

			cf_zero:
				//CF=0
				test	bh, 40h		//ZF
				jz		zf_zero
	
				//ZF=1
				mov		bh, 46h		//ZF,PF=1 \82ق\A9\82\CD0
				jmp		add4s_end

			zf_zero:
				//CF=0, ZF=0
				mov		bh, 02h		//\82\B7\82ׂ\C40

			add4s_end:

				mov		I286_FLAGL, bh
				and		I286_FLAGH, 0f7h		;OF=0

				pop		edi
				pop		esi
				pop		ebx

				GET_NEXTPRE2

				ret
		}
}

I286 v30_ror4_ea8(void) {						// 0F 2A 11/000/reg
												// 0F 2A mod/000/mem disp
		__asm {
									// EAX \82ɂ͖\BD\97߂\CC2\83o\83C\83g\96\DA(0F\82̎\9F)\82\AA\93\FC\82\C1\82Ă\A2\82\E9

				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// PREPART_EA8, MEMORY_EA8\82Ȃǂ̃}\83N\83\8D\82\AA\81A
									// 2\83o\83C\83g\82̖\BD\97ߗp\82ɍ쐬\82\B3\82\EA\82Ă\A2\82邽\82߁B
									// (ROR4\82\CD3\83o\83C\83g)
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA8(25)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// byte ptr I286_REG[eax]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		dh, I286_AL
					mov		ch, dh
					and		ch, 0f0h
					mov		dl, byte ptr I286_REG[eax]
					mov		cl, dl
					and		cl, 0fh
					or		cl, ch
					mov		I286_AL, cl
					shr		dx, 4
					mov		byte ptr I286_REG[eax], dl

					GET_NEXTPRE2	// \96\BD\97\DF2\83o\83C\83g\96ځA3\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA8(25)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		ah, I286_AL
					mov		dh, ah
					and		dh, 0f0h
					mov		al, byte ptr I286_MEM[ecx]
					mov		dl, al
					and		dl, 0fh
					or		dl, dh
					mov		I286_AL, dl

					shr		ax, 4
					mov		byte ptr I286_MEM[ecx], al
					ret

				EXTMEM_EA8
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83o\83C\83g)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		ah, I286_AL
					mov		dh, ah
					and		dh, 0f0h
					mov		dl, al
					and		dl, 0fh
					or		dl, dh
					mov		I286_AL, dl

					shr		ax, 4
					mov		dl, al

					jmp		i286_memorywrite
		}

}


I286 v30_test1_ea8_cl(void) {					// F0 10 11/000/reg
												// F0 10 mod/000/mem disp
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA8(3)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// byte ptr I286_REG[eax]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		cl, CPU_CL
					and		cl, 0007h	// \89\BA\88\CA3bit\82\AA\97L\8C\F8
					mov		dx, 1
					shl		dx, cl
					test	byte ptr I286_REG[eax], dl		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					GET_NEXTPRE2		// \96\BD\97\DF2\81`3\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA8(12)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		al, byte ptr I286_MEM[ecx]
					mov		cl, CPU_CL
					and		cl, 0007h	// \89\BA\88\CA3bit\82\AA\97L\8C\F8
					mov		dx, 1
					shl		dx, cl
					test	al, dl		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					ret

				EXTMEM_EA8
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83o\83C\83g)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		cl, CPU_CL
					and		cl, 0007h	// \89\BA\88\CA3bit\82\AA\97L\8C\F8
					mov		dx, 1
					shl		dx, cl
					test	al, dl		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					ret
		}
}


I286 v30_test1_ea16_cl(void) {					// F0 11 11/000/reg
												// F0 11 mod/000/mem disp
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA16(3)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// word ptr I286_REG[eax*2]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		cl, CPU_CL
					and		cl, 000fh	// \89\BA\88\CA4bit\82\AA\97L\8C\F8
					mov		dx, 1
					shl		dx, cl
					test	word ptr I286_REG[eax*2], dx		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					GET_NEXTPRE2		// \96\BD\97\DF2\81`3\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA16(12)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		ax, word ptr I286_MEM[ecx]
					mov		cl, CPU_CL
					and		cl, 000fh	// \89\BA\88\CA4bit\82\AA\97L\8C\F8
					mov		dx, 1
					shl		dx, cl
					test	ax, dx		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					ret

				EXTMEM_EA16
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83\8F\81[\83h)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		cl, CPU_CL
					and		cl, 000fh	// \89\BA\88\CA4bit\82\AA\97L\8C\F8
					mov		dx, 1
					shl		dx, cl
					test	ax, dx		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					ret
		}
}


I286 v30_clr1_ea8_cl(void) {					// 0F 12 11/000/reg
												// 0F 12 mod/000/mem disp
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA8(5)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// byte ptr I286_REG[eax]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		dl, CPU_CL
					and		dx, 0007h
					mov		cl, byte ptr I286_REG[eax]
					btr		cx, dx		// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g
					mov		byte ptr I286_REG[eax], cl

					GET_NEXTPRE2		// \96\BD\97\DF2\81`3\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA8(14)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, CPU_CL
					and		dx, 0007h
					mov		al, byte ptr I286_MEM[ecx]
					btr		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g
					mov		byte ptr I286_MEM[ecx], al

					ret

				EXTMEM_EA8
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83o\83C\83g)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, CPU_CL
					and		dx, 0007h
					btr		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g
					mov		dl, al
					call	i286_memorywrite

					ret
		}
}


I286 v30_set1_ea8_cl(void) {					// 0F 14 11/000/reg
												// 0F 14 mod/000/mem disp
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA8(4)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// byte ptr I286_REG[eax]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		dl, CPU_CL
					and		dx, 0007h
					mov		cl, byte ptr I286_REG[eax]
					bts		cx, dx		// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g
					mov		byte ptr I286_REG[eax], cl

					GET_NEXTPRE2		// \96\BD\97\DF2\81`3\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA8(13)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, CPU_CL
					and		dx, 0007h
					mov		al, byte ptr I286_MEM[ecx]
					bts		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g
					mov		byte ptr I286_MEM[ecx], al

					ret

				EXTMEM_EA8
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83\8F\81[\83h)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, CPU_CL
					and		dx, 0007h
					bts		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g
					mov		dl, al
					call	i286_memorywrite

					ret
		}
}

I286 v30_test1_ea8_i3(void) {					// F0 18 11/000/reg  imm3
												// F0 18 mod/000/mem disp imm3
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA8(4)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// byte ptr I286_REG[eax]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		ecx, ebx
					shr		ecx, 16
					and		cx, 0007h	// imm3
					mov		al, byte ptr I286_REG[eax]
					mov		dx, 1
					shl		dx, cl
					test	al, dl		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					GET_NEXTPRE3		// \96\BD\97\DF2\81`4\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA8(13)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		al, byte ptr I286_MEM[ecx]
					mov		cl, bl
					and		cl, 0007h	// imm3
					mov		dx, 1
					shl		dx, cl
					test	al, dl		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					GET_NEXTPRE1		// imm3 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret

				EXTMEM_EA8
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83\8F\81[\83h)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		cl, bl
					and		cl, 0007h	// imm3
					mov		dx, 1
					shl		dx, cl
					test	al, dl		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					GET_NEXTPRE1		// imm3 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret
		}
}

I286 v30_test1_ea16_i4(void) {					// F0 19 11/000/reg  imm4
												// F0 19 mod/000/mem disp imm4
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA16(4)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// word ptr I286_REG[eax*2]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		ecx, ebx
					shr		ecx, 16
					and		cx, 000fh	// imm4
					mov		dx, 1
					shl		dx, cl
					test	word ptr I286_REG[eax*2], dx		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					GET_NEXTPRE3		// \96\BD\97\DF2\81`4\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA16(13)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		ax, word ptr I286_MEM[ecx]
					mov		cl, bl
					and		cl, 000fh	// imm4
					mov		dx, 1
					shl		dx, cl
					test	ax, dx		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					GET_NEXTPRE1		// imm4 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret

				EXTMEM_EA16
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83\8F\81[\83h)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		cl, bl
					and		cl, 000fh	// imm4
					mov		dx, 1
					shl		dx, cl
					test	ax, dx		// cl\83r\83b\83g\96ڂ\F0\83e\83X\83g
					FLAG_STORE00

					GET_NEXTPRE1		// imm4 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret
		}
}


I286 v30_clr1_ea8_i3(void) {					// 0F 1A 11/000/reg  imm3
												// 0F 1A mod/000/mem disp imm3
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA8(6)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// byte ptr I286_REG[eax]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		edx, ebx
					shr		edx, 16
					and		dx, 0007h	// imm3
					mov		cl, byte ptr I286_REG[eax]
					btr		cx, dx		// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g
					mov		byte ptr I286_REG[eax], cl

					GET_NEXTPRE3		// \96\BD\97\DF2\81`4\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA8(15)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, bl
					and		dx, 0007h	// imm3
					mov		al, byte ptr I286_MEM[ecx]
					btr		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g
					mov		byte ptr I286_MEM[ecx], al

					GET_NEXTPRE1		// imm3 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret

				EXTMEM_EA8
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83\8F\81[\83h)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, bl
					and		dx, 0007h	// imm3
					btr		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g
					mov		dl, al
					call	i286_memorywrite

					GET_NEXTPRE1		// imm3 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret
		}
}

I286 v30_clr1_ea16_i4(void) {					// 0F 1B 11/000/reg  imm4
												// 0F 1B mod/000/mem disp imm4
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA16(6)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// word ptr I286_REG[eax*2]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		edx, ebx
					shr		edx, 16
					and		dx, 000fh	// imm4
					btr		word ptr I286_REG[eax*2], dx	// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g

					GET_NEXTPRE3	// \96\BD\97\DF2\81`4\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA16(15)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, bl
					and		dx, 000fh	// imm4
					btr		word ptr I286_MEM[ecx], dx	// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g

					GET_NEXTPRE1		// imm4 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret

				EXTMEM_EA16
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83\8F\81[\83h)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, bl
					and		dx, 000fh	// imm4
					btr		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83\8A\83Z\83b\83g
					mov		dx, ax
					call	i286_memorywrite_w

					GET_NEXTPRE1		// imm4 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret
		}
}

I286 v30_set1_ea8_i3(void) {					// 0F 1C 11/000/reg  imm3
												// 0F 1C mod/000/mem disp imm3
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA8(5)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// byte ptr I286_REG[eax]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		edx, ebx
					shr		edx, 16
					and		dx, 0007h	// imm3
					mov		cl, byte ptr I286_REG[eax]
					bts		cx, dx		// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g
					mov		byte ptr I286_REG[eax], cl

					GET_NEXTPRE3		// \96\BD\97\DF2\81`4\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA8(14)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, bl
					and		dx, 0007h	// imm3
					mov		al, byte ptr I286_MEM[ecx]
					bts		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g
					mov		byte ptr I286_MEM[ecx], al

					GET_NEXTPRE1		// imm3 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret

				EXTMEM_EA8
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83\8F\81[\83h)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, bl
					and		dx, 0007h	// imm3
					bts		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g
					mov		dl, al
					call	i286_memorywrite

					GET_NEXTPRE1		// imm3 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret
		}
}

I286 v30_set1_ea16_i4(void) {					// 0F 1D 11/000/reg  imm4
												// 0F 1D mod/000/mem disp imm4
		__asm {
				GET_NEXTPRE1		// 0F \82\CC1\83o\83C\83g\95\AA\81AIP\82\F0\90i\82߁A\96\BD\97߂\F0\90\E6\93ǂ݂\B7\82\E9
									// BL=\96\BD\97\DF2\83o\83C\83g\96ځABH=\96\BD\97\DF3\83o\83C\83g\96\DA ... \82ƂȂ\E9
				movzx	eax, bh		// EAX \82ɖ\BD\97߂\CC3\83o\83C\83g\96ڂ\F0\91\E3\93\FC

				PREPART_EA16(5)
					// \83I\83y\83\89\83\93\83h\82\AA\83\8C\83W\83X\83^\82̏ꍇ
					// word ptr I286_REG[eax*2]\82Ń\8C\83W\83X\83^\82ɃA\83N\83Z\83X\82ł\AB\82\E9

					mov		edx, ebx
					shr		edx, 16
					and		dx, 000fh	// imm4
					bts		word ptr I286_REG[eax*2], dx	// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g

					GET_NEXTPRE3	// \96\BD\97\DF2\81`4\83o\83C\83g\96ڂ̕\AA IP\82\F0\90i\82߂\E9
					ret

				MEMORY_EA16(14)
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A(\92\BC\90ڃA\83N\83Z\83X)\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, bl
					and		dx, 000fh	// imm4
					bts		word ptr I286_MEM[ecx], dx	// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g

					GET_NEXTPRE1		// imm4 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret

				EXTMEM_EA16
					// \83I\83y\83\89\83\93\83h\82\AA\83\81\83\82\83\8A\82̏ꍇ
					// ecx\82ɃA\83h\83\8C\83X\82\AA\81Aeax\82Ƀ\81\83\82\83\8A\82\A9\82\E7\93ǂ񂾒l(\83\8F\81[\83h)\82\AA\93\FC\82\C1\82Ă\A2\82\E9
					// GET_NEXTPRE?\82͎\C0\8Ds\8Dς\DD

					mov		dl, bl
					and		dx, 000fh	// imm4
					bts		ax, dx		// dx\83r\83b\83g\96ڂ\F0\83Z\83b\83g
					mov		dx, ax
					call	i286_memorywrite_w

					GET_NEXTPRE1		// imm4 \82̕\AA\81AIP\82\F0\90i\82߂\E9
					ret
		}
}

static const	I286TBL	v30ope0x0f_xtable[64] = {
			v30_reserved_0x0f,				// 00:
			v30_reserved_0x0f,				// 01:
			v30_reserved_0x0f,				// 02:
			v30_reserved_0x0f,				// 03:
			v30_reserved_0x0f,				// 04:
			v30_reserved_0x0f,				// 05:
			v30_reserved_0x0f,				// 06:
			v30_reserved_0x0f,				// 07:
			v30_reserved_0x0f,				// 08:
			v30_reserved_0x0f,				// 09:
			v30_reserved_0x0f,				// 0A:
			v30_reserved_0x0f,				// 0B:
			v30_reserved_0x0f,				// 0C:
			v30_reserved_0x0f,				// 0D:
			v30_reserved_0x0f,				// 0E:
			v30_reserved_0x0f,				// 0F:

			v30_test1_ea8_cl,				// 10:
			v30_test1_ea16_cl,				// 11:
			v30_clr1_ea8_cl,				// 12:
			v30_reserved_0x0f,				// 13:
			v30_set1_ea8_cl,				// 14:
			v30_reserved_0x0f,				// 15:
			v30_reserved_0x0f,				// 16:
			v30_reserved_0x0f,				// 17:
			v30_test1_ea8_i3,				// 18:
			v30_test1_ea16_i4,				// 19:
			v30_clr1_ea8_i3,				// 1A:
			v30_clr1_ea16_i4,				// 1B:
			v30_set1_ea8_i3,				// 1C:
			v30_set1_ea16_i4,				// 1D:
			v30_reserved_0x0f,				// 1E:
			v30_reserved_0x0f,				// 1F:

			v30_add4s,						// 20:
			v30_reserved_0x0f,				// 21:
			v30_sub4s,						// 22:
			v30_reserved_0x0f,				// 23:
			v30_reserved_0x0f,				// 24:
			v30_reserved_0x0f,				// 25:
			v30_reserved_0x0f,				// 26:
			v30_reserved_0x0f,				// 27:
			v30_reserved_0x0f,				// 28:
			v30_reserved_0x0f,				// 29:
			v30_ror4_ea8,					// 2A:
			v30_reserved_0x0f,				// 2B:
			v30_reserved_0x0f,				// 2C:
			v30_reserved_0x0f,				// 2D:
			v30_reserved_0x0f,				// 2E:
			v30_reserved_0x0f,				// 2F:

			v30_reserved_0x0f,				// 30:
			v30_reserved_0x0f,				// 31:
			v30_reserved_0x0f,				// 32:
			v30_reserved_0x0f,				// 33:
			v30_reserved_0x0f,				// 34:
			v30_reserved_0x0f,				// 35:
			v30_reserved_0x0f,				// 36:
			v30_reserved_0x0f,				// 37:
			v30_reserved_0x0f,				// 38:
			v30_reserved_0x0f,				// 39:
			v30_reserved_0x0f,				// 3A:
			v30_reserved_0x0f,				// 3B:
			v30_reserved_0x0f,				// 3C:
			v30_reserved_0x0f,				// 3D:
			v30_reserved_0x0f,				// 3E:
			v30_reserved_0x0f,				// 3F:
};


I286 v30_ope0x0f(void) {						// 0F: 

		__asm {
				test	bh, 0c0h
				jz		jmpbytbl
												// 40 \88ȏ\E3\82̖\BD\97߂Ƃ\B5\82ẮA
												// FF(BREAKM)\82\AA\82\A0\82\E9\82̂݁B
				jmp		v30_reserved_0x0f

	jmpbytbl:	movzx	eax, bh
				mov		edi, eax
				shl		edi, 2
				jmp		v30ope0x0f_xtable[edi]
		}
}

#endif


// ----------------------------------------------------------------- patch table


static const V30PATCH v30patch_op[] = {
#if defined(SUPPORT_V30ORIGINAL)
			{0x0f, v30_ope0x0f},			// 0F:
#endif
			{0x17, v30pop_ss},				// 17:	pop		ss
			{0x26, v30segprefix_es},		// 26:	es:
			{0x2e, v30segprefix_cs},		// 2E:	cs:
			{0x36, v30segprefix_ss},		// 36:	ss:
			{0x3e, v30segprefix_ds},		// 3E:	ds:
			{0x54, v30push_sp},				// 54:	push	sp
#if !defined(VAEG_FIX)
			/* V30/\83\CAPD9002\82ł\CDPOP SP\82̓X\83^\83b\83N\82̒l\82\F0SP\82ɑ\E3\93\FC\82\B5\82ďI\82\ED\82\E8\81B(2\82\F0\89\C1\8EZ\82\B5\82Ȃ\A2)
			   \88ȉ\BA\82̎\C0\91\95\82͌\EB\82\E8 */
			{0x5c, v30pop_sp},				// 5C:	pop		sp
#endif
			{0x63, v30_reserved},			// 63:	reserved
#if defined(SUPPORT_V30ORIGINAL)
			{0x64, v30_reserved_0x0f},		// 64:	repnc
			{0x65, v30_repc},				// 65:	repc
#else
			{0x64, v30_reserved},			// 64:	reserved
			{0x65, v30_reserved},			// 65:	reserved
#endif
			{0x66, v30_reserved},			// 66:	reserved
			{0x67, v30_reserved},			// 67:	reserved
			{0x8e, v30mov_seg_ea},			// 8E:	mov		segrem, EA
			{0x9c, v30_pushf},				// 9C:	pushf
			{0x9d, v30_popf},				// 9D:	popf
			{0xc0, v30shift_ea8_data8},		// C0:	shift	EA8, DATA8
			{0xc1, v30shift_ea16_data8},	// C1:	shift	EA16, DATA8
#if defined(VAEG_FIX)
			{0xcf, v30_iret},				// CF:	iret
#endif
			{0xd2, v30shift_ea8_cl},		// D2:	shift EA8, cl
			{0xd3, v30shift_ea16_cl},		// D3:	shift EA16, cl
			{0xd4, v30_aam},				// D4:	AAM
			{0xd5, v30_aad},				// D5:	AAD
			{0xd6, v30_xlat},				// D6:	xlat (8086/V30)
#if defined(EXT)
			{0xe2, v30_loop},				// E2:	loop
#endif
			{0xf2, v30_repne},				// F2:	repne
			{0xf3, v30_repe},				// F3:	repe
			{0xf6, v30_ope0xf6},			// F6: 
			{0xf7, v30_ope0xf7}};			// F7: 


// ----------------------------------------------------------------- repe

I286 v30repe_segprefix_es(void) {

		__asm {
				mov		eax, ES_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repe[eax*4]
		}
}

I286 v30repe_segprefix_cs(void) {

		__asm {
				mov		eax, CS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repe[eax*4]
		}
}

I286 v30repe_segprefix_ss(void) {

		__asm {
				mov		eax, SS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repe[eax*4]
		}
}

I286 v30repe_segprefix_ds(void) {

		__asm {
				mov		eax, DS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repe[eax*4]
		}
}


static const V30PATCH v30patch_repe[] = {
			{0x17, v30pop_ss},				// 17:	pop		ss
			{0x26, v30repe_segprefix_es},	// 26:	repe es:
			{0x2e, v30repe_segprefix_cs},	// 2E:	repe cs:
			{0x36, v30repe_segprefix_ss},	// 36:	repe ss:
			{0x3e, v30repe_segprefix_ds},	// 3E:	repe ds:
			{0x54, v30push_sp},				// 54:	push	sp
#if !defined(VAEG_FIX)
			/* V30/\83\CAPD9002\82ł\CDPOP SP\82̓X\83^\83b\83N\82̒l\82\F0SP\82ɑ\E3\93\FC\82\B5\82ďI\82\ED\82\E8\81B(2\82\F0\89\C1\8EZ\82\B5\82Ȃ\A2)
			   \88ȉ\BA\82̎\C0\91\95\82͌\EB\82\E8 */
			{0x5c, v30pop_sp},				// 5C:	pop		sp
#endif
			{0x63, v30_reserved},			// 63:	reserved
			{0x64, v30_reserved},			// 64:	reserved
			{0x65, v30_reserved},			// 65:	reserved
			{0x66, v30_reserved},			// 66:	reserved
			{0x67, v30_reserved},			// 67:	reserved
			{0x8e, v30mov_seg_ea},			// 8E:	mov		segrem, EA
			{0x9c, v30_pushf},				// 9C:	pushf
			{0x9d, v30_popf},				// 9D:	popf
			{0xc0, v30shift_ea8_data8},		// C0:	shift	EA8, DATA8
			{0xc1, v30shift_ea16_data8},	// C1:	shift	EA16, DATA8
#if defined(VAEG_FIX)
			{0xcf, v30_iret},				// CF:	iret
#endif
			{0xd2, v30shift_ea8_cl},		// D2:	shift EA8, cl
			{0xd3, v30shift_ea16_cl},		// D3:	shift EA16, cl
			{0xd4, v30_aam},				// D4:	AAM
			{0xd5, v30_aad},				// D5:	AAD
			{0xd6, v30_xlat},				// D6:	xlat (8086/V30)
			{0xf2, v30_repne},				// F2:	repne
			{0xf3, v30_repe},				// F3:	repe
			{0xf6, v30_ope0xf6},			// F6: 
			{0xf7, v30_ope0xf7}};			// F7: 


// ----------------------------------------------------------------- repne

I286 v30repne_segprefix_es(void) {

		__asm {
				mov		eax, ES_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repne[eax*4]
		}
}

I286 v30repne_segprefix_cs(void) {

		__asm {
				mov		eax, CS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repne[eax*4]
		}
}

I286 v30repne_segprefix_ss(void) {

		__asm {
				mov		eax, SS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repne[eax*4]
		}
}

I286 v30repne_segprefix_ds(void) {

		__asm {
				mov		eax, DS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repne[eax*4]
		}
}

static const V30PATCH v30patch_repne[] = {
			{0x17, v30pop_ss},				// 17:	pop		ss
			{0x26, v30repne_segprefix_es},	// 26:	repne es:
			{0x2e, v30repne_segprefix_cs},	// 2E:	repne cs:
			{0x36, v30repne_segprefix_ss},	// 36:	repne ss:
			{0x3e, v30repne_segprefix_ds},	// 3E:	repne ds:
			{0x54, v30push_sp},				// 54:	push	sp
#if !defined(VAEG_FIX)
			/* V30/\83\CAPD9002\82ł\CDPOP SP\82̓X\83^\83b\83N\82̒l\82\F0SP\82ɑ\E3\93\FC\82\B5\82ďI\82\ED\82\E8\81B(2\82\F0\89\C1\8EZ\82\B5\82Ȃ\A2)
			   \88ȉ\BA\82̎\C0\91\95\82͌\EB\82\E8 */
			{0x5c, v30pop_sp},				// 5C:	pop		sp
#endif
			{0x63, v30_reserved},			// 63:	reserved
			{0x64, v30_reserved},			// 64:	reserved
			{0x65, v30_reserved},			// 65:	reserved
			{0x66, v30_reserved},			// 66:	reserved
			{0x67, v30_reserved},			// 67:	reserved
			{0x8e, v30mov_seg_ea},			// 8E:	mov		segrem, EA
			{0x9c, v30_pushf},				// 9C:	pushf
			{0x9d, v30_popf},				// 9D:	popf
			{0xc0, v30shift_ea8_data8},		// C0:	shift	EA8, DATA8
			{0xc1, v30shift_ea16_data8},	// C1:	shift	EA16, DATA8
#if defined(VAEG_FIX)
			{0xcf, v30_iret},				// CF:	iret
#endif
			{0xd2, v30shift_ea8_cl},		// D2:	shift EA8, cl
			{0xd3, v30shift_ea16_cl},		// D3:	shift EA16, cl
			{0xd4, v30_aam},				// D4:	AAM
			{0xd5, v30_aad},				// D5:	AAD
			{0xd6, v30_xlat},				// D6:	xlat (8086/V30)
			{0xf2, v30_repne},				// F2:	repne
			{0xf3, v30_repe},				// F3:	repe
			{0xf6, v30_ope0xf6},			// F6: 
			{0xf7, v30_ope0xf7}};			// F7: 

#if defined(SUPPORT_V30ORIGINAL)
// ----------------------------------------------------------------- repc
I286 v30_reserved_repc(void) {

		__asm {
				I286CLOCK(2)
				//GET_NEXTPRE1		// \83f\83o\83b\83O\82̖ړI\82̂\BD\82߁AIP\82\F0\90i\82߂\B8\82ɒ\E2\8E~\82\B3\82\B9\82\E9
				mov		esi, I286_REPPOSBAK
				RESET_XPREFETCH
				ret
		}
}

I286 v30repc_segprefix_es(void) {

		__asm {
				mov		eax, ES_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repc[eax*4]
		}
}

I286 v30repc_segprefix_cs(void) {

		__asm {
				mov		eax, CS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repc[eax*4]
		}
}

I286 v30repc_segprefix_ss(void) {

		__asm {
				mov		eax, SS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repc[eax*4]
		}
}

I286 v30repc_segprefix_ds(void) {

		__asm {
				mov		eax, DS_BASE
				mov		DS_FIX, eax
				mov		SS_FIX, eax
				GET_NEXTPRE1
				movzx	eax, bl
				jmp		v30op_repc[eax*4]
		}
}


I286EXT v30repc_xscasb(void) {							// AE 

		__asm {
				cmp		I286_CX, 0
				je		scasb_ed
				mov		edi, ES_BASE
				movzx	ebp, I286_DI
zscasb_lp:		I286CLOCK(8)
				lea		ecx, [edi + ebp]
				call	i286_memoryread
				cmp		I286_AL, al
				FLAG_STORE_OF
				STRING_DIR
				add		bp, ax
				dec		I286_CX
				je		zscasb_ed
				test	I286_FLAG, C_FLAG
				jne		zscasb_lp
zscasb_ed:		mov		I286_DI, bp
scasb_ed:		I286CLOCK(5)
				GET_NEXTPRE1
				ret
		}
}

static const V30PATCH v30patch_repc[] = {
			{0x26, v30repc_segprefix_es},	// 26:	repc es:
			{0x2e, v30repc_segprefix_cs},	// 2E:	repc cs:
			{0x36, v30repc_segprefix_ss},	// 36:	repc ss:
			{0x3e, v30repc_segprefix_ds},	// 3E:	repc ds:
			{0xae, v30repc_xscasb},			// AE:	repc scasb
};
#endif

// ---------------------------------------------------------------------------

static void v30patching(I286TBL *dst, const V30PATCH *patch, int length) {

	while(length--) {
		dst[patch->opnum] = patch->v30opcode;
		patch++;
	}
}

#define	V30PATCHING(a, b)	v30patching(a, b, sizeof(b)/sizeof(V30PATCH))

void v30xinit(void) {

	CopyMemory(v30op, i286op, sizeof(v30op));
	V30PATCHING(v30op, v30patch_op);
	CopyMemory(v30op_repne, i286op_repne, sizeof(v30op_repne));
	V30PATCHING(v30op_repne, v30patch_repne);
	CopyMemory(v30op_repe, i286op_repe, sizeof(v30op_repe));
	V30PATCHING(v30op_repe, v30patch_repe);
	CopyMemory(v30ope0xf6_xtable, ope0xf6_xtable, sizeof(v30ope0xf6_xtable));
	v30ope0xf6_xtable[6] = v30div_ea8;
	v30ope0xf6_xtable[7] = v30idiv_ea8;
	CopyMemory(v30ope0xf7_xtable, ope0xf7_xtable, sizeof(v30ope0xf7_xtable));
	v30ope0xf7_xtable[6] = v30div_ea16;
	v30ope0xf7_xtable[7] = v30idiv_ea16;

#if defined(SUPPORT_V30ORIGINAL)
	{
		int	i;
		for (i = 0; i < 256; i++) {
			v30op_repc[i] = v30_reserved_repc;
		}
		V30PATCHING(v30op_repc, v30patch_repc);
	}
#endif
}

#if defined(VAEG_FIX)
void v30x_initreg(void) {
	I286_CS = 0xf000;
	CS_BASE = 0xf0000;
	I286_IP = 0xfff0;
	I286_FLAG = 0xf002;				// 286\82Ƃ̗B\88\EA\82̍\B7\88\D9
	i286core.s.adrsmask = 0xfffff;
	i286x_resetprefetch();
}
#endif

LABEL void v30x(void) {
	/* ToDo: \83V\83\93\83O\83\8B\83X\83e\83b\83v\8A\84\82荞\82݂̋\93\93\AE\82\AA\91\BD\95\AA\90\B3\82\B5\82\AD\82Ȃ\A2\81B Shinra */

	__asm {
				pushad
				mov		ebx, dword ptr (i286core.s.prefetchque)
				movzx	esi, I286_IP

				cmp		I286_TRAP, 0
				jne		short v30_trapping
				cmp		dmac.working, 0
				jne		short v30_dma_mnlp

				align	4
v30_mnlp:
#if defined(ENABLE_TRAP)
				mov		edx, esi
				movzx	ecx, I286_CS
				call	steptrap
#endif
				movzx	eax, bl
				call	v30op[eax*4]
				cmp		I286_REMCLOCK, 0
				jg		v30_mnlp
				mov		dword ptr (i286core.s.prefetchque), ebx
				mov		I286_IP, si
				popad
				ret

				align	4
v30_dma_mnlp:
#if defined(ENABLE_TRAP)
				mov		edx, esi
				movzx	ecx, I286_CS
				call	steptrap
#endif
				movzx	eax, bl
				call	v30op[eax*4]
				call	dmax86
				cmp		I286_REMCLOCK, 0
				jg		v30_dma_mnlp
				mov		dword ptr (i286core.s.prefetchque), ebx
				mov		I286_IP, si
				popad
				ret

				align	4
v30_trapping:
#if defined(ENABLE_TRAP)
				mov		edx, esi
				movzx	ecx, I286_CS
				call	steptrap
#endif
				movzx	eax, bl
				call	v30op[eax*4]
				cmp		I286_TRAP, 0
				je		v30notrap
				mov		ecx, 1
				call	i286x_localint
v30notrap:		mov		dword ptr (i286core.s.prefetchque), ebx
				mov		I286_IP, si
				popad
				ret
	}
}

#if defined(VAEG_FIX)
				/* \83V\83\93\83O\83\8B\83X\83e\83b\83v\8A\84\82荞\82݂̓\AE\8D\EC\82\F0\8FC\90\B3 */
LABEL void v30x_step(void) {

	__asm {
				pushad
				mov		ebx, dword ptr (i286core.s.prefetchque)
				movzx	esi, I286_IP

				movzx	eax, bl
				cmp		I286_TRAP, 0
				jne		short v30_trapping

				call	v30op[eax*4]
nexts:
				mov		dword ptr (i286core.s.prefetchque), ebx
				mov		I286_IP, si

				call	dmap_i286
				popad
				ret

v30_trapping:
				call	v30op[eax*4]
				mov		ecx, 1
				call	i286x_localint
				jmp		nexts
		}
}
#else
LABEL void v30x_step(void) {

	__asm {
				pushad
				mov		ebx, dword ptr (i286core.s.prefetchque)
				movzx	esi, I286_IP

				movzx	eax, bl
				call	v30op[eax*4]

				cmp		I286_TRAP, 0
				je		short nexts
				mov		ecx, 1
				call	i286x_localint
nexts:
				mov		dword ptr (i286core.s.prefetchque), ebx
				mov		I286_IP, si

				call	dmax86
				popad
				ret
		}
}

