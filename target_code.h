#ifndef TARGET_CODE_H
#define TARGET_CODE_H
#include "symtable.h"
#include "quads.h"

#define EXPAND_SIZE 1024
#define INSTR_CURR_SIZE (total_instr*sizeof(instruction))
#define INSTR_NEW_SIZE  (EXPAND_SIZE*sizeof(instruction)+INSTR_CURR_SIZE)

#define NUMCONST_CURR_SIZE (totalNumConsts*sizeof(double))
#define NUMCONST_NEW_SIZE  (EXPAND_SIZE*sizeof(double)+NUMCONST_CURR_SIZE)

#define STRCONST_CURR_SIZE (totalStringConsts*sizeof(char *))
#define STRCONST_NEW_SIZE  (EXPAND_SIZE*sizeof(char *)+STRCONST_CURR_SIZE)

#define NAMEDLIBFUNC_CURR_SIZE (totalnamedLibfuncs*sizeof(char*))
#define NAMEDLIBFUNC_NEW_SIZE  (EXPAND_SIZE*sizeof(char*)+NAMEDLIBFUNC_CURR_SIZE)

#define USERFUNC_CURR_SIZE (totaluserFuncs*sizeof(userfunc))
#define USERFUNC_NEW_SIZE  (EXPAND_SIZE*sizeof(userfunc)+USERFUNC_CURR_SIZE)


typedef enum vmopcode{
	assign_v, 	  	add_v, 			sub_v, 
	mul_v, 		  	div_v, 			mod_v, 
	uminus_v, 	  	and_v, 			or_v, 
	not_v, 		  	jeq_v, 			jne_v, 
	jle_v,  	  	jge_v,  		jlt_v, 
	jgt_v, 		  	call_v,  		pusharg_v, 
	funcenter_v,  	funcexit_v, 	newtable_v, 
	tablegetelem_v, tablesetelem_v, nop_v, jump_v
}vmopcode;



typedef enum vmarg_t{
	label_a = 0,
	global_a = 1,
	formal_a = 2,
	local_a = 3,
	number_a = 4,
	string_a = 5,
	bool_a = 6,
	nil_a = 7,
	userfunc_a = 8,
	libfunc_a = 9,
	retval_a = 10,
	noaction_a = 11

}vmarg_t;


typedef struct vmarg { 
	vmarg_t type; 
	unsigned val;
	const char * name; 
}vmarg;


typedef struct instruction {
	vmopcode opcode;
	vmarg result;
	vmarg arg1;
	vmarg arg2;
	unsigned srcLine;
}instruction;


typedef struct userfunc{
	 unsigned address;
	 unsigned localSize;
	 char * id;
}userfunc;


typedef struct incomplete_jump{
	unsigned instrNo;				//the jump instruction number
	unsigned iaddress;			    // the i-code jump-target address
	struct incomplete_jump * next;  //a trivial linked list

}incomplete_jump;


unsigned consts_newstring(char *s);
unsigned consts_newnumber(double n);
unsigned libfuncs_newused(const char *s);
unsigned userfuncs_newfunc(SymbolTableEntry * sym);


void instr_expand (void);
void instr_emit(instruction * t);
unsigned nextinstructionlabel (void);
unsigned getcurrprocessedquad();

void make_operand(expr * e,vmarg* arg);
void make_numberoperand(vmarg * arg,double val);
void make_booloperand(vmarg * arg,unsigned val);
void make_retvaloperand(vmarg * arg);

void add_incomplete_jump(unsigned instrNo,unsigned iaddress);
void patch_incomplete_jumps();
void generate();

 void generate_ADD (quad *);
 void generate_SUB (quad *);
 void generate_MUL (quad *);
 void generate_DIV (quad *);
 void generate_MOD (quad *);
 void generate_NEWTABLE (quad *);
 void generate_TABLEGETELEM (quad *);
 void generate_TABLESETELEM (quad *);
 void generate_ASSIGN (quad *);
 void generate_NOP (quad *);
 void generate_JUMP (quad *);
 void generate_IF_EQ (quad *);
 void generate_IF_NOTEQ (quad *);
 void generate_IF_GREATER (quad *);
 void generate_IF_GREATEREQ (quad *);
 void generate_LESS (quad *);
 void generate_LESSEQ (quad *);
 void generate_NOT (quad *);
 void generate_OR (quad *);
 void generate_PARAM (quad *);
 void generate_CALL (quad *);
 void generate_GETRETVAL (quad *);
 void generate_FUNCSTART (quad *);
 void generate_RETURN (quad *);
 void generate_FUNCEND (quad *);
 void generate_AND();
 void generate_UMINUS();



 void funcStackPush(SymbolTableEntry * entry);
 SymbolTableEntry * funcStackPop();
 SymbolTableEntry * funcStackTop();
 int funcStackIsEmpty();
 void append(returnList_t * returnList, unsigned label);


 void printTargetCode();
 void printInstruction(vmopcode opcode,FILE * fp);
 void printVMargm(vmarg * arg,FILE * fp);
 void makeBinary();

#endif