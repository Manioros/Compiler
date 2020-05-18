#include "myheader.h"

extern int alpha_yylex();

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





