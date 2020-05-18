
#ifndef QUADS_H
#define QUADS_H
#include "symtable.h"

#define EXPAND_SIZE 1024
#define CURR_SIZE (total*sizeof(quad))
#define NEW_SIZE  (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)

SymbolTableEntry * functionStackHead;

typedef enum iopcode{

	assign,			add,			sub,
	mul,			division,		mod,
	uminus,			and,			or,		
	not,			if_eq,			if_noteq,
	if_lesseq,		if_greatereq,	if_less,
	if_greater,		call,			param,
	ret,			jump,           getretval,		
	funcstart,      funcend,		tablecreate,	
	tablegetelem,    tablesetelem	,nop
}iopcode;


typedef enum expr_t{
	var_e,
	tableitem_e,

	programfunc_e,
	libraryfunc_e,
	methodcall_e,
	normcall_e,

	arithexpr_e,
	boolexpr_e,
	assignexpr_e,
	newtable_e,
	
	constint_e,
	constreal_e,
	constbool_e,
	conststring_e,

	nil_e
}expr_t;


typedef struct labelList{
	int label;
	struct labelList * next;
}labelList;


typedef struct expr{
	expr_t type;
	SymbolTableEntry * sym;
	struct expr * index;
	double realConst;
	int	   intConst;
	char * strConst;
	unsigned char boolConst;
	struct expr * next;
	int elistIsNull;

	labelList * trueList;
	labelList * falseList;
}expr;



typedef struct quad{
    iopcode op;
    expr *	result;
	expr *	arg1;
	expr *	arg2;
	unsigned label;
	unsigned line;

	unsigned int taddress;
}quad;

typedef struct forprefix{
	int enter;
	int test;
}forprefix;

typedef struct indexedStruct{
	expr * x;
	expr * y;
	struct indexedStruct * next;
}indexedStruct;


typedef struct FunLocalStackEntry{
	int funLocalCounter;
	struct FunLocalStackEntry * next;
}FunLocalStackEntry;


typedef struct LoopStackEntry{
	int loopcounter;
	struct LoopStackEntry * next;
}LoopStackEntry;



typedef struct statementStruct{
	labelList * breakList;
	labelList * contList;
	
}statementStruct;



void expand (void);


void emit(iopcode op,expr * arg1,expr * arg2,expr * result,
		unsigned label,unsigned line);

scopespace_t currscopespace(void);

unsigned currscopeoffset(void);

unsigned inccurrscopeoffset(void);

void enterscopespace (void);

void exitscopespace (void);

const char* newTempName();

void resetTemp();

SymbolTableEntry * newTemp(int scope,int yylineno);

expr * newexpr(expr_t type);

expr * newLvalueExpr(SymbolTableEntry * entry);

expr * newIntExpr(int val);

expr * newRealExpr(double val);

expr * newStringExpr(char * str);

expr * newNilExpr();

expr * newBoolExpr(bool val);

void printQuads();

void resetformalargsoffset(void);

void resetfunctionlocalsoffset(void);

void restorecurrscopeoffset(unsigned n);

unsigned nextquadlabel (void);

void patchlabel(unsigned quadNo,unsigned label);

expr * emit_iftableitem(expr* e,int scope,int line);

expr * member_item(expr * lvalue,expr * name,int scope,int line);

expr * make_call(expr * lvalue,expr * elist,int scope,int line);

void checkuminus(expr * e);

indexedStruct * newIndexedStruct(expr * x,expr * y);

unsigned int istempname(const char * s);

unsigned int istempexpr(expr * e);

void pushFunctionStack(SymbolTableEntry * entry);

SymbolTableEntry * popFunctionStack();

int functionStackIsEmpty();

expr * reverse(expr* head);

FunLocalStackEntry * initFunLocalStackEntry();

void pushfunctionLocalsStack(int funLocalCounter);

int  popfunctionLocalsStack();

int functionLocalsStackIsEmpty();

int getfunctionlocalsoffset();

LoopStackEntry * initLoopStackEntry();

void pushLoopStack(int loopcounter);

int  popLoopStack();

int LoopStackIsEmpty();



statementStruct * initStatementStruct();

labelList * mergeLists(labelList * list1,labelList * list2);

labelList * newlist(int quadlabel);


void backPatch(labelList *list,int label);

#endif