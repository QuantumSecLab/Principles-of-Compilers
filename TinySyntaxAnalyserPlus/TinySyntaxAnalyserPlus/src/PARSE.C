/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode* stmt_sequence(void);
static TreeNode* statement(void);
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);
static TreeNode* function_def(void);
static TreeNode* formal_parameter_list(void);
static TreeNode* formal_parameter(void);
static TreeNode* function_call(void);
static TreeNode* actual_parameter_list(void);
static TreeNode* actual_parameter(void);

static void syntaxError(char* message)
{
	fprintf(listing, "\n>>> ");
	fprintf(listing, "Syntax error at line %d: %s", lineno, message);
	Error = TRUE;
}

static void match(TokenType expected)
{
	if (token == expected) token = getToken();
	else {
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		fprintf(listing, "      ");
	}
}

TreeNode* stmt_sequence(void)
{
	TreeNode* t = statement();
	TreeNode* p = t;
	while ((token != ENDFILE) && (token != END) &&
		(token != ELSE) && (token != UNTIL))
	{
		TreeNode* q;

		if (token == SEMI) match(SEMI);
		else if (token == RBRACE) return t;
		else syntaxError("Expect a ';' or '}' at the end of the statement.");
		
		q = statement();
		if (q != NULL) {
			if (t == NULL) t = p = q;
			else /* now p cannot be NULL either */
			{
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

TreeNode* statement(void)
{
	TreeNode* t = NULL;
	switch (token) {
	case IF: t = if_stmt(); break;
	case REPEAT: t = repeat_stmt(); break;
	case ID: t = assign_stmt(); break;
	case READ: t = read_stmt(); break;
	case WRITE: t = write_stmt(); break;
	case DEF: t = function_def(); break;
	case CALL: t = function_call(); break;
	default: syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
		break;
	} /* end case */
	return t;
}

TreeNode* if_stmt(void)
{
	TreeNode* t = newStmtNode(IfK);
	match(IF);
	if (t != NULL) t->child[0] = exp();
	match(THEN);
	if (t != NULL) t->child[1] = stmt_sequence();
	if (token == ELSE) {
		match(ELSE);
		if (t != NULL) t->child[2] = stmt_sequence();
	}
	match(END);
	return t;
}

TreeNode* repeat_stmt(void)
{
	TreeNode* t = newStmtNode(RepeatK);
	match(REPEAT);
	if (t != NULL) t->child[0] = stmt_sequence();
	match(UNTIL);
	if (t != NULL) t->child[1] = exp();
	return t;
}

TreeNode* assign_stmt(void)
{
	TreeNode* t = newStmtNode(AssignK);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(ID);
	match(ASSIGN);
	if (t != NULL) t->child[0] = exp();
	return t;
}

TreeNode* read_stmt(void)
{
	TreeNode* t = newStmtNode(ReadK);
	match(READ);
	if ((t != NULL) && (token == ID))
		t->attr.name = copyString(tokenString);
	match(ID);
	return t;
}

TreeNode* write_stmt(void)
{
	TreeNode* t = newStmtNode(WriteK);
	match(WRITE);
	if (t != NULL) t->child[0] = exp();
	return t;
}

TreeNode* exp(void)
{
	TreeNode* t = simple_exp();
	if ((token == LT) || (token == EQ)) {
		TreeNode* p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		match(token);
		if (t != NULL)
			t->child[1] = simple_exp();
	}
	return t;
}

TreeNode* simple_exp(void)
{
	TreeNode* t = term();
	while ((token == PLUS) || (token == MINUS))
	{
		TreeNode* p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			t->child[1] = term();
		}
	}
	return t;
}

TreeNode* term(void)
{
	TreeNode* t = factor();
	while ((token == TIMES) || (token == OVER))
	{
		TreeNode* p = newExpNode(OpK);
		if (p != NULL) {
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			match(token);
			p->child[1] = factor();
		}
	}
	return t;
}

TreeNode* factor(void)
{
	TreeNode* t = NULL;
	switch (token) {
	case NUM:
		t = newExpNode(ConstK);
		if ((t != NULL) && (token == NUM))
			t->attr.val = atoi(tokenString);
		match(NUM);
		break;
	case ID:
		t = newExpNode(IdK);
		if ((t != NULL) && (token == ID))
			t->attr.name = copyString(tokenString);
		match(ID);
		break;
	case LPAREN:
		match(LPAREN);
		t = exp();
		match(RPAREN);
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
		break;
	}
	return t;
}

static TreeNode* formal_parameter(void)
{
	TreeNode* t = newStmtNode(FormalParameter);
	if (t && (token == INT || token == FLOAT))
	{
		// add a lchild node for the parameter type
		t->child[0] = newExpNode(TypeK);
		t->child[0]->attr.name = copyString(tokenString);
		match(token);

		// add a rchild node for the parameter name
		if (token == ID)
		{
			t->child[1] = newExpNode(IdK);
			t->child[1]->attr.name = copyString(tokenString);
			match(ID);
		}
		else syntaxError("Expect an identifier.");
	}
	else syntaxError("The parameter type should be either int or float.");

	return t;
}

TreeNode* function_call(void)
{
	TreeNode* root = newStmtNode(FunctionCallK);

	// check the memory allocation result
	if (!root) return root;

	// match the call keyword
	match(CALL);

	// match the function name
	if (token == ID)
	{
		root->attr.name = copyString(tokenString);
		match(ID);
	}

	// match the left parenthesis
	match(LPAREN);

	// match the actual parameter list
	root->child[0] = actual_parameter_list();

	// match the right parenthesis
	match(RPAREN);

	return root;
}

TreeNode* actual_parameter_list(void)
{
	TreeNode* root = newStmtNode(ActualParameterListK);

	// check the memory allocation result
	if (!root) return root;

	// check whether the actual parameter list is empty
	if (token == RPAREN) return root;

	// match the first actual parameter
	root->child[0] = actual_parameter();

	// match all actual parameters
	TreeNode* currentNode = root->child[0];
	while (token == COMMA)
	{
		match(COMMA);
		if (currentNode)
		{
			currentNode->sibling = actual_parameter();
			currentNode = currentNode->sibling;
		}
	}
	if (token != RPAREN)
		syntaxError("Actual parameters should be seperated by commas.");

	return root;
}

TreeNode* actual_parameter(void)
{
	TreeNode* root = newExpNode(ActualParameter);

	// check the memory allocation result
	if (!root) return root;

	// match an actual parameter
	if (token == ID || token == FLOATNUM || token == SCIENTIFIC_NOTATION || token == NUM)
	{
		root->attr.name = copyString(tokenString);
		match(token);
	}
	else
		syntaxError("Actual parameters should be id, float number, scientific notations, or integers.");

	return root;
}

static TreeNode* formal_parameter_list(void)
{
	TreeNode* root = newStmtNode(FormalParameterListK);

	// check whether the parameter list is empty
	if (token != RPAREN)
	{
		if (root)
		{
			root->child[0] = formal_parameter();
			TreeNode* currentNode = root->child[0];
			while (currentNode && token == COMMA)
			{
				match(COMMA);
				currentNode->sibling = formal_parameter();
				currentNode = currentNode->sibling;
			}
		}
	}

	return root;
}

static TreeNode* function_def(void)
{
	TreeNode* root = newStmtNode(FunctionDefK);

	// check whether memory allocation is successful
	if (!root) return root;

	// match the def keyword
	match(DEF);

	// match the type of the function return value
	if (token == INT || token == FLOAT || token == VOID)
	{
		root->child[0] = newExpNode(TypeK);
		root->child[0]->attr.name = copyString(tokenString);
		match(token);
	}
	else
	{
		syntaxError("Expected the type of the return value.");
		return root;
	}

	// match the function name
	if (token == ID)
	{
		root->attr.name = copyString(tokenString);
		match(ID);
	}
	else
	{
		syntaxError("Expected the identifier of the function.");
		return root;
	}

	// match the left parenthesis of the parameter list
	match(LPAREN);

	// match the parameter list
	root->child[1] = formal_parameter_list();

	// match the right parenthesis of the parameter list
	match(RPAREN);

	// match the left brace of the function body
	match(LBRACE);

	// match the function body
	root->child[2] = stmt_sequence();

	// match the right brace of the function body
	match(RBRACE);

	return root;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode* parse(void)
{
	TreeNode* t;
	token = getToken();
	t = stmt_sequence();
	if (token != ENDFILE)
		syntaxError("Code ends before file\n");
	return t;
}
