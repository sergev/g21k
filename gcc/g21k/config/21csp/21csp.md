;; /*$Header: /home/gcc/CORETOOLS/gcc/config/21csp/21csp.md,v 1.32 1997/02/11 20:24:05 levb Exp $*/
; ---------------- md.0 ----------------  -*- rtl -*-
;=;=========== Additional Constraints ===========
;=;    
;=;	e         D_REGS (ax0,ax1,ay0,ay1,mx0,mx1,my0,my1,
;=;                       mr0,mr1,si,sr0,sr1,ar)
;=;	a 	A_X_REGS (ax0,ax1,mr0,mr1,sr0,sr1,ar)
;=;	A	A_Y_REGS (ay0, ay1)
;=;	b	M_X_REGS (mx0,mx1,mr0,mr1,sr0,sr1,ar)     
;=;	B	M_Y_REGS (my0, my1)
;=;	c	A_R_REGS (ar)
;=;	C	MR0_REG  (mr0,mr1,sr0,sr1)
;=;	d	S_I_REGS (mr0,mr1,si,sr0,sr1,ar)
;=;	D	SR1_REG  (sr1)
;=;	f	S_E_REGS (se)
;=;	h	DNAR_REGS  (all D_REGS except ar)
;=;			   (ax0,ax1,mx0,mx1,my0,my1,mr0,mr1,si)
;=;	j	AXNR_REGS (ax0,ax1)
;=;     Z       A_X_NSR_REGS (all A_X_REGS except sr0, sr1)
;=;			     (ax0,ax1,mr0,mr1,ar)
;=;	k	AX_F_REGS (ax0, ax1 for holding floating-point)
;=;	l	AY_F_REGS (ay0, ay1 for holding floating-point)
;=;	q	CNTR_REGS (just cntr)
;=;	w	dm i regs
;=;	x	dm m regs
;=;	y	pm i regs
;=;	z	pm m regs
;=;
;=;     I       16-bit immediate
;=;     J       (const_int 1)
;=;     K       14-bit immediate
;=;     L       8-bit immediate
;=;     N       (const_int -1)
;=;     O       (const_int 0)
;=;
;=;     M       dirty birdy constants. (alu)
;=;     P       dirty birdy constants. (pass)
;=;
;=;     m       any memory operand that is acceptable by go_if_legit
;=;     Q       any pre or post modify (register,register) any mode.
;=;	R	post-modify-update (i-reg,m-reg) mode. (Instr Type 2).

;;;
;=; operand punctuation:
;;;          41xx                    21xx
;;; p : pcrel - 1 of operand       immediate-1
;;; P : pcrel                      immediate
;;; ! : delayed branch (DB)        ignored
;;; - : swcond                     neg
;;; + : not swcond                 pos
;;; m : print register as mr or sr.
;;;
;=;==============================================

;;;  Implementation notes for the 21c0.
;;;
;;;  Inorder to have the compiler do a good job with the 
;;;  new register file I have told the register preferencer
;;;  to ignore the register file until reload.
;;;  This way if an instruction can endup in a parallelizable 
;;;  instruction it will and if not it will just use DREGS.
;;;  This is the best aproach.  You will note:  '*e' in the constraint
;;;  strings for the ALU operations.


(define_attr "cpu"
  "21xx,21c0"
  (const
   (if_then_else (symbol_ref   "target_A21XX") 
		 (const_string "21xx")
		 (const_string "21c0"))))

(define_attr "length" "" (const_int 1))

(define_attr "type"
  "alu,mac,shift,dmdread,dmpread,pmread,dmwrite,pmwrite,move,mindex,misc,branch,lbranch,call,icall,doloop,c0alu,c0mac,bertalu,bertmac"
	(const_string "misc"))


(define_function_unit "alu_mult_shift"
  1 1 
  (eq_attr "type" "alu,mac,shift")
  2 2)

(define_function_unit "misc"
  1 1
  (eq_attr "type" "move,misc,dmdread,pmread,dmpread,dmwrite,pmwrite")
  2 2)


;
;  Here we define how to fill delay branch slots. on the 21xx
;  We don't allow annulling of anykind. i.e. all delay slots are
;  executed no matter what happens on the jump.
;
;  We tell the compiler what can not go in a delay branch slot.
;

; We cannot use delay slots on 21csp. 
; On 21csp only short jumps can have delay slots but we can determine
; whether the jump is long or short in the last optimization phase,
; branch-shortening. At scheduler phase, when delay slots are filled in,
; we do not know the exact size of jumps. At scheduler phase, the size
; of all jumps is set to two, the size of long jumps.
; It IS HIGHLY DESIRABLE THAT IN FUTURE ARCHITECTURES THE DELAY SLOT
; FEATURE IS NOT MADE DEPENDABLE ON THE SIZE OF A JUMP INSTRUCTION.
; -- lev 01/28/96

;(define_delay (eq_attr "type" "branch,icall")
(define_delay (and (eq_attr "type" "branch,icall") (eq_attr "length" "1"))
  [(and (eq_attr "type" "!branch,call,icall,doloop")
        (eq_attr "length" "1"))
   (nil) (nil)
  (and (eq_attr "type" "!branch,call,icall,doloop")
        (eq_attr "length" "1"))
   (nil) (nil)
  ])


;;
(define_insn "abssi2"
  [(set (match_operand:SI 0 "register_operand" "=c,*c")
	(abs:SI (match_operand:SI 1 "register_operand" "a,*e")))]
  ""
  "%0=abs %1;"
  [(set_attr "type" "alu,c0alu")])


;; ALU Type-9A                  21csp01 alows any D_REG as AR and XOP,YOP
;;  ADDSI3   AR = XOP + YOP
;;           AR = YOP - 1
;;           AR = YOP + 1
;;
(define_insn ""
   [(set (match_operand:SI 0 "register_operand" "=c,*c")
	 (plus:SI (match_operand:SI 1 "register_operand" "A,*e")
		  (const_int 1)))]
  ""
  "%0=%1+1;"
  [(set_attr "type" "alu,c0alu")])

(define_insn ""
   [(set (match_operand:SI 0 "register_operand" "=c,*c")
	 (plus:SI (match_operand:SI 1 "register_operand" "A,*e")
		  (const_int -1)))]
  ""
  "%0=%1-1;"
  [(set_attr "type" "alu,c0alu")])

;; In the second alternative the operand constraints are 'c', 'a', 'e'.
;; For the third operand we specified 'e' but not 'A'. The reason
;; for this is that with 'A' we are more likely not to find a 
;; register pair for spill of operand 2. For example, if 'A' is specified,
;; the run-time library file doprnt.c aborts at compile time because
;; no spill register pairs are available.
;; -- lev 09/22/95.
(define_insn ""
   [(set (match_operand:SI 0 "register_operand"        "=c,c,*c")
     (plus:SI (match_operand:SI 1 "register_operand"   "%0,a,*e")
      (match_operand:SI 2 "subreg_reg_operand" 		"A,e,*e") )
      ;(subreg:SI (match_operand:DI 2 "register_operand" "A,e,*e") 1))
    )]
   ""
   "%0=%1+%2;"
  [(set_attr "type" "alu,alu,c0alu")])

(define_insn "addsi3"
   [(set (match_operand:SI 0 "register_operand"      "=c,c,c,c,c,*c")
     (plus:SI (match_operand:SI 1 "register_operand" "%0,a,A,A,a,*e")
       (match_operand:SI 2 "register_or_bert_operand" "A,A,J,N,M,*e")))]
  ""
  "@
   %0=%1+%2;
   %0=%1+%2;
   %0=%1+1;
   %0=%1-1;
   %0=%1+%2;
   %0=%1+%2;"
  [(set_attr "type" "alu,alu,alu,alu,bertalu,c0alu")])



;;
;; ADDDM3  - MODIFY ADDRESS DM
;; ADDPM3  - MODIFY ADDRESS PM
;;
;; I do this here so that loop can eliminate the loop constants that
;; may get rolled into a memory reference.
;;
(define_expand "adddm3"
   [(set (match_operand:DM 0 "register_operand" "")
	 (plus:DM (match_operand:DM 1 "register_operand" "")
		  (match_operand:DM 2 "register_or_const8bit_operand" "")))]
   ""
   "{
       if (0 && !rtx_equal_p (operands[0], operands[1])) {
          emit_move_insn (operands[0], operands[1]);
          operands[1] = operands[0];
       }

       if (CONSTANT_P (operands[1]))
          abort (); /* okay they don't come in a specific order */

       if (CONSTANT_P (operands[2]) && !CONSTANT_8bitP (operands[2])) {
            rtx const_reg = gen_reg_rtx (DMmode);
	    emit_insn (gen_rtx (SET, DMmode, const_reg, operands[2]));
            operands[2] = const_reg;
       }
}")


(define_expand "addpm3"
   [(set (match_operand:PM 0 "register_operand" "=*y,y")
	 (plus:PM (match_operand:PM 1 "register_operand" "*0,z")
		 (match_operand:PM 2 "register_or_const8bit_operand" "*zL,0L")))]
   ""
   "
{
   if (0 && !rtx_equal_p (operands[0], operands[1])) {
      emit_move_insn (operands[0], operands[1]);
      operands[1] = operands[0];
   }
   if (CONSTANT_P (operands[1]))
      abort (); /* okay they don't come in a specific order */

   if (CONSTANT_P (operands[2]) && !CONSTANT_8bitP (operands[2])) {
      rtx const_reg = gen_reg_rtx (PMmode);
      emit_insn (gen_rtx (SET, PMmode, const_reg, operands[2]));
      operands[2] = const_reg;
  }
}")
;;;
;;; These constraints are not obvious at all.
;;;
;;;   lets not ! strongly disparage but instead if we are spilling
;;;    just discourage slightly. by using ?.
;;;
;;; There is a potential bug in reload were best is computed try to
;;; change the ? to ! sometime. <bug21.c splineMakeCoeffs a.k.a. spoks>


(define_insn ""
   [(set (match_operand:DM 0 "register_operand" "=w,y,*?c")
         (plus:DM
           (match_operand:DM 1 "register_operand" "%0*?w?y,0*?y?w,*?e")
           (match_operand:DM 2 "register_or_const8bit_operand" "xL,zL,*?e")))]
  ""
  "*
   {
     switch (which_alternative) {
     case 0: case 1:
	if (rtx_equal_p (operands[0], operands[1]))
	   output_asm_insn (\"modify(%0,%2);\", operands);
        else
	   output_asm_insn (\"%0=%1; modify(%0,%2);\", operands);
	break;
     case 2:
	   output_asm_insn (\"%0=%1+%2;\", operands);
     }
     RET;
   }")

(define_insn ""
   [(set (match_operand:PM 0 "register_operand" "=w,y,*?c")
	 (plus:PM (match_operand:PM 1 "register_operand" "%0*?w?y,0*?y?w,*?e")
		  (match_operand:PM 2 "register_or_const8bit_operand" "xL,zL,*?e")))]
  ""
  "*
   {
     switch (which_alternative) {
     case 0: case 1:
	if (rtx_equal_p (operands[0], operands[1]))
	   output_asm_insn (\"modify(%0,%2);\", operands);
        else
	   output_asm_insn (\"%0=%1; modify(%0,%2);\", operands);
	break;
     case 2:
	   output_asm_insn (\"%0=%1+%2;\", operands);
     }
     RET;
   }")



(define_insn "subsi3"
   [(set (match_operand:SI 0 "register_operand"            "=c,c,*c")
	 (minus:SI (match_operand:SI 1 "register_operand"  "a,a,e")
		   (match_operand:SI 2 "register_or_bert_operand"  "A,M,e")))]
  ""
  "%0=%1-%2;"
  [(set_attr "type" "alu,bertalu,c0alu")])

;; see (query-pr "1037 --full")
(define_expand "subdm3"
  [(set (match_operand:DM 0 "register_operand" "")
	(plus:DM (match_operand:DM 1 "register_operand" "")
		 (match_operand:DM 2 "register_or_const8bit_operand" "")))]
  ""
  "
{
  if (CONSTANT_8bitP (operands[2])) { 
    operands[2] = gen_rtx (CONST, DMmode,
                                  gen_rtx (CONST_INT, VOIDmode,
                                           -AINTVAL (operands[2])));
  } else {
    rtx temp = gen_reg_rtx (SImode);
    emit_insn (gen_rtx (SET, SImode,
	                     temp,
                             gen_rtx (NEG, SImode,
                                           gen_rtx (SUBREG, SImode,
                                                            operands[2], 0))));
    operands[2] = gen_rtx (SUBREG, DMmode, temp, 0);
  } 
}")

(define_expand "subpm3"
  [(set (match_operand:PM 0 "register_operand" "")
	(plus:PM (match_operand:PM 1 "register_operand" "")
		 (match_operand:PM 2 "register_or_const8bit_operand" "")))]
  ""
  "
{
  if (CONSTANT_8bitP (operands[2])) {
    operands[2] = gen_rtx (CONST, DMmode,
                                  gen_rtx (CONST_INT, VOIDmode,
                                           -AINTVAL (operands[2])));
  } else {
    rtx temp = gen_reg_rtx (SImode);
    emit_insn (gen_rtx (SET, SImode,
	                     temp,
                             gen_rtx (NEG, SImode,
                                           gen_rtx (SUBREG, SImode,
                                                            operands[2], 0))));
    operands[2] = gen_rtx (SUBREG, PMmode, temp, 0);
  }
}")


;;(define_insn "multiply_accumulate"
;; [(set (match_operand:SI 0 "register_operand" "=C,*C")
;;   (plus:SI (match_operand:SI 1 "register_operand" "0,*0") ;; (match_dup 0)
;;	(subreg:SI (mult:DI (match_operand:SI 2 "register_operand" "b,*e")
;;	    (match_operand:SI 3 "register_operand" "B,*e")) 1)))]
;; ""
;; "%m0=%m0+%2*%3 (ss);"
;; [(set_attr "type" "mac")])


(define_expand "mulsi3"
  [(set (match_operand:SI 0 "register_operand" "=C")
	(mult:DI (match_operand:SI 1 "register_operand" "b")
		 (match_operand:SI 2 "register_operand" "B")))
  ]
  ""
  "{
     if (target_A21C0)
	{
          rtx reg = gen_reg_rtx (DImode);
	  rtx mult = gen_rtx (SET, DImode, reg,
                     gen_rtx (MULT, DImode, operands[1], operands[2]));
	  emit_insn (mult);
	  emit_move_insn (operands[0], gen_rtx(SUBREG, SImode, reg, 1));
          DONE;
        }
    }")


;; We use define_insn "mulsidi3" to match and generate code.
;; 7/25/95 -- lev
;;(define_insn ""
;;  [
;;    (set (match_operand:DI 0 "register_operand" "=C,*C")
;;         (mult:DI (match_operand:SI 1 "register_operand" "b,*e")
;;                  (match_operand:SI 2 "register_operand" "B,*e")))
;;  ]
;; ""
;; "@
;;  %m0=%1*%2 (ss);
;;  %m0=%1*%2 (ss);"
;; [(set_attr "type" "mac,c0mac")])


(define_insn "divsi3"
  [(set (match_operand:SI 0 "register_operand" "=&c")           	      ;quotient
			(div:SI (match_operand:SI 1 "register_operand" "j")   ;dividend
				(match_operand:SI 2 "register_operand" "Z"))) ;divisor
	;(clobber (match_dup 1))
	; We clobber ax0 and ax1 explicitly because clobber(match_dup 1)
	; did not work. bug13.c
	(clobber (reg 0 ))	; ax0
	(clobber (reg 1 ))	; ax1
	(clobber (reg 2)) 	; ay0
	(clobber (reg 3)) 	; ay1
	(clobber (reg 14)) 	; sr1
	(clobber (reg 15))]	; sr0
  "0"
       "ar = pass %2;
	ay0=ar;
	ar=%1 xor ar;
	ar=abs %1, %1=ar;
	sr=ashift ar by 1 (lo);
	ar=ay0;
	ar=abs ar, ay1=sr1;
	af=abs %1, ay0=sr0;
	divs ay1,ar;   
	divq ar; divq ar; divq ar;
	divq ar; divq ar; divq ar;
	divq ar; divq ar; divq ar;
	divq ar; divq ar; divq ar;
	divq ar; divq ar; divq ar;
	%0=pass ay0;
	if %- %0=-ay0;"
  [(set_attr "length" "26")])

(define_insn "udivsi3"
  [(set (match_operand:SI 0 "register_operand" "=h")                          ;quotient
                        (udiv:SI (match_operand:SI 1 "register_operand" "h")   ;dividend
                                 (match_operand:SI 2 "register_operand" "j"))) ;divisor
        ;(clobber (match_dup 2))
        ; We clobber ax0 and ax1 explicitly because clobber(match_dup 2)
        ; did not work. bug13.c
        (clobber (reg 0 ))      ; ax0
        (clobber (reg 1 ))      ; ax1
        (clobber (reg 2))       ; ay0
        (clobber (reg 3))       ; ay1
        (clobber (reg 14))      ; sr1
        (clobber (reg 15))      ; sr0
        (clobber (reg 16))]     ; ar
;; Temporary disable the inline code generation.
;; Because it causes compiler error in reload. 
"0"
"*
{
   rtx labels[4];
   labels[0]=gen_label_rtx();
   labels[1]=gen_label_rtx();
   labels[2]=gen_label_rtx();
   labels[3]=gen_label_rtx();
   output_asm_insn(\"af=abs %2, ar=%1;\",operands);
   output_asm_insn(\"if %+ jump %P0; \",labels);
   output_asm_insn(\"sr=lshift ar by -1 (lo);\",operands);
   output_asm_insn(\"ay0=sr0;\",operands);
   output_asm_insn(\"af=pass sr1;\",operands);
   output_asm_insn(\"ar=%2;\",operands);
   output_asm_insn(\"jump %P1 (db);\",labels);
   output_asm_insn(\"sr=lshift ar by -1 (lo);\",operands);
   output_asm_insn(\"ar=sr0;\",operands);
   ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, \"L\", CODE_LABEL_NUMBER (labels[0]));
   output_asm_insn(\"sr=lshift ar by 1 (lo);\",operands);
   output_asm_insn(\"ar=sr1;\",operands);
   output_asm_insn(\"af=pass ar, ay0=sr0;\",operands);
   output_asm_insn(\"ar=%2;\",operands);
   ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, \"L\", CODE_LABEL_NUMBER (labels[1]));
   output_asm_insn(\"astat=0x0;\",operands);
   output_asm_insn(\"divq ar;\",operands);
   output_asm_insn(\"divq ar; divq ar; divq ar;\",operands);
   output_asm_insn(\"divq ar; divq ar; divq ar;\",operands);
   output_asm_insn(\"divq ar; divq ar; divq ar;\",operands);
   output_asm_insn(\"divq ar; divq ar; divq ar;\",operands);
   output_asm_insn(\"divq ar; divq ar; divq ar;\",operands);
   output_asm_insn(\"af=abs %2;\",operands);
   output_asm_insn(\"if %+ jump %P3; \",labels);
   output_asm_insn(\"sr=%2*ay0 (uu);\",operands);
   output_asm_insn(\"af=pass %1;\",operands);
   output_asm_insn(\"ar=sr0-af;\",operands);
   output_asm_insn(\"if not ac jump %P2;\",labels);
   output_asm_insn(\"if eq jump %P2;\",labels);
   output_asm_insn(\"ar=ay0;\",operands);
   output_asm_insn(\"ar=ar-1;\",operands);
   output_asm_insn(\"ay0=ar;\",operands);
   output_asm_insn(\"jump %P3;\",labels);
   ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, \"L\", CODE_LABEL_NUMBER (labels[2]));
   output_asm_insn(\"ar=-ar;\",operands);
   output_asm_insn(\"ay1=%2;\",operands);
   output_asm_insn(\"ar=ar-ay1;\",operands);
   output_asm_insn(\"if not ac jump %P3;\",labels);
   output_asm_insn(\"ar=ay0;\",operands);
   output_asm_insn(\"ar=ar+1;\",operands);
   output_asm_insn(\"ay0=ar;\",operands);
   ASM_OUTPUT_INTERNAL_LABEL (asm_out_file, \"L\", CODE_LABEL_NUMBER (labels[3]));
   output_asm_insn(\"%0=ay0;\",operands);
  RET;
}
   "
  [(set_attr "length" "49")])


(define_insn "negsi2"
  [(set (match_operand:SI 0 "register_operand" "=c,*c")
	(neg:SI (match_operand:SI 1 "register_operand" "Aa,*e")))]
  ""
  "%0= -%1;"
 [(set_attr "type" "alu,c0alu")])


(define_insn "andsi3"
  [(set (match_operand:SI 0 "register_operand" "=c,*c")
	(and:SI (match_operand:SI 1 "register_operand" "a,*e")
		(match_operand:SI 2 "register_operand" "A,*e")))]
  ""
  "%0=%1 and %2;"
 [(set_attr "type" "alu,c0alu")])

(define_insn "iorsi3"
  [(set (match_operand:SI 0 "register_operand" "=c,*c")
	(ior:SI (match_operand:SI 1 "register_operand" "a,*e")
		(match_operand:SI 2 "register_operand" "A,*e")))]
  ""
  "%0=%1 or %2;"
 [(set_attr "type" "alu,c0alu")])

(define_insn "xorsi3"
  [(set (match_operand:SI 0 "register_operand" "=c,*c")
	(xor:SI (match_operand:SI 1 "register_operand" "a,*e")
		(match_operand:SI 2 "register_operand" "A,*e")))]
  ""
  "%0=%1 xor %2;"
 [(set_attr "type" "alu,c0alu")])


(define_insn "one_cmplsi2"
  [(set (match_operand:SI 0 "register_operand" "=c,*c")
	(not:SI (match_operand:SI 1 "register_operand" "aA,*e")))]
  ""
  "%0=not %1;"
 [(set_attr "type" "alu,c0alu")])


(define_insn ""
  [(set	(cc0)		
	(match_operand:SI 0 "register_operand" "aA,*e"))]
  ""
  "af=pass %0;"
 [(set_attr "type" "alu,c0alu")])

(define_expand "cmpsi"
 [(set (cc0)
       (compare (match_operand:SI 0 "register_operand" "")
		(match_operand:SI 1 "register_or_zero_operand" "")))
  (use (match_operand:SI 2 "" ""))]
 ""
 "
{
  switch(GET_CODE(operand2))
    {
    case LTU:
    case LEU:
    case GTU:
    case GEU:	
      operands[1]=force_reg (SImode, operands[1]);
      emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, 
			  gen_rtx (COMPARE, VOIDmode, operands[0], operands[1])));
      DONE;
      break;
    case LT:
    case LE:
    case GT:
    case GE:
    case EQ:
    case NE:
      if (CONST_INT_P (operands[1]) && INTVAL (operands[1])==0)
	emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, operands[0]));
      else
	emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, 
			    gen_rtx (COMPARE, VOIDmode, operands[0], operands[1])));
      DONE;
      break;	
    default:
      abort();
    }
}")


(define_insn ""
  [(set (cc0)
	(compare (match_operand:SI 0 "register_operand" "a,A,*e")
		 (const_int -1)))]
  ""
  "af=%0+1;"
 [(set_attr "type" "alu,alu,c0alu")])

(define_insn ""
  [(set (cc0)
	(compare (match_operand:SI 0 "register_operand" "a,A,*e")
		 (const_int 1)))]
  ""
  "af=%0-1;"
 [(set_attr "type" "alu,alu,c0alu")])

(define_insn ""
  [(set (cc0)
	(compare (match_operand:SI 0 "register_operand" "a,A,a,*e")
		 (match_operand:SI 1 "register_or_bert_operand" "A,a,M,*e")))]
  ""
  "af=%0-%1;"
 [(set_attr "type" "alu,alu,bertalu,c0alu")])


(define_expand "cmpdm"
  [(set (cc0)
	(compare (match_operand:DM 0 "register_operand" "")
		 (match_operand:DM 1 "register_operand" "")))
   (use (match_operand:DM 2 "" ""))]
  "" 
  "
{
  if(GET_CODE(operands[0])==SUBREG)
    operands[0]=SUBREG_REG(operands[0]);
  if(GET_CODE(operands[1])==SUBREG)
    operands[1]=SUBREG_REG(operands[1]);
  
  operands[0]=gen_rtx(SUBREG, SImode, operands[0], 0);
  operands[1]=gen_rtx(SUBREG, SImode, operands[1], 0);
  
  switch(GET_CODE(operand2))
    {
    case LTU:
    case LEU:
    case GTU:
    case GEU:	
      emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, 
			  gen_rtx (COMPARE, VOIDmode, operands[0], operands[1])));
      DONE;
    case LT:
    case LE:
    case GT:
    case GE:
    case EQ:
    case NE:
      emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, 
			  gen_rtx (COMPARE, VOIDmode, operands[0], operands[1])));
      DONE;
    default:
      abort();
    }
}")

(define_expand "cmppm"
  [(set (cc0)
	(compare (match_operand:PM 0 "register_operand" "")
		 (match_operand:PM 1 "register_operand" "")))
   (use (match_operand:DM 2 "" ""))]
  "" 
  "
{
  if(GET_CODE(operands[0])==SUBREG)
    operands[0]=SUBREG_REG(operands[0]);
  if(GET_CODE(operands[1])==SUBREG)
    operands[1]=SUBREG_REG(operands[1]);
  
  operands[0]=gen_rtx(SUBREG, SImode, operands[0], 0);
  operands[1]=gen_rtx(SUBREG, SImode, operands[1], 0);
  
  switch(GET_CODE(operand2))
    {
    case LTU:
    case LEU:
    case GTU:
    case GEU:	
      emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, 
			  gen_rtx (COMPARE, VOIDmode, operands[0], operands[1])));
      DONE;
    case LT:
    case LE:
    case GT:
    case GE:
    case EQ:
    case NE:
      emit_insn (gen_rtx (SET, VOIDmode, cc0_rtx, 
			  gen_rtx (COMPARE, VOIDmode, operands[0], operands[1])));
      DONE;
    default:
      abort();
    }
}")


(define_expand "beq"
  [(set (pc)
	(if_then_else (eq (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")

(define_expand "bne"
  [(set (pc)
	(if_then_else (ne (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")

(define_expand "bgt"
  [(set (pc)
	(if_then_else (gt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")
;
(define_expand "bgtu"
  [(set (pc)
	(if_then_else (gtu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")

(define_expand "blt"
  [(set (pc)
	(if_then_else (lt (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")

;
(define_expand "bltu"
  [(set (pc)
	(if_then_else (ltu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")

(define_expand "bge"
  [(set (pc)
	(if_then_else (ge (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")
;
(define_expand "bgeu"
  [(set (pc)
	(if_then_else (geu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")

(define_expand "ble"
  [(set (pc)
	(if_then_else (le (cc0)
			  (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")

(define_expand "bleu"
  [(set (pc)
	(if_then_else (leu (cc0)
			   (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "")

(define_insn ""
  [(set (pc)
	(if_then_else (gtu (cc0) (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		    (pc)))]
  ""
  "*
{
  rtx labels[1];
  labels[0]= gen_label_rtx();

  output_asm_insn(\"if eq jump %P0;\",labels);

  if (get_attr_length (insn) == 2) 
     output_asm_insn(\"if ac jump %P0;\",operands);
  else 
     output_asm_insn(\"if ac jump %0;\",operands);

  ASM_OUTPUT_INTERNAL_LABEL(asm_out_file, \"L\", CODE_LABEL_NUMBER(labels[0]));
  RET;
}"
  [(set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 2)
				      (const_int 3)))
   (set_attr "type" "misc")])

(define_insn ""
  [(set (pc)
	(if_then_else (leu (cc0) (const_int 0))
		      (pc)
		    (label_ref (match_operand 0 "" ""))))]
  ""
 "*
{
  rtx labels[1];
  labels[0]= gen_label_rtx();
  output_asm_insn(\"if eq jump %P0;\",labels);

  if (get_attr_length (insn) == 2) 
     output_asm_insn(\"if ac jump %P0;\",operands);
  else
     output_asm_insn(\"if ac jump %0;\",operands);

  ASM_OUTPUT_INTERNAL_LABEL(asm_out_file, \"L\", CODE_LABEL_NUMBER(labels[0]));
  RET;
}"
  [(set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 2)
				      (const_int 3)))
   (set_attr "type" "misc")])


(define_insn ""
  [(set (pc)
	(if_then_else (leu (cc0) (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		    (pc)))]
  ""
  "*
{
    if (get_attr_length (insn) == 2) 
       return \"if not ac jump %P0;\;if eq jump %P0;\";
    else
       return \"if not ac jump %0;\;if eq jump %0;\";
}"
  [(set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 2)
				      (const_int 4)))
   (set_attr "type" "misc")])

(define_insn ""
  [(set (pc)
	(if_then_else (gtu (cc0) (const_int 0))
		      (pc)
 		    (label_ref (match_operand 0 "" ""))))]
  ""
  "*
{
    if (get_attr_length (insn) == 2) 
       return \"if not ac jump %P0;\;if eq jump %P0;\";
    else
       return \"if not ac jump %0;\;if eq jump %0;\";
}"

  [(set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 2)
				      (const_int 4)))
   (set_attr "type" "misc")])



(define_insn ""
  [(set (pc)
	(if_then_else (ltu (cc0) (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		    (pc)))]
  ""
  "*
{
    if (get_attr_length (insn) == 1) 
       return \"if not ac jump %P0;\";
    else
       return \"if not ac jump %0;\";
}"
  [(set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 1)
				      (const_int 2)))
   (set_attr "type" "misc")])

(define_insn ""
  [(set (pc)
	(if_then_else (geu (cc0) (const_int 0))
		      (pc)
		    (label_ref (match_operand 0 "" ""))))]
  ""
  "*
{
    if (get_attr_length (insn) == 1) 
       return \"if not ac jump %P0;\";
    else 
       return \"if not ac jump %0;\";
}"
  [(set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 1)
				      (const_int 2)))
   (set_attr "type" "misc")])

(define_insn ""
  [(set (pc)
	(if_then_else (geu (cc0) (const_int 0))
		      (label_ref (match_operand 0 "" ""))
		    (pc)))]
  ""

  "*
{
    if (get_attr_length (insn) == 2) 
       return \"if ac jump %P0;\;if eq jump %P0;\";
    else
       return \"if ac jump %0;\;if eq jump %0;\";
}"

  [(set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 2)
				      (const_int 4)))
   (set_attr "type" "misc")])



(define_insn ""
  [(set (pc)
	(if_then_else (ltu (cc0) (const_int 0))
		      (pc)
		    (label_ref (match_operand 0 "" ""))))]
  ""
  "*
{
    if (get_attr_length (insn) == 2) 
       return  \"if ac jump %P0;\;if eq jump %P0;\";
    else
       return  \"if ac jump %0;\;if eq jump %0;\";
}"
  [(set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 2)
				      (const_int 4)))
   (set_attr "type" "misc")])





(define_insn "nop"
  [(const_int 0)]
  ""
  "nop;")

(define_insn "ashlsi3"
  [(set (match_operand:SI 0 "register_operand" "=D,D")
		   (ashift:SI (match_operand:SI 1 "register_operand" "d,d")
			      (match_operand:SI 2 "general_operand" "K,e")))
   (clobber (reg 15))] ; clobber sr0
""
"@
  sr=ashift %1 by %2 (hi);
  se=%2; sr=ashift %1 (hi);"
 [(set_attr_alternative "length" [(const_string "1") (const_string "2")])
  (set_attr "type" "misc,misc")])

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=D,D")
		   (ashiftrt:SI (match_operand:SI 1 "register_operand" "d,d")
				(neg:SI (match_operand:SI 2 "general_operand" "K,e"))))
   (clobber (reg 15))] ; clobber sr0
""
"@
  sr=ashift %1 by %2 (hi);
  se=%2; sr=ashift %1 (hi);"
 [(set_attr_alternative "length" [(const_string "1") (const_string "2")])
  (set_attr "type" "misc,misc")])

(define_expand "ashrsi3"
  [(set (match_dup 3)
	(neg:SI	(match_operand:SI 2 "nonmemory_operand" "")))
   (parallel [(set (match_operand:SI 0 "register_operand" "")
		   (ashiftrt:SI (match_operand:SI 1 "register_operand" "")
			      (neg:SI (match_dup 3 ))))
	      (clobber (reg 15))])] ; clobber sr0
  ""
  "
{
  if ( GET_CODE (operands[2]) == CONST_INT )
    {
      operands[3] = gen_rtx (CONST_INT, SImode, -INTVAL (operands[2]));

      emit_insn (gen_rtx (PARALLEL, VOIDmode, gen_rtvec (2,
		gen_rtx (SET,SImode,
			  operands[0],
			  gen_rtx (ASHIFTRT,SImode,
				   operands[1],
				   gen_rtx (NEG, SImode, operands[3]))),
		gen_rtx (CLOBBER, VOIDmode, gen_rtx (REG, VOIDmode, 15)))));
      DONE;
    }
  else
    operands[3]=gen_reg_rtx (SImode);
}")

(define_insn "lshlsi3"
  [(set (match_operand:SI 0 "register_operand" "=D,D")
	(lshift:SI (match_operand:SI 1 "register_operand" "d,d")
		   (match_operand:SI 2 "general_operand" "K,e")))
   (clobber (reg 15))] ; clobber sr0
 ""
 "@
  sr=lshift %1 by %2 (hi);
  se=%2; sr=lshift %1 (hi);"
 [(set_attr_alternative "length" [(const_string "1") (const_string "2")])
  (set_attr "type" "misc,misc")])

(define_insn ""
  [(set (match_operand:SI 0 "register_operand" "=D,D")
	(lshiftrt:SI (match_operand:SI 1 "register_operand" "d,d")
		     (neg:SI (match_operand:SI 2 "general_operand" "K,e"))))
   (clobber (reg 15))] ; clobber sr0
 ""
 "@
  sr=lshift %1 by %2 (hi);
  se=%2; sr=lshift %1 (hi);"
 [(set_attr_alternative "length" [(const_string "1") (const_string "2")])
  (set_attr "type" "misc,misc")])


(define_expand "lshrsi3"
  [(set (match_dup 3)
	(neg:SI	(match_operand:SI 2 "nonmemory_operand" "")))
  (parallel [(set (match_operand:SI 0 "register_operand" "")
		  (lshiftrt:SI (match_operand:SI 1 "register_operand" "")
			       (neg:SI (match_dup 3 ))))
	     (clobber (reg 15))])] ; clobber sr0
  ""
  "
{
  if ( GET_CODE (operands[2]) == CONST_INT )
    {
      operands[3] = gen_rtx (CONST_INT, SImode, -INTVAL (operands[2]));
      emit_insn (gen_rtx (PARALLEL, VOIDmode, gen_rtvec (2,
		gen_rtx (SET,SImode,
			  operands[0],
			  gen_rtx (LSHIFTRT,SImode,
				   operands[1],
				   gen_rtx (NEG, SImode, operands[3]))),
		gen_rtx (CLOBBER, VOIDmode, gen_rtx (REG, VOIDmode, 15)))));
      DONE;
    }
  else
    operands[3]=gen_reg_rtx (SImode);
}")

;; movsi:
;;
;; c <- P      AR   <- BertConstant
;; e <- rmi    DREG <- REG Any Register
;;                  <- MEM Any Valid Memory Operand
;;                  <- Any Valid Constant
;; r <- irQ    REG  <- Any Valid Constant
;;                  <- REG Any Register
;;                  <- PRE/POST modify with register pair i,m
;; m <- e      MEM  <- DREG
;; Q <- r      PRE/POST(i,m) <- REG Any Register
;; R <- i      POST(i,m) <- Any Constant
;;
(define_insn ""			      
  [(set (match_operand:SI 0 "general_operand" "=c,e,r,m,Q,R")
	(match_operand:SI 1 "general_operand" " P,rmi,irQ,e,r,i"))]
  ""
  "*
    if (insn_need_delay(insn)) {
	 return \"%0=%O1; nop;\";
    }

     if (which_alternative == 0)
       return \"%0=PASS %O1;\";
     else
       return \"%0=%O1;\";
    "
  [(set_attr "length" "2")
   (set_attr "type" "bertalu,misc,misc,misc,misc,misc")])

;; movdm:
;;
;; r <- r      REG  <- REG
;; e <- mi     DREG <- MEM Any Valid Memory Operand
;;                  <- Any Valid Constant
;; r <- iQ     REG  <- Any Valid Constant
;;                  <- PRE/POST modify with register pair i,m
;; m <- e      MEM  <- DREG
;; Q <- r      PRE/POST(i,m) <- REG Any Register
;; R <- i      POST(i,m) <- Any Constant
;;
(define_insn ""
  [(set (match_operand:DM 0 "general_operand" "=r,e,  r, m,Q,R")
	(match_operand:DM 1 "general_operand" " r,mi, iQ,e,r,i"))]
  ""
  "*
    if (insn_need_delay(insn)) {
	 return \"%0=%O1; nop;\";
    }
    return \"%0=%O1;\";
  "
  [(set_attr "length" "2")
   (set_attr "type" "misc,misc,misc,misc,misc,misc")])


;; movpm:
;;
;; r <- r      REG  <- REG
;; e <- mi     DREG <- MEM Any Valid Memory Operand
;;                  <- Any Valid Constant
;; r <- iQ     REG  <- Any Valid Constant
;;                  <- PRE/POST modify with register pair i,m
;; m <- e      MEM  <- DREG
;; Q <- r      PRE/POST(i,m) <- REG Any Register
;;
(define_insn ""
  [(set (match_operand:PM 0 "general_operand" "=r,e, r, m,Q")
	(match_operand:PM 1 "general_operand" " r,mi,iQ,e,r"))]
  ""
  "*
    if (insn_need_delay(insn)) {
	 return \"%0=%O1; nop;\";
    }
    return \"%0=%O1;\";
  "
 [(set_attr "length" "2")
  (set_attr "type" "misc,misc,misc,misc,misc")
 ])


(define_expand "movsi"
  [(set (match_operand:SI 0 "general_operand" "") 
	(match_operand:SI 1 "general_operand" ""))] 
  ""
  "
{
   if (GET_CODE(operands[0]) == MEM
       && (GET_CODE(operands[1]) == MEM
           || (CONSTANT_P (operands[1]))))
     {
	rtx new;
	emit_insn(gen_rtx(SET, SImode,
			new=gen_reg_rtx(SImode),
			operands[1]));
	emit_insn(gen_rtx(SET, SImode,
			operands[0],
			new));
	DONE;
     }

   else 
   if(GET_CODE(operands[0])==MEM && 
	   CONSTANT_P (XEXP (operands[0],0)) && 
	   CONSTANT_P(operands[1]))
    {
      operands[1] = force_reg(SImode,operands[1]);
    }
   /* if we are generating a move that has a paradoxical
      conversion of a constant elliminate the conversion. */
   else if (GET_CODE (operands[1]) == SUBREG
            && GET_CODE (XEXP (operands[1], 0)) == CONST) {
	operands[1] = XEXP (XEXP (operands[1], 0), 0);
   }
}")

(define_insn "movstrictsi"			      
  [(set (strict_low_part
	 (match_operand:SI 0 "general_operand" "=e,r,m,Q,R"))
	(match_operand:SI 1 "general_operand"  "ermi,riQ,e,r,i"))]
  ""
  "*
    if (insn_need_delay(insn)) {
	 return \"%0=%O1; nop;\";
    }
    return \"%0=%O1;\";
  "
 [(set_attr "length" "2")
  (set_attr "type" "misc,misc,misc,misc,misc")])

(define_expand "movdm"			      
  [(set (match_operand:DM 0 "general_operand" "") 
	(match_operand:DM 1 "general_operand" ""))] 
  ""
  "
{
   if(GET_CODE(operands[0])==MEM && GET_CODE(operands[1])==MEM)
     {	
	rtx new;
	emit_insn(gen_rtx(SET, DMmode,
			new=gen_reg_rtx(DMmode),
			operands[1]));
	emit_insn(gen_rtx(SET, DMmode,
			operands[0],
			new));
	DONE;
     }
  else if(GET_CODE(operands[0])==MEM && CONSTANT_P(operands[1]))
    {
      operands[1] = force_reg(DMmode,operands[1]);
    }
  else if(GET_CODE(operands[1])==CONST_INT)
    {
     operands[1]=gen_rtx (CONST_INT,DMmode,INTVAL (operands[1]));
    }
}")

(define_expand "movpm"
  [(set (match_operand:PM 0 "general_operand" "") 
	(match_operand:PM 1 "general_operand" ""))] 
  ""
  "
{
   if(GET_CODE(operands[0])==MEM && GET_CODE(operands[1])==MEM)
     {	
	rtx new;
	emit_insn(gen_rtx(SET, PMmode,
			new=gen_reg_rtx(PMmode),
			operands[1]));
	emit_insn(gen_rtx(SET, PMmode,
			operands[0],
			new));
	DONE;
     }
  else if(GET_CODE(operands[0])==MEM && CONSTANT_P(operands[1]))
    {
      operands[1] = force_reg(PMmode,operands[1]);
    }
  else if(GET_CODE(operands[1])==CONST_INT)
    {
     operands[1]=gen_rtx (CONST_INT,PMmode,INTVAL (operands[1]));
    }

}")




(define_insn ""
  [(set (match_operand:PM 0 "register_operand" "=r")
	(match_operand:DM 1 "register_operand" "r"))]
  ""
  "%0=%1; !! Warning -- conversion from DM to PM")

(define_insn ""
  [(set (match_operand:DM 0 "register_operand" "=r")
	(match_operand:PM 1 "register_operand" "r"))]
  ""
  "%0=%1; !! Warning -- conversion from PM to DM")

(define_expand "truncsidm2"
  [(set (match_operand:DM 0 "general_operand" "")
	(subreg:DM (match_operand:SI 1 "general_operand" "") 0))]
 ""
 "
{
  rtx op1 = operands[1];
  switch(GET_CODE(op1)) 
    {
    case MEM:
    case CONST: 
      {
	rtx new = copy_rtx(operands[1]);
	GET_MODE(new) = DMmode;
	emit_move_insn(operands[0], new);
      }
      DONE;
    case SUBREG:
      if(GET_MODE(SUBREG_REG(operands[1]))==DMmode)
	operands[1]=SUBREG_REG(operands[1]);
      else
	{
	  operands[1] = copy_rtx(operands[1]);
	  GET_MODE(operands[1]) = DMmode;
	}
      break;
    default:
      operands[1] = force_reg(SImode, operands[1]);
    }
}")

(define_expand "truncsipm2"
  [(set (match_operand:PM 0 "general_operand" "")
	(subreg:PM (match_operand:SI 1 "general_operand" "") 0))]
  ""
 "
{
  rtx op1 = operands[1];
  switch(GET_CODE(op1)) 
    {
    case MEM:
    case CONST: 
      {
	rtx new = copy_rtx(operands[1]);
	GET_MODE(new) = PMmode;
	emit_move_insn(operands[0], new);
      }
      DONE;
      break;
    case SUBREG:
      if(GET_MODE(SUBREG_REG(operands[1]))==PMmode)
	operands[1]=SUBREG_REG(operands[1]);
      else
	{
	  operands[1] = copy_rtx(operands[1]);
	  GET_MODE(operands[1]) = PMmode;
	}
      break;
    default:
      operands[1] = force_reg(SImode, operands[1]);
    }
}")

(define_expand "extenddmsi"
  [(set (match_operand:SI 0 "register_operand" "")
	(subreg:SI (match_operand:DM 1 "general_operand" "") 0))]
  ""
  "
 {
   switch(GET_CODE(operands[1])) 
     {
     case MEM:
     case CONST: 
       {
 	rtx new = copy_rtx(operands[1]);
 	GET_MODE(new) = SImode;
 	emit_move_insn(operands[0], new);
       }
       DONE;
       break;
     default:
       operands[1] = force_reg(DMmode, operands[1]);
     }
   }")


(define_expand "extendpmsi"
  [(set (match_operand:SI 0 "general_operand" "")
	(subreg:SI (match_operand:PM 1 "general_operand" "") 0))]
  ""
  "
 {
   switch(GET_CODE(operands[1])) 
     {
     case MEM:
     case CONST: 
       {
 	rtx new = copy_rtx(operands[1]);
 	GET_MODE(new) = SImode;
 	emit_move_insn(operands[0], new);
       }
       DONE;
       break;
     default:
       operands[1] = force_reg(PMmode, operands[1]);
     }
   }")


(define_expand "movdi"
  [(set (match_operand:DI 0 "general_operand" "")
	(match_operand:DI 1 "general_operand" ""))]
  ""
  "
{
  expand_double_move(operands[0], operands[1]);
  DONE;
}")

(define_expand "movdf"
  [(set (match_operand:DF 0 "general_operand" "")
	(match_operand:DF 1 "general_operand" ""))]
  ""
  "
{
  expand_double_move (operands[0], operands[1]);
  DONE;
}")

(define_expand "movsf"
  [(set (match_operand:SF 0 "general_operand" "")
	(match_operand:SF 1 "general_operand" ""))]
  ""
  "
{
  expand_double_move(operands[0], operands[1]);
  DONE;
}")


;; ********************************************************
;; An intermediate register is required to move 
;; between operand0 and operand1.
;; Case 1.
;;     match (mem (plus (reg) (const16))) 
;;     secondary reload register, m-reg, is required for const16
;; Case 2.
;;     match (set (non-Dreg)
;;     	     (mem (plus (reg) (reg_or_const8))) )
;;     secondary reload register, D-reg, is required for 
;;     memory dm(reg,reg_or_const8).
;; Case 3.
;;     match (plus (ireg)
;;		(const8))
;;     secondary reload register, i-reg, is required for 
;;     ireg+const8.
;; Case 4.
;;	match(set (non-Dreg)
;;	    	(memory-psedo-reg)
;; Move from memory to non-DREG requires a secondary register
;;
;; 
;; For details on register class selection see function 
;; secondary_input_reload_class
;; 8/4/95 -- lev
;; ********************************************************
(define_expand "reload_indm"
  [(set (match_operand:DM 0 "always_true" "=r")
	(match_operand:DM 1 "dm_incoming_reload" ""))
   (clobber (match_operand:DM 2 "always_true" "=&y"))
  ]
 ""
 "
{
#ifdef 0
    if (GET_CODE (operands[1]) == MEM &&
	!CONSTANT_8bitP (XEXP (XEXP (operands[1], 0), 1))) {
	emit_move_insn (operands[2], operands[1]);
	emit_move_insn (operands[0], operands[2]);
    } else if (GET_CODE (operands[1]) == PLUS &&
	CONSTANT_P (XEXP (operands[1], 1)) &&
	!CONSTANT_8bitP (XEXP (operands[1], 1))) {
	emit_move_insn (operands[2], operands[1]);
	emit_move_insn (operands[0], operands[2]);
    } else if (GET_CODE (operands[1]) == PLUS) {
	emit_move_insn (operands[0], XEXP (operands[1],0));
	emit_insn (gen_rtx (SET, GET_MODE (operands[1]), operands[0],
	    gen_rtx (PLUS, GET_MODE (operands[1]), operands[0], 
		XEXP (operands[1], 1))));
    }
    DONE;
#endif

if (GET_CODE (operands[1]) == PLUS) {
        emit_move_insn (operands[0], XEXP (operands[1],0));
        emit_insn (gen_rtx (SET, GET_MODE (operands[1]), operands[0],
            gen_rtx (PLUS, GET_MODE (operands[1]), operands[0],
                XEXP (operands[1], 1))));
} else {
    emit_move_insn (operands[2], operands[1]);
    emit_move_insn (operands[0], operands[2]);
}
DONE;
}")

(define_expand "reload_inpm"
  [(set (match_operand:PM 0 "always_true" "=r")
	(match_operand:PM 1 "pm_incoming_reload" ""))
   (clobber (match_operand:PM 2 "always_true" "=&y"))]
 ""
 "
{
  if (GET_CODE (operands[1]) == PLUS) {
     emit_move_insn (operands[0], XEXP (operands[1],0));
     emit_insn (gen_rtx (SET, GET_MODE (operands[1]), operands[0],
                         gen_rtx (PLUS, GET_MODE (operands[1]), operands[0], XEXP (operands[1], 1))));
  }				      
  else {
    emit_move_insn (operands[2], operands[1]);
    emit_move_insn (operands[0], operands[2]);
/*    sec_reload_in(operands[0], operands[1], operands[2], PMmode); */
  }
    DONE;
  }")


(define_expand "reload_outdm"
  [(set (match_operand:DM 0 "dm_outcoming_reload" "")
	(match_operand:DM 1 "always_true" "r"))
   (clobber (match_operand:DM 2 "always_true" "=&y"))]
 "0"
 "
{
  sec_reload_out(operands[0], operands[1], operands[2], DMmode);
  DONE;
  }")

(define_expand "reload_outpm"
  [(set (match_operand:PM 0 "pm_outcoming_reload" "")
	(match_operand:PM 1 "always_true" "r"))
   (clobber (match_operand:PM 2 "always_true" "=&y"))]
 "0"
 "
{
  sec_reload_out(operands[0], operands[1], operands[2], PMmode);
  DONE;
  }")

;;;
;;; Do Loop instruction Type: 10 and 11A.
;;;
(define_insn ""
  [(unspec_volatile [(match_operand:SI 0 "general_operand" "ir")	;; loop counter
		     (match_operand 1 "" "")	;; start label
		     (match_operand 2 "" "")	;; end label
		     (match_operand 3 "immediate_operand" "i")]	;; depth
		     3)]
   ""
   "*
      if (GET_CODE (PREV_INSN (insn)) != CODE_LABEL)
        {
           rtx pattern = PATTERN (PREV_INSN (insn));
           if (GET_CODE (pattern) != SET
               || XEXP (pattern, 0) != operands[0])
             abort();
        }
      if (empty_doloop_p(insn)) 
         return \" !!! Empty loop !! \";

      else
         return \"cntr= %0; do %p2 until ce; !! (depth %3)\";
   "
   [(set_attr "type" "doloop")
    (set_attr "length" "2")])


(define_insn ""
  [(set (cc0)
	 (unspec_volatile [(match_operand 0 "" "")] 4))]
   ""
   "*
	if (tight_doloop_end_p(insn))
	   return \"nop; !! Padded End of Loop\";
	else if (continued_doloop_p(insn))    
	   return \"nop; !! End of Continued Loop\";
	else 
           return \"! End of Doloop\";
   ")

	
(define_insn ""
  [(set (pc)
	(if_then_else
	 (match_operator 0 "comparison_operator"
			 [(match_operand 1 "doloop_cc0_p" "")
			  (const_int 0)])
	 (label_ref (match_operand 0 "" ""))
	 (pc)))]
  "doloop_jump_p(insn)"
  "*
	if (tight_doloop_end_p(insn))
          return \" nop; nop; nop; !! Tight Loop Jump Was Here \";
	if (continued_doloop_p(insn))
          return \" nop; nop; !! Continued Loop Jump Was Here \";
	else
          return \"      !!  Loop Jump Was Here \";
	")

;; Define both directions of branch and return.

(define_insn ""
  [(set (pc)
	(if_then_else (match_operator 1 "comparison_operator"
				      [(cc0) (const_int 0)])
		      (label_ref (match_operand 0 "" ""))
		      (pc)))]
  ""
  "*
{ 
  if (get_attr_length (insn) == 1)
     return \"if %j1 jump %P0 %!;\";
  else
     return \"if %j1 jump %0;\";
}"

  [(set_attr "type" "branch")
   (set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 1)
				      (const_int 2)))])

(define_insn ""
  [(set (pc)
	(if_then_else (match_operator 1 "comparison_operator"
				[(cc0) (const_int 0)])
		      (pc)
		      (label_ref (match_operand 0 "" ""))))]
  ""
  "*
{ 
  if (get_attr_length (insn) == 1)
     return \"if %J1 jump %P0 %!;\";
  else
     return \"if %J1 jump %0;\";
}"

  [(set_attr "type" "branch")
   (set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 1)
				      (const_int 2)))])



(define_insn "jump"
  [(set (pc)
	(label_ref (match_operand 0 "" "")))]
  ""
  "*
{ 
  if (get_attr_length (insn) == 1)
     return \"jump %P0 %!;\";
  else
     return \"jump %0;\";
}"
  [(set_attr "type" "branch")
   (set (attr "length") (if_then_else (lt (abs (minus (pc) (match_dup 0))) (const_int 4096))
				      (const_int 1)
				      (const_int 2)))])


(define_insn "tablejump"
 [(set (pc) 
	(match_operand:PM 0 "register_operand" "yw"))
   (use (label_ref (match_operand 1 "" "")))]
  ""
  "jump (%0) %!;"
  [(set_attr "type" "branch")])


(define_insn "indirect_jump"
  [(set (pc) 
	(match_operand:PM 0 "register_operand" "yw"))]
  ""
  "jump (%0) %!;"
  [(set_attr "type" "branch")])


;; For calls we always specify use of parameter registers to prevent
;; reload to use them as scratch registers for indirect calls in
;; situations like this 
;;      ay1=32;         !! Load parm2
;;      ar=dm(2,i5);    !! Load parm1
;;      ar=dm(1,i5);    !! Load the address of function in ar
;;      i2=ar;          !! Move data reg, ar, into addr reg, i2.
;;      call (i2) ;
;; See perennial test c0651.c. ipr6 --lev.

(define_expand "call"
  [
    (parallel [
    (use (reg 3)) ;; AY1
    (use (reg 1)) ;; AX1
    (call (match_operand:PM 0 "general_operand" "")
	 (match_operand:SI 1 "general_operand" ""))
    ])
  ]
  ""
   "
   if (A21C0 && INDIRECT_CALLING)
	{
	   rtx new = gen_reg_rtx (PMmode);
           emit_move_insn (new, XEXP (operands[0], 0));
	   operands[0] = gen_rtx (MEM, PMmode, new);
        }")

(define_insn ""
  [
    (parallel [
    (use (reg 3)) ;; AY1
    (use (reg 1)) ;; AX1
    (call (mem (match_operand:PM 0 "call_operand" ""))
	 (match_operand:SI 1 "general_operand" "g"))
    ])
  ]
  "!(A21C0 && INDIRECT_CALLING)"
  "*
    if (insn_need_delay (insn)) {
	return \"call %A0; nop;\";
    }
    return \"call %A0;\";
"

  [(set_attr "type" "call")])

(define_insn ""
  [
    (parallel [
    (use (reg 3)) ;; AY1
    (use (reg 1)) ;; AX1
    (call (mem (match_operand:PM 0 "register_operand" "yw"))
	 (match_operand:SI 1 "general_operand" "g"))
    ])
  ]
  ""
  "*
{
  if (A21C0 && INDIRECT_CALLING)
    return 
	\"call (%0) (db);\;dm (i4, m5) = i5, i5 = i4;\;dm (i4, m5) = STACKA;\";
  else
    if (insn_need_delay (insn)) {
	return \"call (%0) %!; nop;\";
    }
    return \"call (%0) %!;\";
}"
  [(set_attr "type" "icall")])

(define_expand "call_value"
  [
    (parallel [
    (use (reg 3)) ;; AY1
    (use (reg 1)) ;; AX1
    (set (match_operand 0 "register_operand" "")
	(call (match_operand:PM 1 "general_operand" "")
	      (match_operand:SI 2 "general_operand" "")))
    ])
  ]
  ""
  "
   if (A21C0 && INDIRECT_CALLING)
	{
	   rtx new = gen_reg_rtx (PMmode);
           emit_move_insn (new, XEXP (operands[1], 0));
	   operands[1] = gen_rtx (MEM, PMmode, new);
        }
   ")


(define_insn ""
  [
    (parallel [
    (use (reg 3)) ;; AY1
    (use (reg 1)) ;; AX1
    (set (match_operand 0 "register_operand" "=e")
	(call (mem (match_operand:PM 1 "call_operand" ""))
	      (match_operand:SI 2 "general_operand" "g")))
    ])
  ]
  "!(A21C0 && INDIRECT_CALLING)"
  "*
    if (insn_need_delay (insn)) {
	return \"call %A1; nop;\";
    }
    return \"call %A1;\";
  "
  [(set_attr "type" "call")
   (set_attr "length" "3")
  ])


(define_insn ""
  [
    (parallel [
    (use (reg 3)) ;; AY1
    (use (reg 1)) ;; AX1
    (set (match_operand 0 "register_operand" "=e")
	(call (mem (match_operand:PM 1 "register_operand" "yw"))
	      (match_operand:SI 2 "general_operand" "g")))
    ])
    ]
  ""
  "*
{
  if (A21C0 && INDIRECT_CALLING)
    return
        \"call (%1) (db);\;dm (i4, m5) = i5, i5 = i4;\;dm (i4, m5) = STA
CKA;\";
  else
    if (insn_need_delay (insn)) {
	return \"call (%1) %!; nop;\";
    }
    return \"call (%1) %!;\";
}"
  [(set_attr "type" "icall")
  (set_attr "length" "2")
  ])


(define_expand "truncdmpm2"
  [(set (match_operand:PM 0 "general_operand" "")
	(subreg:PM (match_operand:DM 1 "general_operand" "") 0))]
  ""
 "{
     rtx op1 = operands[1];

     switch(GET_CODE(op1)) 
       {
        case MEM:
        case CONST: 
          {
          rtx new = copy_rtx(operands[1]);
          GET_MODE(new) = PMmode;
          emit_move_insn(operands[0], new);
       }
       DONE;
       break;
     default:
       operands[1] = force_reg(DMmode, operands[1]);
    }
  }"
)

(define_expand "extendpmdm"
  [(set (match_operand:DM 0 "general_operand" "")
	(subreg:DM (match_operand:PM 1 "general_operand" "") 0))]
  ""
  "{
     rtx op1 = operands[1];

     switch(GET_CODE(op1)) 
        {
        case MEM:
        case CONST: 
          {
          rtx new = copy_rtx(operands[1]);
          GET_MODE(new) = DMmode;
          emit_move_insn(operands[0], new);
          }
       DONE;
       break;
     default:
       operands[1] = force_reg(PMmode, operands[1]);
    }
  }"
)


;; dreg0 = ar;
;; ar=-dreg0; ! dreg0 is dead	=> ar=-ar;
;; 
(define_peephole
  [(set (match_operand 0 "register_operand" "")
        (match_operand 1 "ar_register_operand" ""))
   (set (match_operand 2 "ar_register_operand" "")
	(neg:SI (match_operand 3 "register_operand" "")))
 ]
  " (dreg_operand (operands[0], GET_MODE (operands[0]))
    && rtx_similar_p(operands[0], operands[3])
    && dead_or_set_p (insn, operands[3]))
  "
  "%1=-%1;"
)

(define_peephole 
  [(set (match_operand 0 "general_operand" "")
	(match_operand 1 "general_operand" ""))
   (set (match_operand 2 "general_operand" "")
	(match_operand 3 "general_operand" ""))]
  "
     (rtx_similar_p(operands[0], operands[3]) &&
      rtx_similar_p(operands[1], operands[2]))
  "
  "%0=%O1;"
)

;; Delete unnecessary moves to intermediate registers. PR 845.
;; For example:
;; dreg0 = ar;
;; mem=dreg0; ! dreg0 is dead	=> mem=dreg0;
(define_peephole 
  [(set (match_operand 0 "nonsubreg_register_operand" "")
	(match_operand 1 "nonsubreg_nonmemory_operand" ""))
   (set (match_operand 2 "general_operand" "")
	(match_operand 3 "nonsubreg_register_operand" ""))]

  "!(memory_operand (operands[2], GET_MODE (operands[2]))
    && !dreg_operand (operands[1], GET_MODE (operands[1])))
    && rtx_similar_p (operands[0], operands[3])
    && dead_or_set_p (insn, operands[3])
   "

  "*
	output_asm_insn (\"%2=%1;!--peep-2\", operands);
	RET;
  "

)

;; reg0 = ar;
;; reg1 = reg0; ==> reg1=ar; ! reg0 is dead.
;;(define_peephole 
;;  [(set (match_operand 0 "nonsubreg_register_operand" "")
;;	(match_operand 1 "nonsubreg_nonmemory_operand" ""))
;;   (set (match_operand 2 "nonsubreg_register_operand" "")
;;	(match_operand 3 "nonsubreg_register_operand" ""))
;;    (use (match_dup 2))]
;;
;;  " rtx_similar_p (operands[0], operands[3])
;;    && dead_or_set_p (insn, operands[3])
;;   "
;;
;;  "*
;;	output_asm_insn (\"%2=%1;!--peep-5\", operands);
;;      return "";
;;  "
;;
;;)


(define_peephole 
  [(set (match_operand:SI 0 "general_operand" "")
	(match_operand:SI 1 "general_operand" ""))
   (set (match_operand:DM 2 "general_operand" "")
	(match_operand:DM 3 "general_operand" ""))]

  "
     (rtx_similar_p(operands[0], operands[3]) &&
      rtx_similar_p(operands[1], operands[2]))
  "
  "%0=%O1;"
)


(define_peephole	;;; Any 2 insns
  [(match_operand 0 "" "")
   (match_operand 1 "" "")]
  "!A21C0 && peep_parallelizeable_2(1)"
  "*
  {
    dump_parallel(2);
    return (\"!!PRLL-2 \");
  }"
)


(define_expand "adddi3"
  [(set (match_operand:DI 0 "register_operand" "")
	(plus:DI (match_operand:DI 1 "register_operand" "")
		 (match_operand:DI 2 "register_operand" "")))]
  "INLINE_DOUBLE"
  "
   { 
       rtx
	   op0_high = gen_reg_rtx(SImode),
	   op0_low = gen_reg_rtx(SImode),
	   op1_high = force_reg(SImode, 
		    operand_subword(operands[1], 0, 1, DImode)),
	   op1_low  = force_reg(SImode,
		    operand_subword(operands[1], 1, 1, DImode)),
	   op2_high = force_reg(SImode,
		    operand_subword(operands[2], 0, 1, DImode)),
	   op2_low  = force_reg(SImode, 
		    operand_subword(operands[2], 1, 1, DImode));
       if (op0_high == 0 || op0_low == 0 ||
           op1_high == 0 || op1_low == 0 ||
           op2_high == 0 || op2_low == 0)
         FAIL;
       else {
	 rtx
	    low_add = gen_rtx(SET, SImode, op0_low,
			       gen_rtx(PLUS, SImode, op1_low, op2_low)),
	    high_add = gen_rtx(SET, SImode, op0_high,
			    gen_rtx(UNSPEC, SImode, 
				gen_rtvec(2, op1_high, op2_high), 12)),
            clobb = gen_rtx(CLOBBER, SImode, gen_rtx(REG, SImode, 3));
         emit_insn(gen_rtx (PARALLEL, VOIDmode,
           		     gen_rtvec(3, low_add, high_add, clobb)));
	 emit_insn(gen_rtx (SET, SImode,
			    operand_subword(operands[0], 1, 1, DImode),
			  	op0_low));
	 emit_insn(gen_rtx (SET, SImode,
		     gen_rtx (STRICT_LOW_PART, VOIDmode,
	                operand_subword(operands[0], 0, 1, DImode)),
				op0_high));
	/*
	 * USE insn is needed to prevent the compiler from generating
	 * incorrect REG_UNUSED note for op0_high which results in
	 * wrong code for (long int) v1 + (long int) v2 + (long int)v3
	 * This a temporary fix, the real problem is in flow.
	 */
	emit_insn(gen_rtx(USE, SImode, op0_high));
	 DONE;
     }
   }
 "
)

;; In the following insn AR is implicitly clobbered, but we do not
;; need a special "clobber" for it because of "=&c" for operand 3.
;; lev, suggests changing single_reg to register_operand, mmh concurs
(define_insn ""
  [
   (parallel
     [(set (match_operand:SI 0 "register_operand" "=e")
   	(plus:SI (match_operand:SI 1 "register_operand" "a")
	         (match_operand:SI 2 "register_operand" "A")))
      (set (match_operand:SI 3 "register_operand" "=&c")
   	(unspec:SI [(match_operand:SI 4 "register_operand" "a")
	            (match_operand:SI 5 "register_operand" "e")] 12))
      (clobber (reg 3)) ;;; ay1
	])]
   ""
   "  ar = %1+%2, ay1=%5;\;  %3 = %4+ay1+C, %0=ar;"
   [(set_attr "type" "misc")])

 
;;USE insn is needed to prevent the compiler from generating
;;incorrect REG_UNUSED note for op0_high which results in
;;wrong code for "(long int) v1 - (long int) v2 - (long int)v3"
;;This a temporary fix, the real problem is in flow.
(define_expand "subdi3"
  [(set (match_operand:DI 0 "register_operand" "")
	(minus:DI (match_operand:DI 1 "register_operand" "")
	          (match_operand:DI 2 "register_operand" "")))]
  "INLINE_DOUBLE"
  "
   { 
       rtx
	   op0_high = gen_reg_rtx(SImode),
	   op0_low = gen_reg_rtx(SImode),
	   op1_high = force_reg(SImode, 
			operand_subword(operands[1], 0, 1, DImode)),
	   op1_low  = force_reg(SImode,
			    operand_subword(operands[1], 1, 1, DImode)),
	   op2_high = force_reg(SImode,
			    operand_subword(operands[2], 0, 1, DImode)),
	   op2_low  = force_reg(SImode, 
			    operand_subword(operands[2], 1, 1, DImode));
       if (op0_high == 0 || op0_low == 0 ||
           op1_high == 0 || op1_low == 0 ||
           op2_high == 0 || op2_low == 0)
         FAIL;
       else {
	 rtx
	    low_sub = gen_rtx(SET, SImode, op0_low,
			       gen_rtx(MINUS, SImode, op1_low, op2_low)),
	    high_sub = gen_rtx(SET, SImode, op0_high,
			gen_rtx(UNSPEC, SImode, 
				gen_rtvec(2, op1_high, op2_high), 15)),
            clobb = gen_rtx(CLOBBER, SImode, gen_rtx(REG, SImode, 3));
         emit_insn(gen_rtx (PARALLEL, VOIDmode,
           		     gen_rtvec(3, low_sub, high_sub, clobb)));
	 emit_insn(gen_rtx (SET, SImode,
			    operand_subword(operands[0], 0, 1, DImode),
			  	op0_high));
	 emit_insn(gen_rtx
		     (SET, SImode, gen_rtx(STRICT_LOW_PART, VOIDmode,
			   operand_subword(operands[0], 1, 1, DImode)),
				op0_low));
	emit_insn(gen_rtx(USE, SImode, op0_high));
	 DONE;
     }
   }
 "
)

;; In the following insn AR is implicitly clobbered, but we do not
;; need a special "clobber" for it because of "=&c" for operand 3.

(define_insn ""
  [
   (parallel
     [(set (match_operand:SI 0 "single_reg" "=e")
   	(minus:SI (match_operand:SI 1 "single_reg" "a")
	         (match_operand:SI 2 "single_reg" "A")))
      (set (match_operand:SI 3 "single_reg" "=&c")
   	(unspec:SI [(match_operand:SI 4 "single_reg" "a")
	            (match_operand:SI 5 "single_reg" "e")] 15))
      (clobber (reg 3)) ;;; ay1
	])]
   ""
   "  ar = %1-%2, ay1=%5;\;  %3 = %4-ay1+C-1, %0=ar;"
   [(set_attr "type" "misc")])


; Note this pattern is also used by mulsi3.
; 7/25/95 -- lev
(define_insn "mulsidi3"
    [(set (match_operand:DI 0 "register_operand" "=C,C")
	  (mult:DI (match_operand:SI 1 "register_operand" "b,*e")
                   (match_operand:SI 2 "register_operand" "B,*e")))
   ]
  ""
  "@
   %m0=%1*%2 (ss);
   %m0=%1*%2 (ss);"
  [(set_attr "type" "mac,c0mac")]
  )

(define_insn ""
    [(set (match_operand:DI 0 "register_operand" "=&C,C")
	  (plus:DI (match_operand:DI 1 "register_operand" "0,0")
	       (mult:DI (match_operand:SI 2 "register_operand" "b,*e")
		    (match_operand:SI 3 "register_operand" "B,*e"))))
   ]
  ""
  "@
   %m0=%m0+%2*%3 (ss);
   %m0=%m0+%2*%3 (ss);"
  [(set_attr "type" "mac,c0mac")]
  )

(define_expand "macdisi3"
    [(set (match_operand:DI 0 "register_operand" "=C,C")
	  (plus:DI (match_operand:DI 1 "register_operand" "C,C")
	       (mult:DI (match_operand:SI 2 "register_operand" "b,*e")
		    (match_operand:SI 3 "register_operand" "B,*e"))))
   ]
  ""
  "{
     emit_macdisi3 (operands, PLUS);
     DONE;
   }"
  )
(define_insn ""
    [(set (match_operand:DI 0 "register_operand" "=&C,C")
	  (minus:DI (match_operand:DI 1 "register_operand" "0,0")
	       (mult:DI (match_operand:SI 2 "register_operand" "b,*e")
		    (match_operand:SI 3 "register_operand" "B,*e"))))
   ]
  ""
  "@
   %m0=%m0-%2*%3 (ss);
   %m0=%m0-%2*%3 (ss);"
  [(set_attr "type" "mac,c0mac")]
  )

(define_expand "smacdisi3"
    [(set (match_operand:DI 0 "register_operand" "=C,C")
	  (minus:DI (match_operand:DI 1 "register_operand" "&C,C")
	       (mult:DI (match_operand:SI 2 "register_operand" "b,*e")
		    (match_operand:SI 3 "register_operand" "B,*e"))))
   ]
  ""
  "{
     emit_macdisi3 (operands, MINUS);
     DONE;
   }"
  )

