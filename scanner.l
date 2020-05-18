
%option noyywrap
%option yylineno

%{

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "myheader.h"
#include "parser.h"

int tokenNumber=0;
char* strbuf;
char stringArr[5000];
int stringCntr=0;
int block_comment_start_line=0;
int block_comment_end_line=0;
char bcsl_arr[5];
char bcel_arr[5];



struct alpha_token_t * head=NULL;
struct alpha_token_t * last=NULL;

int comment_counter=0;

void add_token(unsigned int  numline,unsigned int  numToken,char * content,char *type){
    struct alpha_token_t * newNode;
    newNode = malloc(sizeof(struct alpha_token_t));
    newNode->numline=numline;
    newNode->numToken=numToken;
    newNode->content=strdup(content);
    newNode->type=strdup(type);

    if(head==NULL && last==NULL){
        head=newNode;
        last=newNode;
        newNode->next=NULL;
    }
    else{
        last->next=newNode;
        newNode->next=NULL;
        last=newNode;
    }
}

void print_list(){
    struct alpha_token_t * current = head;

    printf("\n-------------------- Lexical Analysis --------------------\n\n");
    while (current != NULL) {
        printf("%d:   #%d  \"%s\"  %s ", current->numline,current->numToken,current->content,current->type);
        if(strcmp(current->type,"CONST_INT")==0){
            printf("  <-- integer\n");
        }
        else if(strcmp(current->type,"REAL_CONST")==0){
            printf("  <-- real\n");
        }
        else if(strcmp(current->type,"IDENTIFIER")==0||strcmp(current->type,"STRING")==0){
            printf("  <-- char *\n");
        }
        else {
            printf("  <-- enumerated\n");
        }
            
        current = current->next;
    }
}

void file_print_list(FILE *input_file){
    struct alpha_token_t * current = head;

    fprintf(input_file, "\n-------------------- Lexical Analysis --------------------\n\n");
    while (current != NULL) {
        fprintf(input_file, "%d:   #%d  \"%s\"  %s ", current->numline,current->numToken,current->content,current->type);
        if(strcmp(current->type,"CONST_INT")==0){
            fprintf(input_file, "  <-- integer\n");
        }
        else if(strcmp(current->type,"REAL_CONST")==0){
            fprintf(input_file, "  <-- real\n");
        }
        else if(strcmp(current->type,"IDENTIFIER")==0||strcmp(current->type,"STRING")==0){
            fprintf(input_file, "  <-- char *\n");
        }
        else {
            fprintf(input_file, "  <-- enumerated\n");
        }
            
        current = current->next;
    }
}




%}


if  "if"
else    "else"
while   "while"
for "for"
function    "function"
return  "return"
break   "break"
continue    "continue"
and "and"
not "not"
or "or"
local   "local"
true    "true"
false   "false"
nil "nil"

assignment  "="
plus    "+"
minus   "-"
mult    "*"
div "/"
mod "%"
double_equal    "=="
not_equal   "!="
plusplus    "++"
minusminus  "--"
greater_than    ">"
less_than   "<" 
greater_equal   ">="
less_equal  "<="

digit [0-9]
integer {digit}+
real {integer}"."{integer}
string \"
left_curly  \{
right_curly \}
left_bracket \[
right_bracket \]
left_parenthesis \(
right_parenthesis \)
semicolon \;
comma \,
colon \:
double_colon \:\:
dot \.
double_dot \.\.

identifier [a-zA-Z][a-zA-Z_0-9]*
line_comment ("//".*)
block_comment "/*"
new_line "\n"
spaces [ \t\r\v]
undefined [$_~?#&|!]

%x BLOCK_COMMENT
%x NESTED_COMMENT
%x STRINGSTATE

%%
{if}            {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"if","KEYWORD IF");
                    return IF;
                }

{else}          {  
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "else", "KEYWORD ELSE");
                    return ELSE;
                }

{while}         {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber,"while", "KEYWORD WHILE");
                    return WHILE;
                }
{for}           {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"for","KEYWORD FOR");
                    return FOR;
                }
{function}      {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"function","KEYWORD FUNCTION");
                    return FUNCTION;
                }
{return}        {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"return","KEYWORD RETURN");
                    return RETURN;
                }
{break}         {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"break","KEYWORD BREAK");
                    return BREAK;
                }
{continue}      {  
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"continue","KEYWORD CONTINUE");
                    return CONTINUE;
                }
{and}           {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"and","KEYWORD AND");
                    return AND;
                }
{not}           {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"not","KEYWORD NOT");
                    return NOT;
                }
{or}            {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"or","KEYWORD OR");
                    return OR;
                }
{local}         {  
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"local","KEYWORD LOCAL");
                    return LOCAL;
                }
{true}          {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"true","KEYWORD TRUE");
                    return TRUE;
                }
{false}         {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"false","KEYWORD FALSE");
                    return FALSE;
                }
{nil}           {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"nil","KEYWORD NIL");
                    return NIL;
                }

{assignment}    {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"=","OPERATOR EQUAL");
                    return ASSIGNOP;
                }
{plus}          {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"+","OPERATOR PLUS");
                    return PLUS;
                }   
{minus}         {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"-","OPERATOR MINUS");
                    return MINUS;
                }
{mult}          {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"*","OPERATOR MULTIPLY");
                    return MULTI;
                }
{div}           {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"/","OPERATOR DIVIDE");
                    return DIV;
                }
{mod}           {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"%","OPERATOR MOD");
                    return MOD;
                }
{double_equal}  {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"==","OPERATOR EQUAL EQUAL");
                    return EQUAL;
                }
{not_equal}     {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"!=","OPERATOR NOT EQUAL");
                    return NOTEQUAL;
                }
{plusplus}      {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"++","OPERATOR PLUS PLUS");
                    return DBPLUS;
                }
{minusminus}    {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"--","OPERATOR MINUS MINUS");
                    return DBMINUS;
                }
{greater_than}  {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,">","OPERATOR GREATER");
                    return GREATER;
                }
{less_than}     {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"<","OPERATOR LESSER");
                    return LESSER;
                }
{greater_equal} {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,">=","OPERATOR GREATER EQUAL");
                    return GREATEREQUAL;
                }
{less_equal}    {   
                    tokenNumber++;
                    add_token(yylineno,tokenNumber,"<=","OPERATOR LESSER EQUAL");
                    return LESSEREQUAL;
                }

{integer}       {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, yytext ,"CONST_INT");
                    yylval.intValue=atoi(yytext);
                    return INTEGER;
                }

{real}          {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, yytext, "REAL_CONST");
                    yylval.realValue=atof(yytext);
                    return REAL;
                }


{left_curly}    {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "{", "PUNCTUATION LEFT BRACE");
                    return LEFTCURLY;
                }
{right_curly}    {  
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "}", "PUNCTUATION RIGHT BRACE");
                    return RIGHTCURLY;
                }
{left_bracket}  {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "[", "PUNCTUATION LEFT BRACKET");
                    return LEFTBRACKET;
                }
{right_bracket} {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "]", "PUNCTUATION RIGHT BRACKET");
                    return RIGHTBRACKET;
                }
{left_parenthesis} {
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "(", "PUNCTUATION LEFT PARENTHESIS");
                    return LEFTPAREN;
                }
{right_parenthesis} { 
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, ")", "PUNCTUATION RIGHT PARENTHESIS");
                    return RIGHTPAREN;
                }
{semicolon}     {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, ";", "PUNCTUATION SEMICOLON");
                    return SEMICOLON;
                }
{comma}         {  
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, ",", "PUNCTUATION COMMA");
                    return COMMA;
                }
{colon}         {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, ":", "PUNCTUATION COLON");
                    return COLON;
                }
{double_colon}  {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "::", "PUNCTUATION DOUBLE COLON");
                    return DOUBLECOLON;
                }
{dot}           {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, ".", "PUNCTUATION DOT");
                    return DOT;
                }
{double_dot}    {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "..", "PUNCTUATION DOT DOT");
                    return DOUBLEDOT;
                }
{identifier}    {   
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, yytext, "IDENTIFIER");
                    yylval.stringValue=strdup(yytext);
                    return IDENTIFIER;

                }
{line_comment}  {
                    
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, "", "LINE_COMMENT");
                }

{new_line}      {}

{spaces}        {}

"/*"            {   
                    comment_counter++;
                    BEGIN(BLOCK_COMMENT); 
                    block_comment_start_line=yylineno;
                }

<BLOCK_COMMENT>"*/" {   comment_counter--;
                        tokenNumber++; 
                        block_comment_end_line=yylineno;
                        sprintf(bcsl_arr,"%d",block_comment_start_line);
                        sprintf(bcel_arr,"%d",block_comment_end_line);
                        strcat(bcsl_arr,"-");
                        strcat(bcsl_arr,bcel_arr);
                        add_token(yylineno, tokenNumber,bcsl_arr, "BLOCK_COMMENT"); 
                        BEGIN(INITIAL); 
                    }

<BLOCK_COMMENT>\n   { }
<BLOCK_COMMENT>.    { }
<BLOCK_COMMENT>"/*" {   comment_counter++;
                        BEGIN(NESTED_COMMENT); 
                    }

<BLOCK_COMMENT><<EOF>> {printf("ERROR NOT CLOSING COMMENT.EXITING...");  exit(EXIT_FAILURE);}


<NESTED_COMMENT>"/*" { 
                       comment_counter++;
                       BEGIN(NESTED_COMMENT); 
                     }


<NESTED_COMMENT>"*/" {  comment_counter--; 
                        tokenNumber++; 
                        add_token(yylineno, tokenNumber, "", "NESTED_COMMENT"); 
                        if(comment_counter==1){
                            BEGIN(BLOCK_COMMENT);
                        }
                     }

<NESTED_COMMENT>\n   { }
<NESTED_COMMENT>.    { }
<NESTED_COMMENT><<EOF>> { printf("ERROR NOT CLOSING COMMENT.EXITING...");  exit(EXIT_FAILURE);}

                
				
{string}     {
                BEGIN(STRINGSTATE);
              }

<STRINGSTATE>[^\\\"]+   {   
                        char *yptr = yytext;
                        while ( *yptr ){
                            stringArr[stringCntr++]=*yptr++;
                        }    
                     }    

<STRINGSTATE>\\n   {
                stringArr[stringCntr++]='\n';
              }

<STRINGSTATE>\\t   {
                stringArr[stringCntr++]='\t';
              }
<STRINGSTATE>\\\\    {
                stringArr[stringCntr++]='\\';
               }

<STRINGSTATE>\\\"   {
                  stringArr[stringCntr++]='\"';
                }

 <STRINGSTATE>\\[^nt\\]  {   
                        printf("AT LINE :%d ERROR BAD ESCAPE CHARACTER:%s\n EXITING...\n",yylineno,yytext);  
                        exit(EXIT_FAILURE);
                    }           

<STRINGSTATE>{string} {
                    stringArr[stringCntr++]='\0';
                    tokenNumber++;
                    add_token(yylineno, tokenNumber, stringArr, "STRING");

                    yylval.stringValue=strdup(stringArr);

                    stringCntr=0;
                    memset(stringArr,'0',5000);

                    

                    BEGIN(INITIAL);
                    return STRING;
                }

<STRINGSTATE><<EOF>> { 
                  printf("ERROR NOT CLOSING STRING.EXITING...\n");  
                  exit(EXIT_FAILURE); 
                 }   
           

.               {  
                    printf("AT LINE :%d ERROR UNDEFINED CHARACTER:%s\n",yylineno,yytext); 
                    exit(EXIT_FAILURE);
                }


                