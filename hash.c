#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "ast.h"
#include "y.tab.h"

void astFind(int level, AST* node, char* text);
AST* getAST();
int temp_Flag = 0; 


void hashInit (void){
    int i;
    for (i = 0; i < HASH_SIZE; ++i){
        Table[i] = 0;
    }
}

int hashAddress(char *text){
    int i;
    int address = 1;
    for(i = 0; i < strlen(text); ++i){
        address = (address*text[i])%HASH_SIZE + 1;
    }
    return address - 1;
}

HASH_NODE* hashInsert(int type, char *text){
    HASH_NODE *newnode;
    int address;
    address = hashAddress(text);

    if((newnode = hashFind(text)))
        return newnode;

    //fprintf(stderr, "%d, type %s \n", type, text);

    newnode = (HASH_NODE*) calloc(1, sizeof(HASH_NODE));
    newnode->type = type;

    newnode->isLiteral = 0;
    newnode->assemblyLabel = 0;


    if(type == LIT_INTEGER)
    {
        newnode->datatype = DATATYPE_INT;
        newnode->isLiteral = 1;
    }
    else if(type == LIT_FLOAT)
    {
        newnode->datatype = DATATYPE_FLOAT;
        newnode->isLiteral = 1;
    }
    else if(type == LIT_CHAR)
    {
        newnode->datatype = DATATYPE_CHAR;
        newnode->isLiteral = 1;
    }

    if (temp_Flag == 1)
    { 
        newnode->datatype = DATATYPE_TEMP;
        temp_Flag = 0;
    }

    //fprintf(stderr, "[HASH] Text: %s; Type: %d\n", text, type);

    newnode->text = calloc(strlen(text)+1, sizeof(char));
    strcpy(newnode->text, text);
    newnode->next = Table[address];
    Table[address] = newnode;
    return newnode;
}

void hashSetType(char * text, int type){
    HASH_NODE *newnode;

    newnode = hashFind(text);

    newnode->type = type;
}

HASH_NODE* hashFind(char *text){
    HASH_NODE *node;
    int address;
    address = hashAddress(text);

    for(node = Table[address]; node; node = node->next){
        if(!strcmp(text, node->text)){
            return node;
        }
    }

    return 0;
}

void hashPrint(void){
    HASH_NODE *node;

    int i;
    for (i = 0; i < HASH_SIZE; ++i){
        for (node = Table[i]; node; node = node->next){
            fprintf(stderr, "Table[%d] has %s, type %d \n", i, node->text, node->type);
        }
    }
}

void hashCheckUndeclared(void)
{
    HASH_NODE *node;

    int i;

    for(i =0; i < HASH_SIZE; i++)
    {
        for(node = Table[i]; node; node = node->next)
        {
            astFind(0, getAST(), node->text);
            
            if(node->type == TK_IDENTIFIER)
            {
                fprintf(stderr, "[SEMANTIC] Variable %s not declared!\n", node->text);
                exit(4);
            }
            else if(node->type == LIT_CHAR || node->type == LIT_INTEGER || node->type == LIT_FLOAT )
            {
                switch(node->type)
                {
                    case LIT_CHAR: node->datatype = DATATYPE_INT; break;
                    case LIT_INTEGER: node->datatype = DATATYPE_INT; break;
                    case LIT_FLOAT: node->datatype = DATATYPE_FLOAT; break;
                }

                node->type = SYMBOL_SCALAR;
            }
        }
    }
}

HASH_NODE* makeTemp()
{
    temp_Flag = 1;
    static int serial = 0;
    static char name[100];
    sprintf(name, "GerofrigoliciousTemp%d", serial++);
    return hashInsert(SYMBOL_TEMP, name);
}

HASH_NODE* makeLabel()
{
    static int serial = 0;
    static char name[100];
    sprintf(name, "Label%d", serial++);
    return hashInsert(SYMBOL_LABEL, name);
}
