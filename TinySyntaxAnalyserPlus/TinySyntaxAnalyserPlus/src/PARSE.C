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
static TreeNode* assign_stmt(char* id);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);
static TreeNode* function_def(char* type, char* id);
static TreeNode* formal_parameter_list(void);
static TreeNode* formal_parameter(void);
static TreeNode* function_call(char* id);
static TreeNode* actual_parameter_list(void);
static TreeNode* actual_parameter(void);
static TreeNode* start_with_id(void);
static TreeNode* start_with_type(void);
static TreeNode* array_reference(char* id);
static TreeNode* array_index(void);
static TreeNode* variable_declaration(char* type, char* id);

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
	case ID: t = start_with_id(); break;
	case READ: t = read_stmt(); break;
	case WRITE: t = write_stmt(); break;
	case INT:
	case FLOAT:
		t = start_with_type(); break;
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

TreeNode* assign_stmt(char* id)
{
	TreeNode* t = newStmtNode(AssignK);
	if ((t != NULL))
		t->attr.name = copyString(id);
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
		t = start_with_id();
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

TreeNode* function_call(char* id)
{
	TreeNode* root = newExpNode(CallK);

	// check the memory allocation result
	if (!root) return root;

	// assign the function name
	root->attr.name = copyString(id);

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

TreeNode* start_with_id(void)
{
	TreeNode* root = NULL;

	// save the id literal and match the id
	char* idBackup = copyString(tokenString);
	match(ID);

	// function call, assign expression, or array reference
	if (token == LPAREN)
		root = function_call(idBackup);
	else if (token == ASSIGN)
		root = assign_stmt(idBackup);
	else if (token == LBOX)
		root = array_reference(idBackup);
	else
	{
		root = newExpNode(IdK);
		root->attr.name = idBackup;
	}

	return root;
}

TreeNode* start_with_type(void)
{
	TreeNode* root = NULL;
	char* typeBackup = NULL;
	char* idBackup = NULL;

	// void is only allowed in the function return value type
	// function call or assign expressions
	if (token == VOID)
	{
		typeBackup = copyString(tokenString);
		match(token);
		idBackup = copyString(tokenString);
		match(token);
		root = function_def(typeBackup, idBackup);
	}
	else if (token == INT || token == FLOAT)
	{
		typeBackup = copyString(tokenString);
		match(token);
		idBackup = copyString(tokenString);
		match(token);

		if (token == LPAREN)
			root = function_def(typeBackup, idBackup);
		else
			root = variable_declaration(typeBackup, idBackup);
	}
	else
		syntaxError("Unknown error. An type reserved word is expected.");

	return root;
}

TreeNode* array_reference(char* id)
{
	TreeNode* root = newExpNode(ArrayRef);

	// check memory allocation
	if (!root) return root;

	// assign id literal
	root->attr.name = copyString(id);

	// match array index
	root->child[0] = array_index();

	return root;
}

TreeNode* array_index(void)
{
	TreeNode* root = newExpNode(ArrayIndex);

	// check memory allocation
	if (!root) return root;

	// match the left box bracket
	match(LBOX);

	// match an integer
	if (token == NUM)
	{
		sscanf(tokenString, "%d", &(root->attr.val));
		match(NUM);
	}
	else
		syntaxError("Array index should be an integer.");

	// match the right box bracket
	match(RBOX);

	// match the index for higher dimension if exists
	if (token == LBOX)
		root->sibling = array_index();

	return root;
}

TreeNode* variable_declaration(char* type, char* id)
{
	TreeNode* root = NULL;

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

static TreeNode* function_def(char* type, char* id)
{
	TreeNode* root = newStmtNode(FunctionDefK);

	// check whether memory allocation is successful
	if (!root) return root;

	// assign the type of the function return value
	root->child[0] = newExpNode(TypeK);
	root->child[0]->attr.name = copyString(type);

	// assign the function name
	root->attr.name = copyString(id);

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
