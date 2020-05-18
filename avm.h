#ifndef AVM_H
#define AVM_H
#include "target_code.h"
#include "quads.h"
#include "assert.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define AVM_MAX_INSTRUCTIONS (unsigned) jump_v
#define AVM_ENDING_PC codeSize
#define AVM_NUMACTUALS_OFFSET +4
#define AVM_SAVEDPC_OFFSET +3
#define AVM_SAVEDTOP_OFFSET +2
#define AVM_SAVEDTOPSP_OFFSET +1
#define AVM_STACKSIZE 5000
#define AVM_STACKENV_SIZE 4
#define AVM_HASH 211
 
#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

unsigned char executionFinished = 0;
unsigned pc = 0;
unsigned currLine = 0;
unsigned codeSize = 0;
instruction * code = (instruction*) 0;
unsigned totalActuals = 0;

double * AVMnumConsts=(double *) 0;
unsigned AVMtotalNumConsts=0;

char ** AVMstringConsts=(char **) 0;
unsigned AVMtotalStringConsts=0;

char** AVMnamedLibfuncs=(char **) 0;
unsigned AVMtotalnamedLibfuncs=0;

userfunc * AVMuserFuncs=(userfunc *) 0;
unsigned AVMtotaluserFuncs=0;


void printVals();

void printArrays();

void read_binary();

typedef enum avm_memcell_t{
    number_m,
    string_m,
    bool_m,
    table_m,
    userfunc_m,
    libfunc_m,
    nil_m,
    undef_m
}avm_memcell_t;





typedef struct avm_memcell{
    avm_memcell_t type;
    union{
        double numVal;
        char * strVal;
        unsigned char boolVal;
        struct avm_table * tableVal;
        unsigned funcVal;
        char * libfuncVal;
    }data;
    
}avm_memcell;


typedef struct avm_table_elem{
    avm_memcell key;
    avm_memcell val;
    struct avm_table_elem * next;
}avm_table_elem;

typedef struct avm_table {
    unsigned int ref;
    unsigned int total;
    avm_table_elem * strIndex[AVM_HASH];
    avm_table_elem * numIndex[AVM_HASH];
}avm_table;

/*-----------------------------------------------------------------------*/
extern void execute_assign (instruction*);
/*-----------------------------------------------*/
extern void execute_add (instruction*);
extern void execute_sub (instruction*);
extern void execute_mul (instruction*);
extern void execute_div (instruction*);
extern void execute_mod (instruction*);
extern void execute_uminus (instruction*);
/*-----------------------------------------------*/
extern void execute_and (instruction*);
extern void execute_or (instruction*);
extern void execute_not (instruction*);
/*-----------------------------------------------*/
extern void execute_jeq (instruction*);
extern void execute_jne (instruction*);
extern void execute_jle (instruction*);
extern void execute_jge (instruction*);
extern void execute_jlt (instruction*);
extern void execute_jgt (instruction*);
/*-----------------------------------------------*/
extern void execute_call (instruction*);
extern void execute_pusharg (instruction*);
extern void execute_funcenter (instruction*);
extern void execute_funcexit (instruction*);
/*-----------------------------------------------*/
extern void execute_newtable (instruction*);
extern void execute_tablegetelem (instruction*);
extern void execute_tablesetelem (instruction*);
/*-----------------------------------------------*/
extern void execute_nop (instruction*);
extern void execute_jump (instruction*);
 
/*-----------------------------------------------------------------------*/

 extern void memclear_string(avm_memcell* m);
 
 extern void memclear_table(avm_memcell* m);
 
 extern void avm_warning(char* format, ...);

 extern void avm_error(char* format, ...);

 extern void avm_assign(avm_memcell* lv, avm_memcell* rv);

 extern void avm_calllibfunc(char * funcName);
 extern void avm_callsaveenvironment(void);

 extern userfunc* avm_getfuncinfo (unsigned address);

 extern char * number_tostring(avm_memcell* m);
 extern char * string_tostring(avm_memcell* m);
 extern char * bool_tostring(avm_memcell* m);
 extern char * table_tostring(avm_memcell* );
 extern char * userfunc_tostring(avm_memcell* );
 extern char * libfunc_tostring(avm_memcell* );
 extern char * nil_tostring(avm_memcell* );
 extern char * undef_tostring(avm_memcell* );

 extern char * avm_tostring(avm_memcell*); /*Caller Frees.*/

typedef void (*execute_func_t) (instruction*);
typedef void (*memclear_func_t) (avm_memcell*);
typedef void (*library_func_t)(void);
typedef char * (*tostring_func_t)(avm_memcell*);
typedef double (*arithmetic_func_t)(double x,double y);
typedef unsigned char (*tobool_func_t)(avm_memcell*);

double add_impl (double x, double y );
double sub_impl (double x, double y );
double mul_impl (double x, double y );
double div_impl (double x, double y );
double mod_impl (double x, double y );

unsigned char number_tobool (avm_memcell * m );
unsigned char string_tobool (avm_memcell * m );
unsigned char bool_tobool (avm_memcell * m );
unsigned char table_tobool (avm_memcell * m );
unsigned char userfunc_tobool (avm_memcell * m );
unsigned char libfunc_tobool (avm_memcell * m );
unsigned char nil_tobool (avm_memcell * m );
unsigned char undef_tobool (avm_memcell * m );


execute_func_t executeFuncs[] = {
    execute_assign,
    execute_add,
    execute_sub,
    execute_mul,
    execute_div,
    execute_mod,
    execute_uminus,
    execute_and,
    execute_or,
    execute_not,
    execute_jeq,
    execute_jne,
    execute_jle,
    execute_jge,
    execute_jlt,
    execute_jgt,
    execute_call,
    execute_pusharg, 
    execute_funcenter, 
    execute_funcexit, 
    execute_newtable,
    execute_tablegetelem, 
    execute_tablesetelem, 
    execute_nop, 
    execute_jump
};

memclear_func_t memclearFuncs[] = {
    0, /*number*/
    memclear_string,
    0,/*bool*/
    memclear_table,
    0,/*userfunc*/
    0,/*libfunc*/
    0,/*nil*/
    0/*undef*/
};

tostring_func_t tostringFuncs[]={
	number_tostring,
	string_tostring,
	bool_tostring,
	table_tostring,
	userfunc_tostring,
	libfunc_tostring,
	nil_tostring,
	undef_tostring,
};

/*Dispatcher just for arithmetic functions. */
arithmetic_func_t arithmeticFuncs[] = {
	add_impl,
	sub_impl,
	mul_impl,
	div_impl,
	mod_impl
};

tobool_func_t toboolFuncs[]={
	number_tobool,
	string_tobool,
	bool_tobool,
	table_tobool,	
	userfunc_tobool,
	libfunc_tobool,
	libfunc_tobool,
	undef_tobool
};



 void execute_cycle(void);
 
 void avm_memcellclear(avm_memcell* m);
 
 void execute_assign(instruction* instr);

 void avm_assign(avm_memcell* lv, avm_memcell* rv);

 void execute_call(instruction * instr);

 void avm_dec_top (void);

 void avm_push_envvalue (unsigned val);

 void execute_funcenter (instruction* instr);

 unsigned avm_get_envvalue (unsigned i);

 void execute_funcexit (instruction* unused);

 library_func_t avm_getlibraryfunc (char* id); /* Typical hashing. */

 unsigned avm_totalactuals(void);

 avm_memcell * avm_getactual(unsigned i);

void libfunc_print(void);

/*	With the folowing every libray function  is manually
	added in the VM library function resolution map.
*/
void avm_registerlibfunc(char * id ,library_func_t addr );

void execute_pusharg(instruction * instr);

char * avm_tostring(avm_memcell * m);

void execute_arithmetic(instruction * instr);

unsigned char avm_tobool(avm_memcell * m);

void execute_jeq(instruction * instr);

void libfunc_typeof(void);

void execute_newtable(instruction * instr);

avm_memcell * avm_tablegetelem(avm_table * table,avm_memcell* index);

void avm_tablesetelem(avm_table * table,avm_memcell* index,avm_memcell* content);

void execute_tablegetelem(instruction * instr);

void execute_tablesetelem(instruction * instr);

void avm_initialize(void);

void libfunc_totalarguments(void);

void libfunc_argument(void);

void libfunc_input(void);

void avm_initstack();
avm_table* avm_tablenew();
double consts_getnumber (unsigned index);

char * consts_getstring (unsigned index);

char * libfuncs_getused (unsigned index);

void libfunc_sqrt(void);
void libfunc_cos(void);
void libfunc_sin(void);
void libfunc_strtonum(void);
void libfunc_objecttotalmembers(void);
#endif
