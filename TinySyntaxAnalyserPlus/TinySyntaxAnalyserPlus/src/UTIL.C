/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char* tokenString)
{
	switch (token)
	{
	case IF:
	case THEN:
	case ELSE:
	case END:
	case REPEAT:
	case UNTIL:
	case READ:
	case WRITE:
	case INT:
	case FLOAT:
	case VOID:
	case RETURN:
		fprintf(listing,
			"reserved word: %s\n", tokenString);
		break;
	case ASSIGN: fprintf(listing, ":=\n"); break;
	case LT: fprintf(listing, "<\n"); break;
	case EQ: fprintf(listing, "=\n"); break;
	case LPAREN: fprintf(listing, "(\n"); break;
	case RPAREN: fprintf(listing, ")\n"); break;
	case LBRACE: fprintf(listing, "{\n"); break;
	case RBRACE: fprintf(listing, "}\n"); break;
	case LBOX: fprintf(listing, "[\n"); break;
	case RBOX: fprintf(listing, "]\n"); break;
	case SEMI: fprintf(listing, ";\n"); break;
	case PLUS: fprintf(listing, "+\n"); break;
	case MINUS: fprintf(listing, "-\n"); break;
	case TIMES: fprintf(listing, "*\n"); break;
	case OVER: fprintf(listing, "/\n"); break;
	case COMMA: fprintf(listing, ",\n"); break;
	case ENDFILE: fprintf(listing, "EOF\n"); break;
	case NUM:
		fprintf(listing,
			"NUM, val= %s\n", tokenString);
		break;
	case FLOATNUM:
		fprintf(listing,
			"FLOATNUM, val= %s\n", tokenString);
		break;
	case SCIENTIFIC_NOTATION:
		fprintf(listing,
			"SCIENTIFIC_NOTATION, val= %s\n", tokenString);
		break;
	case ID:
		fprintf(listing,
			"ID, name= %s\n", tokenString);
		break;
	case ERROR:
		fprintf(listing,
			"ERROR: %s\n", tokenString);
		break;
	default: /* should never happen */
		fprintf(listing, "Unknown token: %d\n", token);
	}
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StmtKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0;i < MAXCHILDREN;i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = kind;
		t->lineno = lineno;
	}
	return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode* newExpNode(ExpKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
	int i;
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else {
		for (i = 0;i < MAXCHILDREN;i++) t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno = lineno;
		t->type = Void;
	}
	return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char* s)
{
	int n;
	char* t;
	if (s == NULL) return NULL;
	n = strlen(s) + 1;
	t = malloc(n);
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else strcpy(t, s);
	return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{
	int i;
	for (i = 0;i < indentno;i++)
		fprintf(listing, " ");
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode* tree)
{
	int i;
	INDENT;
	while (tree != NULL) {
		printSpaces();
		if (tree->nodekind == StmtK)
		{
			switch (tree->kind.stmt) {
			case IfK:
				fprintf(listing, "If\n");
				break;
			case RepeatK:
				fprintf(listing, "Repeat\n");
				break;
			case AssignK:
				fprintf(listing, "Assign to: %s\n", tree->attr.name);
				break;
			case ReadK:
				fprintf(listing, "Read: %s\n", tree->attr.name);
				break;
			case WriteK:
				fprintf(listing, "Write\n");
				break;
			case FunctionDefK:
				fprintf(listing, "Function definition: %s\n", tree->attr.name);
				break;
			case VarDeclarationK:
				fprintf(listing, "Variable declaration:\n");
				break;
			case ReturnK:
				fprintf(listing, "Return:\n");
				break;
			default:
				fprintf(listing, "Unknown ExpNode kind\n");
				break;
			}
		}
		else if (tree->nodekind == ExpK)
		{
			switch (tree->kind.exp) {
			case OpK:
				fprintf(listing, "Op: ");
				printToken(tree->attr.op, "\0");
				break;
			case IntConstK:
				fprintf(listing, "Integer const: %d\n", tree->attr.val);
				break;
			case FloatConstK:
				fprintf(listing, "Float const: %lf\n", tree->attr.fval);
				break;
			case IdK:
				fprintf(listing, "Id: %s\n", tree->attr.name);
				break;
			case TypeK:
				fprintf(listing, "Value type: %s\n", tree->attr.name);
				break;
			case CallK:
				fprintf(listing, "Function call: %s\n", tree->attr.name);
				break;
			case FormalParameterListK:
				fprintf(listing, "Formal parameter list:\n");
				break;
			case FormalParameterK:
				fprintf(listing, "Formal parameter:\n");
				break;
			case ActualParameterListK:
				fprintf(listing, "Actual parameter List:\n");
				break;
			case ArrayRefK:
				fprintf(listing, "Array reference: %s\n", tree->attr.name);
				break;
			case ArrayIndexK:
				fprintf(listing, "Array index: %d\n", tree->attr.val);
				break;
			case VariableK:
				fprintf(listing, "Variable:\n");
				break;
			case InitValListK:
				fprintf(listing, "Initial value list:\n");
				break;
			default:
				fprintf(listing, "Unknown ExpNode kind: %s\n", tree->attr.name);
				break;
			}
		}
		else fprintf(listing, "Unknown node kind\n");
		for (i = 0;i < MAXCHILDREN;i++)
			printTree(tree->child[i]);
		tree = tree->sibling;
	}
	UNINDENT;
}
