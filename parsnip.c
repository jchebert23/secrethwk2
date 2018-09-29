#include "/c/cs323/Hwk2/parsnip.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <inttypes.h>
#include <errno.h>

int debugPrint1=1;
typedef struct sortedTokens{
    int curIndex;
    int sizeOfOutputArr;
    int error;
    int *argSize;
    int *localSize;
    token *input;
    token **args;
    token **locals;
    token *inputRedirection;
    token *outputRedirection;
} sortedTokens;

char * copyof(char *str)
{
	int length=strlen(str);
	char *c=malloc(sizeof(char)*(length+1));
	strcpy(c, str);
	c[length]='\0';
	return c;

}

sortedTokens * initializeSortedToken(void)
{

	sortedTokens *sortedTokenObject= malloc(sizeof(sortedTokens));
	sortedTokenObject->sizeOfOutputArr=0;
	sortedTokenObject->input=0;
	sortedTokenObject->argSize=malloc(sizeof(int));
	*(sortedTokenObject->argSize)=0;
	sortedTokenObject->localSize=malloc(sizeof(int));
	*(sortedTokenObject->localSize)=0;
	sortedTokenObject->curIndex=0;
	sortedTokenObject->args=0;
	sortedTokenObject->error=0;
	sortedTokenObject->locals=0;
	sortedTokenObject->inputRedirection=0;
	sortedTokenObject->outputRedirection=0;
	return sortedTokenObject;
}
void printOutputArr(sortedTokens *s, token **ar)
{
	token *arr =*ar;
	printf("PRINTING OUTPUT ARR: sizeOfOutputArr %d\n", s->sizeOfOutputArr);
	for(int i=0; i<s->sizeOfOutputArr; i++)
	{
		printf("Token Type: %d\nToken Text: %s\n", arr[i].type, arr[i].text);
	}
}
token ** addToTokenArray(token **array, int type, char *text,int *argOrLocalSize)
{
	*argOrLocalSize+=1;
	token *curToken=malloc(sizeof(token));
	curToken->type=type;
	curToken->text=text;
	if(array)
	{
		array=realloc(array, (*argOrLocalSize)*sizeof(token*));
	}
	else
	{
		array=malloc(sizeof(token*));
	}
	array[(*argOrLocalSize)-1]=curToken;
	return array;
}

void printTokenArray(token **array, int *size)
{
	for(int i=0; i<(*size); i++)
	{
		printf("Token type:%d\nToken text:%s\n", array[i]->type, array[i]->text);
	}

}

void destroyTokenArray(token **arr, int *size)
{
	if(debugPrint1)
	{
		printf("LINE %d: size of array:%d\n", __LINE__, *size);
	}
	for(int i=0; i<(*size); i++)
	{
		if(debugPrint1)
		{
			printf("LINE %d: token text:%s\n", __LINE__, arr[i]->text);
		}
		free(arr[i]);
	}
	free(arr);
}

void destroySortedToken(sortedTokens *s)
{
	destroyTokenArray(s->args, s->argSize);
	free(s->argSize);
	destroyTokenArray(s->locals, s->localSize);
	free(s->localSize);
	if(s->inputRedirection)
	{
		free(s->inputRedirection->text);
		free(s->inputRedirection);
	}
	if(s->outputRedirection)
	{
		free(s->outputRedirection->text);
		free(s->outputRedirection);
	}
	free(s);

}

void addToArr(sortedTokens*s, token **arr)
{
	if(debugPrint1)
	{
		printf("Line %d: in addtoarr\n", __LINE__);
		printf("Size of args: %d\n", *(s->argSize));
	}
	int newSize= *(s->argSize) + *(s->localSize);
	s->sizeOfOutputArr+=newSize;
	if((*arr))
	{
	}
	else
	{
	    (*arr)=malloc(sizeof(token)*(newSize));
	    for(int i=0; i<*(s->argSize); i++)
	    {
		    (*arr)[i].type=s->args[i]->type;
		    (*arr)[i].text=s->args[i]->text;
		    if(debugPrint1)
		    {
			    printf("Line %d, added toke of type: %d, and text: %s\n", __LINE__, (*arr)[i].type, (*arr)[i].text); 
		    }
	    }
	    if(*(s->argSize)<newSize)
	    {

		    for(int i=*(s->argSize); i<newSize;i++)
		    {

			    printf("Line %d, added toke of type: %d, and text: %s\n", __LINE__, (*arr)[i].type, (*arr)[i].text); 
			    (*arr)[i].type=s->locals[i]->type;
			    (*arr)[i].text=s->locals[i]->text;
		    }
	    }

	}
	if(debugPrint1)
	{
		printf("Line %d: at end of addtorarr\n", __LINE__);
	}
}

void command(sortedTokens*s, token **arr);

token * parse(token *tok){
    sortedTokens *sortedToken=initializeSortedToken();
    sortedToken->input=tok;
    token *arr=0;
    command(sortedToken, &arr);
    if(debugPrint1)
    {
	    printOutputArr(sortedToken, &arr);
    }
    if(tok)
    {
    arr=realloc(arr,sizeof(token)*(sortedToken->sizeOfOutputArr+1));
    arr[sortedToken->sizeOfOutputArr].type=NONE;
    arr[sortedToken->sizeOfOutputArr].text=0;
    }
    if(debugPrint1)
    {
	    printOutputArr(sortedToken, &arr);
	    dumpList(arr);
    }

    destroySortedToken(sortedToken);
    return arr;
}


int redirect(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in redirect\n", __LINE__);
    }
    int type=s->input[s->curIndex].type;
    if(type==RED_OUT || type==RED_OUT_APP || type==RED_ERR_OUT || type==RED_ERR_CLS)
    {

	    s->outputRedirection=malloc(sizeof(token));
	    s->outputRedirection->type=type;
	    if(type==RED_OUT || type==RED_OUT_APP)
	    {
		    s->curIndex++;
		    s->outputRedirection->text = copyof(s->input[s->curIndex].text);
	    }
	    else
	    {
	    s->outputRedirection->text=0;
	    }
	    s->curIndex++;
    }
    else if(type==RED_IN || type==RED_IN_HERE)
    {
	    s->curIndex++;
	    s->inputRedirection=malloc(sizeof(token));
	    s->inputRedirection->type=type;
	    s->inputRedirection->text= copyof(s->input[s->curIndex].text);
	    s->curIndex++;
    }
    else
    {
	    return 0;
    }
    return 1;	
}


void local(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in local\n", __LINE__);
    }
char* text=s->input[s->curIndex].text;
s->locals=addToTokenArray(s->locals, LOCAL, copyof(text), s->localSize);
s->curIndex++;
}

void argument(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in argument\n", __LINE__);
    }
char* text=s->input[s->curIndex].text;
s->args=addToTokenArray(s->args, ARG, copyof(text), s->argSize);
s->curIndex++;
}

void suffix(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in suffix\n", __LINE__);
    }
	int type=s->input[s->curIndex].type;
	if(type==SIMPLE)
	{
		argument(s, arr);
		suffix(s, arr);
	}
	else if(RED_OP(type))
	{
		redirect(s, arr);
		suffix(s, arr);
	}

}

void prefix(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in prefix\n", __LINE__);
    }
	int type=s->input[s->curIndex].type;
	char *text=s->input[s->curIndex].text;
	//HAVE TO CHECK FOR EXCEPTION HERE WHEN EQUALS IS LAST CHARACTER IN TEXT
	if(type==SIMPLE && strchr(text,'='))
	{
		local(s, arr);
		prefix(s, arr);
	}
	else if(RED_OP(type))
	{
		redirect(s, arr);
		prefix(s, arr);
	}

}



void redlist(sortedTokens *s, token **arr)
{  

    if(debugPrint1)
    {
	    printf("Line Number: %d, in redlist\n", __LINE__);
    }
    while(redirect(s,arr)){}
}



void stage(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in stage\n", __LINE__);
    }
    prefix(s, arr);
    if(s->input[s->curIndex].type==PAR_LEFT)
    {
	    command(s, arr);
	    redlist(s, arr);
    }
    else
    {
	    argument(s, arr);
	    suffix(s, arr);
    }

}


void pipeline(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in pipeline\n", __LINE__);
    }
    stage(s, arr);
    addToArr(s, arr);
    if(debugPrint1)
    {
	    printf("About to print to ouput arr\n");
	    printOutputArr(s,arr);
    }
    while(s->input[s->curIndex].type==PIPE)
    {
	    stage(s, arr);
    }

}



void andOr(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in andOr\n", __LINE__);
    }
   pipeline(s, arr);
   while(s->input[s->curIndex].type==SEP_AND || s->input[s->curIndex].type==SEP_OR)
   {
	   pipeline(s, arr);
   }
}

void list(sortedTokens *s, token **arr)
{
    
    if(debugPrint1)
    {
	    printf("Line Number: %d, in list\n", __LINE__);
    }
    andOr(s, arr);
    while(s->input[s->curIndex].type==SEP_END || s->input[s->curIndex].type==SEP_BG)
    {
	andOr(s, arr);
    }
}

void command(sortedTokens *s, token **arr)
{
    if(debugPrint1)
    {
	    printf("Line Number: %d, in command\n", __LINE__);
	    printOutputArr(s,arr);
    }
    list(s, arr);

}





