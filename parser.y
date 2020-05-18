%{
  	#include <stdio.h>
  	#include <stdlib.h>
  	#include <string.h>
  	#include <assert.h>
  	#include "myheader.h"
	#include "symtable.h"
	#include "quads.h"
	#include "target_code.h"

  	int yylex(void);
    int yyerror (char* yaccProvidedMessage);
    extern int yylex (void);
    FILE * yyin;
    extern int yylineno;
    extern char * yyval;
    extern char * yytext;
    extern int yyparse();
    extern struct alpha_token_t * head;

    unsigned int scope=0;
    SymbolTableEntry * globalFuncEntry=NULL;
     SymbolTableEntry *  tempFuncEntry=NULL;
    int funcNameNumber=1;
    
    extern unsigned currQuad;


    expr * tmpExprStruct=NULL;
    int loopcounter=0;
    int inFunctionCounter=0;
%}


/*TOKEN TYPES UNION*/
%union{
	char * stringValue;
	int intValue;
	double realValue;
	struct quad* quad;
	struct expr* expression;
	struct indexedStruct * indexedStruct;
	struct forprefix * forprefix;
	struct statementStruct * statement ;
	
}


%token <stringValue>  IDENTIFIER
%token <intValue>     INTEGER

%start program

%token   IF
%token   ELSE
%token   WHILE
%token   FOR
%token   FUNCTION
%token   RETURN
%token   BREAK
%token   CONTINUE
%token   AND
%token   NOT
%token   OR
%token   LOCAL
%token   TRUE
%token   FALSE
%token   NIL
%token   ASSIGNOP
%token   PLUS
%token   MINUS
%token   MULTI
%token   DIV
%token   MOD
%token   EQUAL
%token   NOTEQUAL
%token   DBPLUS
%token   DBMINUS
%token   GREATER
%token   LESSERS
%token   LESSEREQUAL

%token   REAL
%token   LEFTCURLY
%token   RIGHTCURLY
%token   LEFTBRACKET
%token   RIGHTBRACKET
%token   LEFTPAREN
%token   RIGHTPAREN
%token   SEMICOLON
%token   COMMA
%token   COLON
%token   DOUBLECOLON
%token   DOT
%token   DOUBLEDOT
%token   STRING

%expect 1

%right      ASSIGNOP
%left       OR
%left       AND
%nonassoc   EQUAL NOTEQUAL
%nonassoc   GREATER GREATEREQUAL LESSER LESSEREQUAL
%left       PLUS MINUS
%left       MULTI DIV MOD
%right      NOT DBPLUS DBMINUS UMINUS
%left       DOT DOUBLEDOT
%left       LEFTBRACKET RIGHTBRACKET
%left       LEFTPAREN RIGHTPAREN

%type <expression> program
%type <statement> stmt
%type <statement> stmts
%type <expression> expr
%type <expression> term
%type <expression> assignexpr
%type <expression> primary
%type <expression> lvalue
%type <expression> member
%type <expression> call
%type <expression> callsuffix
%type <expression> normcall
%type <expression> methodcall
%type <expression> elist
%type <expression> objectdef

%type <indexedStruct> indexed
%type <indexedStruct> indexedelem

%type <statement> block
%type <expression> funcdef
%type <expression> const
%type <expression> idlist
%type <statement> ifstmt
%type <statement> whilestmt
%type <statement> forstmt
%type <statement> returnstmt

%type <intValue> ifprefix elseprefix whilecond whilestart N M FUNCTION funcblockend
%type <forprefix> forprefix
%type <statement> loopstmt 

%%


program: stmts			   	{printf("stmts Reduced to program ! [line : %d]\n",yylineno);}
		 |					{;}
		 ;

stmt:	expr SEMICOLON    	{
								$$=initStatementStruct(); 
								resetTemp(); 
								printf("expr SEMICOLON Reduced to stmt[line : %d]\n",yylineno);}

		|ifstmt             {	
								$$=initStatementStruct(); 
								if($1->breakList!=NULL)
								{	
									$$->breakList = mergeLists($$->breakList, $1->breakList);
								}
								$$->contList = mergeLists($$->contList, $1->contList);
								printf("ifstmt Reduced to stmt [line : %d]\n",yylineno);
							}

		|whilestmt          {	
								$$=initStatementStruct();
								printf("whilestmt Reduced to stmt [line : %d]\n",yylineno);
							}

		|forstmt            {
								$$=initStatementStruct(); 
								printf("forstmt Reduced to stmt [line : %d]\n",yylineno);
							}

		|returnstmt         {
								$$=initStatementStruct(); 
								printf("returnstmt Reduced to stmt [line : %d]\n",yylineno);
							}

		|BREAK SEMICOLON    {
								
								if(loopcounter<1){
									printf(RED_COLOR "ERROR break statement outside of loop in line: %d\n" COLOR_RESET, yylineno);
									exit(-1);
								}

								$$=initStatementStruct();
								$$->breakList=newlist(nextquadlabel());
								emit(jump, NULL, NULL, NULL, -1, nextquadlabel());
								printf("BREAK SEMICOLON Reduced to stmt [line : %d]\n",yylineno);
							}
		|CONTINUE SEMICOLON {
								if(loopcounter<1){
									printf(RED_COLOR "ERROR continue statement outside of loop in line: %d\n" COLOR_RESET, yylineno);
									exit(-1);
								}
								$$=initStatementStruct();
								$$->contList=newlist(nextquadlabel());
								emit(jump, NULL, NULL, NULL, -1, nextquadlabel());
								printf("CONTINUE SEMICOLON Reduced to stmt [line : %d]\n",yylineno);
							}	
		|block              {$$=initStatementStruct(); $$=$block; printf("block Reduced to stmt [line : %d]\n",yylineno);}
		|funcdef            {$$=initStatementStruct(); printf("funcdef Reduced to stmt [line : %d]\n",yylineno);}
		|SEMICOLON          {$$=initStatementStruct(); printf("SEMICOLON Reduced to stmt [line : %d]\n",yylineno);}
		;

stmts:	stmts stmt          {	
								$$=initStatementStruct();
								$$->breakList=mergeLists($1->breakList,$2->breakList);
								$$->contList=mergeLists($1->contList,$2->contList);
								printf("stmts stmt Reduced to stmts [line : %d]\n",yylineno);
							}
		|stmt               {	
								//$$=initStatementStruct();
								$$=$1; 
								printf("stmt Reduced to stmts[line : %d]\n",yylineno);
							}

		;



expr:	assignexpr        		{	
									$$=$1;
									printf("assignexpr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr PLUS expr       	{	
									$$=newexpr(arithexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(add, $1, $3, $$, -1, currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr MINUS expr       	{	
									$$=newexpr(arithexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(sub, $1, $3, $$, -1, currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr MULTI expr      	{	
									$$=newexpr(arithexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(mul, $1, $3, $$, -1, currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr DIV expr      	{	$$=newexpr(arithexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(division, $1, $3, $$, -1, currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr MOD expr       	{	$$=newexpr(arithexpr_e);

									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(mod, $1, $3, $$, -1, currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr GREATER expr      {	$$=newexpr(boolexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(if_greater,$1,$3,NULL,currQuad+3,currQuad);
									emit(assign,newBoolExpr(0),NULL,$$,-1,currQuad);
									emit(jump,NULL,NULL,NULL,currQuad+2,currQuad);
									emit(assign,newBoolExpr(1),NULL,$$,-1,currQuad);

									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr GREATEREQUAL expr {	$$=newexpr(boolexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(if_greatereq,$1,$3,NULL,currQuad+3,currQuad);
									emit(assign,newBoolExpr(0),NULL,$$,-1,currQuad);
									emit(jump,NULL,NULL,NULL,currQuad+2,currQuad);
									emit(assign,newBoolExpr(1),NULL,$$,-1,currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr LESSER expr       {	$$=newexpr(boolexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(if_less,$1,$3,NULL,currQuad+3,currQuad);
									emit(assign,newBoolExpr(0),NULL,$$,-1,currQuad);
									emit(jump,NULL,NULL,NULL,currQuad+2,currQuad);
									emit(assign,newBoolExpr(1),NULL,$$,-1,currQuad);
									printf("expr lesser expr Reduced to expr[line : %d]\n",yylineno);

								}
		|expr LESSEREQUAL expr  {	$$=newexpr(boolexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));	
									emit(if_lesseq,$1,$3,NULL,currQuad+3,currQuad);
									emit(assign,newBoolExpr(0),NULL,$$,-1,currQuad);
									emit(jump,NULL,NULL,NULL,currQuad+2,currQuad);
									emit(assign,newBoolExpr(1),NULL,$$,-1,currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr EQUAL expr      	{	$$=newexpr(boolexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(if_eq,$1,$3,NULL,currQuad+3,currQuad);
									emit(assign,newBoolExpr(0),NULL,$$,-1,currQuad);
									emit(jump,NULL,NULL,NULL,currQuad+2,currQuad);
									emit(assign,newBoolExpr(1),NULL,$$,-1,currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr NOTEQUAL expr     {	
									$$=newexpr(boolexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									emit(if_noteq,$1,$3,NULL,currQuad+3,currQuad);
									emit(assign,newBoolExpr(0),NULL,$$,-1,currQuad);
									emit(jump,NULL,NULL,NULL,currQuad+2,currQuad);
									emit(assign,newBoolExpr(1),NULL,$$,-1,currQuad);
									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr AND  expr       	{	

									$$=newexpr(boolexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));

									//emit(and,newBoolExpr($1),newBoolExpr($3),$$,-1,currQuad);
									emit(if_eq,$1,newBoolExpr(1),NULL,nextquadlabel()+2,nextquadlabel());

									emit(jump,NULL,NULL,NULL,nextquadlabel()+5,nextquadlabel());

									emit(if_eq,$3,newBoolExpr(1),NULL,nextquadlabel()+2,nextquadlabel());

									emit(jump,NULL,NULL,NULL,nextquadlabel()+3,nextquadlabel());

									emit(assign,NULL,newBoolExpr(1),$$,-1,nextquadlabel());

									emit(jump,NULL,NULL,NULL,nextquadlabel()+2,nextquadlabel());

									emit(assign,NULL,newBoolExpr(0),$$,-1,nextquadlabel());

									printf("expr op expr Reduced to expr [line : %d]\n",yylineno);
								}
		|expr OR  expr       	{	

									$$=newexpr(boolexpr_e);
									$$->sym=newTemp(scope,yylineno);
									$$->strConst=strdup(getEntryName($$->sym));
									//emit(or,newBoolExpr($1),newBoolExpr($3),$$,-1,currQuad);
									emit(if_eq,$1,newBoolExpr(1),NULL,nextquadlabel()+3,nextquadlabel());
									
									emit(if_eq,$3,newBoolExpr(1),NULL,nextquadlabel()+2,nextquadlabel());
									
									emit(jump,NULL,NULL,NULL,nextquadlabel()+3,nextquadlabel());
									
									emit(assign,NULL,newBoolExpr(1),$$,-1,nextquadlabel());
									
									emit(jump,NULL,NULL,NULL,nextquadlabel()+2,nextquadlabel());
									
									emit(assign,NULL,newBoolExpr(0),$$,-1,nextquadlabel());

									printf("expr op expr Reduced to expr[line : %d]\n",yylineno);
								}	
		|term               	{	$$=$1;
									printf("term Reduced to expr [line : %d]\n",yylineno);
								}
		;


term:   LEFTPAREN expr RIGHTPAREN   {$$=$expr; printf("LEFTPAREN expr RIGHTPAREN Reduced to term \n");}
		| MINUS expr %prec UMINUS	{
										checkuminus($expr);
										$term=newexpr(arithexpr_e);
										$term->sym=istempexpr($expr) ? $expr->sym : newTemp(scope,yylineno);
										emit(uminus,NULL,$expr,$term,-1,currQuad);
										printf("-expr Reduced to term [line : %d]\n",yylineno);

									}	
		| NOT expr					{	
										$term=newexpr(boolexpr_e);
										$term->sym=newTemp(scope,yylineno);
										$$->strConst=strdup(getEntryName($$->sym));
										//emit(not,NULL,$expr,$term,-1,currQuad);

										emit(if_eq,$2,newBoolExpr(1),NULL,nextquadlabel()+3,nextquadlabel());

										emit(assign,NULL,newBoolExpr(1),$$,-1,nextquadlabel());

										emit(jump,NULL,NULL,NULL,nextquadlabel()+2,nextquadlabel());

										emit(assign,NULL,newBoolExpr(0),$$,-1,nextquadlabel());
									
									

										printf("NOT expr Reduced to term [line : %d]\n",yylineno);
									}	
		| DBPLUS lvalue			{
									
									printf("++lvalue Reduced to term [line : %d]\n",yylineno);
									SymbolTableEntry * scopeTmp=NULL;
									SymbolTableEntry * globalTmp=NULL;
									

									scopeTmp=scopeLookup(yylval.stringValue,scope);
									globalTmp=scopeLookup(yylval.stringValue,0);

									if(scopeTmp==NULL){
										if((globalTmp)==NULL){
											printf(RED_COLOR "ERROR operation on undefined variable: %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
										}
									}


									if(scopeTmp!=NULL){
										if(scopeTmp->type==USERFUNC || scopeTmp->type==LIBFUNC)
											printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
									}
									else if(globalTmp!=NULL){
										if(globalTmp->type==USERFUNC || globalTmp->type==LIBFUNC)
											printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
									}
									if($lvalue->type==tableitem_e){
										$term=emit_iftableitem($lvalue,scope,yylineno);
										emit(add,$term,newIntExpr(1),$term,-1,currQuad);
										emit(tablesetelem,$lvalue,$lvalue->index,$term,-1,currQuad);
									}
									else{
										emit(add,$lvalue,newIntExpr(1),$lvalue,-1,currQuad);
										$term=newexpr(arithexpr_e);
										$term->sym=newTemp(scope,yylineno);
										emit(assign,$lvalue,NULL,$term,-1,currQuad);
									}
								}

		| lvalue 	{				
						printf("lvalue++ Reduced to term [line : %d]\n",yylineno);
						SymbolTableEntry * scopeTmp=NULL;
						SymbolTableEntry * globalTmp=NULL;
						
						scopeTmp=scopeLookup(yylval.stringValue,scope);
						globalTmp=scopeLookup(yylval.stringValue,0);

						if(scopeTmp==NULL){
							if((globalTmp)==NULL){
								printf(RED_COLOR "ERROR operation on undefined variable: %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
							}
						}

						if(scopeTmp!=NULL){
							if(scopeTmp->type==USERFUNC || scopeTmp->type==LIBFUNC)
								printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
						}
						else if(globalTmp!=NULL){
							if(globalTmp->type==USERFUNC || globalTmp->type==LIBFUNC)
								printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
						}
					}
			DBPLUS		{
							$term=newexpr(var_e);
							$term->sym=newTemp(scope,yylineno);
							if($lvalue->type==tableitem_e){
								expr * value=emit_iftableitem($lvalue,scope,yylineno);
								emit(assign,value,NULL,$term,-1,currQuad);
								emit(add,value,newIntExpr(1),value,-1,currQuad);
								emit(tablesetelem,$lvalue,$lvalue->index,value,-1,currQuad);
							}else{
								emit(assign,$lvalue,NULL,$term,-1,currQuad);
								emit(add,$lvalue,newIntExpr(1),$lvalue,-1,currQuad);
							}
						}		
		| DBMINUS lvalue	{		printf("--lvalue Reduced to term [line : %d]\n",yylineno);
									SymbolTableEntry * scopeTmp=NULL;
									SymbolTableEntry * globalTmp=NULL;
									

									scopeTmp=scopeLookup(yylval.stringValue,scope);
									globalTmp=scopeLookup(yylval.stringValue,0);

									if(scopeTmp==NULL){
										if((globalTmp)==NULL){
											printf(RED_COLOR "ERROR operation on undefined variable: %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
										}
									}


									if(scopeTmp!=NULL){
										if(scopeTmp->type==USERFUNC || scopeTmp->type==LIBFUNC)
											printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
									}
									else if(globalTmp!=NULL){
										if(globalTmp->type==USERFUNC || globalTmp->type==LIBFUNC)
											printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
									}
									if($lvalue->type==tableitem_e){
										$term=emit_iftableitem($lvalue,scope,yylineno);
										emit(sub,$term,newIntExpr(1),$term,-1,currQuad);
										emit(tablesetelem,$lvalue,$lvalue->index,$term,-1,currQuad);
									}
									else{
										emit(sub,$lvalue,newIntExpr(1),$lvalue,-1,currQuad);
										$term=newexpr(arithexpr_e);
										$term->sym=newTemp(scope,yylineno);
										emit(assign,$lvalue,NULL,$term,-1,currQuad);
									}

							}
		| lvalue 				{
										printf("lvalue-- Reduced to term [line : %d]\n",yylineno);
										SymbolTableEntry * scopeTmp=NULL;
									SymbolTableEntry * globalTmp=NULL;
									
									scopeTmp=scopeLookup(yylval.stringValue,scope);
									globalTmp=scopeLookup(yylval.stringValue,0);

									if(scopeTmp==NULL){
										if((globalTmp)==NULL){
											printf(RED_COLOR "ERROR operation on undefined variable: %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
										}
									}
									if(scopeTmp!=NULL){
										if(scopeTmp->type==USERFUNC || scopeTmp->type==LIBFUNC)
											printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
									}
									else if(globalTmp!=NULL){
										if(globalTmp->type==USERFUNC || globalTmp->type==LIBFUNC)
											printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
									}
								} 
			DBMINUS{
							$term=newexpr(var_e);
							$term->sym=newTemp(scope,yylineno);
							if($lvalue->type==tableitem_e){
								expr * value=emit_iftableitem($lvalue,scope,yylineno);
								emit(assign,value,NULL,$term,-1,currQuad);
								emit(sub,value,newIntExpr(1),value,-1,currQuad);
								emit(tablesetelem,$lvalue,$lvalue->index,value,-1,currQuad);
							}else{
								emit(assign,$lvalue,NULL,$term,-1,currQuad);
								emit(sub,$lvalue,newIntExpr(1),$lvalue,-1,currQuad);
							}
						}

		| primary					{
										$$ = $1;
										printf("primary Reduced to term [line : %d]\n",yylineno);
									}
		;	
assignexpr: lvalue 				{
									SymbolTableEntry * scopeTmp=NULL;
									SymbolTableEntry * globalTmp=NULL;
									SymbolTableEntry * tmp=NULL;
									int tempscope = scope;

									if($lvalue->type != tableitem_e)
									{
										scopeTmp=scopeLookup(yylval.stringValue,scope);
										globalTmp=scopeLookup(yylval.stringValue,0);
									}
								
									if(scopeTmp!=NULL){
										if(scopeTmp->type==USERFUNC || scopeTmp->type==LIBFUNC)
											printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
									}
									else if(globalTmp!=NULL){
										if(globalTmp->type==USERFUNC || globalTmp->type==LIBFUNC)
											printf(RED_COLOR "ERROR Illegal operation for function : %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );

									}
									
									else if(scopeTmp==NULL && $lvalue->type != tableitem_e){
										while(tempscope>=0){
											tmp = scopeLookup(yylval.stringValue,tempscope);
											if(tmp!=NULL){
												if(tmp->isActive==false && tmp->type!=FORMAL){
													tmp =initSymTableEntry(yylval.stringValue,scope, yylineno,calculateType(scope));
													SymTableInsert(tmp);
													ScopeTableInsert(tmp,scope);
												}
												break;
											}
											tempscope--;
										}
										if(tmp==NULL){
											printf(RED_COLOR "ERROR operation on undefined variable: %s In line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );
										}
									}
			}	ASSIGNOP expr 	{	
									//*************************
									if($lvalue->type==tableitem_e){
										emit(tablesetelem,$lvalue,$lvalue->index,$expr, -1,currQuad);
										$$=emit_iftableitem($lvalue,scope,yylineno);
										$$->type=assignexpr_e;

									}
									else{
										emit(assign,$expr,NULL,$lvalue,-1,currQuad);

										$$=newexpr(assignexpr_e);
										$$->sym=newTemp(scope,yylineno);
										$$->strConst = strdup(getEntryName($$->sym));
										emit(assign,$lvalue,NULL,$$, -1,currQuad);
										
									}
									printf("lvalue ASSIGNOP expr Reduced to assignexpr [line : %d]\n", yylineno);
								}
			;

primary :	lvalue							{
												$$=emit_iftableitem($lvalue,scope,yylineno);
											 	printf("lvalue Reduced to primary [line : %d]\n",yylineno);
											 }	
			| call							{$$=$1; printf("call Reduced to primary [line : %d]\n",yylineno);}
			| objectdef						{$$=$1; printf("objectdef Reduced to primary [line : %d]\n",yylineno);}
			| LEFTPAREN funcdef RIGHTPAREN  {	
												$$=newexpr(programfunc_e);
												//$$->sym=newTemp(scope,yylineno);
												$$->sym = tempFuncEntry;
        									    $$->strConst = strdup(getEntryName($$->sym));
        										printf("\n\n\nHELLOO In primary :%s\n\n\n",$$->strConst);
										     	printf("LEFTPAREN funcdef RIGHTPAREN Reduced to primary [line : %d]\n",yylineno);
										 	}	
			| const							{	$$=$1;
												 printf("const Reduced to primary [line : %d]\n",yylineno);
											}	
			;

lvalue:		IDENTIFIER 	{	
							int tempscope=scope;
							SymbolTableEntry * tmp=NULL;					
							while(tempscope>=0){
								tmp = scopeLookup(yylval.stringValue,tempscope);
								if(tmp!=NULL){
									$$=newLvalueExpr(tmp);
									if(tmp->isActive==false && tmp->type!=FORMAL){
										tmp =initSymTableEntry(yylval.stringValue,scope, yylineno,calculateType(scope));
										SymTableInsert(tmp);
										ScopeTableInsert(tmp,scope);
										tmp->scopeSpace = currscopespace();
										tmp->offset = currscopeoffset();
										inccurrscopeoffset();
										
									}
									break;
								}
								tempscope--;
							}
							if(tmp==NULL){
								tmp =initSymTableEntry(yylval.stringValue,scope, yylineno,calculateType(scope));
								SymTableInsert(tmp);
								ScopeTableInsert(tmp,scope);
								$$=newLvalueExpr(tmp);
								tmp->scopeSpace = currscopespace();
								tmp->offset = currscopeoffset();
								inccurrscopeoffset();
							}else{
								if(IsFunctionInBetween(scope-1)==false || 
									getScope(tmp)==0 ||
									(IsFunctionInBetween(scope-1)==true&&tmp->type==FORMAL &&scope==getScope(tmp)) || 
									getScope(tmp)==scope ||
									tmp->type==USERFUNC || tmp->type==LIBFUNC)
								{ 

								}else{
									printf(RED_COLOR "ERROR: WE DONT HAVE ACCESS for variable %s In Line : %d\n" COLOR_RESET,yylval.stringValue,yylineno );

								}
							}
						}	
						
			|LOCAL IDENTIFIER{
												SymbolTableEntry * tmp = scopeLookup(yylval.stringValue,scope);
												
												if(checkLibFuncCollisions(yylval.stringValue)==false){	
													if(tmp==NULL){
														SymbolTableEntry *entry =initSymTableEntry(yylval.stringValue,scope, yylineno,calculateType(scope));
														SymTableInsert(entry);
														ScopeTableInsert(entry,scope);
														$$=newLvalueExpr(entry);
														entry->scopeSpace=currscopespace();
														entry->offset=currscopeoffset();
														inccurrscopeoffset();
													}else if(tmp!=NULL){
														if(tmp->isActive==TRUE){
															printf(RED_COLOR "ERROR found local redecleration in line: %d\n" COLOR_RESET, yylineno);
														}
														else{
															SymbolTableEntry *entry =initSymTableEntry(yylval.stringValue,scope, yylineno,calculateType(scope));
															SymTableInsert(entry);
															ScopeTableInsert(entry,scope);
															$$=newLvalueExpr(entry);
															entry->scopeSpace=currscopespace();
															entry->offset=currscopeoffset();
															inccurrscopeoffset();
														}
													}

												}
												else{
													if(scope!=0){
														printf(RED_COLOR "ERROR library function collision in line: %d\n" COLOR_RESET, yylineno);
													}

												}
												printf("LOCAL IDENTIFIER  Reduced to lvalue [line : %d]\n",yylineno);
												
											}	
			|DOUBLECOLON IDENTIFIER         {
												SymbolTableEntry * tmp = scopeLookup(yylval.stringValue,0);
												if(tmp==NULL){
													printf(RED_COLOR "ERROR : global variable \"%s\" in line: %d not found \n" COLOR_RESET,yylval.stringValue, yylineno);
												}else{
													$$=newLvalueExpr(tmp);
												}
												
												printf(":: IDENTIFIER Reduced to lvalue [line : %d]\n",yylineno);
											}	
			|member					        {
												$$=$1;
												printf("member Reduced to lvalue [line : %d]\n",yylineno);
											}	
			;


member:		lvalue DOT IDENTIFIER                  {
													expr * name=newStringExpr(yylval.stringValue);
													$$=member_item($lvalue,name,scope,yylineno);
													printf("lvalue.IDENTIFIER     Reduced to member [line : %d]\n",yylineno);
													}
			|lvalue LEFTBRACKET expr RIGHTBRACKET	{	
														$lvalue=emit_iftableitem($lvalue,scope,yylineno);
														$$=newexpr(tableitem_e);
														$$->sym=$lvalue->sym;
														$$->index=$expr;
														printf("lvalue LEFTBRACKET expr RIGHTBRACKET Reduced to member [line : %d]\n",yylineno);

													}
			|call DOT IDENTIFIER					  {printf("call.IDENTIFIER	 Reduced to member [line : %d]\n",yylineno);}
			|call LEFTBRACKET expr RIGHTBRACKET       {printf("call LEFTBRACKET expr RIGHTBRACKET Reduced to member [line : %d]\n",yylineno);}
			;

call:		call LEFTPAREN elist RIGHTPAREN     {
													$$=make_call($1,$elist,scope,yylineno);
													printf("call LEFTPAREN elist RIGHTPAREN Reduced to call [line : %d]\n",yylineno);
												}
			|lvalue callsuffix		{
										if($callsuffix->boolConst){
											expr * self=$lvalue;
											$lvalue=emit_iftableitem(
												member_item(self,newStringExpr($callsuffix->strConst),scope,yylineno),
												scope,
												yylineno
											);
											$callsuffix=$callsuffix->next;
											self->next=$callsuffix;
											$callsuffix=self;
										}
										$call=make_call($lvalue,$callsuffix,scope,yylineno);
										printf("lvalue callsuffix Reduced to call \n");
			}
			|LEFTPAREN funcdef RIGHTPAREN LEFTPAREN elist RIGHTPAREN  
				{
					expr * func=newexpr(programfunc_e);
					func->sym=$funcdef->sym;
					$call=make_call(func,$elist,scope,yylineno);
					printf("LEFTPAREN funcdef RIGHTPAREN LEFTPAREN elist RIGHTPAREN Reduced to call [line : %d]\n",yylineno);
				}
			;

callsuffix: normcall	{	
						$$=$1;
						printf("normcall Reduced to callsuffix [line : %d]\n",yylineno);
						}
			|methodcall{
						$$=$1;
						printf("methodcall Reduced to callsuffix [line : %d]\n",yylineno);
						}
			;

normcall: LEFTPAREN elist RIGHTPAREN{
										if($elist != NULL)
                                        {$$=$elist;}
                                        else{
                                            $$=newexpr(normcall_e);
                                            $$->sym=newTemp(scope,yylineno);
                                            $$->strConst = strdup(getEntryName($$->sym));
											$$->elistIsNull=1;
                                        }
                                        $$->boolConst=0;
                                        printf(" LEFTPAREN elist RIGHTPAREN Reduced to normcall [line : %d]\n",yylineno);
                                    }
									

methodcall: DOUBLEDOT IDENTIFIER LEFTPAREN elist RIGHTPAREN	{
																$methodcall=newexpr(methodcall_e);
																if($elist==NULL){
																	$methodcall->sym=newTemp(scope,yylineno);
																	$methodcall->strConst=strdup(getEntryName($methodcall->sym));
																}else{
																	$$->next=$elist;
																}
															
																$methodcall->boolConst=1;
																$methodcall->strConst=strdup($IDENTIFIER);
																printf("DOUBLEDOT IDENTIFIER LEFTPAREN elist RIGHTPAREN Reduced to methodcall [line : %d]\n",yylineno);
															}

elist: 	expr                      						{
															
															$elist=$1; 
															tmpExprStruct=$elist;
														
															printf("expr Reduced to elist [line : %d]\n",yylineno);
														}
		| elist COMMA expr       						{    
															if($expr!=NULL){
																tmpExprStruct->next=$expr;
																tmpExprStruct=tmpExprStruct->next;
															}
															printf("elist COMMA expr  Reduced to elist [line : %d]\n",yylineno);

														}
		|						  						{$$=NULL; printf("empty thingy reduced to elist [line : %d]\n",yylineno);}
		;	

objectdef: LEFTBRACKET elist RIGHTBRACKET	{
												expr * t =newexpr(newtable_e);
												t->sym=newTemp(scope,yylineno);
												emit(tablecreate,t,NULL,NULL,-1,currQuad);
												int i=0;
												expr * tmp=$elist;
												while(tmp!=NULL){
													emit(tablesetelem,t,newIntExpr(i++),tmp,-1,currQuad);
													tmp=tmp->next;
												}
												$$=t;
												printf("LEFTBRACKET elist RIGHTBRACKET Reduced to objectdef [line : %d]\n",yylineno);
											}
					|LEFTBRACKET indexed RIGHTBRACKET
							{	
								expr * t = newexpr(newtable_e);
								t->sym=newTemp(scope,yylineno);
								emit(tablecreate,t,NULL,NULL,-1,currQuad);
								indexedStruct * tmp=$indexed;
								while(tmp!=NULL){
									emit(tablesetelem,t,tmp->x,tmp->y,-1,currQuad);
									tmp=tmp->next;
								}
								$$=t;
								printf("LEFTBRACKET indexed RIGHTBRACKET Reduced to objectdef [line : %d]\n",yylineno);
							}
					;

indexed:		indexedelem	{
								$$=$1;
								printf("indexedelem Reduced to indexed [line : %d]\n",yylineno);
							}
				| indexed COMMA indexedelem	
						{	
							$$=$1;
							indexedStruct * tmp=$1;
							while(tmp->next!=NULL)
								tmp=tmp->next;
							tmp->next=$3;
							printf("indexed COMMA indexedelem Reduced to indexed [line : %d]\n",yylineno);
						}
				;

indexedelem:	LEFTCURLY expr COLON expr RIGHTCURLY	
				{
					$$=newIndexedStruct($2,$4);
					printf("LEFTCURLY expr COLON expr RIGHTCURLY Reduced to indexedelem [line : %d]\n",yylineno);
				}
				;

block:	LEFTCURLY{scope++;} stmts RIGHTCURLY	{	
													$block=$stmts;
													Hide(scope--);
													printf("LEFTCURLY stmts RIGHTCURLY Reduced to block [line : %d]\n",yylineno); 
												}
			|LEFTCURLY RIGHTCURLY						{;}
				;


funcblockstart: {
				pushLoopStack(loopcounter);
				loopcounter=0;
				inFunctionCounter++;

}
funcblockend: {
				loopcounter=popLoopStack();
				inFunctionCounter--;

}


funcdef:	
				FUNCTION{
                    char* Buffer = malloc(1000);

                    snprintf(Buffer, 1000,"$func%d",funcNameNumber);
                     funcNameNumber++;

                    pushfunctionLocalsStack(currscopeoffset());
				    resetfunctionlocalsoffset();

                    SymbolTableEntry *entry;
                    SymbolTableEntry * tmp = scopeLookup(Buffer,scope);
                    entry =initSymTableEntry(Buffer,scope, yylineno,USERFUNC);
                  	tempFuncEntry=entry;
                    assert(entry);

                    SymTableInsert(entry);
                    ScopeTableInsert(entry,scope);

                    globalFuncEntry=entry;
                    assert(globalFuncEntry);


                    pushFunctionStack(globalFuncEntry);
					emit(jump,NULL,NULL,NULL,-1,currQuad);
					$1=nextquadlabel()-1;
                    emit(funcstart,NULL,NULL,newLvalueExpr(entry),-1,currQuad);
                    resetformalargsoffset();
                }
						
					
				LEFTPAREN {scope++; enterscopespace();} idlist RIGHTPAREN {scope--; enterscopespace();} funcblockstart block funcblockend                      
					{	
						$funcblockend=nextquadlabel();
						SymbolTableEntry * poped=popFunctionStack();
						assert(poped);
						$$=newexpr(programfunc_e);
						$$->sym=poped;
						$$->sym->totalLocals=currscopeoffset();
						exitscopespace();
						exitscopespace();
						
						int localoffset=popfunctionLocalsStack();
						backPatchReturnList(poped->returnList,$funcblockend);

						emit(funcend,NULL,NULL,newLvalueExpr(poped),-1,currQuad);

						patchlabel($1, nextquadlabel());
						globalFuncEntry=functionStackHead;
						printf("FUNCTION LEFTPAREN idlist RIGHTPAREN block Reduced to funcdef [line : %d]\n",yylineno);
					}


				|	FUNCTION IDENTIFIER {
											pushfunctionLocalsStack(currscopeoffset());
											resetfunctionlocalsoffset();

											SymbolTableEntry *entry;
											SymbolTableEntry * tmp = scopeLookup(yylval.stringValue,scope);
											if(checkLibFuncCollisions(yylval.stringValue)==true){
												printf(RED_COLOR "ERROR library function shadowing in line:%d\n" COLOR_RESET,yylineno);
											}
											else if(tmp==NULL){
												entry =initSymTableEntry(yylval.stringValue,scope, yylineno,USERFUNC);
												assert(entry);

												
												entry->functionAddress=nextquadlabel();

												SymTableInsert(entry);
												ScopeTableInsert(entry,scope);

												globalFuncEntry=entry;
												assert(globalFuncEntry);
												pushFunctionStack(globalFuncEntry);
												emit(jump,NULL,NULL,NULL,-1,currQuad);
												$1=nextquadlabel()-1;
												emit(funcstart,NULL,NULL,newLvalueExpr(entry),-1,currQuad);
												
												//enterscopespace();
												resetformalargsoffset();
												
											}
											else if(tmp!=NULL){
												if(tmp->isActive && scope==getScope(tmp)){
													if(tmp->type==LCL || tmp->type==GLOBAL || tmp->type==FORMAL )
														printf(RED_COLOR "ERROR trying to declare function with existing variable's name in Line:%d\n" COLOR_RESET,yylineno);
													else
														printf(RED_COLOR "ERROR found function redecleration in Line:%d\n" COLOR_RESET,yylineno);	
												}
												else{
													entry =initSymTableEntry(yylval.stringValue,scope, yylineno,USERFUNC);
													assert(entry);
													
													entry->functionAddress=nextquadlabel();

													SymTableInsert(entry);
													ScopeTableInsert(entry,scope);

													globalFuncEntry=entry;
													assert(globalFuncEntry);
													pushFunctionStack(globalFuncEntry);
													emit(jump,NULL,NULL,NULL,-1,currQuad);
													$1=nextquadlabel()-1;
													emit(funcstart,NULL,NULL,newLvalueExpr(entry),-1,currQuad);
													//enterscopespace();
													resetformalargsoffset();
												}
											}
											
										} 
						LEFTPAREN {scope++; enterscopespace();} idlist RIGHTPAREN {scope--; enterscopespace();} funcblockstart block  funcblockend
						{
							$funcblockend=nextquadlabel();
							if(functionLocalsStackIsEmpty()==0){
							}
							SymbolTableEntry * poped=popFunctionStack();
							assert(poped);
							

							$$=newexpr(programfunc_e);
							$$->sym=poped;
							$$->sym->totalLocals=currscopeoffset();
							exitscopespace();
							exitscopespace();
							
							int localoffset=popfunctionLocalsStack();
							
							backPatchReturnList(poped->returnList,$funcblockend);

							emit(funcend,NULL,NULL,newLvalueExpr(poped),-1,currQuad);
							patchlabel($1, nextquadlabel());
							globalFuncEntry=functionStackHead;
							printf("FUNCTION IDENTIFIER LEFTPAREN idlist RIGHTPAREN block Reduced to funcdef [line : %d]\n",yylineno);
						}
				;

const:	INTEGER			{	$$ = newIntExpr(yyval.intValue);
							printf("INTEGER Reduced to const[line : %d]\n",yylineno); 
						}
		|	REAL		{	$$ = newRealExpr(yylval.realValue);
							printf("REAL Reduced to const [line : %d]\n",yylineno);
						}
		|	STRING		{	$$ = newStringExpr(yylval.stringValue);
							printf("STRING Reduced to const [line : %d]\n",yylineno);
						}
		|	NIL			{	$$ = newNilExpr();
							printf("NIL Reduced to const [line : %d]\n",yylineno);
						}
		|   TRUE		{	$$ = newBoolExpr(true);
							printf("TRUE Reduced to const [line : %d]\n",yylineno);
						}
		|   FALSE		{	$$ = newBoolExpr(false);
							printf("FALSE Reduced to const [line : %d]\n",yylineno);
						}
		;

idlist:		IDENTIFIER						{	
												resetformalargsoffset();
												SymbolTableEntry *argEntry;
												SymbolTableEntry * tmp = scopeLookup(yylval.stringValue,scope);
												if(checkLibFuncCollisions(yylval.stringValue)==true){
													printf(RED_COLOR "ERROR library function shadowing in line: %d\n" COLOR_RESET,yylineno);
												}
												else if(tmp!=NULL ){
													if(tmp->isActive == TRUE){
														printf(RED_COLOR "ERROR found formal redecleration in line: %d\n" COLOR_RESET,yylineno);
													}
													else{
														argEntry =initSymTableEntry(yylval.stringValue,scope, yylineno,FORMAL);
														SymTableInsert(argEntry);
														ScopeTableInsert(argEntry,scope);
														InsertArgInFunction(argEntry,globalFuncEntry);
														argEntry->scopeSpace=currscopespace();
														argEntry->offset=currscopeoffset();
														inccurrscopeoffset();
													}
												}
												else{
														argEntry =initSymTableEntry(yylval.stringValue,scope, yylineno,FORMAL);
														SymTableInsert(argEntry);
														ScopeTableInsert(argEntry,scope);
														InsertArgInFunction(argEntry,globalFuncEntry);
														argEntry->scopeSpace=currscopespace();
														argEntry->offset=currscopeoffset();
														inccurrscopeoffset();
												}
												printf("IDENTIFIER Reduced to idlist [line : %d]\n",yylineno);
											}
			|	idlist COMMA IDENTIFIER		{
												SymbolTableEntry *argEntry;
												SymbolTableEntry * tmp = scopeLookup(yylval.stringValue,scope);
												if(checkLibFuncCollisions(yylval.stringValue)==true){
													printf(RED_COLOR "ERROR library function shadowing in line: %d\n" COLOR_RESET,yylineno);
												}
												else if(tmp!=NULL){
													if( ArgumentLookUp(yylval.stringValue,globalFuncEntry)){
														printf(RED_COLOR "ERROR: found formal redecleration in line: %d\n" COLOR_RESET, yylineno);
													}
												
													else{
														argEntry =initSymTableEntry(yylval.stringValue,scope, yylineno,FORMAL);
														SymTableInsert(argEntry);
														ScopeTableInsert(argEntry,scope);
														InsertArgInFunction(argEntry,globalFuncEntry);
														argEntry->scopeSpace=currscopespace();
														argEntry->offset=currscopeoffset();
														inccurrscopeoffset();
													}
												}
												else{
														argEntry =initSymTableEntry(yylval.stringValue,scope, yylineno,FORMAL);
														SymTableInsert(argEntry);
														ScopeTableInsert(argEntry,scope);
														InsertArgInFunction(argEntry,globalFuncEntry);
														argEntry->scopeSpace=currscopespace();
														argEntry->offset=currscopeoffset();
														inccurrscopeoffset();
												}
												printf("IDENTIFIER Reduced to idlist\n");
											}
			|{;}
			;

ifstmt:	ifprefix stmt							{	patchlabel($ifprefix, nextquadlabel());
													$ifstmt = $stmt;
													printf("IF LEFTPAREN expr RIGHTPAREN stmt Reduced to ifstmt [line : %d]\n",yylineno);
												}
			|	ifprefix stmt elseprefix stmt	{	$$=initStatementStruct();
													$$->breakList = mergeLists($2->breakList, $4->breakList);
													$$->contList = mergeLists($2->contList, $4->contList);
													patchlabel($ifprefix, $elseprefix+1);
													patchlabel($elseprefix, nextquadlabel());
													printf("IF LEFTPAREN expr RIGHTPAREN stmt ELSE stmt Reduced to ifstmt [line : %d]\n",yylineno);
												}
			;

ifprefix: IF LEFTPAREN expr RIGHTPAREN 	{
											emit(if_eq, $expr, newBoolExpr(1), NULL, nextquadlabel()+2,nextquadlabel());
											$ifprefix = nextquadlabel();
											emit(jump,NULL,NULL, NULL, currQuad+2, currQuad);	
										}
										;

elseprefix: ELSE 	{
						$elseprefix = nextquadlabel();
						emit(jump, NULL, NULL, NULL, -1, currQuad);
					}
					;


loopstart: {
				++loopcounter;
			}
			;


loopend: 	{
				--loopcounter;
			}
			;



loopstmt: loopstart stmt loopend{
									$loopstmt=$stmt;
								}
								;


whilestart: WHILE {
						$$=nextquadlabel();
						loopcounter++;
						printf("WHILE Reduced to whilestart [line : %d]\n",yylineno);
					}

whilecond: LEFTPAREN expr RIGHTPAREN	{
											emit(if_eq,$expr,newBoolExpr(1),NULL,nextquadlabel()+2,nextquadlabel());
											$$ = nextquadlabel();
											emit(jump,NULL,NULL,NULL,-1,currQuad);
											printf("LEFTPAREN expr RIGHTPAREN Reduced to whilecond [line : %d]\n",yylineno);
										}


whilestmt: whilestart whilecond loopstmt	{	
												emit(jump,NULL,NULL,NULL,$1,currQuad);
												patchlabel($whilecond,nextquadlabel());
												$$=$3;
												if($loopstmt->breakList!=NULL)
													backPatch($loopstmt->breakList,nextquadlabel());
												if($loopstmt->contList!=NULL)
													backPatch($loopstmt->contList,$whilestart);
												
												loopcounter--;
												printf("whilestart whilecond loopstmt whilestmt [line : %d]\n",yylineno);
											}
			;

N:	{
		$N = nextquadlabel();
		emit(jump,NULL,NULL,NULL,-1,currQuad);
	}
	;

M:	{
	
		$M = nextquadlabel();
	}
	;

forprefix: FOR LEFTPAREN elist SEMICOLON M expr SEMICOLON {
	$$=(struct forprefix *)malloc(sizeof(struct forprefix));
	$forprefix->test = $M;
	$forprefix->enter = nextquadlabel();

	emit(if_eq, $expr, newBoolExpr(1), NULL, -1, currQuad);
	}
	;

forstmt: forprefix N elist RIGHTPAREN N loopstmt N 
							{
								patchlabel($forprefix->enter, $5+1);
								patchlabel($2, nextquadlabel());
								patchlabel($5, $forprefix->test);
								patchlabel($7, $2+1);

								if($loopstmt->breakList!=NULL)
									backPatch($loopstmt->breakList,nextquadlabel());

							
								if($loopstmt->contList!=NULL)
									backPatch($loopstmt->contList,$2+1);

								printf("forprefix elist RIGHTPAREN stmt Reduced to forstmt [line : %d]\n",yylineno);
							}
							;

returnstmt:	RETURN expr SEMICOLON	{
										if(inFunctionCounter<1){
											printf(RED_COLOR "ERROR return statement outside of function in line: %d\n" COLOR_RESET, yylineno);
											exit(-1);
										}
										emit(ret,$expr,NULL,NULL,-1,nextquadlabel());
										insertReturn(globalFuncEntry);
										emit(jump,NULL,NULL,NULL,-1,currQuad);
										printf("RETURN expr SEMICOLON Reduced to returnstmt [line : %d]\n",yylineno);
									}
					| RETURN SEMICOLON	{
											if(inFunctionCounter<1){
												printf(RED_COLOR "ERROR return statement outside of function in line: %d\n" COLOR_RESET, yylineno);
												exit(-1);
											}
											emit(ret,NULL,NULL,NULL,-1,nextquadlabel());
											insertReturn(globalFuncEntry);
											emit(jump,NULL,NULL,NULL,-1,currQuad);
											
											printf("RETURN SEMICOLON Reduced to returnstmt [line : %d]\n",yylineno);
										}
					;

%%

int yyerror(char * yaccProvidedMessage){
	fprintf(stderr, "%s: at line %d, before token: %s\n",yaccProvidedMessage,yylineno,yytext);
	fprintf(stderr, "INPUT NOT VALID\n" );
}

int main( int argc, char **argv ){

       	SymTableInit();
		ScopeTableInit();
		LibFunctionsInit();
		FILE *fp;
		if ( argc == 2 )
                yyin = fopen( argv[1], "r" );
		else if(argc == 3)
		{
			yyin = fopen(argv[1], "r");
			fp = fopen(argv[2], "w");
			if(fp == NULL)
			{
				printf(RED_COLOR "ERROR In Main : OPENING FILE\n" COLOR_RESET);
				exit(-1);
			}
		}
		else 
		{
			printf(RED_COLOR "ERROR In Main : Incorrect arguments!\n" COLOR_RESET);
			exit(-1);
		}

        yyparse();
        //SymTablePrint();
        ScopeTablePrint();
		printQuads();
		generate();
		printTargetCode();
		makeBinary();
		printf("\n");
}