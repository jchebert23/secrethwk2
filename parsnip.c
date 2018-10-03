#include "/c/cs323/Hwk2/parsnip.h"
#define _GNU_SOURCE
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

int debugPrint1=0;
int debugPrint2=0;
typedef struct sortedTokens{
    int curIndex;
    int sizeOfOutputArr;
    int error;
    int *argSize;
    int *localSize;
    token *input;
    token **args;
    token **locals;
    token *first;
    token *second;
    token *third;
    int errorRedirection;
    int inputRedirection;
    int outputRedirection;
} sortedTokens;

char * copyof(char *str)
{
	//IMPORTANT, error if str is null
	int length=strlen(str);
	char *c=malloc(sizeof(char)*(length+1));
	strcpy(c, str);
	c[length]='\0';
	return c;

}

token * initializeToken(int type, char *text)
{
	token *t=malloc(sizeof(token));
	t->type=type;
	t->text=text;
	return t;

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
	sortedTokenObject->first=0;
	sortedTokenObject->second=0;
	sortedTokenObject->third=0;
	sortedTokenObject->errorRedirection=0;
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

void destroySortedToken(sortedTokens *s, int fullDelete)
{
	destroyTokenArray(s->args, s->argSize);
	destroyTokenArray(s->locals, s->localSize);
	if(s->first)
	{
		free(s->first);
	}
	if(s->second)
	{
		free(s->second);
	}
	if(s->third)
	{
		free(s->third);
	}
	if(fullDelete)
	{	
	free(s->localSize);
	free(s->argSize);
	free(s);
	}
	else
	{
	*(s->localSize)=0;
	*(s->argSize)=0;
	s->first=0;
	s->second=0;
	s->third=0;
	s->inputRedirection=0;
	s->outputRedirection=0;
	s->errorRedirection=0;
	s->args=0;
	s->locals=0;
	}

}

void addExtraToken(sortedTokens *s, token **arr, int character, int delete)
{

	    s->sizeOfOutputArr++;
	    if((*arr))
	    {
	    (*arr)=realloc(*arr, sizeof(token) * (s->sizeOfOutputArr));
	    }
	    else
	    {
		    (*arr)=malloc(sizeof(token));
	    }
	    (*arr)[s->sizeOfOutputArr-1].type=character;
	    (*arr)[s->sizeOfOutputArr-1].text=0;
	    s->curIndex++;
	    if(delete)
	    {
	    destroySortedToken(s, 0);
	    }
}

void addToArr(sortedTokens*s, token **arr)
{
	if(debugPrint1)
	{
		printf("Line %d: in addtoarr\n", __LINE__);
		printf("Size of args: %d\n", *(s->argSize));
	}
	int newSize= *(s->argSize) + *(s->localSize);
	//IMPORTANT HAVE TO DEAL WITH WHEN output redirection is just a pipe
	if(s->outputRedirection)
	{
	newSize+=1;
	}
	if(s->inputRedirection)
	{
	newSize+=1;
	}
	if(s->errorRedirection)
	{
	newSize+=1;
	}

	if(newSize)
	{
	if(debugPrint1)
	{
		printf("NEW SIZE: %d\n", newSize);
	}
	int curIndex=0;
	s->sizeOfOutputArr+=newSize;
	if((*arr))
	{
		curIndex=s->sizeOfOutputArr-newSize;
		(*arr)=realloc((*arr), sizeof(token)*s->sizeOfOutputArr);
	}
	else
	{
	    (*arr)=malloc(sizeof(token)*(newSize));
	}
	    for(int i=0; i<*(s->argSize); i++)
	    {
		    (*arr)[curIndex].type=s->args[i]->type;
		    (*arr)[curIndex].text=s->args[i]->text;
		    if(debugPrint1)
		    {
			    printf("Line %d, added toke of type: %d, and text: %s\n", __LINE__, (*arr)[i].type, (*arr)[i].text); 
		    }
		    curIndex++;
	    }
	    if(*(s->localSize)!=0)
	    {
		    int x=0;
		    for(int i=*(s->argSize); i<*(s->argSize)+*(s->localSize);i++)
		    {
			    (*arr)[curIndex].type=s->locals[x]->type;
			    (*arr)[curIndex].text=s->locals[x]->text;
			    if(debugPrint1)
			    {
			    printf("Line %d, added toke of type: %d, and text: %s\n", __LINE__, (*arr)[i].type, (*arr)[i].text); 
			    }
			    curIndex++;
			    x++;
		    }
	    }

	
	//IMPORTANT HAVE TO DEAL WITH HANDLING WHICH REDIRECTION CAME FIRST
	if(s->first)
	{
		(*arr)[curIndex].type=s->first->type;
		(*arr)[curIndex].text=s->first->text;
		
			    if(debugPrint1)
			    {
			    printf("Line %d, added toke of type: %d, and text: %s\n", __LINE__, (*arr)[curIndex].type, (*arr)[curIndex].text); 
			    }
		curIndex++;
	}
	if(s->second)
	{
		(*arr)[curIndex].type=s->second->type;
		(*arr)[curIndex].text=s->second->text;

			    if(debugPrint1)
			    {
			    printf("Line %d, added toke of type: %d, and text: %s\n", __LINE__, (*arr)[curIndex].type, (*arr)[curIndex].text); 
			    }
		curIndex++;
	}
	if(s->third)
	{

		(*arr)[curIndex].type=s->third->type;
		(*arr)[curIndex].text=s->third->text;
		
			    if(debugPrint1)
			    {
			    printf("Line %d, added toke of type: %d, and text: %s\n", __LINE__, (*arr)[curIndex].type, (*arr)[curIndex].text); 
			    }
	}

	
	if(debugPrint1)
	{
		printf("Line %d: at end of addtorarr\n", __LINE__);
	}
	}
}

void command(sortedTokens*s, token **arr);
char *hereDocument(char *text);
token * parse(token *tok){
    sortedTokens *sortedToken=initializeSortedToken();
    sortedToken->input=tok;
    token *arr=0;
    if(debugPrint1)
    {
	    dumpList(tok);
    }
    command(sortedToken, &arr);
    if(debugPrint1)
    {
	    printOutputArr(sortedToken, &arr);
    }
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
    if(sortedToken->input[sortedToken->curIndex].type!=NONE && sortedToken->error==0)
    {
	    fprintf(stderr, "Error input not on none token at end\n");
	    sortedToken->error=1;
    }
    int index;
    if(sortedToken->error==1)
    {
	    if(arr)
	    {
		    freeList(arr);
	    }
	    return 0;
    }

    destroySortedToken(sortedToken,1);

	    index=0;
	    while(arr[index].type!=NONE)
	    {
		    if(arr[index].type==RED_IN_HERE)
		    {
			    arr[index].text=hereDocument(arr[index].text);
		    }
		    index++;
	    }
    
    
    
    
    return arr;
}

void combineArrays(sortedTokens *s,token **arr, token *arr2)
{
    int i=0;
    while(arr2[i].type!=NONE)
    {
	    i++;
    }
    (*arr)=realloc((*arr), sizeof(token) * (s->sizeOfOutputArr+i));
    i=0;
    while(arr2[i].type!=NONE)
    {

	    (*arr)[s->sizeOfOutputArr].type=arr2[i].type;

	    (*arr)[s->sizeOfOutputArr].text=arr2[i].text;
	    s->sizeOfOutputArr++;
	    i++;
    }

}

int same(char *one, char *two)
{
	if(strlen(one)+1==strlen(two) && two[strlen(two)-1]=='\n')
	{
		for(int i=0; i<strlen(one); i++)
		{
			if(one[i]!=two[i])
			{
				return 0;
			}
		}
		return 1;
	}
	else if(strlen(one)==strlen(two))
	{
	    
		for(int i=0; i<strlen(one); i++)
		{
			if(one[i]!=two[i])
			{
				return 0;
			}
		}
		return 1;
	}
	else
	{
		return 0;
	}
}


token ** firstSecondOrThird(sortedTokens *s)
{
	if(s->first)
	{
		if(s->second)
		{
			return &(s->third);
		}
		else
		{
			return &(s->second);
		}
	}
	else
	{
	    return &(s->first);
	}
}

int tooManyHere(sortedTokens *s)
{
	int curIndex=s->curIndex+1;
	while(s->input[curIndex].type!=NONE)
	{
	    if(s->input[curIndex].type==RED_IN || s->input[curIndex].type==RED_IN_HERE)
	    {
		return 1;
	    }
	    else if(s->input[curIndex].type==PAR_LEFT)
	    {
		    curIndex++;
		    while(s->input[curIndex].type!=PAR_RIGHT)
		    {
			    if(s->input[curIndex].type==NONE)
			    {
				    return 1;
			    }
			    curIndex++;
		    }
	    }
	    else if(s->input[curIndex].type==PIPE || s->input[curIndex].type==SEP_AND || s->input[curIndex].type==SEP_OR ||s->input[curIndex].type==SEP_END  ||s->input[curIndex].type==SEP_BG)
	    {
		    return 0;
	    }
	    else
	    {
		    curIndex++;
	    } 
	}
	return 0;
}


char * hereDocument(char *text)
{

	    char *stdinString=malloc(sizeof(char)*1);
	    char *outputString=malloc(sizeof(char));
	    size_t size=1;
	    int oldSize=0;
	    int endOfFile;
	    endOfFile=getline(&stdinString, &size, stdin);
	    //IMPORTANT, make sure to not add above limit for stdin, after been expanded
	    while(same(text,stdinString)==0 && endOfFile!=-1 )
	    {
		    int index=0;
		    char *curWord;
		    while(index<endOfFile-1)
		    {
			    int lengthOfWord=0;
			    while(strchr(VARCHR, stdinString[index]) || (stdinString[index]=='$' && lengthOfWord==0))
			    {
				    lengthOfWord++;
				    index++; 
			    }

			    curWord=malloc(sizeof(char)*(lengthOfWord+1));
			    index=index-lengthOfWord;
			    int i=0;
			    while(strchr(VARCHR, stdinString[index]) || (stdinString[index]=='$' && i==0))
			    {
				    curWord[i]=stdinString[index];
				    i++;
				    index++;
			    }
			    curWord[lengthOfWord]='\0';
			    char *envVar=0;
			    if(curWord[0]=='$')
			    {
				    curWord++;	
				    envVar=getenv(curWord);
			    }
			    if(envVar)
			    {
				    lengthOfWord=strlen(envVar);
				    free(curWord-1);
				    curWord=malloc(sizeof(char) * (lengthOfWord+1));
				    strcpy(curWord,envVar);
				    curWord[lengthOfWord]='\0';
			    }
			    outputString=realloc(outputString, sizeof(char) * (lengthOfWord+oldSize));
			    for(int i=oldSize; i<lengthOfWord+oldSize; i ++)
			    {
				    outputString[i]=curWord[i-oldSize];
			    }
			    oldSize+=lengthOfWord;
			    int escapeCharacter=0;
			    while(strchr(VARCHR, stdinString[index])==0 && stdinString[index]!='\n' )
			    {
				    if(escapeCharacter && (stdinString[index]=='$' || stdinString[index]=='\\'))
				    {
				    escapeCharacter=0;
				    outputString[oldSize-1]=stdinString[index];
				    }
				    else
				    {
				    if(stdinString[index]=='\\')
				    {
					    escapeCharacter=1;
				    }
				    outputString=realloc(outputString, sizeof(char) * (oldSize+1));
				    outputString[oldSize]=stdinString[index];
				    oldSize++;
				    }
				    index++;
			    }
			    free(curWord);

		    }
		    outputString=realloc(outputString, sizeof(char) * (oldSize+1));

		    outputString[oldSize]='\n';
		    oldSize++;
		    endOfFile=getline(&stdinString, &size, stdin);
	    }

	    outputString=realloc(outputString, sizeof(char) * (oldSize+1));
	    outputString[oldSize]='\0';
	    return outputString;
}


int redirect(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in redirect\n", __LINE__);
    }
    int type=s->input[s->curIndex].type;
    token **output=firstSecondOrThird(s);
    if(type==RED_OUT || type==RED_OUT_APP) 
    {
	    //IMPORTANT HAVE TO CHECK FOR DIFFERENCE B/T output and error redirection
	    if(s->outputRedirection)
	    {
		    //IMPORTANT WHAT IF TRY TO OUPUT TWICE TO SAME PLACE
		    s->error=1;
		    fprintf(stderr, "trying too many output redirections\n");
		    return 0;

	    }
	    s->outputRedirection=1;
	    (*output)=malloc(sizeof(token));
	    (*output)->type=type;
		    s->curIndex++;
		    if(s->input[s->curIndex].type!=SIMPLE)
		    {
			    s->error=1;
			    fprintf(stderr, "file arg needed\n");
			    (*output)->text=0;
			    return 0;
		    }
		    (*output)->text = copyof(s->input[s->curIndex].text);
	    if(s->error!=1)
	    {
	    s->curIndex++;
	    }
    }
    else if(type==RED_ERR_OUT || type==RED_ERR_CLS)
    {
	    if(s->errorRedirection)
	    {
		    s->error=1;
		    fprintf(stderr, "trying too many error redirection\n");
		    return 0;
	    }
	    s->errorRedirection=1;
	    s->curIndex++;
	    (*output)=malloc(sizeof(token));
	    (*output)->type=type;
	    (*output)->text=0;
    }
    else if(type==RED_IN || type==RED_IN_HERE)
    {
	    if(s->inputRedirection)
	    {
		    s->error=1;
		    fprintf(stderr, "trying too many input redirection\n");
		    return 0;
	    }
	    if(type==RED_IN_HERE && tooManyHere(s))
	    {
		    s->error=1;
		    fprintf(stderr, "try too many input redirections\n");
		    return 0;
	    }
	    s->inputRedirection=1;
	    s->curIndex++;

	    if(s->input[s->curIndex].type!=SIMPLE)
	    {
		    s->error=1;
		    fprintf(stderr, "file arg needed\n");
		    return 0;
	    }
	    //HERE STUFF
	    (*output)=malloc(sizeof(token));
	    (*output)->type=type;
	    (*output)->text= copyof(s->input[s->curIndex].text);
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
if(s->input[s->curIndex].type!=SIMPLE)
{
	s->error=1;
	fprintf(stderr, "Missing Argument\n");
}
else
{
char* text=s->input[s->curIndex].text;
s->args=addToTokenArray(s->args, ARG, copyof(text), s->argSize);
s->curIndex++;
}
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
		if(s->error==0)
		{
		suffix(s, arr);
		}
	}
	else if(type==PAR_LEFT)
	{
		s->error=1;
		//IMPORTANT: CAN REDIRECTION BE IN PARENS
		fprintf(stderr, "unexpected command\n");
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
		if(s->error==0)
		{
		prefix(s, arr);
		}
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



token * stage(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in stage\n", __LINE__);
    }
    prefix(s, arr);
    if(s->error==1)
    {
	    return 0;
    }
    if(s->input[s->curIndex].type==PAR_LEFT)
    {
	    token *arr2=0;

	    sortedTokens *s2=initializeSortedToken();
	    s2->curIndex=s->curIndex;
	    s2->input=s->input;
	    
	    addExtraToken(s2, &arr2, PAR_LEFT, 0);
	    command(s2, &arr2);
	    if(debugPrint1)
	    {
		    printf("Line %d, done with recursive command call\n", __LINE__);
	    }
	    s->curIndex=s2->curIndex;
	    if(debugPrint1)
	    {
		    printf("Current Index And Size of Output Arr, %d : %d\n", s->curIndex, s->sizeOfOutputArr);
	    }
	    //IMPORTANT ERROR IF NEXT THING IS NOT PAR RIGHT
	    //IMPORTANT HAVE TO FREE S2
	    if(s->input[s->curIndex].type==PAR_RIGHT)
	    {
		    addExtraToken(s2, &arr2, PAR_RIGHT,0);
	    }
	    else
	    {
		    s->error=1;
		    fprintf(stderr, "Missing Closing Parenthesis\n");
		    return 0;
	    }
	    s->curIndex++;
	    addExtraToken(s2, &arr2, NONE, 0);
	    destroySortedToken(s2, 1);
	    redlist(s, arr);
	    return arr2;
    }
    else
    {
	    argument(s, arr);
	    if(s->error==1)
	    {
		    return 0;
	    }
	    suffix(s, arr);
	    if(s->error==1)
	    {
		    return 0;
	    }
    }
    return 0;

}


void pipeline(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in pipeline\n", __LINE__);
    }

    token *arr2=stage(s, arr);
    addToArr(s, arr);
    if(arr2)
    {
	    if(debugPrint1)
	    {
		    printf("about to combine ARRAYS!!!\n");
	    }
	    combineArrays(s,arr, arr2);
	    free(arr2);
    }

    if(debugPrint1)
    {
	    printf("About to print to ouput arr\n");
	    printOutputArr(s,arr);
    }
    while(s->input[s->curIndex].type==PIPE)
    {
	    if(s->outputRedirection)
	    {
		    s->error=1;
		    fprintf(stderr, "trying to pipe when output already redirected\n");
		    break;
	    }
	    addExtraToken(s, arr, PIPE,1);
	    arr2=stage(s, arr);
	    if(s->error==1)
	    {break;}
	    
	    addToArr(s, arr);

	    if(arr2)
	    {
		    combineArrays(s, arr, arr2);
		    free(arr2);
	    }
	    
    }
    if(debugPrint1)
    {
	    printf("LINE %d, end of pipeline\n", __LINE__);
    }

}



void andOr(sortedTokens *s, token **arr)
{

    if(debugPrint1)
    {
	    printf("Line Number: %d, in andOr\n", __LINE__);
    }
   pipeline(s, arr);
   int type= s->input[s->curIndex].type;
   if(s->error==1)
    {
	    type=0;
    }
   while(type==SEP_AND || type==SEP_OR)
   {
	   addExtraToken(s, arr, type,1);
	   pipeline(s, arr);
	   if(s->error==1){break;}
	   type=s->input[s->curIndex].type;
   }
   if(debugPrint1)
   {
	   printf("Line Number: %d, end of andOr\n", __LINE__);
   }
}

void list(sortedTokens *s, token **arr)
{
    
    if(debugPrint1)
    {
	    printf("Line Number: %d, in list\n", __LINE__);
    }
    andOr(s, arr);
    int type= s->input[s->curIndex].type;
    if(s->error==1){type=0;}
    while((s->input[s->curIndex].type==SEP_END || s->input[s->curIndex].type==SEP_BG) && s->input[s->curIndex+1].type!=NONE && s->input[s->curIndex+1].type!=PAR_RIGHT)
    {
	addExtraToken(s, arr, type,1);
	andOr(s, arr);
	if(s->error==1){break;}
	type=s->input[s->curIndex].type;
	if(debugPrint1)
	{
		printf("TYPE IN LIST:%d\n", type);
	}
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
    if(s->error!=1)
    {
	    int type= s->input[s->curIndex].type;
	    if(type==SEP_END || type==SEP_BG)
	    {
		    addExtraToken(s, arr, type,1);
	    }
	    else
	    {
		    
		    (*arr)=realloc((*arr), sizeof(token) * (s->sizeOfOutputArr+1));
		    s->sizeOfOutputArr++;
		    (*arr)[s->sizeOfOutputArr-1].type=SEP_END;
		    (*arr)[s->sizeOfOutputArr-1].text=0;
	    }
    }
}





