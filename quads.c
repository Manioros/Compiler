

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "quads.h"



FILE *fp;
quad * quads= (quad*) 0;
unsigned total = 0;
unsigned currQuad = 0;
int tempcounter = 0;

FunLocalStackEntry * functionLocalsStackHead;	
LoopStackEntry * loopStackHead;		

labelList * labelListHead=NULL;			

void expand (void){
	assert(total==currQuad);
	quad * p =(quad*)malloc(NEW_SIZE);
	if(quads){
		memcpy(p,quads,CURR_SIZE);
		free(quads);
	}
	quads=p;
	total+=EXPAND_SIZE;
}


void emit(
		iopcode op,
		expr * arg1,
		expr * arg2,
		expr * result,
		unsigned label,
		unsigned line
	){

	if(currQuad == total)
		expand();

	quad * p = quads+currQuad++;
	p->op=op;
	p->arg1 = arg1;
	p->arg2 = arg2;
	p->result = result;
	p->label = label;
	p->line = line;
}

unsigned programVarOffset = 0;
unsigned functionLocalOffset = 0;
unsigned formalArgOffset = 0;
unsigned scopeSpaceCounter = 1;



scopespace_t currscopespace(void){
	if(scopeSpaceCounter==1)
		return programvar;
	else if(scopeSpaceCounter % 2 == 0)
		return formalarg;
	else 
		return functionlocal;
}


unsigned currscopeoffset(void){
	switch(currscopespace()){
		case programvar: return programVarOffset;
		case functionlocal: return functionLocalOffset;
		case formalarg: return formalArgOffset;
		default :assert(0);
	}
}


unsigned inccurrscopeoffset(void){
	switch(currscopespace()){
		case programvar: return ++programVarOffset; break;
		case functionlocal: return ++functionLocalOffset; break;
		case formalarg: return ++formalArgOffset; break;
		default :assert(0);
	}
}

void enterscopespace (void ){
	++scopeSpaceCounter;
}

void exitscopespace (void){
	assert(scopeSpaceCounter>1);
	--scopeSpaceCounter;
}


/*TEMPORARY VARIABLE FUNCTIONS*/
const char * newtempname(){
	char * buffer = malloc(1000);
    snprintf(buffer, 1000,"$t%d",tempcounter);
    tempcounter++;
    return buffer;
}

void resetTemp(){
	tempcounter=0;
}

SymbolTableEntry * newTemp(int scope,int yylineno){
	const char * name=newtempname();
	SymbolTableEntry * entry =scopeLookup(name,scope);
	if(entry==NULL){
		entry =initSymTableEntry(name,scope, yylineno,calculateType(scope));
		entry->scopeSpace = currscopespace();
		entry->offset=currscopeoffset();
	    SymTableInsert(entry);
	    ScopeTableInsert(entry,scope);
	    inccurrscopeoffset();
	}

	return entry;
}

expr * newexpr(expr_t type){
	expr * new_expr=(expr *)malloc(sizeof(expr));
	memset(new_expr,0,sizeof(expr));
	new_expr->type=type;
	new_expr->next=NULL;
	
	new_expr->trueList=NULL;
	new_expr->falseList=NULL;
	
	return new_expr;
}


expr * newLvalueExpr(SymbolTableEntry * entry){
	assert(entry);
	expr_t exprType;
	if(entry->type==GLOBAL || entry->type==LCL || entry->type==FORMAL ){
		exprType=var_e;
	}
	else if(entry->type==USERFUNC) {
		exprType=programfunc_e;
	}
	else if(entry->type==LIBFUNC){
		exprType=libraryfunc_e;
	}

	expr * newExpr =newexpr(exprType);
	newExpr->strConst = strdup(getEntryName(entry));
	newExpr->sym=entry;

	return newExpr;
}


expr * newIntExpr(int val){
	expr* tmp=newexpr(constint_e);
	tmp->intConst=val;
	return tmp;
}

expr * newRealExpr(double val){
	expr* tmp=newexpr(constreal_e);
	tmp->realConst=val;
	return tmp;
}

expr * newStringExpr(char * str){
	expr* tmp=newexpr(conststring_e);
	tmp->strConst=strdup(str);
	return tmp;
}

expr * newNilExpr(){
	expr* tmp=newexpr(nil_e);
	return tmp;
}

expr * newBoolExpr(bool val){
	expr* tmp=newexpr(constbool_e);
	tmp->boolConst=val;
	return tmp;
}


void getExprName(expr * entry){
	
	expr * tmp = entry;
	if(entry==NULL){
		//printf("%-20s"," ");
	}
	else if(tmp->type== conststring_e){
		fprintf(fp, "%-20s", tmp->strConst);
	}
	else if(tmp->type==constint_e){
		fprintf(fp, "%-20d", tmp->intConst);
	}
	else if(tmp->type==constreal_e){
		fprintf(fp,"%-20f", tmp->realConst);
	}
	else if(tmp->type==programfunc_e){
		fprintf(fp,"%-20s", tmp->sym->value.funcVal->name);
	}
	else if(tmp->type==libraryfunc_e){
		fprintf(fp, "%-20s", tmp->sym->value.funcVal->name);
	}
	else if(tmp->type==constbool_e){
			if(tmp->boolConst){
				fprintf(fp, "%-20s","true");
			}
			else{
				fprintf(fp, "%-20s","false");
			}
	}
	else if(tmp->type==arithexpr_e){
		fprintf(fp, "%-20s",getEntryName(tmp->sym));
	}
	else if(tmp->type==var_e){
		fprintf(fp, "%-20s", getEntryName(tmp->sym));
	}
	else if(tmp->type==boolexpr_e){
		fprintf(fp, "%-20s", tmp->strConst);
	}
	else if(tmp->type==assignexpr_e){
		fprintf(fp,"%-20s", getEntryName(tmp->sym));
	}	
	else if(tmp->type==tableitem_e){
		fprintf(fp, "%-20s", getEntryName(tmp->sym));
	}
	else if(tmp->type==newtable_e){
		fprintf(fp, "%-20s", getEntryName(tmp->sym));
	}
	else if(tmp->type==nil_e){
		fprintf(fp, "%-20s", "NIL");
	}
	else{
		tmp=NULL;
	}
}


void printQuads(){
	quad * tmp = quads;
	int i = 0;
	
	fp = fopen("quads.txt", "w");
	if(fp == NULL)
	{
		printf("ERROR Writing in quads.txt!\n");
		exit(-1);
	}
	fprintf(fp,"%-10s %-15s %-20s %-20s %-20s %-20s %s" ,"quad#","opcode","result","arg1","arg2","label","\n");
	fprintf(fp, "---------------------------------------------------------------------------------------------------------\n");
	if(quads == NULL)
	{
		return;
	}
	while(i < currQuad){
		fprintf(fp, "%d", i+1);
		if(i+1<10)
			fprintf(fp,"%-10s", ":");
		else if(i+1<100)
			fprintf(fp, "%-9s", ":");
		else 
			fprintf(fp, "%-8s", ":");
		switch((tmp+i)->op){
			case assign:
				fprintf(fp, "%-16s", "assign");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case add:
				fprintf(fp,"%-16s","add");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case sub:
			fprintf(fp,"%-16s", "sub");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case mul:
				fprintf(fp,"%-16s", "mul");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case division:
				fprintf(fp,"%-16s", "div");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case mod:
				fprintf(fp,"%-16s", "mod");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case uminus:
			fprintf(fp,"%-16s", "uminus");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case and:
				fprintf(fp,"%-16s", "and");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case or:
				fprintf(fp,"%-16s", "or");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case not:
				fprintf(fp,"%-16s", "not");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case if_eq:
				fprintf(fp,"%-16s", "if_eq");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				fprintf(fp, "%-26s"," ");
				fprintf(fp, "%-20d",((tmp+i)->label)+1);
				break;
			case if_noteq:
				fprintf(fp,"%-16s", "if_noteq");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				fprintf(fp, "%-26s"," ");
				fprintf(fp, "%-20d",((tmp+i)->label)+1);
				break;
			case if_lesseq:
				fprintf(fp, "%-16s","if_lesseq");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				fprintf(fp, "%-26s"," ");
				fprintf(fp, "%-20d",((tmp+i)->label)+1);
				break;
			case if_greatereq:
				fprintf(fp, "%-16s","if_greatereq");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				fprintf(fp, "%-26s"," ");
				fprintf(fp, "%-20d",((tmp+i)->label)+1);
				break;
			case if_less:
				fprintf(fp,"%-16s", "if_less");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				fprintf(fp, "%-26s"," ");
				fprintf(fp, "%-20d",((tmp+i)->label)+1);
				break;
			case if_greater:
				fprintf(fp, "%-16s","if_greater");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				fprintf(fp, "%-26s"," ");
				fprintf(fp, "%-20d",((tmp+i)->label)+1);
				break;
			case call:
				fprintf(fp,"%-16s","call");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case param:
				fprintf(fp,"%-16s","param");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case ret:
				fprintf(fp,"%-16s","ret");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case jump:                                                
				fprintf(fp,"%-15s %-16d","jump",((tmp+i)->label)+1);
				break;
			case getretval:
				fprintf(fp,"%-16s","getretval");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case funcstart:
				fprintf(fp,"%-16s","funcstart");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case funcend:
				fprintf(fp, "%-16s","funcend");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case tablecreate:
				fprintf(fp, "%-16s","tablecreate");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case tablegetelem:
				fprintf(fp,"%-16s", "tablegetelem");
				getExprName((tmp+i)->result);
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				break;
			case tablesetelem:
				fprintf(fp, "%-16s","tablesetelem");
				getExprName((tmp+i)->arg1);
				getExprName((tmp+i)->arg2);
				getExprName((tmp+i)->result);
				break;
		default:
			printf("nothing happened\n");
		}
		fprintf(fp, "\n");
		i++;
	}
	fprintf(fp, "---------------------------------------------------------------------------------------------------------\n");

}
void resetformalargsoffset(void){
    formalArgOffset=0;
}

void resetfunctionlocalsoffset(void){
    functionLocalOffset=0;
}

int getfunctionlocalsoffset(){
	return functionLocalOffset;
}

void restorecurrscopeoffset(unsigned n){
    switch(currscopespace()){
        case programvar    : programVarOffset=n;
        case functionlocal : functionLocalOffset=n;
        case formalarg     : formalArgOffset=n;
        default : assert(0);
    }
}

unsigned nextquadlabel (void){
    return currQuad;
}

void patchlabel(unsigned quadNo,unsigned label){
   assert(quadNo<currQuad);
    quads[quadNo].label=label;
    
}


void backPatch(labelList *list,int label){
    labelList *tmp=list;
    while(tmp!=NULL){
    	assert(tmp->label<=currQuad);
        quads[tmp->label].label=label;
        tmp=tmp->next;
    }
}


void backPatchReturnList(returnList_t *list,int label){
    returnList_t *tmp=list;
    while(tmp!=NULL){
    	assert(tmp->value<=currQuad);
        quads[tmp->value].label=label;
        tmp=tmp->next;
    }


}

void insertReturn(SymbolTableEntry * entry){
	returnList_t * newReturn=(returnList_t *)malloc(sizeof(returnList_t));
	newReturn->value=currQuad;
	newReturn->next=NULL;

	newReturn->next=entry->returnList;
	entry->returnList=newReturn;

}


expr * emit_iftableitem(expr* e,int scope,int line){
	if(e->type != tableitem_e)
		return e;
	else{
		expr * result =newexpr(var_e);
		result->sym=newTemp(scope,line);
		emit(tablegetelem,e,e->index,result,-1,currQuad);
		return result;
	}

}

expr * member_item(expr * lvalue,expr * name,int scope,int line){
	lvalue=emit_iftableitem(lvalue,scope,line);
	expr * item=newexpr(tableitem_e);
	item->sym=lvalue->sym;
	item->index=name;
	return item;
}



/* Function to reverse the linked list */
expr * reverse( expr* head) 
{ 
	if(head == NULL)
		return NULL;
	expr* prev=NULL; 
	expr* current=head; 
	expr* next=NULL; 
	while(current!=NULL){       
		next  = current->next;   
		current->next = prev;           
        prev = current; 
        current = next; 
    } 
    head = prev; 
    return head;
} 
   


expr * make_call(expr * lvalue,expr * elist,int scope,int line){
	expr * func=emit_iftableitem(lvalue,scope,line);
	expr * tmp=reverse(elist);
	
	while(tmp!=NULL && tmp->elistIsNull!=1){
		emit(param,tmp,NULL,NULL,-1,currQuad);
		tmp=tmp->next;
	}
	emit(call,func,NULL,NULL,-1,currQuad);
	expr * result=newexpr(var_e);
	result->sym=newTemp(scope,line);
	assert(result);
    emit(getretval,NULL,NULL,result,-1,currQuad);
	return result;


}

void checkuminus(expr * e){
	if(e->type==constbool_e      ||
		e->type == conststring_e ||
		e->type == nil_e         ||
		e->type == newtable_e    ||
		e->type == programfunc_e ||
		e->type == libraryfunc_e ||
		e->type == boolexpr_e )
		{
			printf("ERROR Illigal expr to unary -\n");
			exit(0);
		}
}


indexedStruct * newIndexedStruct(expr * x,expr * y){
	indexedStruct * newStruct=(indexedStruct *)malloc(sizeof(indexedStruct));
	newStruct->x=x;
	newStruct->y=y;
	newStruct->next=NULL;
	return newStruct;
}


unsigned int istempname(const char * s){
	return * s=='$';
}

unsigned int istempexpr(expr * e){
	return e->sym && e->sym->type==var_e && istempname(getEntryName(e->sym));
}


/*--------------------------------------------------------*/
void pushFunctionStack(SymbolTableEntry * entry){
	assert(entry);
	entry->functionStackNext=functionStackHead ;
	functionStackHead=entry;
	assert(functionStackHead);

}

SymbolTableEntry * popFunctionStack(){
	SymbolTableEntry * tmp=NULL;
	if(functionStackIsEmpty()==0){
		tmp=functionStackHead;
		functionStackHead=functionStackHead->functionStackNext;
		
	}
	else{
//		printf("Function Stack is Empty Returning NULL\n");
		}
	return tmp;	
}

int functionStackIsEmpty(){
	return functionStackHead==NULL;
}


/*stack implementation for FunLocalStackEntry structs  */
FunLocalStackEntry * initFunLocalStackEntry(){
	FunLocalStackEntry * entry= (FunLocalStackEntry *)malloc(sizeof(FunLocalStackEntry));
	entry->funLocalCounter=-1;
	entry->next=NULL;
}

void pushfunctionLocalsStack(int funLocalCounter){
	FunLocalStackEntry* entry = initFunLocalStackEntry();
	entry->funLocalCounter = funLocalCounter;
	entry->next = functionLocalsStackHead ;
	functionLocalsStackHead=entry;
}

int popfunctionLocalsStack(){
	FunLocalStackEntry * tmp=NULL;
	if(functionLocalsStackIsEmpty()!=1){
		tmp = functionLocalsStackHead;
		functionLocalsStackHead=functionLocalsStackHead->next;
	}
	functionLocalOffset = tmp->funLocalCounter;
	return functionLocalOffset;	
}

int functionLocalsStackIsEmpty(){
	return functionLocalsStackHead==NULL;
}

LoopStackEntry * initLoopStackEntry(){
	LoopStackEntry * entry= (LoopStackEntry *)malloc(sizeof(LoopStackEntry));
	entry->loopcounter=-1;
	entry->next=NULL;
}

void pushLoopStack(int loopcounter){
	LoopStackEntry* entry = initLoopStackEntry();
	entry->loopcounter = loopcounter;
	entry->next = loopStackHead ;
	loopStackHead=entry;
}

int  popLoopStack(){
	LoopStackEntry * tmp=NULL;
	if(LoopStackIsEmpty()!=1){
		tmp = loopStackHead;
		loopStackHead=loopStackHead->next;
	}
	int loopcounter = tmp->loopcounter;
	return loopcounter;	
}

int LoopStackIsEmpty(){
	return loopStackHead==NULL;
}

statementStruct * initStatementStruct(){
		statementStruct * mystruct= (statementStruct *)malloc(sizeof(statementStruct));
		mystruct->breakList=NULL;
		mystruct->contList=NULL;
		
		return mystruct;
}

labelList * mergeLists(labelList * list1,labelList * list2){
	if(list2==NULL)
		return list1;
	
	if(list1==NULL)
		return list2;

	while(list1->next!=NULL){
		list1=list1->next;
	}
	list1->next=list2;
	return list1;
}

labelList * newlist(int quadlabel){
	labelList * mylist= (labelList *)malloc(sizeof(labelList));
	mylist->label=quadlabel;
	mylist->next=NULL;
	return mylist;
}