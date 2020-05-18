/****Symbol Table Header File ****/
#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define TABLESIZE 2000
#define SCOPESIZE 500
#define RED_COLOR "\x1b[31m"
#define CYAN_COLOR "\x1b[36m"
#define COLOR_RESET "\x1b[0m"

typedef enum scopespace_t{
	programvar,functionlocal,formalarg
}scopespace_t;

typedef struct Variable {
	const char *name;
	unsigned int scope;
	unsigned int line;
} Variable;


typedef struct Function {
	const char *name;
	unsigned int scope;
	unsigned int line;
	} Function;



enum SymbolType {
	GLOBAL, LCL, FORMAL,
	USERFUNC, LIBFUNC
};

typedef struct returnList_t{
	unsigned value;
	struct returnList_t * next;
}returnList_t;


typedef struct SymbolTableEntry {
    bool isActive;
    union {
    	Variable *varVal;
    	Function *funcVal;
	} value;
	enum SymbolType type;
	int offset;
	int functionAddress;
	int totalLocals;
	scopespace_t scopeSpace;
	struct SymbolTableEntry * next;
	struct SymbolTableEntry * scopeNext;
	struct SymbolTableEntry * argNext;
	struct SymbolTableEntry * functionStackNext;

	unsigned int taddress;
	returnList_t * returnList;

} SymbolTableEntry;



unsigned int hashFunction(unsigned const char *str);

void SymTableInit();

void ScopeTableInit();

void SymTableInsert(SymbolTableEntry * entry);

void ScopeTableInsert(SymbolTableEntry *entry,unsigned int scope);

void SymTablePrint();

void ScopeTablePrint();

void LibFunctionsInit();

enum SymbolType calculateType(int scope);

const char* getSymbolTypeString(enum SymbolType type);

SymbolTableEntry * initSymTableEntry(const char *name, unsigned int scope, unsigned int line,enum SymbolType type);


void InsertArgInFunction(SymbolTableEntry * argEntry,SymbolTableEntry * globalFuncEntry);

SymbolTableEntry * lookup(const char* name);

SymbolTableEntry * scopeLookup(const char* name,int scope);

void Hide(int scope);

bool checkLibFuncCollisions(const char* name);

bool IsFunctionInBetween(int tempscope);


unsigned int getScope(SymbolTableEntry * entry);

SymbolTableEntry * ArgumentLookUp(const char* name,SymbolTableEntry * functionEntry);

const char* getEntryName(SymbolTableEntry * entry);

void backPatchReturnList(returnList_t *list,int label);

void insertReturn(SymbolTableEntry * entry);

#endif