/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"

/* states in scanner DFA */
typedef enum
{
	START, INASSIGN, INNUM, INID, DONE, 
	DIV_OR_MULTILINE_COMMENT,                       /* Add an intermediate state when it comes to '/' to make the code more human-readable */
	IN_MULTILINE_COMMENT_1, IN_MULTILINE_COMMENT_2, /* states of DFA for c-style multiline comments */
	IN_UPPER_HALF_FLOAT,                            /* states of DFA for float numbers which at least have the upper half */
	IN_LOWER_HALF_FLOAT_1, IN_LOWER_HALF_FLOAT_2,    /* states of DFA for float numbers which only have the lower half */
	IN_SCIENTIFIC_NOTATION_1, IN_SCIENTIFIC_NOTATION_2, IN_SCIENTIFIC_NOTATION_3 /* states of DFA for scientific notation */
}
StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{
	if (!(linepos < bufsize))
	{
		lineno++;
		if (fgets(lineBuf, BUFLEN - 1, source))
		{
			if (lineBuf[strlen(lineBuf) - 1] != '\n') fprintf(listing, "ERROR: line %d exceeds the maximum length.\n", lineno);
			char lineBufCopy[BUFLEN] = "";
			memcpy(lineBufCopy, lineBuf, strlen(lineBuf) - 1);
			if (EchoSource) fprintf(listing, "%4d: %s\n", lineno, lineBufCopy);
			bufsize = strlen(lineBuf);
			linepos = 0;
			return lineBuf[linepos++];
		}
		else
		{
			// linepos = 0; // infinite loop!
			EOF_flag = TRUE;
			return EOF;
		}
	}
	else return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{
	if (!EOF_flag) linepos--;
}

/* lookup table of reserved words */
static struct
{
	char* str;
	TokenType tok;
} reservedWords[MAXRESERVED]
= { {"if",IF},{"then",THEN},{"else",ELSE},{"end",END},
   {"repeat",REPEAT},{"until",UNTIL},{"read",READ},
   {"write",WRITE},{"int",INT},{"float",FLOAT},{"void",VOID},{"def",DEF} };

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup(char* s)
{
	int i;
	for (i = 0;i < MAXRESERVED;i++)
		if (!strcmp(s, reservedWords[i].str))
			return reservedWords[i].tok;
	return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void)
{  /* index for storing into tokenString */
	int tokenStringIndex = 0;
	/* holds current token to be returned */
	TokenType currentToken;
	/* current state - always begins at START */
	StateType state = START;
	/* flag to indicate save to tokenString */
	int save;
	/* the last character read from lineBuf */
	int c;
	while (state != DONE)
	{
		c = getNextChar();
		save = TRUE;
		switch (state)
		{
		case START:
			if (isdigit(c))
				state = INNUM;
			else if (isalpha(c))
				state = INID;
			else if (c == ':')
				state = INASSIGN;
			else if ((c == ' ') || (c == '\t') || (c == '\n'))
				save = FALSE;
			else if (c == '.')
			{
				state = IN_LOWER_HALF_FLOAT_1;
			}
			else
			{
				state = DONE;
				switch (c)
				{
				case EOF:
					save = FALSE;
					currentToken = ENDFILE;
					break;
				case '=':
					currentToken = EQ;
					break;
				case '<':
					currentToken = LT;
					break;
				case '+':
					currentToken = PLUS;
					break;
				case '-':
					currentToken = MINUS;
					break;
				case '*':
					currentToken = TIMES;
					break;
				case '/':
					state = DIV_OR_MULTILINE_COMMENT;
					break;
				case '(':
					currentToken = LPAREN;
					break;
				case ')':
					currentToken = RPAREN;
					break;
				case '{':
					currentToken = LBRACE;
					break;
				case '}':
					currentToken = RBRACE;
					break;
				case ';':
					currentToken = SEMI;
					break;
				case ',':
					currentToken = COMMA;
					break;
				default:
					currentToken = ERROR;
					break;
				}
			}
			break;
		case INASSIGN:
			state = DONE;
			if (c == '=')
				currentToken = ASSIGN;
			else
			{ /* backup in the input */
				ungetNextChar();
				save = FALSE;
				currentToken = ERROR;
			}
			break;
		case INNUM:
			if (!isdigit(c))
			{
				if (c == '.')
				{
					state = IN_UPPER_HALF_FLOAT;
				}
				else if (isalpha(c))
				{
					/* backup in the input */
					ungetNextChar();
					save = FALSE;
					state = DONE;
					currentToken = ERROR;
					fprintf(listing, "\t(%d, %d): ERROR: Invalid unsigned integer.\n", lineno - 1, linepos);
				}
				else
				{
					/* backup in the input */
					ungetNextChar();
					save = FALSE;
					state = DONE;
					currentToken = NUM;
				}
			}
			break;
		case INID:
			if (!isalpha(c))
			{ /* backup in the input */
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = ID;
			}
			break;
		case DIV_OR_MULTILINE_COMMENT:
			if (c == '*')
			{
				save = FALSE;
				state = IN_MULTILINE_COMMENT_1;
				memset(tokenString, 0, MAXTOKENLEN + 1);
				tokenStringIndex = 0;
			}
			else
			{
				ungetNextChar();
				state = DONE;
				save = FALSE;
				currentToken = OVER;
			}
			break;
		case IN_MULTILINE_COMMENT_1:
			save = FALSE;
			if (c == '*')
			{
				state = IN_MULTILINE_COMMENT_2;
			}
			else if (c == EOF)
			{
				state = DONE;
				currentToken = ENDFILE;
				fprintf(listing, "\t(%d, %d): ERROR: Non-terminated comment.\n", lineno - 1, linepos);
			}
			else
			{
				state = IN_MULTILINE_COMMENT_1;
			}
			break;
		case IN_MULTILINE_COMMENT_2:
			save = FALSE;
			if (c == '/')
			{
				state = START;
			}
			else if (c == '*')
			{
				state = IN_MULTILINE_COMMENT_2;
			}
			else if (c == EOF)
			{
				state = DONE;
				currentToken = ENDFILE;
				fprintf(listing, "\t(%d, %d): ERROR: Non-terminated comment.\n", lineno - 1, linepos);
			}
			else
			{
				state = IN_MULTILINE_COMMENT_1;
			}
			break;
		case IN_UPPER_HALF_FLOAT:
			if (!isdigit(c) && c != 'e' && c != 'E')
			{
				/* backup the input */
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = FLOATNUM;
			}
			else if (!isdigit(c) && (c == 'e' || c == 'E'))
			{
				state = IN_SCIENTIFIC_NOTATION_1;
			}
			else
			{
				state = IN_UPPER_HALF_FLOAT;
			}
			break;
		case IN_LOWER_HALF_FLOAT_1:
			if (!isdigit(c))
			{
				/* backup the input */
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = ERROR;
				fprintf(listing, "\t(%d, %d): ERROR: Invalid float number. Expected at least one digit at the either side of the dot.\n", lineno, linepos);
			}
			else
			{
				state = IN_LOWER_HALF_FLOAT_2;
			}
			break;
		case IN_LOWER_HALF_FLOAT_2:
			if (!isdigit(c) && c != 'e' && c != 'E')
			{
				/* backup the input */
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = FLOATNUM;
			}
			else if (!isdigit(c) && (c == 'e' || c == 'E'))
			{
				state = IN_SCIENTIFIC_NOTATION_1;
			}
			else
			{
				state = IN_LOWER_HALF_FLOAT_2;
			}
			break;
		case IN_SCIENTIFIC_NOTATION_1:
			if (isdigit(c))
			{
				state = IN_SCIENTIFIC_NOTATION_3;
			}
			else if (c == '+' || c == '-')
			{
				state = IN_SCIENTIFIC_NOTATION_2;
			}
			else
			{
				// error
				ungetNextChar();
				state = DONE;
				currentToken = ERROR;
				save = FALSE;
				fprintf(listing, "\t(%d, %d): ERROR: Invalid scientific notation. Expect digits or sign in exponential feild.\n", lineno, linepos);
			}
			break;
		case IN_SCIENTIFIC_NOTATION_2:
			if (isdigit(c))
			{
				state = IN_SCIENTIFIC_NOTATION_3;
			}
			else
			{
				// error
				ungetNextChar();
				save = FALSE;
				currentToken = ERROR;
				state = DONE;
				fprintf(listing, "\t(%d, %d): ERROR: Invalid scientific notation. Expect digits after the sign.\n", lineno, linepos);
			}
			break;
		case IN_SCIENTIFIC_NOTATION_3:
			if (isdigit(c))
			{
				state = IN_SCIENTIFIC_NOTATION_3;
			}
			else if (c == '.')
			{
				// error
				ungetNextChar();
				save = FALSE;
				currentToken = ERROR;
				state = DONE;
				fprintf(listing, "\t(%d, %d): ERROR: Invalid scientific notation. Exponent cannot be float number.\n", lineno, linepos);
			}
			else
			{
				/* backup the input */
				ungetNextChar();
				save = FALSE;
				state = DONE;
				currentToken = SCIENTIFIC_NOTATION;
			}
			break;
		default: /* should never happen */
			fprintf(listing, "Scanner Bug: state= %d\n", state);
			state = DONE;
			currentToken = ERROR;
			break;
		}
		if ((save) && (tokenStringIndex <= MAXTOKENLEN))
			tokenString[tokenStringIndex++] = (char)c;
		else if (tokenStringIndex > MAXTOKENLEN)
		{
			state = DONE;
			currentToken = ERROR;
			fprintf(listing, "\t(%d, %d): ERROR: Token length exceeded.\n", lineno, linepos);
		}
		if (state == DONE)
		{
			tokenString[tokenStringIndex] = '\0';
			if (currentToken == ID)
				currentToken = reservedLookup(tokenString);
		}
	}
	if (TraceScan) {
		if (c == EOF && currentToken != ENDFILE) lineno --; // bug fix
		fprintf(listing, "\t%d: ", lineno);
		printToken(currentToken, tokenString);
	}
	return currentToken;
} /* end getToken */

