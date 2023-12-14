
	PRESERVE8
	AREA Vect, CODE, READONLY


		EXPORT	Vector_Table
	    IMPORT  SWI_Handler2
		IMPORT	IRQ_Handler2
; *****************
; Exception Vectors
; *****************

; Note: LDR PC instructions are used here, though branch (B) instructions
; could also be used, unless the ROM is at an address >32MB.

;        ENTRY
Vector_Table		
        B       Reset_Go
        LDR     PC, Undefined_Addr
        LDR     PC, SWI_Addr
        LDR     PC, Prefetch_Addr
        LDR     PC, Abort_Addr
        NOP                             ; Reserved vector
        LDR     PC, IRQ_Addr
        LDR     PC, FIQ_Addr
        
				
        IMPORT  Reset_Go           ; In sys_start.s
        
Reset_Addr      DCD     Reset_Go
Undefined_Addr  DCD     Undefined_Handler
SWI_Addr        DCD     SWI_Handler
Prefetch_Addr   DCD     Prefetch_Handler
Abort_Addr      DCD     Abort_Handler
				        DCD		  0
IRQ_Addr        DCD     IRQ_Handler
FIQ_Addr        DCD     FIQ_Handler


; ************************
; Exception Handlers
; ************************

; The following dummy handlers do not do anything useful in this example.
; They are set up here for completeness.


Undefined_Handler
        B       Undefined_Handler
SWI_Handler
		B 			SWI_Handler2 
Prefetch_Handler
        B       Prefetch_Handler
Abort_Handler
        B       Abort_Handler
		NOP
IRQ_Handler
        B       IRQ_Handler1
FIQ_Handler
        B       FIQ_Handler
				
	
			
IRQ_Handler1


	push {lr};/* 保存lr地址 */
	push {r0-r3, r12};/* 保存r0-r3，r12寄存器 */
	mrs r0, spsr;/* 读取spsr寄存器 */
	push {r0};/* 保存spsr寄存器 */
	
    MSR    CPSR_c, #(0x80:OR:0x13);/* 进入SVC模式，允许其他中断再次进去 */
    push {lr};/* 保存lr地址 */
   
;  //-----------------------------------------------
    bl IRQ_Handler2
;  //-----------------------------------------------

    pop {lr};/* lr出栈 */
    MSR    CPSR_c, #(0x80:OR:0x12);/* 进入IRQ模式 */

	pop {r0};						
	msr spsr_cxsf, r0;/* 恢复spsr */
	pop {r0-r3, r12};/* r0-r3,r12出栈 */
	pop {lr};/* lr出栈 */
	subs pc, lr, #4;/* 将lr-4赋给pc */ 
  

	END
