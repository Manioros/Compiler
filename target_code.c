#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "quads.h"
#include "symtable.h"
#include "target_code.h"

extern quad * quads;



incomplete_jump * ij_head = (incomplete_jump *) 0;
unsigned ij_total = 0 ;


instruction * instructions= (instruction*) 0;
unsigned currInstruction = 0;
unsigned total_instr = 0;
unsigned currprocessedquad = 0;

double * numConsts=(double *) 0;
unsigned totalNumConsts=0;
unsigned currNumConst=0;

char ** stringConsts=(char **) 0;
unsigned totalStringConsts=0;
unsigned currStringConst=0;

char** namedLibfuncs=(char **) 0;
unsigned totalnamedLibfuncs=0;
unsigned currNamedLibfunc=0;

userfunc * userFuncs=(userfunc *) 0;
unsigned totaluserFuncs=0;
unsigned currUserFunc=0;

SymbolTableEntry * funcStackHead;
extern unsigned programVarOffset;

int printStringCounter=0;
int printNumCounter=0;
int printFuncsCounter=0;

typedef void (*generator_func_t)(quad *);

generator_func_t generators[]={
	generate_ASSIGN,
	generate_ADD,
	generate_SUB,
	generate_MUL,
	generate_DIV,
	generate_MOD,
	generate_UMINUS,
	generate_AND,
	generate_OR,
	generate_NOT,
	generate_IF_EQ,
	generate_IF_NOTEQ,
	generate_LESSEQ,
	generate_IF_GREATEREQ,
	generate_LESS,
	generate_IF_GREATER,
	generate_CALL,
	generate_PARAM,
	generate_RETURN,
	generate_JUMP,
	generate_GETRETVAL,
	generate_FUNCSTART,
	generate_FUNCEND,
	generate_NEWTABLE,
	generate_TABLEGETELEM,
	generate_TABLESETELEM,
	generate_NOP	
	};


void instr_expand (void){
	assert(total_instr==currInstruction);
	instruction * p =(instruction*)malloc(INSTR_NEW_SIZE);
	if(instructions){
		memcpy(p,instructions,INSTR_CURR_SIZE);
		free(instructions);
	}
	instructions=p;
	total_instr+=EXPAND_SIZE;
}

void numConsts_expand (void){
	assert(totalNumConsts==currNumConst);
	double * p =(double *)malloc(NUMCONST_NEW_SIZE);
	if(numConsts){
		memcpy(p,numConsts,NUMCONST_CURR_SIZE);
		free(numConsts);
	}
	numConsts=p;
	totalNumConsts+=EXPAND_SIZE;
}

void stringConsts_expand (void){
	assert(totalStringConsts==currStringConst);
	char **  p =(char ** )malloc(STRCONST_NEW_SIZE);
	if(stringConsts){
		memcpy(p,stringConsts,STRCONST_CURR_SIZE);
		free(stringConsts);
	}
	stringConsts=p;
	totalStringConsts+=EXPAND_SIZE;
}

void namedLibfuncs_expand (void){
	assert(totalnamedLibfuncs==currNamedLibfunc);
	char **  p =(char ** )malloc(NAMEDLIBFUNC_NEW_SIZE);
	if(namedLibfuncs){
		memcpy(p,namedLibfuncs,NAMEDLIBFUNC_CURR_SIZE);
		free(namedLibfuncs);
	}
	namedLibfuncs=p;
	totalnamedLibfuncs+=EXPAND_SIZE;
}

void userFuncs_expand (void){
	assert(totaluserFuncs==currUserFunc);
	userfunc * p =(userfunc *)malloc(USERFUNC_NEW_SIZE);
	if(userFuncs){
		memcpy(p,userFuncs,USERFUNC_CURR_SIZE);
		free(userFuncs);
	}
	userFuncs=p;
	totaluserFuncs+=EXPAND_SIZE;
}


void instr_emit(instruction * t){
	
	if(currInstruction == total_instr)
		instr_expand();

	instructions[currInstruction]=*t;
	currInstruction++;
	
	/*p->opcode=op;
	p->arg1 = *arg1;
	p->arg2 = *arg2;
	t->result = *result;
	t->srcLine=currInstruction-1;*/
}

unsigned consts_newstring(char *s){
	if(currStringConst == totalStringConsts)
		stringConsts_expand();
	int i;
	for(i=0;i<currStringConst;i++){
		if(strcmp(stringConsts[i],s)==0){
			return i;
		}
	}
	stringConsts[currStringConst++]=strdup(s);
	return currStringConst-1; 
}


unsigned consts_newnumber(double n){
	if(currNumConst == totalNumConsts)
		numConsts_expand();
	numConsts[currNumConst++]=n;
	return currNumConst-1;
}

unsigned libfuncs_newused(const char *s){
	
	int i;
	for(i=0;i<currNamedLibfunc;i++){
		if(strcmp(namedLibfuncs[i],s)==0){
			return i;
		}
	}

	if(currNamedLibfunc == totalnamedLibfuncs)
		namedLibfuncs_expand();
	namedLibfuncs[currNamedLibfunc++]=strdup(s);
	return currNamedLibfunc-1;
}
unsigned userfuncs_newfunc(SymbolTableEntry * sym){

	int i;
	for(i=0;i<currUserFunc;i++){
		if(strcmp(userFuncs[i].id,getEntryName(sym))==0){
			return i;
		}
	}

	if(currUserFunc == totaluserFuncs)
		userFuncs_expand();
	userFuncs[currUserFunc].id=strdup(getEntryName(sym));
	userFuncs[currUserFunc].address=sym->taddress;
	userFuncs[currUserFunc].localSize=sym->totalLocals;
	currUserFunc++;
	return currUserFunc-1;
}

unsigned nextinstructionlabel (void){
    return currInstruction;
}

void reset_operand(vmarg * arg){
	//arg=(vmarg*) NULL; 
	arg->type=noaction_a;
	arg->val=-1;

}

void make_operand(expr * e,vmarg* arg){
	
	if(e==NULL){
		reset_operand(arg);
		return;
	}

	switch(e->type){

		/*All those below use a variable for storage*/
		case var_e:
		case tableitem_e:
		case arithexpr_e:
		case boolexpr_e:
		case assignexpr_e:
		case newtable_e: {
			
			assert(e->sym);
			arg->val=e->sym->offset;

			switch(e->sym->scopeSpace){
				case programvar: 	
					arg->type = global_a; 
					arg->name = getEntryName(e->sym);
					break;
				case functionlocal: 
					arg->type = local_a; 
					arg->name = getEntryName(e->sym);
					break;
				case formalarg: 	
					arg->type = formal_a; 
					arg->name = getEntryName(e->sym);
					break;

				default:assert(0);
			}			
			break; /* from case newtable_e */

		}

		/*Constants*/

		case constbool_e:{
			
			arg->val = e->boolConst;
			arg->type = bool_a;
			//arg->name = getEntryName(e->sym);
			break;
		}

		case conststring_e:{
			
			arg->val = consts_newstring(e->strConst);
			arg->type = string_a; 
			break;
		}
		case constreal_e:
		case constint_e:{
			
			make_numberoperand(arg,e->intConst);
			//arg->val = consts_newnumber(e->intConst);
			//arg->type = number_a; 
			break;
		}

		case nil_e: {
			
			arg->type = nil_a;
			break;
		}

		/*Functions*/

		case programfunc_e: {
			
			arg->type=userfunc_a;
			//arg->val=e->sym->taddress;//apeftheias dief8unsh
			arg->name = getEntryName(e->sym);
			//or index sto pinaka sunarthsewn
			arg->val=userfuncs_newfunc(e->sym);
			break;
		}
		case libraryfunc_e:{
			
			arg->type = libfunc_a;
			arg->val = libfuncs_newused(getEntryName(e->sym));
			arg->name = getEntryName(e->sym);
			break;
		}
		case  methodcall_e:{
			break;
		}
		case normcall_e:{
			break;
		}
		default:{
			assert(0);
		}

	}
}


void make_numberoperand(vmarg * arg,double val){
	arg->val = consts_newnumber(val);
	arg->type = number_a;
}

void make_booloperand(vmarg * arg,unsigned val){
	arg->val=val;
	arg->type=bool_a;
}

void make_retvaloperand(vmarg * arg){
	arg->type = retval_a;
}

incomplete_jump * init_ij_jump(){
	return (incomplete_jump *) malloc(sizeof(incomplete_jump));
}

void add_incomplete_jump(unsigned instrNo,unsigned iaddress){
	
	incomplete_jump * ij_node=init_ij_jump();
	assert(ij_node);
	ij_node->instrNo=instrNo;
	ij_node->iaddress=iaddress;

	ij_node->next=ij_head;
	ij_head=ij_node;

	++ij_total;

}

void patch_incomplete_jumps() {
	
	incomplete_jump * tmp=ij_head;
	int i;

	for(i=0; i < ij_total; i++){
		
		if(tmp->iaddress == nextquadlabel()){
			instructions[tmp->instrNo].result.val= currInstruction+1;
		}else{
			instructions[tmp->instrNo].result.val=(quads[tmp->iaddress].taddress)+1;
		}
		tmp=tmp->next;

	}
}

void generate(){

	currprocessedquad=0;
	unsigned i;

	for(i=0; i<nextquadlabel(); ++i){
		currprocessedquad++;
		(*generators[quads[i].op])(quads+i);
	}
	patch_incomplete_jumps();
}	

unsigned getcurrprocessedquad(){
	return currprocessedquad;
}

void generate_arithm(vmopcode op,quad * q){

	instruction t;
	t.opcode=op;

	make_operand(q->arg1,&t.arg1);

	make_operand(q->arg2,&t.arg2);

	make_operand(q->result,&t.result);

	q->taddress=nextinstructionlabel();
	instr_emit(&t);
}

void generate_ADD (quad * q){
 	generate_arithm(add_v,q);
 }

 void generate_SUB (quad * q){
 	generate_arithm(sub_v,q);
 }

 void generate_MUL (quad * q){
 	generate_arithm(mul_v,q);
 }

 void generate_DIV (quad * q){
 	generate_arithm(div_v,q);
 }

 void generate_MOD (quad * q){
 	generate_arithm(mod_v,q);
 }

 void generate_NEWTABLE (quad * q){
 	generate_arithm(newtable_v,q);
 }

 void generate_TABLEGETELEM (quad * q){
 	generate_arithm(tablegetelem_v,q);
 }

 void generate_TABLESETELEM (quad * q){
 	generate_arithm(tablesetelem_v,q);
 }

 void generate_ASSIGN (quad * q){
 	generate_arithm(assign_v,q);
 }

 void generate_NOP (quad * q){
 	instruction t;
 	t.opcode=nop_v;
 	instr_emit(&t);
 }

void generate_relational(vmopcode op,quad * q){
	instruction t;
	t.opcode=op;



	make_operand(q->arg1,&t.arg1);
	make_operand(q->arg2,&t.arg2);

	if(op==jump_v){
		reset_operand(&t.arg1);
		reset_operand(&t.arg2);
	}

	
	t.result.type=label_a;
	if(q->label<getcurrprocessedquad()){
		t.result.val=quads[q->label].taddress+1;
		
	}
	else{
		
		add_incomplete_jump(nextinstructionlabel(),q->label);
	}

	q->taddress=nextinstructionlabel();
	instr_emit(&t);
}


 void generate_JUMP (quad * q){
 	generate_relational(jump_v ,q);
 }

 void generate_IF_EQ (quad * q){
 	generate_relational(jeq_v,q);
 }

 void generate_IF_NOTEQ (quad * q){
 	generate_relational(jne_v,q);
 }

 void generate_IF_GREATER (quad * q){
 	generate_relational(jgt_v,q);
 }

 void generate_IF_GREATEREQ (quad * q){
 	generate_relational(jge_v,q);
 }

 void generate_LESS (quad * q){
 	generate_relational(jlt_v,q);
 }

 void generate_LESSEQ (quad * q){
 	generate_relational(jle_v,q);
 }

 void generate_NOT (quad * q){

 }

 void generate_AND(){

 }

 void generate_UMINUS(){

 }

 void generate_OR (quad * q){
 }

 void generate_PARAM (quad * q){
 	q->taddress = nextinstructionlabel();
	instruction t;
	t.opcode = pusharg_v;
	make_operand(q->arg1, &t.arg1);
	reset_operand(&t.result);
	reset_operand(&t.arg2);
	instr_emit(&t);
 }

 void generate_CALL (quad * q){
 	q->taddress = nextinstructionlabel();
	instruction t;
	t.opcode = call_v;
	make_operand(q->arg1, &t.arg1);
	reset_operand(&t.result);
	reset_operand(&t.arg2);
	instr_emit(&t);
 }

 void generate_GETRETVAL (quad * q){

 	q->taddress = nextinstructionlabel();
	instruction t;
	t.opcode = assign_v;
	
	make_operand(q->result, &t.result);
	make_retvaloperand(&t.arg1);
	reset_operand(&t.arg2);
	instr_emit(&t);
 }

 void generate_FUNCSTART (quad * q){
 	SymbolTableEntry * f;
 	f=q->result->sym;
 
 	f->taddress=nextinstructionlabel();
 	q->taddress=nextinstructionlabel();
 
 	userfuncs_newfunc(f); // add 
 	funcStackPush(f);
 	instruction t;
 	t.opcode=funcenter_v;
 	make_operand(q->result,&t.result);
 	reset_operand(&t.arg1);
 	reset_operand(&t.arg2);
 	instr_emit(&t);
 }


void funcStackPush(SymbolTableEntry * entry){
	assert(entry);
	entry->functionStackNext=funcStackHead ;
	funcStackHead=entry;
	assert(funcStackHead);
}

SymbolTableEntry * funcStackPop(){
	SymbolTableEntry * tmp=NULL;
	if(funcStackHead!=NULL){
		tmp=funcStackHead;
		funcStackHead=funcStackHead->functionStackNext;
	}
	else{
   		 printf("Function Stack is Empty Returning NULL\n");
   		 exit(-1);
		}
	return tmp;	
}

SymbolTableEntry * funcStackTop(){
	SymbolTableEntry * tmp=NULL;
	if(funcStackIsEmpty()==0){
		tmp=funcStackHead;
	}
	else{
		//printf("Function Stack is Empty Returning NULL\n");
		}
	return tmp;	
}

int funcStackIsEmpty(){
	return funcStackHead==NULL;
}
 void append(returnList_t * returnList, unsigned label){
 	
 }

 void generate_RETURN (quad * q){
 	SymbolTableEntry * f;
 	printf("q->taddress for quad no %d\n",getcurrprocessedquad() );
 	q->taddress=nextinstructionlabel();
 	instruction t;
 	t.opcode=assign_v;
 	make_retvaloperand(&t.result);

 	make_operand(q->arg1,&t.arg1);
 	instr_emit(&t);

 	f=funcStackTop();
 	
 	returnList_t * newReturnList = (returnList_t *) malloc(sizeof(returnList_t));
 	
 	newReturnList->value=nextinstructionlabel();
 	newReturnList->next=f->returnList;
 	f->returnList=newReturnList;


 }

 void generate_FUNCEND (quad * q){
 	printf("In generate FUNCEND \n");
 	SymbolTableEntry * f;
 	f=funcStackPop();
 	
 	patch_incomplete_jumps();
 	q->taddress=nextinstructionlabel();
 	instruction t;
 	t.opcode=funcexit_v;
 	reset_operand(&t.arg1);
 	reset_operand(&t.arg2);
 	make_operand(q->result,&(t.result));
 	instr_emit(&t);
 }

void printTargetCode(){
	
	int i ;
	instruction * tmpInstr;
	FILE * fp;
	fp = fopen("instructions.txt", "w");
	if(fp == NULL)
	{
		printf("ERROR Writing in instructions.txt!\n");
		exit(-1);
	}
	fprintf(fp,"%-10s %-15s %-49s %-35s %-20s %s" ,"instr#","opcode","result","arg1","arg2","\n");
	fprintf(fp, "-----------------------------------------------------------------------------------------------\n");
	if(instructions == NULL)
	{
		return;
	}
	for(i=0;i<currInstruction;i++){
		fprintf(fp, "%d", i+1);
		if(i+1<10)
			fprintf(fp,"%-10s", ":");
		else if(i+1<100)
			fprintf(fp, "%-9s", ":");
		else 
			fprintf(fp, "%-8s", ":");

		tmpInstr=&instructions[i];
		
		printf("OPCODE %d\n",instructions[i].opcode );

		printInstruction(tmpInstr->opcode,fp);
		printVMargm(&tmpInstr->result,fp);
		printVMargm(&tmpInstr->arg1,fp);
		printVMargm(&tmpInstr->arg2,fp);
		fprintf(fp,"\n");
	}
	fclose(fp);
}

void printInstruction(vmopcode opcode,FILE * fp){

	switch(opcode){
		
		case assign_v:
			fprintf(fp, "%-16s", "assign");
			break;
		case add_v:
			fprintf(fp,"%-16s","add");
			break;
		case sub_v:
			fprintf(fp,"%-16s", "sub");
			break;
		case mul_v:
			fprintf(fp,"%-16s", "mul");
			break;
		case div_v:
			fprintf(fp,"%-16s", "div");
			break;
		case mod_v:
			fprintf(fp,"%-16s", "mod");
			break;
		case uminus_v:
			fprintf(fp,"%-16s", "uminus");
			break;
		case and_v:
			fprintf(fp,"%-16s", "and");
			break;
		case or_v:
			fprintf(fp,"%-16s", "or");
			break;
		case not_v:
			fprintf(fp,"%-16s", "not");
			break;
		case jeq_v:
			fprintf(fp,"%-16s", "jeq");
			break;
		case jne_v:
			fprintf(fp,"%-16s", "jne");
			break;
		case jle_v:
			fprintf(fp, "%-16s","jle");
			break;
		case jge_v:
			fprintf(fp, "%-16s","jge");
			break;
		case jlt_v:
			fprintf(fp,"%-16s", "jlt");
			break;
		case jgt_v:
			fprintf(fp, "%-16s","jgt");
			break;
		case call_v:
			fprintf(fp,"%-16s","call");
			break;
		case pusharg_v:
			fprintf(fp,"%-16s","pusharg");
			break;
		case funcenter_v:
			fprintf(fp,"%-16s","funcenter");
			break;
		case funcexit_v:
			fprintf(fp,"%-16s","funcexit");
			break;
		case newtable_v:
			fprintf(fp,"%-16s","newtable");
			break;
		case tablegetelem_v:
			fprintf(fp,"%-16s","tablegetelem");
			break;
		case tablesetelem_v:
			fprintf(fp, "%-16s","tablesetelem");
			break;
		case nop_v:
			fprintf(fp, "%-16s","nop");
			break;
		case jump_v:
			fprintf(fp,"%-16s", "jump");
			break;
	}
}

void printVMargm(vmarg * arg,FILE * fp){
	switch(arg->type){
		
		case label_a:
			fprintf(fp, "00(label_a),%-30d ", arg->val);
			break;
		case global_a:
			fprintf(fp, "01(global_a),%d:%-30s", arg->val,arg->name);
			break;
		case formal_a:
			fprintf(fp, "02(formal_a),%d:%-30s", arg->val,arg->name);
			break;
		case local_a:
			fprintf(fp, "03(local_a),%d:%-30s", arg->val,arg->name);
			break;
		case number_a:
			fprintf(fp, "04(number_a),%d : \"%f%-30s ", arg->val,numConsts[printNumCounter++],"\"");
			break;
		case string_a:
			fprintf(fp, "05(string_a),%d : \"%s%-30s ", arg->val,stringConsts[printStringCounter++],"\"");
			break;
		case bool_a:
			//fprintf(fp, "06(bool_a),%-30d", arg->val);
			if(arg->val==0){
				fprintf(fp, "06(bool_a),FALSE %-30c",' ');
			}else{
				fprintf(fp, "06(bool_a),TRUE %-30c",' ');
			}
			break;
		case nil_a:
			fprintf(fp, "07(nil_a),%-30d", arg->val);
			break;
		case userfunc_a:
			fprintf(fp, "08(userfunc_a),%d : \"%s%-30s ", arg->val,arg->name,"\"");
			break;
		case libfunc_a:
			fprintf(fp, "09(libfunc_a),%-30d", arg->val);
			break;
		case retval_a:
			fprintf(fp, "10(retval_a),%-30d", arg->val);
			break;
		case noaction_a:
			fprintf(fp, "%-40c",' ');
			break;
		default:
			fprintf(fp, "%s", "");
	}
}


void makeBinary(){
	FILE * fp=fopen("binary.abc","wb");
	if(fp==NULL){
		printf("ERROR opening binary file\n");
	}
	unsigned magicNumber=340200501;
	int i,strSize;


	fwrite(&magicNumber,sizeof(unsigned),1,fp);
	
	//stringConsts
	fwrite(&currStringConst,sizeof(unsigned),1,fp);

	for(i=0;i<currStringConst;i++){
		strSize=strlen(stringConsts[i]);
		fwrite(&strSize,sizeof(int),1,fp);
		fwrite(stringConsts[i],sizeof(char),strSize,fp);
	}


	//NumConsts
	fwrite(&currNumConst,sizeof(unsigned),1,fp);
	for(i=0;i<currNumConst;i++){
		fwrite(&numConsts[i],sizeof(double),1,fp);
	}


	//userFuncs
	fwrite(&currUserFunc,sizeof(unsigned),1,fp);
	for(i=0;i<currUserFunc;i++){
		//address
		fwrite(&userFuncs[i].address,sizeof(unsigned),1,fp);
		//localSize
		fwrite(&userFuncs[i].localSize,sizeof(unsigned),1,fp);
		//id
		strSize=strlen(userFuncs[i].id);
		fwrite(&strSize,sizeof(int),1,fp);
		fwrite(userFuncs[i].id,sizeof(char),strSize,fp);
	}
	

	//libFuncs
	fwrite(&currNamedLibfunc,sizeof(unsigned),1,fp);
	for(i=0;i<currNamedLibfunc;i++){
		strSize=strlen(namedLibfuncs[i]);
		fwrite(&strSize,sizeof(int),1,fp);
		fwrite(namedLibfuncs[i],sizeof(char),strSize,fp);
	}

	fwrite(&programVarOffset,sizeof(unsigned),1,fp);

	instruction * instr;
	instr = instructions;
	fwrite(&currInstruction,sizeof(unsigned),1,fp);
	i = 0;
	while(i<currInstruction){
		//opcode
		fwrite(&instr->opcode,sizeof(int),1,fp);
		//result type and val

		fwrite(&instr->result.type,sizeof(int),1,fp);
		fwrite(&instr->result.val,sizeof(unsigned),1,fp);

		//arg1 type and val
		fwrite(&instr->arg1.type,sizeof(int),1,fp);
		fwrite(&instr->arg1.val,sizeof(unsigned),1,fp);

		//arg2 type and val
		fwrite(&instr->arg2.type,sizeof(int),1,fp);
		fwrite(&instr->arg2.val,sizeof(unsigned),1,fp);
		i++;
		instr=instructions+i;
	}


	fclose(fp);
}

