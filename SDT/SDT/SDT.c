#include <stdio.h>
#include <ctype.h>

int currentToken;

/* 
 * function prototypes
 */
void error(char* msg);
int exp();
int factor();
void match(int tokenMatched, int expectedToken);
int expPrime(int inherited);

/*
 * description: 
 * Execute errors.
 * 
 * parameter(s):
 * char* msg: The error message to be printed to the stderr.
 */
void error(char* msg)
{
	fprintf(stderr, "%s The current token is %c\n", msg, (char)currentToken);
	exit(1);
}

/*
 * description:
 * Match the token.
 *
 * parameter(s):
 * int tokenMatched: The token to be matched.
 * int expectedToken: The expected token to be matched.
 */
void match(int tokenMatched, int expectedToken)
{
	if (tokenMatched == expectedToken) currentToken = getchar();
	else error("Token mismatched.");
}

/*
 * description:
 * Get the result of a factor.
 */
int factor()
{
	// basic lexical check
	if (!isdigit(currentToken) && currentToken != '(') error("A valid factor should starts with either a decimal number or a '('.");

	int temp;
	if (isdigit(currentToken))
	{
		// get the number
		ungetc(currentToken, stdin);
		scanf("%d", &temp);
		currentToken = getchar();

		return temp;
	}
	else
	{
		// get the value of an expression
		match(currentToken, '(');
		temp = exp();
		match(currentToken, ')');

		return temp;
	}
}

/*
 * description:
 * Get the result of the whole expression recursively.
 * 
 * parameter(s);
 * int inherited: current partial summation
 */
int expPrime(int inherited)
{
	switch (currentToken)
	{
	case '+':
		match(currentToken, '+');
		// call factor()
		// and execute the semantic rule: exp'_2.inh = exp'_1.inh + factor.syn
		// at last execute the semantic rule: exp'_1.syn = exp'_2.syn
		return expPrime(inherited + factor());
	default:
		// encounter the EOF
		// execute the semantic rule: exp'.syn = exp'.inh
		return inherited;
	}
}

/*
 * description:
 * Get the result of the whole expression.
 */
int exp()
{
	// basic lexical check
	if (!isdigit(currentToken) && currentToken != '(') error("A valid expression should starts with either a decimal number or a '('.");

	// call factor()
	// and execute the semantic rule: exp'.inh = factor.syn
	// at last, execute the semantic rule: exp.syn = exp'.syn
	return expPrime(factor());
}

int main()
{
	printf("Input a simple arithmic expression:\n");

	// look ahead for one char
	currentToken = getchar();
	int result = exp();

	printf("=%d\n", result);
	
	return 0;
}