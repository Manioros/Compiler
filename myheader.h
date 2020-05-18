#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int yylex();
extern char* yytext;
extern FILE* yyin;
extern FILE* yyout;

struct alpha_token_t {
	unsigned int  numline;
	unsigned int  numToken;
	char          *content;
	char          *type;
	struct alpha_token_t *next; 
};


int comment_counter;


void add_token(unsigned int  numline,unsigned int  numToken,char * content,char *type);

void print_list();

void file_print_list(FILE *input_file);
