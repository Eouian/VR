
	PRESERVE8
    AREA F1C100S_INIT, CODE, READONLY
		
    IMPORT  Vector_Table	
    IMPORT  __main

;--------------------------------------------
; Mode bits and interrupt flag (I&F) defines
;--------------------------------------------
USR_MODE    EQU     0x10
FIQ_MODE    EQU     0x11
IRQ_MODE    EQU     0x12
SVC_MODE    EQU     0x13
ABT_MODE    EQU     0x17
UDF_MODE    EQU     0x1B
SYS_MODE    EQU     0x1F
I_BIT       EQU     0x80
F_BIT       EQU     0x40

DRAM_ADDR    EQU (0x80000000)	
DRAM_SIZE    EQU (0x2000000) ;F1C100S=0x2000000 F1C200S=0x4000000	
MMU_TAB_SIZE EQU (0x4000) ;16KB

RAM_Limit               EQU             (DRAM_ADDR+DRAM_SIZE-MMU_TAB_SIZE)
USR_Stack				EQU				RAM_Limit
IRQ_Stack       		EQU     		(USR_Stack-1024*50)      	
SVC_Stack       		EQU     		(IRQ_Stack-1024*50)      

    ENTRY
    EXPORT  Reset_Go
	
Reset_Go

    ;--------------------------------
    ; Initial Stack Pointer register
    ;--------------------------------
    ;INIT_STACK
     MSR    CPSR_c, #IRQ_MODE :OR: I_BIT :OR: F_BIT
     LDR    SP, =IRQ_Stack

     MSR    CPSR_c, #SYS_MODE :OR: I_BIT :OR: F_BIT
     LDR    SP, =USR_Stack

     MSR    CPSR_c, #SVC_MODE :OR: I_BIT :OR: F_BIT
     LDR    SP, =SVC_Stack

    ;------------------------------------------------------
    ; Set the normal exception vector of CP15 control bit
    ;------------------------------------------------------
	MRC p15, 0, r0 , c1, c0     ; r0 := cp15 register 1
	bic r0, r0, #0x2000         ; Clear bit13 in r1 =0低端向量表 =1高端向量表
	MCR p15, 0, r0 , c1, c0     ; cp15 register 1 := r0


	ldr r0, = Vector_Table
	mrc p15, 0, r2, c1, c0, 0  ;读出CP15
	ands r2, r2, #(1 << 13)    ;取13位判断
	ldreq r1, =0x00000000      ;值=0  Z=1相等
	ldrne r1, =0xFFFF0000	   ;值!=0 Z=0 不相等


	ldmia r0!, {r2-r8, r10}
	stmia r1!, {r2-r8, r10}
	ldmia r0!, {r2-r8, r10}
	stmia r1!, {r2-r8, r10}


    ;-----------------------------
    ;   enter the C code
    ;-----------------------------
    B   __main



    END






