#include "symtable.h"
#include "quads.h"
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>


SymbolTableEntry * array[TABLESIZE];

SymbolTableEntry * scopeArray[SCOPESIZE];




unsigned int hashFunction(unsigned const  char *str){

    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % TABLESIZE;
  
}


void SymTableInit(){
	int i;
	for(i=0;i<TABLESIZE;i++){
		array[TABLESIZE]=NULL;
	}
}

void ScopeTableInit(){
	int i;
	for(i=0;i<SCOPESIZE;i++){
		scopeArray[SCOPESIZE]=NULL;
	}
}

void SymTableInsert(SymbolTableEntry *entry){
	
	SymbolTableEntry * tmpNode;
	unsigned int hash;
	const char  * entryName;
	if(entry->type==GLOBAL || entry->type==LCL || entry->type==FORMAL){
		entryName=entry->value.varVal->name;
	}
	else if(entry->type==USERFUNC || entry->type==LIBFUNC){   
		entryName=entry->value.funcVal->name;
	}
	else{
		printf("something wrong in the SymTableInsert\n");
	}

	hash=hashFunction(entryName);

	if(array[hash]==NULL){
		array[hash]=entry;
	}
	else{
		tmpNode=array[hash];
		while(tmpNode->next!=NULL){
			tmpNode=tmpNode->next;
		}
		tmpNode->next=entry;
		entry->next=NULL;
	}
	
}
void ScopeTableInsert(SymbolTableEntry *entry,unsigned int scope){

	SymbolTableEntry * tmpNode;
	if(scopeArray[scope]==NULL){
		scopeArray[scope]=entry;
	}
	else{
		tmpNode=scopeArray[scope];
		while(tmpNode->scopeNext!=NULL){
			tmpNode=tmpNode->scopeNext;
		}
		tmpNode->scopeNext=entry;
		entry->scopeNext=NULL;
	}
	
}

void SymTablePrint(){
	printf("SYMBOL TABLE\n");
	SymbolTableEntry * tmpNode;
	int i;
	for(i=0;i<TABLESIZE;i++){
		if(array[i]!=NULL) {
			tmpNode=array[i];
			while(tmpNode!=NULL){
				printf("array[%d]: \"%s\"  [%s] (line %d) (scope %d)\n",i,
				tmpNode->value.varVal->name,
				getSymbolTypeString(tmpNode->type),
				tmpNode->value.varVal->line,
				tmpNode->value.varVal->scope);
				tmpNode=tmpNode->next;
			}
		}
	}
}

void ScopeTablePrint(){
	printf("SCOPE TABLE\n");
	SymbolTableEntry * tmpNode;
	int i;
	for(i=0;i<SCOPESIZE;i++){
		if(scopeArray[i]!=NULL) {
			printf("\n------------    SCOPE #%d    ------------\n",i);
			tmpNode=scopeArray[i];
			while(tmpNode!=NULL){
				printf("\"%s\"  [%s] (line %d) (scope %d)\n",
				tmpNode->value.varVal->name,
				getSymbolTypeString(tmpNode->type),
				tmpNode->value.varVal->line,
				tmpNode->value.varVal->scope);
				tmpNode=tmpNode->scopeNext;
			}
		}
	}
}


SymbolTableEntry * initSymTableEntry(const char *name, unsigned int scope, unsigned int line,enum SymbolType type){

		SymbolTableEntry *entry= (SymbolTableEntry * ) malloc(sizeof(SymbolTableEntry));
		assert(entry);
		entry->isActive=true;
		entry->type=type;
	    entry->next=NULL;
	    entry->scopeNext=NULL;
	    entry->argNext=NULL;
	    entry->functionStackNext=NULL;
	    entry->returnList=NULL;
	  
	    

	    if(type==GLOBAL || type==LCL || type==FORMAL){

		    entry->value.varVal=(Variable *) malloc(sizeof(Variable));
		    assert(entry->value.varVal);

		    entry->value.varVal->name=name;
			entry->value.varVal->scope=scope;
			entry->value.varVal->line=line;
		}
		else if(type==USERFUNC || type==LIBFUNC){

			entry->value.funcVal=(Function *) malloc(sizeof(Function));
			assert(entry->value.funcVal);

		    entry->value.funcVal->name=name;
			entry->value.funcVal->scope=scope;
			entry->value.funcVal->line=line;
		}
		else{
			printf(RED_COLOR "ERROR at initialising SymTableEntry\n" COLOR_RESET);
		}

		return entry;
}

bool checkLibFuncCollisions(const char* name){
	
	if( strcmp(name,"print")==0             ||
		strcmp(name,"input")==0             ||
		strcmp(name,"objectmemberkeys")==0  ||
		strcmp(name,"objecttotalmembers")==0||
		strcmp(name,"objectcopy")==0        ||
		strcmp(name,"totalarguments")==0    ||
		strcmp(name,"argument")==0          ||
		strcmp(name,"typeof")==0            ||
		strcmp(name,"strtonum")==0          ||
		strcmp(name,"sqrt")==0              ||
		strcmp(name,"cos")==0               ||
		strcmp(name,"sin")==0){
		return true;
	}
	return false;

}
void LibFunctionsInit(){

	SymbolTableEntry * entry =initSymTableEntry("print",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("input",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("objectmemberkeys",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("objecttotalmembers",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("objectcopy",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("totalarguments",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("argument",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("typeof",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("strtonum",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("sqrt",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("cos",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

	entry =initSymTableEntry("sin",0,0,LIBFUNC);
	SymTableInsert(entry);
	ScopeTableInsert(entry,0);

}


const char* getSymbolTypeString(enum SymbolType type) 
{
   switch (type) 
   {
      case 0: return "global variable";
      case 1: return "local variable";
      case 2: return "formal argument";
      case 3: return "user defined function";
      case 4: return "library function";
   }
}

enum SymbolType calculateType(int scope){
	if(scope==0) 
		return GLOBAL;
	else 
		return LCL;

}


void Hide(int scope){
	SymbolTableEntry * tmpNode;
	if(scopeArray[scope]!=NULL) {
		
		tmpNode=scopeArray[scope];
		
		while(tmpNode->scopeNext!=NULL){
			tmpNode->isActive=false;
			tmpNode=tmpNode->scopeNext;
		}
	}
}


void InsertArgInFunction(SymbolTableEntry * argEntry,SymbolTableEntry * globalFuncEntry){
	assert(globalFuncEntry);
	assert(argEntry);
	SymbolTableEntry * tmpEntry=globalFuncEntry;

	if(globalFuncEntry->argNext==NULL){
		globalFuncEntry->argNext=argEntry;
	}
	else{
		while(tmpEntry->argNext!=NULL){
			tmpEntry=tmpEntry->argNext;
		}
		tmpEntry->argNext=argEntry;

	}
}

SymbolTableEntry * lookup(const char* name){
	SymbolTableEntry * tmpNode;
	int hash = hashFunction(name);
	int flag = 0;
		if(array[hash]!=NULL) {

			tmpNode=array[hash];
			while(tmpNode != NULL){
				if(tmpNode->type == USERFUNC || tmpNode->type == LIBFUNC){
					if(strcmp(tmpNode->value.funcVal->name,name)==0){
						//return tmpNode;
						flag = 1;
						break;
					}
				}
				else if(tmpNode->type == GLOBAL || tmpNode->type == LCL || tmpNode->type == FORMAL){
					if(strcmp(tmpNode->value.varVal->name,name)==0){
						//return tmpNode;
						flag = 1;
						break;
					}
				}
				tmpNode=tmpNode->next;
			}
		}
		if(flag == 1){
			return tmpNode;
		}
		else 
		{
			return NULL;
		}
}

SymbolTableEntry * scopeLookup(const char* name,int scope){
	
	SymbolTableEntry * tmpNode;
	
	if(name==NULL){
		return NULL;
	}
	if(scopeArray[scope]!=NULL) {

		tmpNode=scopeArray[scope];
		while(tmpNode != NULL){

			if(tmpNode->type == USERFUNC || tmpNode->type == LIBFUNC){
				if(strcmp(tmpNode->value.funcVal->name,name)==0){
					return tmpNode;
				}
			}
			else if(tmpNode->type == GLOBAL || tmpNode->type == LCL || tmpNode->type == FORMAL){
				if(strcmp(tmpNode->value.varVal->name,name)==0){
					return tmpNode;
				}
			}
			tmpNode=tmpNode->scopeNext;
		}
	}
	return NULL;
		
}

bool IsFunctionInBetween(int scope){

	SymbolTableEntry * tmpNode;
	
	if(scopeArray[scope]!=NULL) {
		tmpNode=scopeArray[scope];
		while(tmpNode->scopeNext != NULL){
			tmpNode=tmpNode->scopeNext;
		}
		if(tmpNode->type == USERFUNC && tmpNode->isActive){
				return true; 
			}
	}
	return false;

}


unsigned int getScope(SymbolTableEntry * entry){

	if(entry->type == USERFUNC || entry->type == LIBFUNC)		
		return entry->value.funcVal->scope;
						
	else if(entry->type == GLOBAL || entry->type == LCL || entry->type == FORMAL)
		return entry->value.varVal->scope;			
}



SymbolTableEntry * ArgumentLookUp(const char* name,SymbolTableEntry * functionEntry){
	SymbolTableEntry * tmpNode=functionEntry;
	while(tmpNode->argNext != NULL){
			if(strcmp(tmpNode->argNext->value.varVal->name,name)==0){
				return tmpNode;
			}
			tmpNode=tmpNode->argNext;
		}
	return NULL;

}


const char* getEntryName(SymbolTableEntry * entry){

	if(entry->type==GLOBAL || entry->type==LCL || entry->type==FORMAL){
		return entry->value.varVal->name;
	}
	else if(entry->type==USERFUNC || entry->type==LIBFUNC){   
		return entry->value.funcVal->name;
	}
	else{
		
	}
}
