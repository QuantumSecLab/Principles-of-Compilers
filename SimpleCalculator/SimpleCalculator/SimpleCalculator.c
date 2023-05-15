#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>

char token; /* global token variable */

/*function prototype for recursive calls*/
int exp(void);
int term(void);
int factor(void);

/*********************************************************************************
 * description: get a random number from [lower, upper]
 * 
 * arguments:
 * int lower: the lower boundery of the range where the number will be generated
 * int upper: the upper boundery of the range where the number will be generated
 * 
 * return:
 * a random number in the range [lower, upper]
 *********************************************************************************/
int getARandomNumber(int lower, int upper)
{
	if (lower <= upper)
		return rand() % (upper - lower + 1) + lower;
	else
		return rand() % (lower - upper + 1) + upper;
}

/*******************************************************************
 * description: get a random operator from {'+', '-', '*', '/'}
 *
 * return:
 * a random operator
 *******************************************************************/
char getARandomOperator()
{
	int temp = rand((unsigned int)time(NULL)) % 4;
	switch (temp)
	{
	case 0:
		return '+';
	case 1:
		return '-';
	case 2:
		return '*';
	case 3:
		return '/';
	}
}

/**********************************************************************
 * description: convert the given decimal digit to its character form
 * 
 * argument:
 * int digit: a decimal digit
 * 
 * return:
 * the decimal digit in its character form
 **********************************************************************/
char digit2Char(int digit)
{
	if (digit >= 0 && digit <= 9) return (char)digit + '0';
	else
	{
		fprintf(stderr, "The given digit is not in the range [0, 9].\n");
		exit(1);
	}
}

/****************************************************************
 * description:
 * put a number to the `expression`
 *
 * argument:
 * int number: the number which will be put into the `expression`
 * char* expression: the dst expression buffer where the number will be put
 * size_t currentIndex: the position where the next character will be put
 ****************************************************************/
void putANumber(int number, char* expression, size_t* currentIndex)
{
	char buffer[10000] = { '\0' };
	int length = 0;
	// put the number into the `buffer`
	do
	{
		buffer[length++] = digit2Char(number % 10);
		number = number / 10;
	} while (number);

	// put the characters into the `expression` from the `buffer` reversely
	while (length) expression[(*currentIndex)++] = buffer[--length];
}

/****************************************************************
 * description: 
 * get an randomly generated expression 
 * 
 * argument:
 * size_t numberOfOperators: number of operators in the expression
 * 
 * return:
 * char* expression: a string representing the expression
 ****************************************************************/ 
char* getAnExpression(size_t numberOfOperators)
{
	// allocate the heap memory for the expression to be returned
	char* expression = (char*)malloc(numberOfOperators + 3 * (numberOfOperators + 1));
	if (!expression) 
	{
		fprintf(stderr, "Cannot allocate the memory.\n");
		exit(1);
	}
	memset(expression, 0, numberOfOperators + 3 * (numberOfOperators + 1));
	
	// generate the expression
	size_t expressionIndex = 0;
	for (size_t i = 0; i < numberOfOperators; i++)
	{
		// generate a random number from [0, 100] and put the it into the `expression`
		putANumber(getARandomNumber(0, 100), expression, &expressionIndex);
		// put a random operator to the `expression`
		expression[expressionIndex++] = getARandomOperator();
	}
	// put the last operand to the expression
	putANumber(getARandomNumber(0, 100), expression, &expressionIndex);

	return expression;
}

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

//int main()
//{
//	srand((unsigned int)time(NULL));
//	int result;
//	/*load token with first character for lookahead*/
//	token = getchar();
//	result = exp();
//	if (token == '\n') /*check for end of line*/
//		printf("Result = % d\n", result);
//	else error(); /*extraneous chars on line*/
//	return 0;
//}

int main()
{
	srand((unsigned int)time(NULL));

	printf("%s\n", getAnExpression(10));

	return 0;
}