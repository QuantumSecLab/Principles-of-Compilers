#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#define TRUE 1
#define FALSE 0

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

/*****************************************************************************
 * description:
 * put a number to the `expression`
 *
 * argument:
 * int number: the number which will be put into the `expression`
 * char* expression: the dst expression buffer where the number will be put
 * size_t currentIndex: the position where the next character will be put
 *****************************************************************************/
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

/************************************************
 * description:
 * test whether a character is an operator
 *
 * argument:
 * char character: the character to be tested
 * 
 * return:
 * TRUE: the given character is an operator
 * FLASE: the given character is not an operator
 ************************************************/
int isOperator(char character)
{
	switch (character)
	{
	case '=':
	case '+':
	case '-':
	case '*':
	case '/':
		return TRUE;
	default:
		return FALSE;
	}
}

/*******************************************************************************************
 * description:
 * test whether the given expression is encompassed by parenthesis or contains "(number)"
 *
 * argument:
 * char* expression: the expression to be tested
 *
 * return:
 * TRUE: the given expression is a special one
 * FALSE: the given expression is not a special one
 *******************************************************************************************/
int isSpecialCase(char* expression)
{
	// test whether the `expression` is "(exp)"
	if (expression[0] == '(' && expression[strlen(expression) - 1] == ')') return TRUE;

	// test whether the `expression` contains "(number)"
	int leftFound = FALSE;
	for (int i = 0; i < strlen(expression); i++)
	{
		if (expression[i] == '(') leftFound = TRUE;
		else if (expression[i] == ')' && leftFound) return TRUE;
		else if (isOperator(expression[i])) leftFound = FALSE;
	}

	return FALSE;
}

/*********************************************************************************************
 * description: insert the `character` to the `index + 1` position of the string `expression`
 *
 * arguments:
 * char* expression: the dst string
 * char character: the character to be inserted to the `expression`
 * int index: the `index + 1` is the place for the `character` to be inserted
 * 
 * note:
 * the caller should guarantee that the `expression[strlen(expression)]` is available
 * and the `expression[strlen(expression) + 1]` should be reserved for '\0'
 *********************************************************************************************/
void insert(char* expression, char character, int index)
{
	// move the characters
	for (int i = strlen(expression) - 1; i > index; i--) expression[i + 1] = expression[i];
	// insert the character
	expression[index + 1] = character;

	return;
}

/********************************************************************
 * description: add a pair of parenthesis to the expression randomly
 * 
 * argument:
 * char* expression: the string which holds the expression
 * 
 * return:
 * the string which holds the expression
 ********************************************************************/
char* addAPairOfParenthesis(char* expression)
{
	// initialize the buffer
	size_t originalLength = strlen(expression);
	char* newExpression = (char*)malloc(originalLength + 2 + 2 + 1);
	if (!newExpression)
	{
		fprintf(stderr, "Cannot allocate the memory.\n");
		exit(1);
	}

	// add the parenthesis and exclude the special case, which is "(exp)"
	while (TRUE)
	{
		// reset the buffer
		memset(newExpression, 0, originalLength + 2 + 2 + 1);

		// put the sentinels
		newExpression[0] = '=';
		memcpy(newExpression + 1, expression, originalLength);
		newExpression[strlen(newExpression)] = '=';

		// put the left parenthesis
		int leftIndex = -1;
		int currentIndex = 0;
		while (TRUE)
		{
			// judge whether insert a left parenthesis
			if (isOperator(newExpression[currentIndex]) && isdigit(newExpression[currentIndex + 1]) && !(rand((unsigned int)time(NULL)) % 3))
			{
				insert(newExpression, '(', currentIndex);
				leftIndex = currentIndex + 1;
				currentIndex = (currentIndex + 1) % strlen(newExpression);
				break;
			}
			// update the current index
			currentIndex = (currentIndex + 1) % strlen(newExpression);
		}

		// put the right parenthesis
		while (TRUE)
		{
			// judge whether insert a right parenthesis
			if (isdigit(newExpression[currentIndex]) && isOperator(newExpression[currentIndex + 1]) && !(rand((unsigned int)time(NULL)) % 3))
			{
				insert(newExpression, ')', currentIndex);
				currentIndex = (currentIndex - leftIndex + 1) % (strlen(newExpression) - leftIndex) + leftIndex;
				break;
			}
			// update the current index
			currentIndex = (currentIndex - leftIndex + 1) % (strlen(newExpression) - leftIndex) + leftIndex;
		}

		// check whether the result is the special case
		newExpression[strlen(newExpression) - 1] = '\0';
		if (!isSpecialCase(newExpression + 1))
		{
			// copy the result to the dst
			memcpy(expression, newExpression + 1, strlen(newExpression + 1));
			// free the buffer
			free(newExpression);
			// return to the caller
			return expression;
		}
	}
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
	char* expression = (char*)malloc(numberOfOperators + 3 * (numberOfOperators + 1) + 2 + 1);
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

	printf("%s\n", addAPairOfParenthesis(getAnExpression(3)));

	return 0;
}