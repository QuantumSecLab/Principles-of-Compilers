#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#define TRUE 1
#define FALSE 0
#define NUMBER_OF_OPERATOR 10
#define NUMBER_OF_QUESTION 10000
#define RIGHT -1
#define LEFT -2

typedef struct
{
	char* rewrittenExpressions[NUMBER_OF_QUESTION];
	size_t currentRewrittenExpressionIndex;
	size_t currentRewrittenExpressionNextCharIndex;
} RewrittenExpressions;

typedef long long LL;

LL exp(char* expression, size_t* nextIndex, char* globalToken, int* isDividedByZero, int associativity, RewrittenExpressions* rewrittenExpressions);
LL factor(char* expression, size_t* nextIndex, char* globalToken, int* isDividedByZero, int associativity, RewrittenExpressions* rewrittenExpressions);
LL term(char* expression, size_t* nextIndex, char* globalToken, int* isDividedByZero, int associativity, RewrittenExpressions* rewrittenExpressions);
LL getInt(char* expression, size_t* nextIndex, char* globalToken, int associativity, RewrittenExpressions* rewrittenExpressions);

/*********************************************************************************
 * description: 
 * unget the current token. Actually just decrease the `nextIndex`
 *
 * arguments:
 * size_t* nextIndex: the position where the next character lies
 *********************************************************************************/
void ungetNextChar(size_t* nextIndex)
{
	if (*nextIndex > 0) --(*nextIndex);
	return;
}

/*********************************************************************************
 * description: 
 * get the next token from `expression[*nextIndex]`
 * then increase the `*nextIndex`
 *
 * arguments:
 * char* expression: a string which holds the current expression to be processed
 * size_t* nextIndex: the index of `expression` where the next token lies
 * 
 * return:
 * the character `expression[*nextIndex]`
 *********************************************************************************/
char getNextChar(char* expression, size_t* nextIndex)
{
	// does not reach the end of the line
	if (expression[*nextIndex]) return expression[(*nextIndex)++];
	// reach the end of the line
	else return expression[*nextIndex];
}

/*********************************************************************************
 * description: 
 * get a random number from [lower, upper]
 * 
 * arguments:
 * int lower: the lower boundery of the range where the number will be generated
 * int upper: the upper boundery of the range where the number will be generated
 * 
 * return:
 * a random number in the range [lower, upper]
 *********************************************************************************/
int getRandomNumber(int lower, int upper)
{
	if (lower <= upper)
		return rand() % (upper - lower + 1) + lower;
	else
		return rand() % (lower - upper + 1) + upper;
}

/*******************************************************************
 * description: 
 * get a random operator from {'+', '-', '*', '/'}
 *
 * return:
 * a random operator
 *******************************************************************/
char getRandomOperator()
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
 * description: 
 * convert the given decimal digit to its character form
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

/**********************************************************************
 * description: 
 * convert the given decimal digit character to its int form
 *
 * argument:
 * char character: a decimal digit in its character form
 *
 * return:
 * the int form of the given decimal digit character
 **********************************************************************/
int char2Digit(char character)
{
	if (character >= '0' && character <= '9') return (int)(character - '0');
	else
	{
		fprintf(stderr, "The given character is not a digit.\n");
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
void putNumber(int number, char* expression, size_t* currentIndex)
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
 * description: 
 * insert the `character` to the `index + 1` position of the string `expression`
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
 * description: 
 * add a pair of parenthesis to the expression randomly
 * 
 * argument:
 * char* expression: the string which holds the expression
 * 
 * return:
 * the string which holds the expression
 ********************************************************************/
char* addParenthesis(char* expression)
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
char* getExpression(size_t numberOfOperators)
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
		putNumber(getRandomNumber(0, 100), expression, &expressionIndex);
		// put a random operator to the `expression`
		expression[expressionIndex++] = getRandomOperator();
	}
	// put the last operand to the expression
	putNumber(getRandomNumber(0, 100), expression, &expressionIndex);
	
	// add a pair of parenthesis to the expression
	return addParenthesis(expression);
}

/****************************************************************
 * description:
 * handle the exception which occurs when parsing
 ****************************************************************/
void error(void)
{
	fprintf(stderr, "error\n");
	exit(1);
}

/****************************************************************
 * description:
 * match a token
 *
 * arguments:
 * char expectedToken: the expected token to be matched
 * char* expression: the current expression in process
 * size_t* nextIndex: the index where you will get the next token
 * from `expression`
 * char* globalToken: the current token
 ****************************************************************/
void match(char expectedToken, char* expression, size_t* nextIndex, char* globalToken)
{
	if (*globalToken == expectedToken) *globalToken = getNextChar(expression, nextIndex);
	else error();
}

/****************************************************************
 * description:
 * parse a factor
 *
 * arguments:
 * char* expression: the current expression in process
 * size_t* nextIndex: the index where you will get the next token
 * from `expression`
 * char* globalToken: the current token
 * int* isDividedByZero: the flag which indicates whether
 * the "divied by zero" exception happens when parsing
 * int associativity: LEFT or RIGHT
 * RewrittenExpressions* rewrittenExpressions:
 * if associativity == RIGHT, store rewritten expressions
 *
 * return:
 * the value of the factor
 ****************************************************************/
LL factor(char* expression, size_t* nextIndex, char* globalToken, int* isDividedByZero, int associativity, RewrittenExpressions* rewrittenExpressions)
{
	LL temp;
	if (*globalToken == '(')
	{
		match('(', expression, nextIndex, globalToken);
		// synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
		if (associativity == RIGHT) (rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
		temp = exp(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
		match(')', expression, nextIndex, globalToken);
		// synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
		if (associativity == RIGHT) (rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
	}
	else
		if (isdigit(*globalToken))
		{
			ungetNextChar(nextIndex);
			// synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
			if (associativity == RIGHT) (rewrittenExpressions->currentRewrittenExpressionNextCharIndex)--;
			temp = getInt(expression, nextIndex, globalToken, associativity, rewrittenExpressions);
			*globalToken = getNextChar(expression, nextIndex, globalToken);
			// synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
			if (associativity == RIGHT) (rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
		}
		else error();
	return temp;
}

/****************************************************************
 * description:
 * parse a term
 *
 * arguments:
 * char* expression: the current expression in process
 * size_t* nextIndex: the index where you will get the next token
 * from `expression`
 * char* globalToken: the current token
 * int* isDividedByZero: the flag which indicates whether
 * the "divied by zero" exception happens when parsing
 * int associativity: LEFT or RIGHT
 * RewrittenExpressions* rewrittenExpressions:
 * if associativity == RIGHT, store rewritten expressions
 *
 * return:
 * the value of the term
 ****************************************************************/
LL term(char* expression, size_t* nextIndex, char* globalToken, int* isDividedByZero, int associativity, RewrittenExpressions* rewrittenExpressions)
{
	switch (associativity)
	{
	case LEFT:
	{
		LL temp = factor(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
		while (*globalToken == '*' || *globalToken == '/')
		{
			if (*globalToken == '*')
			{
				match('*', expression, nextIndex, globalToken);
				temp *= factor(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
			}
			if (*globalToken == '/')
			{
				match('/', expression, nextIndex, globalToken);
				int divisor = factor(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
				if (divisor) temp /= divisor;
				else
				{
					temp /= 1;
					*isDividedByZero = TRUE;
				}
			}
		}

		return temp;
	}
	case RIGHT:
	{
		LL temp = factor(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
		while (*globalToken == '*' || *globalToken == '/')
		{
			if (*globalToken == '*')
			{
				match('*', expression, nextIndex, globalToken);

				// first, insert a '('
				insert(rewrittenExpressions->rewrittenExpressions[rewrittenExpressions->currentRewrittenExpressionIndex], '(', rewrittenExpressions->currentRewrittenExpressionNextCharIndex - 1);
				// skip the '('
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
				// then, synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;

				temp *= term(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);

				// first, insert a ')'
				insert(rewrittenExpressions->rewrittenExpressions[rewrittenExpressions->currentRewrittenExpressionIndex], ')', rewrittenExpressions->currentRewrittenExpressionNextCharIndex - 2);
				// skip the ')'
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
			}
			if (*globalToken == '/')
			{
				match('/', expression, nextIndex, globalToken);

				// first, insert a '('
				insert(rewrittenExpressions->rewrittenExpressions[rewrittenExpressions->currentRewrittenExpressionIndex], '(', rewrittenExpressions->currentRewrittenExpressionNextCharIndex - 1);
				// skip the '('
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
				// then, synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;

				LL divisor = term(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);

				// first, insert a ')'
				insert(rewrittenExpressions->rewrittenExpressions[rewrittenExpressions->currentRewrittenExpressionIndex], ')', rewrittenExpressions->currentRewrittenExpressionNextCharIndex - 2);
				// skip the ')'
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;

				if (divisor) temp /= divisor;
				else
				{
					temp /= 1;
					*isDividedByZero = TRUE;
				}
			}
		}

		return temp;
	}
	}
}

/****************************************************************
 * description:
 * parse an expression
 *
 * arguments:
 * char* expression: the current expression in process
 * size_t* nextIndex: the index where you will get the next token
 * from `expression`
 * char* globalToken: the current token
 * int* isDividedByZero: the flag which indicates whether 
 * the "divied by zero" exception happens when parsing
 * int associativity: LEFT or RIGHT
 * RewrittenExpressions* rewrittenExpressions:
 * if associativity == RIGHT, store rewritten expressions
 * 
 * return:
 * the value of the expression
 ****************************************************************/
LL exp(char* expression, size_t* nextIndex, char* globalToken, int* isDividedByZero, int associativity, RewrittenExpressions* rewrittenExpressions)
{
	switch (associativity)
	{
	case LEFT: 
	{
		LL temp = term(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
		while ((*globalToken == '+') || (*globalToken == '-'))
			switch (*globalToken)
			{
			case '+':
				match('+', expression, nextIndex, globalToken);
				temp += term(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
				break;
			case '-':
				match('-', expression, nextIndex, globalToken);
				temp -= term(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
				break;
			}
		return temp;
	}
	case RIGHT:
	{
		LL temp = term(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);
		while ((*globalToken == '+') || (*globalToken == '-'))
			switch (*globalToken)
			{
			case '+':
				match('+', expression, nextIndex, globalToken);

				// first, insert a '('
				insert(rewrittenExpressions->rewrittenExpressions[rewrittenExpressions->currentRewrittenExpressionIndex], '(', rewrittenExpressions->currentRewrittenExpressionNextCharIndex - 1);
				// skip the '('
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
				// then, synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;

				temp += exp(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);

				// first, insert a ')'
				insert(rewrittenExpressions->rewrittenExpressions[rewrittenExpressions->currentRewrittenExpressionIndex], ')', rewrittenExpressions->currentRewrittenExpressionNextCharIndex - 2);
				// skip the ')'
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;

				break;
			case '-':
				match('-', expression, nextIndex, globalToken);

				// first, insert a '('
				insert(rewrittenExpressions->rewrittenExpressions[rewrittenExpressions->currentRewrittenExpressionIndex], '(', rewrittenExpressions->currentRewrittenExpressionNextCharIndex - 1);
				// skip the '('
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
				// then, synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;

				temp -= exp(expression, nextIndex, globalToken, isDividedByZero, associativity, rewrittenExpressions);

				// first, insert a ')'
				insert(rewrittenExpressions->rewrittenExpressions[rewrittenExpressions->currentRewrittenExpressionIndex], ')', rewrittenExpressions->currentRewrittenExpressionNextCharIndex - 2);
				// skip the ')'
				(rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;

				break;
			}
		return temp;
	}
	}
}

/****************************************************************
 * description:
 * get the 10's power
 *
 * arguments:
 * int power: the power
 *
 * return:
 * an int, whose value is 10^`power`
 ****************************************************************/
LL powerOfTen(int power)
{
	LL result = 1;
	for (int i = power; i > 0; i--) result *= 10;
	return result;
}

/****************************************************************
 * description:
 * get an int from `expression`. This method will read all digits
 * from the `nextIndex` position until encounter the first non-
 * digital character
 *
 * arguments:
 * char* expression: the current expression in process
 * size_t* nextIndex: the index where you will get the next token
 * from `expression`
 * char* globalToken: the current token
 * RewrittenExpressions* rewrittenExpressions:
 * if associativity == RIGHT, store rewritten expressions
 *
 * return:
 * an int whose the most significant digit locates at `nextInt`
 * position
 ****************************************************************/
LL getInt(char* expression, size_t* nextIndex, char* globalToken, int associativity, RewrittenExpressions* rewrittenExpressions)
{
	// temp buffer to store digits in reverse order
	int* digits = (int*)malloc(sizeof(int) * 10000);
	if (!digits)
	{
		fprintf(stderr, "Cannot allocate the memory.\n");
		exit(1);
	}
	memset(digits, 0, sizeof(int) * 10000);
	// index variable for the do-while loop
	int i = -1;
	// put digits into the temp buffer in reverse order
	while (isdigit((int)(*globalToken = getNextChar(expression, nextIndex))) && i < 10000) digits[++i] = char2Digit(*globalToken);
	// synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
	if (associativity == RIGHT) rewrittenExpressions->currentRewrittenExpressionNextCharIndex += i + 2;
	// check the condition which lead to the exit
	if (i >= 10000)
	{
		fprintf(stderr, "Too big integer.\n");
		exit(1);
	}
	// put the first non-digital character back to the expression string
	ungetNextChar(nextIndex);
	// synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
	if (associativity == RIGHT) (rewrittenExpressions->currentRewrittenExpressionNextCharIndex)--;
	// construct the int
	LL tempSum = 0;
	int highestPower = i;
	while (i >= 0)
	{
		tempSum += digits[i] * powerOfTen(highestPower - i);
		i--;
	}
	
	free(digits);
	return tempSum;
}

/****************************************************************
 * description:
 * get the result of an expression. If the "divided by zero"
 * exception happens, rewrite the expression until the exception
 * won't happen.
 *
 * arguments:
 * char* expression: the current expression to be processed
 * int associativity: LEFT or RIGHT
 * RewrittenExpressions* rewrittenExpressions: 
 * if associativity == RIGHT, store rewritten expressions
 *
 * return:
 * the value of the expression
 ****************************************************************/
LL getResult(char** expression, int associativity, RewrittenExpressions* rewrittenExpressions)
{
	// initialize the rewritten expression
	if (associativity == RIGHT)
	{
		// allocate new heap memory for the rewritten expression
		if ((rewrittenExpressions->rewrittenExpressions)[++(rewrittenExpressions->currentRewrittenExpressionIndex)] = (char*)malloc(2 * strlen(*expression)))
		{
			memset((rewrittenExpressions->rewrittenExpressions)[rewrittenExpressions->currentRewrittenExpressionIndex], 0, 2 * strlen(*expression));
			memcpy((rewrittenExpressions->rewrittenExpressions)[rewrittenExpressions->currentRewrittenExpressionIndex], *expression, strlen(*expression));
		}
		else
		{
			fprintf(stderr, "Cannot allocate the memory.\n");
			exit(1);
		}
		// rewind the position pointer
		rewrittenExpressions->currentRewrittenExpressionNextCharIndex = 0;
	}

	int isDividedByZero = FALSE;
	size_t nextIndex = 0;
	char globalToken = getNextChar(*expression, &nextIndex);
	// synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
	if (associativity == RIGHT) (rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;

	// get the result;
	LL result = exp(*expression, &nextIndex, &globalToken, &isDividedByZero, associativity, rewrittenExpressions);
	while (isDividedByZero)
	{
		// rewrite the expression
		if (*expression) free(*expression);
		*expression = getExpression(NUMBER_OF_OPERATOR);
		// reset the rewritten expression
		if (associativity == RIGHT)
		{
			// release the heap memory allocated to the current rewritten expression
			free((rewrittenExpressions->rewrittenExpressions)[rewrittenExpressions->currentRewrittenExpressionIndex]);
			// allocate new heap memory for the rewritten expression
			if ((rewrittenExpressions->rewrittenExpressions)[rewrittenExpressions->currentRewrittenExpressionIndex] = (char*)malloc(2 * strlen(*expression)))
			{
				memset((rewrittenExpressions->rewrittenExpressions)[rewrittenExpressions->currentRewrittenExpressionIndex], 0, 2 * strlen(*expression));
				memcpy((rewrittenExpressions->rewrittenExpressions)[rewrittenExpressions->currentRewrittenExpressionIndex], *expression, strlen(*expression));
			}
			else
			{
				fprintf(stderr, "Cannot allocate the memory.\n");
				exit(1);
			}
			// rewind the position pointer
			rewrittenExpressions->currentRewrittenExpressionNextCharIndex = 0;
		}
		// reset the flag
		isDividedByZero = FALSE;
		// reset the "global variables"
		nextIndex = 0;
		globalToken = getNextChar(*expression, &nextIndex);
		// synchronize `nextIndex` with `rewrittenExpressions->currentRewrittenExpressionNextCharIndex`
		if (associativity == RIGHT) (rewrittenExpressions->currentRewrittenExpressionNextCharIndex)++;
		// get the result;
		result = exp(*expression, &nextIndex, &globalToken, &isDividedByZero, associativity, rewrittenExpressions);
	}
	
	return result;
}

/****************************************************************
 * description:
 * the driver code
 *
 * return:
 * the status code for OS
 ****************************************************************/
int main()
{
	srand((unsigned int)time(NULL));

	// initialize RewrittenExpressions
	RewrittenExpressions rewrittenExpressions;
	rewrittenExpressions.currentRewrittenExpressionNextCharIndex = 0;
	rewrittenExpressions.currentRewrittenExpressionIndex = -1;
	for (int i = 0; i < NUMBER_OF_QUESTION; i++) rewrittenExpressions.rewrittenExpressions[i] = NULL;

	// generate NUMBER_OF_QUESTION expressions
	char* expressions[NUMBER_OF_QUESTION] = { NULL };
	for (int i = 0; i < NUMBER_OF_QUESTION; i++) expressions[i] = getExpression(NUMBER_OF_OPERATOR);

	// solve the questions in the left associative order one by one
	// printf("Answers in the left associative order:\n");
	for (int i = 0; i < NUMBER_OF_QUESTION; i++)
	{
		LL result = getResult(&(expressions[i]), LEFT, &rewrittenExpressions);
		printf("%s == %lld\n", expressions[i], result);
	}

	// printf("\n=============================================\n\n");

	// solve the questions in the right associative order one by one
	//printf("Answers in the right associative order:\n");
	//for (int i = 0; i < NUMBER_OF_QUESTION; i++)
	//{
	//	LL result = getResult(&(expressions[i]), RIGHT, &rewrittenExpressions);
	//	printf("%s=%lld\n", expressions[i], result);
	//}


	//printf("\n=============================================\n\n");
	
	// print the rewritten expressions
	//printf("Rewritten expressions:\n");
	//for (int i = 0; i < NUMBER_OF_QUESTION; i++) printf("%s\n", rewrittenExpressions.rewrittenExpressions[i]);

	// free the memory
	for (int i = 0; i < NUMBER_OF_QUESTION; i++)
		if (expressions[i])
			free(expressions[i]);

	return 0;
}