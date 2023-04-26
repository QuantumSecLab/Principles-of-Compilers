#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

char token; /* global token variable */

/*function prototype for recursive calls*/
int exp(void);
int term(void);
int factor(void);

void error(void)
{
	fprintf(stderr, "error\n");
	exit(1);
}

void match(char expectedToken)
{
	if (token == expectedToken) token = getchar();
	else error();
}
int exp(void)
{
	int temp = term();
	while ((token == '+') || (token == '-'))
		switch (token)
		{
		case '+': 
			match('+'); 
			temp += term(); 
			break;
		case '-': 
			match('-'); 
			temp -= term(); 
			break;
		}
	return temp;
}

int term(void)
{
	int temp = factor();
	while (token == '*')
	{
		match('*');
		temp *= factor();
	}
	return temp;
}

int factor(void)
{
	int temp;
	if (token == '(')
	{
		match('(');
		temp = exp();
		match(')');
	}
	else
		if (isdigit(token))
		{
			ungetc(token, stdin);
			scanf("%d", &temp);
			token = getchar();
		}
		else error();
	return temp;
}

// get a random number from [lower, upper]
int getRandomNumber(int lower, int upper)
{
	if (lower <= upper)
		return rand() % (upper - lower + 1) + lower;
	else
		return rand() % (lower - upper + 1) + upper;
}

int printQuiz()
{

}

int printAnswer()
{

}

int main()
{
	srand((unsigned int)time(NULL));
	int result;
	/*load token with first character for lookahead*/
	token = getchar();
	result = exp();
	if (token == '\n') /*check for end of line*/
		printf("Result = % d\n", result);
	else error(); /*extraneous chars on line*/
	return 0;
}