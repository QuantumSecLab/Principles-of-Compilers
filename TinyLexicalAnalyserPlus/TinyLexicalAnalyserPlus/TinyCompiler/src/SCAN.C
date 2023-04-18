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
	START, INASSIGN, INCOMMENT, INNUM, INID, DONE, INCMULTICOMMENT, INFLOAT
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
			if (EchoSource) fprintf(listing, "%4d: %s", lineno, lineBuf);
			bufsize = strlen(lineBuf);
			linepos = 0;
			return lineBuf[linepos++];
		}
		else
		{
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
   {"write",WRITE} };

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

/* Probe whether continue to state 2 when in state 4 for multicomment scanning */
static int ProbeBackTrack(int c)
{
	// save the spot
	FILE* sourceBackup = malloc(sizeof(FILE));
	char* lineBufBackup = malloc(BUFLEN);
	if (!sourceBackup || !lineBufBackup)
	{
		printf("Out of memory!\n");
		exit(1);
	}
	memcpy(sourceBackup, source, sizeof(FILE));
	memcpy(lineBufBackup, lineBuf, BUFLEN);

	int lineposBackup = linepos;
	int bufsizeBackup = bufsize;
	int linenoBackup = lineno;
	int EOF_flag_Backup = EOF_flag;
	long streamPos = ftell(source);

	int firstChar = c, secondChar = getNextChar();
	while (!EOF_flag)
	{
		if (firstChar == '*' && secondChar == '/')
		{
			// recover the spot
			memcpy(source, sourceBackup, sizeof(FILE));
			memcpy(lineBuf, lineBufBackup, BUFLEN);
			free(sourceBackup);
			free(lineBufBackup);

			linepos = lineposBackup;
			bufsize = bufsizeBackup;
			lineno = linenoBackup;
			EOF_flag = EOF_flag_Backup;

			return FALSE;
		}

		firstChar = secondChar;
		secondChar = getNextChar();
	}

	// recover the spot
	memcpy(source, sourceBackup, sizeof(FILE));
	memcpy(lineBuf, lineBufBackup, BUFLEN);
	free(sourceBackup);
	free(lineBufBackup);

	linepos = lineposBackup;
	bufsize = bufsizeBackup;
	lineno = linenoBackup;
	EOF_flag = EOF_flag_Backup;
	fseek(source, streamPos, SEEK_SET);

	return TRUE;
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
	while (state != DONE)
	{
		int c = getNextChar();
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
				else if (c == '{')
				{
					save = FALSE;
					state = INCOMMENT;
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
							if (linepos < bufsize && lineBuf[linepos] == '*')
							{
								state = INCMULTICOMMENT;
								save = FALSE;
							}
							else currentToken = OVER;
							break;
						case '(':
							currentToken = LPAREN;
							break;
						case ')':
							currentToken = RPAREN;
							break;
						case ';':
							currentToken = SEMI;
							break;
						default:
							currentToken = ERROR;
							break;
					}
				}
				break;
			case INCOMMENT:
				save = FALSE;
				if (c == EOF)
				{
					state = DONE;
					currentToken = ENDFILE;
				}
				else if (c == '}') state = START;
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
						state = INFLOAT;
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
			case INCMULTICOMMENT:
				save = FALSE;
				ungetNextChar();
				int multicommentState = 1;

				while (multicommentState == 1 || multicommentState == 2
					|| multicommentState == 3 || multicommentState == 4)
				{
					c = getNextChar();
					switch (multicommentState)
					{
						case 1:
							switch (c)
							{
								case '*':
									multicommentState = 2;
									break;
								default:
									/* Should never happen */
									fprintf(listing, "Scanner Bug: state= %d\n", state);
									state = DONE;
									currentToken = ERROR;
									multicommentState = -1;
									break;
							}
							break;
						case 2:
							switch (c)
							{
								case '*':
									multicommentState = 3;
									break;
								default:
									multicommentState = 2;
									break;
							}
							break;
						case 3:
							switch (c)
							{
								case '*':
									multicommentState = 3;
									break;
								case '/':
									multicommentState = 4;
									break;
								default:
									multicommentState = 2;
									break;
							}
							break;
						case 4:
							/* Probe whether need to backtrack */
							if (!ProbeBackTrack(c))
							{
								switch (c)
								{
								case '*':
									multicommentState = 3;
									break;
								default:
									multicommentState = 2;
									break;
								}
							}
							else
							{
								ungetNextChar();
								state = START;
								multicommentState = 5;
							}
							break;
						default:
							/* Should never happen */
							fprintf(listing, "Scanner Bug: state= %d\n", state);
							state = DONE;
							currentToken = ERROR;
							multicommentState = -1;
							break;
					}
				}
				break;
			case INFLOAT:
				ungetNextChar();
				int inFloatState = 1;
				while (inFloatState == 1 || inFloatState == 2)
				{
					c = getNextChar();
					switch (inFloatState)
					{
					case 1:
						if (c == '0' || c == '1' || c == '2' ||
							c == '3' || c == '4' || c == '5' ||
							c == '6' || c == '7' || c == '8' ||
							c == '9')
						{
							inFloatState = 2;
						}
						else
						{
							/* Should never happen */
							fprintf(listing, "Scanner Bug: state= %d\n", state);
							state = DONE;
							currentToken = ERROR;
							inFloatState = -1;
						}
						break;
					case 2:
						if (c == '0' || c == '1' || c == '2' ||
							c == '3' || c == '4' || c == '5' ||
							c == '6' || c == '7' || c == '8' ||
							c == '9')
						{
							inFloatState = 2;
						}
						else
						{
							ungetNextChar();
							state = DONE;
							currentToken = FLOATNUM;
							inFloatState = 3;
						}
						break;
					default:
						/* Should never happen */
						fprintf(listing, "Scanner Bug: state= %d\n", state);
						state = DONE;
						currentToken = ERROR;
						inFloatState = -1;
						break;
					}
					if ((save) && (tokenStringIndex <= MAXTOKENLEN))
						tokenString[tokenStringIndex++] = (char)c;
					if (state == DONE)
					{
						tokenString[tokenStringIndex] = '\0';
						if (TraceScan) {
							fprintf(listing, "\t%d: ", lineno);
							printToken(currentToken, tokenString);
						}
						return currentToken;
					}
				}
				break;
			case DONE:
			default: /* should never happen */
				fprintf(listing, "Scanner Bug: state= %d\n", state);
				state = DONE;
				currentToken = ERROR;
				break;
		}
		if ((save) && (tokenStringIndex <= MAXTOKENLEN))
			tokenString[tokenStringIndex++] = (char)c;
		if (state == DONE)
		{
			tokenString[tokenStringIndex] = '\0';
			if (currentToken == ID)
				currentToken = reservedLookup(tokenString);
		}
	}
	if (TraceScan) {
		fprintf(listing, "\t%d: ", lineno);
		printToken(currentToken, tokenString);
	}
	return currentToken;
} /* end getToken */

