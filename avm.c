#include "avm.h"
#include <math.h>
char * typeStrings[]={
	"number",
	"string",
	"bool",
	"table",
	"userfunc",
	"libfunc",
	"nil",
	"undef",
};


avm_memcell stack[AVM_STACKSIZE];

avm_memcell ax , bx, cx;

avm_memcell retval;

unsigned top,topsp;

int N;

unsigned programVarOffset;


void avm_error(char* format, ...){
 	printf("%s\n",format );
}


userfunc* avm_getfuncinfo (unsigned address){
	return &AVMuserFuncs[address];
}

library_func_t avm_getlibraryfunc (char* id){

	if(strcmp(id,"print")==0){
		return libfunc_print; 
	}
	else if(strcmp(id,"typeof")==0){
		return libfunc_typeof;
	}
	else if(strcmp(id,"totalarguments")==0){
		return libfunc_totalarguments; 
	}
	else if(strcmp(id, "argument")==0){
		return libfunc_argument;
	}
	else if(strcmp(id, "sqrt")==0){
		return libfunc_sqrt;
	}
	else if(strcmp(id, "cos")==0){
		return libfunc_cos;
	}
	else if(strcmp(id, "sin")==0){
		return libfunc_sin;
	}
	else if(strcmp(id, "input")==0){
		return libfunc_input;
	}
	else if(strcmp(id, "strtonum")==0){
		return libfunc_strtonum;
	}
	else if(strcmp(id, "objecttotalmembers")==0){
		return libfunc_objecttotalmembers;
	}
	else{
		return NULL;
	}


}
void avm_tableElemsInit(avm_table * t){
	unsigned int i = 0;
	avm_table_elem ** num=t->numIndex;
	avm_table_elem ** str=t->strIndex;

	for(i = 0; i < AVM_HASH; ++i){
		num[i] = (avm_table_elem *) 0;
		str[i] = (avm_table_elem *) 0;
	}

}

avm_table* avm_tablenew(){
	avm_table * table = (avm_table*) malloc(sizeof(avm_table));

	memset(&(table), 0, sizeof(table));

	table->total = 0;
	table->ref = table->total ;
	avm_tableElemsInit(table);
	
	return table;
}

void avm_initstack(){
	int i;
	for(i=0;i<AVM_STACKSIZE;i++){
		memset(&(stack[i]), 0, sizeof(stack[i]));
		stack[i].type=undef_m;
	}
}

void avm_tableDel(avm_table* table){
	unsigned int i = 0;
	avm_table_elem *tmp=NULL;
	avm_table_elem *tmpToDel=NULL;
	avm_table_elem ** tableNum=table->numIndex;
	avm_table_elem ** tableStr=table->strIndex;

	for(i = 0; i < AVM_HASH; ++i){
		tableNum++;	
		tmp=*tableNum;
		
		while(tmp){
			tmpToDel=tmp;
			tmp=tmp->next;
			avm_memcellclear(&tmpToDel->key);
			avm_memcellclear(&tmpToDel->val);
			free(tmpToDel);
		}
		tableNum[i]=NULL;
	}

	for(i = 0; i < AVM_HASH; ++i){
		tableStr++;	
		tmp=*tableStr;
		
		while(tmp){
			tmpToDel=tmp;
			tmp=tmp->next;
			avm_memcellclear(&tmpToDel->key);
			avm_memcellclear(&tmpToDel->val);
			free(tmpToDel);
		}
		tableStr[i]=NULL;
	}

	free(table);
}


unsigned strHash(char *str){
	unsigned hash,i;
	i=0;
	while(str[i]!='\0'){
		hash=hash*AVM_HASH+str[i];
		i++;
	}
	hash=hash%AVM_HASH;

	return hash;
}

unsigned numHash(int num){
	return num % AVM_HASH;
}


unsigned hashFunc(avm_memcell * index){
	unsigned hash=-1;
	if(index->type==number_m)
		hash=numHash(index->data.numVal);
	else if(index->type==string_m)
		hash=strHash(index->data.strVal);
	else if(index->type==bool_m)
			hash= index->data.boolVal % AVM_HASH;
	else{
		avm_error("ERROR index not string or number type");
		executionFinished=1;
	}
	return hash;
	
}

void avm_registerlibfunc(char * id ,library_func_t addr ){
	library_func_t libVar;
	if(strcmp(id,"print")==0){
		libVar= libfunc_print; 
	}
	else if(strcmp(id,"totalarguments")==0){
		libVar= libfunc_totalarguments; 
	}else{
		libVar=NULL;
		
	}
	libVar=addr;
}

void execute_uminus(instruction * instr){return;}
void execute_and(instruction * instr){return;}
void execute_or(instruction * instr){return;}
void execute_not(instruction * instr){return;}
void execute_nop(instruction * instr){return;}
void execute_jump(instruction * instr){pc=instr->result.val - 1;}



char * number_tostring(avm_memcell* m ){
	char  str[50];
	sprintf(str,"%f",m->data.numVal);
	return strdup(str);

}

char * string_tostring(avm_memcell* m){
	return strdup(m->data.strVal);
}

char * bool_tostring(avm_memcell* m){
	char * truestr="true";
	char * falsestr="false";
	if(m->data.boolVal==0)
		return truestr;
	
	return falsestr;
}

char * table_tostring(avm_memcell* m){}


char * userfunc_tostring(avm_memcell* m){
		char * str = (char *) malloc(sizeof(char) * 50);
		char * addr = (char *) malloc(sizeof(char)* 50);
		sprintf(str, "user function: ");
		sprintf(addr, "%d", m->data.funcVal);
		strcat(str,addr);
		return str;
}


char * libfunc_tostring(avm_memcell* m){
	char * str = (char *) malloc(sizeof(char) * 50);
	char * funcname = strdup(m->data.libfuncVal);
	sprintf(str, "library function: ");
	strcat(str, funcname);
	return str;
}


void libfunc_input(void){
	char str[100];
  	int i;
	unsigned argumentsNumber=avm_totalactuals();
	char  buffer[500];
	if(argumentsNumber!=0){
		avm_error("ERROR function \"input\" takes no arguments");
		executionFinished=1;
		return;
	}
  	fgets(buffer,500,stdin);
	

	if(strcmp(buffer,"nil\n")==0){
		retval.type=nil_m;
	}
	else if(strcmp(buffer,"true\n")==0){
		retval.type=bool_m;
		retval.data.boolVal=1;
	}
	else if(strcmp(buffer,"false\n")==0){
		retval.type=bool_m;
		retval.data.boolVal=0;
	}
	else if(atof(buffer)){
		retval.type=number_m;
		retval.data.numVal=atof(buffer);
	}else{
		retval.type=string_m;
		
		if((buffer[0]!='\"' && buffer[strlen(buffer)-1]!='\"'))
		{	
			buffer[strlen(buffer)-2]='\0';
			char * tmp=strdup(buffer);
			sprintf(buffer,"\"%s\"",tmp);
		}
		retval.data.strVal=strdup(buffer);
	}
	
	

}

char * nil_tostring(avm_memcell* m){
	char * str = "nil";
	return str;
}

char * undef_tostring(avm_memcell* m){
	char * str="undefined";
	return str;
}

/*	Reverse translation for constants:
	getting constant value from index
*/

double consts_getnumber (unsigned index){
	return AVMnumConsts[index];
}

char * consts_getstring (unsigned index){
	return AVMstringConsts[index];
}

char * libfuncs_getused (unsigned index){
	return AVMnamedLibfuncs[index];
}


avm_memcell * avm_translate_operand(vmarg * arg,avm_memcell * reg){
	switch(arg->type){

		/*Variables*/
		case global_a:  return &stack[(AVM_STACKSIZE-1)-arg->val];
		case local_a:   return &stack[topsp-arg->val];
		case formal_a:  return &stack[topsp+AVM_STACKENV_SIZE + 1 + arg->val];

		case retval_a:  return &retval;

		case number_a : {
			reg->type = number_m;
			reg->data.numVal = consts_getnumber(arg->val);
			return reg;
		}

		case string_a : {
			
			reg->type = string_m;
			reg->data.strVal = strdup(consts_getstring(arg->val));
			return reg;
		}

		case bool_a : {
			reg->type = bool_m;
			reg->data.boolVal = arg->val;
			return reg;
		}

		case nil_a : reg->type = nil_m; return reg;


		case userfunc_a : {
			reg->type = userfunc_m;		
			reg->data.funcVal = arg->val; /* Address already stored */
			return reg;
		}

		case libfunc_a : {
			reg->type = libfunc_m;
			reg->data.libfuncVal = libfuncs_getused(arg->val); /* Address already stored */
			return reg;
		}
		case noaction_a:{
			reg->type=undef_m;
			return reg;
		}
		default:reg->type = undef_m; return reg; assert(0);
	}
}

void execute_cycle(void){
	if(executionFinished)
		return;
	else{
		if(pc==AVM_ENDING_PC){
			executionFinished=1;
			return;
		}
		else{

			assert(pc<AVM_ENDING_PC);
			instruction* instr = code+pc;
			assert(instr->opcode>=0 && instr->opcode<=AVM_MAX_INSTRUCTIONS);

			if(instr->srcLine)
				currLine=instr->srcLine;

			unsigned oldPC=pc;
			(*executeFuncs[instr->opcode]) (instr);
			if(pc==oldPC)
				++pc;
		}
	}
}
 
 void avm_memcellclear(avm_memcell* m){
	 if(m->type!=undef_m){
		 memclear_func_t f=memclearFuncs[m->type];
		 if(f){
			 (*f)(m);
			 
		 }
		 m->type=undef_m;
	 }
 }
 
 typedef void (*memclear_func_t) (avm_memcell*);
 
 extern void memclear_string(avm_memcell* m){
	assert(m->data.strVal);
	free(m->data.strVal);
 }
 

 void avm_tableincrefcounter( avm_table * table){
	 table->ref++;
 }

 void avm_tabledecrefcounter( avm_table * table){
	assert(table->ref>0);
	table->ref--;
	if(table->ref==0)
		avm_tableDel(table);
 }


 extern void memclear_table(avm_memcell* m){
		 assert(m->data.tableVal);
		 avm_tabledecrefcounter(m->data.tableVal);
 }

 extern void avm_warning(char* format, ...){
 	printf("%s\n",format );
 }

 void execute_assign(instruction* instr){

	avm_memcell* lv=avm_translate_operand(&instr->result, (avm_memcell*) 0);
	avm_memcell* rv=avm_translate_operand(&instr->arg1, &ax);

	assert(lv && ( &stack[AVM_STACKSIZE-1]>=lv && lv>&stack[top] || lv==&retval));
	assert(rv); //should do similar assertion tests here
	avm_assign(lv,rv);
 }

void avm_assign(avm_memcell* lv, avm_memcell* rv){

	if(lv==rv){	/*Same cells? destructive to assign!*/
		
		return;
	}
	if(lv->type==table_m &&	/*same tables? no need to assign*/
	   rv->type==table_m &&
	   lv->data.tableVal==rv->data.tableVal){
		return;
	 	
	}
	if(rv->type==undef_m){	/*From undefined r-value? warn!*/
		avm_warning("assigning from 'undef' content!");
		
	}
	avm_memcellclear(lv);	/*clear old cell contents*/
	
	memcpy(lv,rv,sizeof(avm_memcell));	/*in C++ dispatch instead*/
	
	/*Now take care of copied values or reference counting.*/

	if(lv->type==string_m){
		lv->data.strVal=strdup(rv->data.strVal);
	}
	else
	if(lv->type==table_m){
		avm_tableincrefcounter(lv->data.tableVal);
	}

}

void execute_call(instruction * instr){

	avm_memcell* func=avm_translate_operand(&instr->arg1, &ax);
	assert(func);
	avm_callsaveenvironment();
	
	//printStack();

	switch(func->type){
		case userfunc_m: {
			pc=(AVMuserFuncs[func->data.funcVal].address);
			assert(pc<AVM_ENDING_PC);
			assert(code[pc].opcode==funcenter_v);
			break;
		}
		case string_m: avm_calllibfunc(func->data.strVal); break;
		case libfunc_m: {
			avm_calllibfunc(func->data.libfuncVal); 
			break;
		}
		case undef_m : break;
		default: {
			char * s = avm_tostring(func);
			avm_error("call: cannot bind '%s' to function!", s);
			free(s);
			executionFinished=1;
		}
	}
}

void avm_dec_top(void){
	if (!top){ /*Stack overflow*/
		avm_error("stack overflow");
		executionFinished = 1;
	}
	else
		--top;
}

void avm_push_envvalue (unsigned val){
	stack[top].type = number_m;
	stack[top].data.numVal = val;
	avm_dec_top();
}

void avm_callsaveenvironment (void){
	avm_push_envvalue(totalActuals);
	avm_push_envvalue(pc+1);
	avm_push_envvalue(top + totalActuals + 2);
	avm_push_envvalue(topsp);
}

void execute_funcenter (instruction* instr){
	avm_memcell* func = avm_translate_operand(&instr->result, &ax);
	assert(func);
	assert(pc == AVMuserFuncs[func->data.funcVal].address); /*Func address should match PC. */
	/* callee actions here. */
	totalActuals = 0;
	userfunc* funcInfo = avm_getfuncinfo (func->data.funcVal);

	topsp = top;
	top = top - funcInfo->localSize;
}

unsigned avm_get_envvalue (unsigned i) {
	assert(stack[i].type == number_m);
	unsigned val = (unsigned) stack[i].data.numVal;
	assert(stack[i].data.numVal == ((double) val));
	return val;
}

void execute_funcexit (instruction* unused){
	unsigned oldTop = top;
	top = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
	pc = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
	topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);

	while (++oldTop <= top) /* Intentionally ignoring first. */
		avm_memcellclear(&stack[oldTop]);
}
void avm_calllibfunc (char* id){
	library_func_t f  = avm_getlibraryfunc(id);

	if (!f) {
		avm_error("unsupported lib func '%s' called!", id);
		executionFinished = 1;
	}
	else {
		
		/* Notice that enter function and exit function
			are called manually!
		*/
		topsp = top; /* Enter function sequence. No stack locals. */
		totalActuals = 0; 
		(*f)();
		if (!executionFinished) /* Call libraty function. */
			execute_funcexit((instruction*) 0); /* Return sequnce.*/
	}
}

unsigned avm_totalactuals(void){
	return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}

avm_memcell * avm_getactual(unsigned i){
	assert(i<avm_totalactuals());
	return &stack[topsp + AVM_STACKENV_SIZE + 1 + i];
}

/*
	Implementation of the library function 'print'.
	It displays every argument at the console
*/
void libfunc_print(void){
	unsigned n = avm_totalactuals();
	unsigned i;
	for( i = 0; i<n; ++i){
		char * s =avm_tostring(avm_getactual(i));
		puts(s);
		free(s);
	}
}


void libfunc_argument(void){
	unsigned totalargs;
	unsigned prev_topsp;
	unsigned n;
	avm_memcell * m;
	totalargs=avm_totalactuals();

	if(totalargs>1){
		avm_error("more than one arguments");
		executionFinished=1;
		return;
	}

	if(topsp==0){
		avm_error("called \"argument\" function outside of a fuction\n");
		executionFinished=1;
		return;
	}

	prev_topsp=avm_get_envvalue(topsp+AVM_SAVEDTOP_OFFSET);
	m=avm_getactual(0);
	n=atoi(avm_tostring(m));
	if(n>avm_get_envvalue(prev_topsp+2*AVM_NUMACTUALS_OFFSET)){
		avm_error("argument position error\n");
		retval.type = nil_m;
		executionFinished = 1;
	}else{
		retval=stack[prev_topsp+2*AVM_NUMACTUALS_OFFSET+n-1];
	}
}



void execute_pusharg(instruction * instr){
	avm_memcell * arg = avm_translate_operand(&instr->arg1, &ax);
	assert(arg);
	/*This is actually stack[top] = arg ,but we have to use avm_assign.*/
	avm_assign(&stack[top],arg);
	++totalActuals;
	avm_dec_top();
}

char * avm_tostring(avm_memcell * m){
	assert(m->type >= 0 && m->type<=undef_m);
	return (*tostringFuncs[m->type])(m);
}

double add_impl (double x, double y ){ return x+y;}
double sub_impl (double x, double y ){ return x-y;}
double mul_impl (double x, double y ){ return x*y;}
double div_impl (double x, double y ){ return x/y;}
double mod_impl (double x, double y ){ 
	return  ((unsigned) x)  % ((unsigned) y) ;
}


void execute_arithmetic(instruction * instr){
	avm_memcell * lv = avm_translate_operand(&instr->result,(avm_memcell*) 0 );
	avm_memcell * rv1 = avm_translate_operand(&instr->arg1, &ax );
	avm_memcell * rv2 = avm_translate_operand(&instr->arg2, &bx );


	assert(&stack[AVM_STACKSIZE - 1] >= lv);

	assert(&stack[top] < lv);
	assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[top] || lv==&retval));
	assert(rv1 && rv2);
	if(rv1->type !=number_m || rv2->type != number_m){
		avm_error("not a number in arithmetic");
		executionFinished = 1;
	}
	else{
		arithmetic_func_t op = arithmeticFuncs[instr->opcode -add_v];
		avm_memcellclear(lv);
		lv->type = number_m;
		lv->data.numVal = (*op)(rv1->data.numVal,rv2->data.numVal);
	}
}

unsigned char number_tobool (avm_memcell * m ){ return m->data.numVal != 0;}
unsigned char string_tobool (avm_memcell * m ){ return m->data.strVal != 0;}
unsigned char bool_tobool (avm_memcell * m ){ return m->data.boolVal != 0;}
unsigned char table_tobool (avm_memcell * m ){ return 1;}
unsigned char userfunc_tobool (avm_memcell * m ){ return 1;}
unsigned char libfunc_tobool (avm_memcell * m ){ return 1;}
unsigned char nil_tobool (avm_memcell * m ){ return 0;}
unsigned char undef_tobool (avm_memcell * m ){ assert(0); return 0;}

unsigned char avm_tobool(avm_memcell * m){
	assert(m->type >= 0 && m->type < undef_m);
	return (*toboolFuncs[m->type]) (m);
}

void execute_jeq(instruction * instr){
	assert(instr->result.type == label_a);
	avm_memcell * rv1 =avm_translate_operand(&instr->arg1,&ax);
	avm_memcell * rv2 =avm_translate_operand(&instr->arg2,&bx);
	unsigned char result = 0 ;

	if(rv1->type ==undef_m || rv2->type == undef_m){
		avm_error("'undef' involved in equality!");
		executionFinished=1;
	}
	
	else
	if(rv1->type ==nil_m || rv2->type == nil_m)
		result=rv1->type==nil_m && rv2->type==nil_m;

	else
	if(rv1->type ==bool_m || rv2->type == bool_m)
		result=(avm_tobool(rv1)== avm_tobool(rv2));

	else
	if(rv1->type != rv2->type){
		avm_error("%s == %s is illigal!",typeStrings[rv1->type],
			typeStrings[rv2->type]);
		executionFinished=1;
	}
	else{
		switch(rv1->type){
			case number_m:
				if(rv1->data.numVal == rv2->data.numVal)
					result = 1;
				else
					result = 0;
				break;
			case string_m:
				if(strcmp(rv1->data.strVal,rv2->data.strVal)==0)
					result = 1;
				else
					result = 0;
				break;
			case table_m:
				if(rv1->data.tableVal == rv2->data.tableVal)
					result = 1;
				else
					result = 0;
				break;
			case userfunc_m:
				if(rv1->data.funcVal == rv2->data.funcVal)
					result = 1;
				else
					result = 0;
				break;
			default:
				if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) == 0)
					result = 1;
				else
					result = 0;
		}
	}

	if(!executionFinished && result)
		pc = instr->result.val - 1;
}

void execute_jne(instruction * instr){
assert(instr->result.type == label_a);
	avm_memcell * rv1 =avm_translate_operand(&instr->arg1,&ax);
	avm_memcell * rv2 =avm_translate_operand(&instr->arg2,&bx);
	unsigned char result = 0 ;

	if(rv1->type ==undef_m || rv2->type == undef_m){
		avm_error("'undef' involved in equality!");
		executionFinished=1;
	}
	
	else
	if(rv1->type ==nil_m || rv2->type == nil_m)
		result=rv1->type==nil_m && rv2->type==nil_m;

	else
	if(rv1->type ==bool_m || rv2->type == bool_m)
		result=(avm_tobool(rv1)== avm_tobool(rv2));

	else
	if(rv1->type != rv2->type){
		avm_error("%s == %s is illigal!",typeStrings[rv1->type],
			typeStrings[rv2->type]);
		executionFinished=1;
	}
	else{
		switch(rv1->type){
			case number_m:
				if(rv1->data.numVal != rv2->data.numVal)
					result = 1;
				else
					result = 0;
				break;
			case string_m:
				if(strcmp(rv1->data.strVal,rv2->data.strVal) != 0)
					result = 1;
				else
					result = 0;
				break;
			case table_m:
				if(rv1->data.tableVal != rv2->data.tableVal)
					result = 1;
				else
					result = 0;
				break;
			case userfunc_m:
				if(rv1->data.funcVal != rv2->data.funcVal)
					result = 1;
				else
					result = 0;
				break;
			default:
			if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) != 0)
					result = 1;
				else
					result = 0;
		}
	}

	if(!executionFinished && result)
		pc = instr->result.val - 1;

}
void execute_jle(instruction * instr){
	assert(instr->result.type == label_a);
	avm_memcell * rv1 =avm_translate_operand(&instr->arg1,&ax);
	avm_memcell * rv2 =avm_translate_operand(&instr->arg2,&bx);
	unsigned char result = 0 ;

	if(rv1->type ==undef_m || rv2->type == undef_m){
		avm_error("'undef' involved in equality!");
		executionFinished=1;
	}
	
	else
	if(rv1->type ==nil_m || rv2->type == nil_m)
		result=rv1->type==nil_m && rv2->type==nil_m;

	else
	if(rv1->type ==bool_m || rv2->type == bool_m)
		result=(avm_tobool(rv1)== avm_tobool(rv2));

	else
	if(rv1->type != rv2->type){
		avm_error("%s == %s is illigal!",typeStrings[rv1->type],
			typeStrings[rv2->type]);
		executionFinished=1;
	}
	else{
		switch(rv1->type){
			case number_m:
				if(rv1->data.numVal <= rv2->data.numVal)
					result = 1;
				else
					result = 0;
				break;
			case string_m:
				if(strcmp(rv1->data.strVal, rv2->data.strVal) <= 0)
					result = 1;
				else
					result = 0;
				break;
			case table_m:
				if(rv1->data.tableVal <= rv2->data.tableVal)
					result = 1;
				else
					result = 0;
				break;
			case userfunc_m:
				if(rv1->data.funcVal <= rv2->data.funcVal)
					result = 1;
				else
					result = 0;
				break;
			default:
				if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) <= 0)
					result = 1;
				else
					result = 0;
		}
	}

	if(!executionFinished && result)
		pc = instr->result.val - 1;
}

void execute_jge(instruction * instr){
	assert(instr->result.type == label_a);
	avm_memcell * rv1 =avm_translate_operand(&instr->arg1,&ax);
	avm_memcell * rv2 =avm_translate_operand(&instr->arg2,&bx);
	unsigned char result = 0 ;

	if(rv1->type ==undef_m || rv2->type == undef_m){
		avm_error("'undef' involved in equality!");
		executionFinished=1;
	}
	
	else
	if(rv1->type ==nil_m || rv2->type == nil_m)
		result=rv1->type==nil_m && rv2->type==nil_m;

	else
	if(rv1->type ==bool_m || rv2->type == bool_m)
		result=(avm_tobool(rv1)== avm_tobool(rv2));

	else
	if(rv1->type != rv2->type){
		avm_error("%s == %s is illigal!",typeStrings[rv1->type],
			typeStrings[rv2->type]);
		executionFinished=1;
	}
	else{
		switch(rv1->type){
			case number_m:
				if(rv1->data.numVal >= rv2->data.numVal)
					result = 1;
				else
					result = 0;
				break;
			case string_m:
				if(strcmp(rv1->data.strVal,rv2->data.strVal) >= 0)
					result = 1;
				else
					result = 0;
				break;
			case table_m:
				if(rv1->data.tableVal >= rv2->data.tableVal)
					result = 1;
				else
					result = 0;
				break;
			case userfunc_m:
				if(rv1->data.funcVal >= rv2->data.funcVal)
					result = 1;
				else
					result = 0;
				break;
			default:
				if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) >= 0)
					result = 1;
				else
					result = 0;
		}
	}

	if(!executionFinished && result)
		pc = instr->result.val - 1;
}

void execute_jlt(instruction * instr){
	assert(instr->result.type == label_a);
	avm_memcell * rv1 =avm_translate_operand(&instr->arg1,&ax);
	avm_memcell * rv2 =avm_translate_operand(&instr->arg2,&bx);
	unsigned char result = 0 ;

	if(rv1->type ==undef_m || rv2->type == undef_m){
		avm_error("'undef' involved in equality!");
		executionFinished=1;
	}
	
	else
	if(rv1->type ==nil_m || rv2->type == nil_m)
		result=rv1->type==nil_m && rv2->type==nil_m;

	else
	if(rv1->type ==bool_m || rv2->type == bool_m)
		result=(avm_tobool(rv1)== avm_tobool(rv2));

	else
	if(rv1->type != rv2->type){
		avm_error("%s == %s is illigal!",typeStrings[rv1->type],
			typeStrings[rv2->type]);
		executionFinished=1;
	}
	else{
		switch(rv1->type){
			case number_m:
				if(rv1->data.numVal < rv2->data.numVal)
					result = 1;
				else
					result = 0;
				break;
			case string_m:
				if(strcmp(rv1->data.strVal, rv2->data.strVal) < 0)
					result = 1;
				else
					result = 0;
				break;
			case table_m:
				if(rv1->data.tableVal < rv2->data.tableVal)
					result = 1;
				else
					result = 0;
				break;
			case userfunc_m:
				if(rv1->data.funcVal < rv2->data.funcVal)
					result = 1;
				else
					result = 0;
				break;
			default:
				if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) < 0)
					result = 1;
				else
					result = 0;
		}
	}

	if(!executionFinished && result)
		pc = instr->result.val - 1;
}

void execute_jgt(instruction * instr){
	assert(instr->result.type == label_a);
	avm_memcell * rv1 =avm_translate_operand(&instr->arg1,&ax);
	avm_memcell * rv2 =avm_translate_operand(&instr->arg2,&bx);
	unsigned char result = 0 ;

	if(rv1->type ==undef_m || rv2->type == undef_m){
		avm_error("'undef' involved in equality!");
		executionFinished=1;
	}
	
	else
	if(rv1->type ==nil_m || rv2->type == nil_m)
		result=rv1->type==nil_m && rv2->type==nil_m;

	else
	if(rv1->type ==bool_m || rv2->type == bool_m)
		result=(avm_tobool(rv1)== avm_tobool(rv2));

	else
	if(rv1->type != rv2->type){
		avm_error("%s == %s is illigal!",typeStrings[rv1->type],
			typeStrings[rv2->type]);
		executionFinished=1;
	}
	else{
		switch(rv1->type){
			case number_m:
				if(rv1->data.numVal > rv2->data.numVal)
					result = 1;
				else
					result = 0;
				break;
			case string_m:
				if(strcmp(rv1->data.strVal,rv2->data.strVal) > 0)
					result = 1;
				else
					result = 0;
				break;
			case table_m:
				if(rv1->data.tableVal > rv2->data.tableVal)
					result = 1;
				else
					result = 0;
				break;
			case userfunc_m:
				if(rv1->data.funcVal > rv2->data.funcVal)
					result = 1;
				else
					result = 0;
				break;
			default:
				
				if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) > 0)
					result = 1;
				else
					result = 0;
		}
	}

	if(!executionFinished && result)
		pc = instr->result.val - 1;
}


void libfunc_typeof(void){

	unsigned n = avm_totalactuals();
		if(n!=1)
			avm_error("one argument (not %d) expected in 'typeof'! ",n);
		else{

				/* Thats how a library function returns a result.
				It has to only set the 'retVal' register!
				*/
			avm_memcellclear(&retval);/*Dont forget to clean it up ! */
			retval.type=string_m;
			retval.data.strVal = strdup(typeStrings[avm_getactual(0)->type]);
		}
}

void execute_newtable(instruction * instr){
	avm_memcell * lv = avm_translate_operand(&instr->arg1,(avm_memcell*)0);

	assert(lv && (&stack[AVM_STACKSIZE-1]>=lv && lv>&stack[top] || lv==&retval));
	avm_memcellclear(lv);
		
	lv->type =table_m;
	lv->data.tableVal=avm_tablenew();
	avm_tableincrefcounter(lv->data.tableVal);
}


void execute_tablegetelem(instruction * instr){
	avm_memcell * lv = avm_translate_operand(&instr->result, (avm_memcell *) 0 );
	avm_memcell * t = avm_translate_operand(&instr->arg1, (avm_memcell *) 0  );
	avm_memcell * i = avm_translate_operand(&instr->arg2, &ax );

	assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv> &stack[top] || lv ==&retval));
	assert(t &&  &stack[AVM_STACKSIZE-1] >= t && t> &stack[top]);
	assert(i);

	avm_memcellclear(lv);
	lv->type = nil_m ; /*Default value.*/

	if(t->type != table_m){
		avm_error("illigal use of type %s as table! ",typeStrings[t->type]);
	}
	else{
		avm_memcell * content =avm_tablegetelem(t->data.tableVal,i);
		if(content)
			avm_assign(lv,content);
		else{
			char * ts = avm_tostring(t);
			char * is = avm_tostring(i);
			avm_warning("%s[%s] not found!",ts,is);
			free(ts);
			free(is);
		}
	}

}

void execute_tablesetelem(instruction * instr){
	avm_memcell * t = avm_translate_operand(&instr->result, (avm_memcell *) 0 );
	avm_memcell * i = avm_translate_operand(&instr->arg1, &ax );
	avm_memcell * c = avm_translate_operand(&instr->arg2, &bx );

	assert(t && &stack[AVM_STACKSIZE-1] >= t && t > &stack[top]);
	assert(i && c);
	
	if(t->type != table_m){
		avm_error("illegal use of type %s as table!", typeStrings[t->type]);
		executionFinished = 1;
	}
	else{
		avm_tablesetelem(t->data.tableVal, i, c);
	}
}

avm_memcell * avm_tablegetelem(avm_table * table,avm_memcell* index){
	assert(index);
	int hash;
	avm_table_elem * tmp;
	hash=hashFunc(index);
	if(hash==-1){
		avm_error("ERROR hash == -1 \n");
		executionFinished=1;
		return NULL;
	}

	if(index->type == number_m){
		while((tmp->key.data.numVal!=index->data.numVal) && tmp!=NULL)
			tmp=tmp->next;
		if(tmp!=NULL)
			return &tmp->val;
		else{
			avm_error("element not found");
			return NULL;
		}

	}
	else if(index->type == string_m){
		while((strcmp(tmp->key.data.strVal, index->data.strVal)!=0 && tmp != NULL ))
			tmp = tmp->next;
		if(tmp!=NULL)
			return &tmp->val;
		else{
			avm_error("element not found");
			return NULL;
		}

	}
	
	
}

avm_table_elem * newTableNumberElem(avm_table_elem* numElem, avm_memcell* index, avm_memcell* content){
	avm_table_elem * newElem=(avm_table_elem * ) malloc(sizeof(avm_table_elem));
	newElem->key.type=number_m;
	newElem->key.data.numVal=index->data.numVal;
	

	if(content->type==number_m){
		newElem->val.type=number_m;
		newElem->val.data.numVal=content->data.numVal;
	}
	else if(content->type==string_m){
		newElem->val.type=string_m;
		newElem->val.data.strVal=strdup(content->data.strVal);
	}
	else if(content->type==bool_m){
		newElem->val.type=bool_m;
		newElem->val.data.boolVal=content->data.boolVal;
	}
	else if(content->type==table_m){
		newElem->val.type=table_m;
		newElem->val.data.tableVal=content->data.tableVal;
		avm_tableincrefcounter(newElem->val.data.tableVal);
	}
	else if(content->type==userfunc_m){
			newElem->val.type=userfunc_m;
		newElem->val.data.funcVal=content->data.funcVal;
	}
	else if(content->type==libfunc_m){
		newElem->val.type=libfunc_m;
		newElem->val.data.libfuncVal=strdup(content->data.libfuncVal);
	}

	newElem->next=numElem;
    return newElem;
    
}

avm_table_elem * newTableStringElem(avm_table_elem* strElem, avm_memcell* index, avm_memcell* content){
	
	
	avm_table_elem * newElem=(avm_table_elem * ) malloc(sizeof(avm_table_elem));
	newElem->key.type=string_m;
	newElem->key.data.strVal=strdup(index->data.strVal);


	if(content->type==number_m){
		newElem->val.type=number_m;
		newElem->val.data.numVal=content->data.numVal;
	}
	else if(content->type==string_m){
		newElem->val.type=string_m;
		newElem->val.data.strVal=strdup(content->data.strVal);
	}
	else if(content->type==bool_m){
		newElem->val.type=bool_m;
		newElem->val.data.boolVal=content->data.boolVal;
	}
	else if(content->type==table_m){
		newElem->val.type=table_m;
		newElem->val.data.tableVal=content->data.tableVal;
		avm_tableincrefcounter(newElem->val.data.tableVal);
	}
	else if(content->type==userfunc_m){
			newElem->val.type=userfunc_m;
		newElem->val.data.funcVal=content->data.funcVal;
	}
	else if(content->type==libfunc_m){
		newElem->val.type=libfunc_m;
		newElem->val.data.libfuncVal=strdup(content->data.libfuncVal);
	}


	newElem->next=strElem;
    return newElem;
    
}

void avm_tablesetelem(avm_table * table,avm_memcell* index,avm_memcell* content){

	assert(index);
	int hash;
	hash=hashFunc(index);
	avm_table_elem * tmp;
	avm_table_elem * newElem;

	if(hash==-1 || content->type==nil_m){
		avm_error("hash==1 or content->type==nil_m " );
		executionFinished=1;
		return;
	}

	if(index->type==number_m){

		tmp=table->numIndex[hash];
		while((tmp->key.data.numVal!=index->data.numVal) && tmp!=NULL)
			tmp=tmp->next;

		if(tmp!=NULL){
			 if(content->type==number_m){
				 tmp->val.type=number_m;
				tmp->val.data.numVal=content->data.numVal;
			}
			else if(content->type==string_m){
				tmp->val.type=string_m;
				tmp->val.data.strVal=strdup(content->data.strVal);
			}
			else if(content->type==bool_m){
				tmp->val.type=bool_m;
				tmp->val.data.boolVal=content->data.boolVal;
			}
			else if(content->type==table_m){
				tmp->val.type=table_m;
				tmp->val.data.tableVal=content->data.tableVal;
				avm_tableincrefcounter(tmp->val.data.tableVal);
			}
			else if(content->type==userfunc_m){
				 tmp->val.type=userfunc_m;
			    tmp->val.data.funcVal=content->data.funcVal;
			}
			else if(content->type==libfunc_m){
				tmp->val.type=libfunc_m;
				tmp->val.data.libfuncVal=strdup(content->data.libfuncVal);
			}
		}
		else{
			newElem=newTableNumberElem(table->numIndex[hash],index,content);
			table->numIndex[hash]=newElem;
		}

	}



	else if(index->type==string_m){

		while((strcmp(tmp->key.data.strVal, index->data.strVal)!=0 && tmp != NULL ))
			tmp = tmp->next;
		if(tmp!=NULL){
			 if(content->type==number_m){
				 tmp->val.type=number_m;
				tmp->val.data.numVal=content->data.numVal;
			}
			else if(content->type==string_m){
				tmp->val.type=string_m;
				tmp->val.data.strVal=strdup(content->data.strVal);
			}
			else if(content->type==bool_m){
				tmp->val.type=bool_m;
				tmp->val.data.boolVal=content->data.boolVal;	
			}
			else if(content->type==table_m){
				tmp->val.type=table_m;
				tmp->val.data.tableVal=content->data.tableVal;
				avm_tableincrefcounter(tmp->val.data.tableVal);
			}
			else if(content->type==userfunc_m){
				 tmp->val.type=userfunc_m;
			    tmp->val.data.funcVal=content->data.funcVal;
			}
			else if(content->type==libfunc_m){
				tmp->val.type=libfunc_m;
				tmp->val.data.libfuncVal=strdup(content->data.libfuncVal);
			}
		}
		else{
			newElem=newTableStringElem(table->strIndex[hash],index,content);
			table->strIndex[hash]=newElem;
		}			
	}

	table->total++;

}

void libfunc_sqrt(void){

	avm_memcell * argument;
	if(avm_totalactuals()!=1){
		avm_error("wrong number of arguments");
		executionFinished=1;
		return;
	}
	argument=avm_getactual(0);
	if(argument->type!=number_m){
		avm_error("wrong argument type");
		executionFinished=1;
		return;
	}
	if(argument->data.numVal < 0){
		avm_error("argument less than zero");
		retval.type = nil_m;
		return;
	}
	avm_memcellclear(&retval);
	retval.type=number_m;
	retval.data.numVal=sqrt(argument->data.numVal);

}

void libfunc_sin(void){
	double temp;
	double d = 3.14159265/180.0;
	avm_memcell * argument;
	if(avm_totalactuals()!=1){
		avm_error("wrong number of arguments");
		executionFinished=1;
		return;
	}
	argument=avm_getactual(0);
	if(argument->type!=number_m){
		avm_error("wrong argument type");
		executionFinished=1;
		return;
	}
	avm_memcellclear(&retval);
	temp = fmod(argument->data.numVal, 360.0);
	retval.type=number_m;
	retval.data.numVal=sin(d * temp);

}

void libfunc_cos(void){
	double temp;
	double d = 3.14159265/180.0;
	avm_memcell * argument;
	if(avm_totalactuals()!=1){
		avm_error("wrong number of arguments");
		executionFinished=1;
		return;
	}
	argument=avm_getactual(0);
	if(argument->type!=number_m){
		avm_error("wrong argument type");
		executionFinished=1;
		return;
	}
	avm_memcellclear(&retval);
	temp = fmod(argument->data.numVal, 360.0);
	retval.type=number_m;
	retval.data.numVal=cos(d * temp);
}

void libfunc_strtonum(void){
	avm_memcell * argument;
	if(avm_totalactuals()!=1){
		avm_error("wrong number of arguments");
		executionFinished=1;
		return;
	}
	argument=avm_getactual(0);
	if(argument->type!=string_m){
		avm_error("wrong argument type");
		executionFinished=1;
		return;
	}
	retval.data.numVal =  atof(argument->data.strVal);

	if(retval.data.numVal==0)
		retval.type = nil_m;
	else
		retval.type = number_m;
	
}

void libfunc_objecttotalmembers(void){
	avm_memcell * argument;
	if(avm_totalactuals()!=1){
		avm_error("wrong number of arguments");
		executionFinished=1;
		return;
	}
	argument=avm_getactual(0);
	if(argument->type!=table_m){
		avm_error("wrong argument type");
		retval.type = nil_m;
		return;
	}
	retval.type = number_m;
	retval.data.numVal = argument->data.tableVal->total;
}

void avm_initialize(void){
	
	avm_initstack();
	top=((AVM_STACKSIZE-1)-programVarOffset);
	topsp=top;
	avm_registerlibfunc("print",libfunc_print);
	avm_registerlibfunc("typeof",libfunc_typeof);
	avm_registerlibfunc("sqrt", libfunc_sqrt);
    avm_registerlibfunc("cos", libfunc_cos);
 	avm_registerlibfunc("sin", libfunc_sin);

	/* Same for all the rest library functions.*/
}




void libfunc_totalarguments(void){
	/*Get topsp of previous activations record.*/
	unsigned p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcellclear(&retval);

	if(!p_topsp){
		avm_error("totalarguments called outside a function!");
		retval.type=nil_m;
		executionFinished = 1;
	}
	else{
		/*	extract the number of actual arguments for the previous 
			activation record. */
		retval.type=number_m;
		retval.data.numVal = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
	}
}


void read_binary(){
	int i;
	FILE* fp=fopen("binary.abc","rb");
	if(fp==NULL){
		printf("ERROR reading file\n");
		exit(-1);
	}
	unsigned magicNumber;
	int strSize;

	fread(&magicNumber,sizeof(unsigned),1,fp);
	

	//*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&

	//read total number of strings
	fread(&AVMtotalStringConsts,sizeof(unsigned),1,fp);

	//make array of strings
	AVMstringConsts=(char **) malloc(sizeof(char*)* AVMtotalStringConsts);


	for(i=0;i<AVMtotalStringConsts;i++){
		strSize=0;
		fread(&strSize,sizeof(int),1,fp);
		AVMstringConsts[i]=(char*)malloc(sizeof(char)*strSize);
		fread(AVMstringConsts[i],sizeof(char),strSize,fp);
	}

	//*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&

	//read total number of nums
	fread(&AVMtotalNumConsts,sizeof(unsigned),1,fp);
	

	//make array of nums
	AVMnumConsts=(double *) malloc(sizeof(double)* AVMtotalNumConsts);

	for(i=0;i<AVMtotalNumConsts;i++){
		fread(&AVMnumConsts[i],sizeof(double),1,fp);
	}


	//*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&


	//read total number of userFuncs
	fread(&AVMtotaluserFuncs,sizeof(unsigned),1,fp);


	//make array of userFuncs
	AVMuserFuncs = (userfunc *) malloc (sizeof( userfunc ) * AVMtotaluserFuncs);
	for(i=0;i<AVMtotaluserFuncs;i++){
		//address
		fread(&AVMuserFuncs[i].address,sizeof(unsigned),1,fp);
		//localSize
		fread(&AVMuserFuncs[i].localSize,sizeof(unsigned),1,fp);
		//id
		fread(&strSize,sizeof(int),1,fp);
		AVMuserFuncs[i].id=(char *) malloc(sizeof(char)*strSize);
		fread(AVMuserFuncs[i].id,sizeof(char),strSize,fp);		
	}
	

	//*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&



	//read total number of libFuncs
	fread(&AVMtotalnamedLibfuncs,sizeof(unsigned),1,fp);

	//make array of libFuncs
	AVMnamedLibfuncs=(char **) malloc(sizeof(char *) *AVMtotaluserFuncs);
		
	for(i=0;i<AVMtotalnamedLibfuncs;i++){
		fread(&strSize,sizeof(int),1,fp);
		AVMnamedLibfuncs[i]=(char *) malloc(sizeof(char)*strSize);
		fread(AVMnamedLibfuncs[i],sizeof(char),strSize,fp);
	}

	fread(&programVarOffset,sizeof(unsigned),1,fp);
	
	

	//read total number of instructions
	fread(&codeSize,sizeof(unsigned),1,fp);

	//make array of libFuncs
	code=(instruction *) malloc(sizeof(instruction) *codeSize);

	instruction * instr=code;
	i = 0;
	while(i<codeSize){


		//opcode
		fread(&instr[i].opcode,sizeof(int),1,fp);

		//result
		fread(&instr[i].result.type,sizeof(int),1,fp);
		fread(&instr[i].result.val,sizeof(unsigned),1,fp);

		//arg1
		fread(&instr[i].arg1.type,sizeof(int),1,fp);
		fread(&instr[i].arg1.val,sizeof(unsigned),1,fp);

		//arg2
		fread(&instr[i].arg2.type,sizeof(int),1,fp);
		fread(&instr[i].arg2.val,sizeof(unsigned),1,fp);

		i++;
		//instr=code+i;
		
	}

}





void printArrays(){
	unsigned int i;

	for(i = 0; i < AVMtotalNumConsts; i++){
		printf(CYAN_COLOR"num[%d] = %f\n"COLOR_RESET , i, consts_getnumber(i));
	}
	for(i = 0; i < AVMtotalStringConsts; i++){
		printf(CYAN_COLOR"strings[%d] = %s\n"COLOR_RESET, i, consts_getstring(i));
	}
	for(i = 0; i< AVMtotalnamedLibfuncs; i++){
		printf(CYAN_COLOR"libfunc[%d] = %s\n"COLOR_RESET, i, AVMnamedLibfuncs[i]);
	}
	for(i = 0; i < AVMtotaluserFuncs; i++){
		printf(CYAN_COLOR"userfunc[%d] = %s\n"COLOR_RESET, i, AVMuserFuncs[i].id);
	}
}


void printVals(){
	unsigned int i;
	for (i = 0; i < codeSize; i++)
	{	printf("code[%d].result.type = %d | code[%d].arg1.type = %d | code[%d].arg2.type = %d\n",i, code[i].result.type,i, code[i].arg1.type, i, code[i].arg2.type);
		printf("code[%d].result.val  = %d | code[%d].arg1.val  = %d | code[%d].arg2.val = %d\n",i, code[i].result.val,i, code[i].arg1.val, i, code[i].arg2.val);
		printf("\n");
	}
}



int main(){
	read_binary();
	avm_initialize();
	
	
    //printArrays();
	//printVals();
	while(executionFinished!=1){

		execute_cycle();
	}
	return 0;
}