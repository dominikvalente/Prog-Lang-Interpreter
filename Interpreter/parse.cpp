#include <iostream>
#include <fstream>
#include <map>
#include "lex.h"
#include "val.h"
#include "parseRun.h"

using namespace std;
using namespace Parser;

/** EBNF Program Rules:
 Prog := begin StmtList end
 StmtList := Stmt; {Stmt;}
 Stmt := PrintStmt | AssignStmt | IfStmt
 PrintStmt := print ExprList
 IfStmt := if (Expr) then Stmt
 AssignStmt := Var = Expr
 ExprList := Expr {,Expr}
 Expr := Term {(+|-) Term}
 Term := Factor {(*|/) Factor}
 Var := ident
 Factor := ident | iconst | rconst | sconst | (Expr)
 */

//Eclipse run test files: Run tab -> Run Configurations -> Arguments tab -> FileName -> Apply -> Run

int main(int argc, char *argv[]) {

	ifstream file;
	file.open(argv[1]);

	if (argc > 2) {
		for (int i = 2; argv[i] != NULL; i++) {
			string arg = argv[i];

			if (arg.find(".txt") != string::npos) {
				cout << "ONLY ONE FILE NAME ALLOWED" << endl;
				exit(1);
			}
		}
	} else if (file.is_open()) {
		int line = 1;

		if (Prog(file, line)) {
			cout << "Successful Interpretation" << endl;
		} else {
			cout << "Unsuccessful Interpretation\n"
					<< "Number of Syntax Errors: " << error_count << endl;
			exit(1);
		}
	} else {
		cout << "CANNOT OPEN THE FILE " << argv[1] << endl;
		exit(1);
	}
	file.close();
	return 0;
}

//Program is: Prog := begin StmtList end
bool Prog(istream &in, int &line) {
	//cout << "In Prog" << endl;

	bool status;
	LexItem tok = GetNextToken(in, line);

	if (tok.GetToken() == BEGIN) {
		status = StmtList(in, line);

		if (!status) {
			ParseError(line, "No statements in program");
			return false;
		}
	} else if (tok.GetToken() == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	tok = GetNextToken(in, line);

	if (tok.GetToken() == END) {
		return true;
	} else if (tok.GetToken() == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return true;
}

//StmtList is a Stmt followed by semicolon followed by a StmtList
bool StmtList(istream &in, int &line) {
	//cout << "in StmtList" << endl;

	bool status;
	LexItem tok = GetNextToken(in, line);

	while (tok.GetToken() != BEGIN && tok.GetToken() != END
			&& tok.GetToken() != DONE && tok.GetToken() != ERR) {

		PushBackToken(tok);
		status = Stmt(in, line);

		if (!status) {
			return false;
		}

		tok = GetNextToken(in, line);

		if (tok.GetToken() != SCOMA) {
			ParseError(line, "Missing semicolon");
			return false;
		} else {
			tok = GetNextToken(in, line);
		}
	}

	switch (tok.GetToken()) {
	case BEGIN:
		ParseError(line, "Unexpected BEGIN");
		return false;
	case DONE:
		ParseError(line, "Unexpected DONE");
		return false;
	case ERR:
		ParseError(line, "Lexical Error");
		return false;
	case END:
		PushBackToken(tok);
		return true;
	}
}

//PrintStmt := print ExprList
bool PrintStmt(istream &in, int &line) {
	//cout << "in PrintStmt" << endl;

	ValQue = new queue<Value>;
	bool status = ExprList(in, line);

	if (!status) {
		ParseError(line, "Missing expression after print");

		//empty queue
		while (!(*ValQue).empty()) {
			ValQue->pop();
		}
		delete ValQue;
		return false;
	}

	LexItem tok = GetNextToken(in, line);

	if (tok.GetToken() == SCOMA) {

		//while queue is not empty, get first element from queue, print, then remove
		while (!(*ValQue).empty()) {
			Value val = (*ValQue).front();
			cout << val;
			ValQue->pop();
		}
		cout << endl;
	}
	PushBackToken(tok);
	return status;
}

//IfStmt := if (Expr) then Stmt
bool IfStmt(istream &in, int &line) {
	//cout << "in IfStmt" << endl;

	LexItem tok = GetNextToken(in, line);

	if (tok.GetToken() != LPAREN) {
		ParseError(line, "Missing LPAREN");
		return false;
	} else {
		Value val;
		bool status = Expr(in, line, val);

		if (!status) {
			ParseError(line, "Missing expression after If");
			return false;
		}
		if (!val.IsInt()) {
			ParseError(line,
					"Run-Time Error-Illegal Type for If Statement Expression");
			return false;
		}
		tok = GetNextToken(in, line);

		if (tok.GetToken() != RPAREN) {
			ParseError(line, "Missing RPAREN");
			return false;
		}
		tok = GetNextToken(in, line);

		if (tok.GetToken() != THEN) {
			ParseError(line, "Missing THEN");
			return false;
		}
		if (val.GetInt() == 0) {
			tok = GetNextToken(in, line);

			if (tok.GetToken() != PRINT && tok.GetToken() != IF
					&& tok.GetToken() != IDENT) {
				ParseError(line, "Missing statement after If");
				return false;
			}
			while (tok.GetToken() != BEGIN && tok.GetToken() != END
					&& tok.GetToken() != DONE && tok.GetToken() != ERR) {

				tok = GetNextToken(in, line);

				if (tok.GetToken() == SCOMA) {
					PushBackToken(tok);
					return true;
				}
			}
		}
		status = Stmt(in, line);

		if (!status) {
			ParseError(line, "Missing statement after If");
			return false;
		}
	}
	return true;
}

//Var := ident
bool Var(istream &in, int &line, LexItem &tok) {
	//cout << "in Var" << endl;

	string identstr;
	LexItem t = GetNextToken(in, line);

	if (t.GetToken() == IDENT) {
		identstr = t.GetLexeme();
		auto it = defVar.find(identstr);
		if (it == defVar.end()) {
			defVar[identstr] = false;
			Value val = Value();
			symbolTable[identstr] = val;
		}
		tok = t;
		return true;
	}
	return false;
}

//AssignStmt := Var = Expr
bool AssignStmt(istream &in, int &line) {
	//cout << "in AssignStmt" << endl;

	LexItem tok;
	bool status = Var(in, line, tok);

	if (!status) {
		ParseError(line, "Missing Var");
		return false;
	}
	LexItem tokEqual = GetNextToken(in, line);
	if (tokEqual.GetToken() != EQ) {
		ParseError(line, "Missing EQ after assignment stmt");
		return false;
	}
	Value val;
	status = Expr(in, line, val);
	if (!status) {
		ParseError(line, "Missing Expr after assignment stmt");
		return false;
	}
	string str = tok.GetLexeme();
	if (!(defVar.find(str)->second)) {
		defVar[str] = true;
		symbolTable[str] = val;
	} else {
		ValType type = (symbolTable.find(str)->second).GetType();
		if (type == VSTR) {
			if (!val.IsStr()) {
				ParseError(line, "Run-Time Error-Illegal Assignment Operation");
				return false;
			}
			symbolTable[str] = val;
		} else {
			if (val.IsStr()) {
				ParseError(line, "Run-Time Error-Illegal Assignment Operation");
				return false;
			}
			if (type == VINT) {
				if (type == val.GetType()) {
					symbolTable[str] = val;
				} else {
					int num = (int) val.GetReal();
					Value newVal = Value(num);
					symbolTable[str] = newVal;
				}
			} else if (type == VREAL) {
				if (type == val.GetType()) {
					symbolTable[str] = val;
				} else {
					float num = (float) val.GetInt();
					Value newVal = Value(num);
					symbolTable[str] = newVal;
				}
			}
		}
	}
	return true;
}

//ExprList := Expr {, Expr}
bool ExprList(istream &in, int &line) {
	//cout << "in ExprList" << endl;

	LexItem tok;
	Value val;
	bool status = Expr(in, line, val);

	if (!status) {
		ParseError(line, "Missing expression");
		return false;
	}
	ValQue->push(val);
	tok = GetNextToken(in, line);
	if (tok.GetToken() == COMA) {
		PushBackToken(tok);
		while (tok.GetToken() != BEGIN && tok.GetToken() != END
				&& tok.GetToken() != DONE && tok.GetToken() != ERR) {
			tok = GetNextToken(in, line);
			if (tok.GetToken() == COMA) {
				status = Expr(in, line, val);
				if (!status) {
					ParseError(line, "Missing expression after coma");
					return false;
				}
				ValQue->push(val);
			} else {
				PushBackToken(tok);
				break;
			}
		}
	} else {
		PushBackToken(tok);
	}
	return true;
}

//Stmt is either a PrintStmt, IfStmt, or an AssignStmt
bool Stmt(istream &in, int &line) {
	//cout << "in Stmt" << endl;

	bool status;
	LexItem tok = GetNextToken(in, line);

	switch (tok.GetToken()) {
	case PRINT:
		status = PrintStmt(in, line);
		return status;
	case IF:
		status = IfStmt(in, line);
		return status;
	case IDENT:
		//cout << "in IDENT" << endl;
		PushBackToken(tok);
		status = AssignStmt(in, line);
		return status;
	case END:
		PushBackToken(tok);
		return true;
	}
	ParseError(line, "Invalid statement");
	return false;
}

//Expr := Term {(+|-) Term}
bool Expr(istream &in, int &line, Value &retVal) {
	//cout << "in Expr" << endl;

	LexItem tok;
	Value val;
	bool status = Term(in, line, val);

	if (!status) {
		return false;
	}
	tok = GetNextToken(in, line);
	if (tok.GetToken() == PLUS || tok.GetToken() == MINUS) {
		PushBackToken(tok);
		while (tok.GetToken() != BEGIN && tok.GetToken() != END
				&& tok.GetToken() != DONE && tok.GetToken() != ERR) {
			tok = GetNextToken(in, line);
			if (tok.GetToken() == PLUS || tok.GetToken() == MINUS) {
				Value rval;
				status = Term(in, line, rval);
				if (!status) {
					ParseError(line, "Missing term after operator");
					return false;
				}
				if (rval.IsStr()) {
					ParseError(line,
							"Run-Time Error-Illegal String Type operation");
					return false;
				}
				if (tok.GetToken() == PLUS) {
					val = val + rval;
				} else {
					val = val - rval;
				}
			} else {
				PushBackToken(tok);
				break;
			}
		}
	} else {
		PushBackToken(tok);
	}
	retVal = val;
	return true;
}

//Term := Factor {(*|/) Factor}
bool Term(istream &in, int &line, Value &retVal) {
	//cout << "in Term" << endl;

	LexItem tok;
	Value val;
	bool status = Factor(in, line, val);

	if (!status) {
		return false;
	}
	tok = GetNextToken(in, line);
	if (tok.GetToken() == MULT || tok.GetToken() == DIV) {
		PushBackToken(tok);
		while (tok.GetToken() != BEGIN && tok.GetToken() != END
				&& tok.GetToken() != DONE && tok.GetToken() != ERR) {
			tok = GetNextToken(in, line);
			if (tok.GetToken() == MULT || tok.GetToken() == DIV) {
				Value rval;
				status = Factor(in, line, rval);
				if (!status) {
					ParseError(line, "Missing factor after operator");
					return false;
				}
				if (rval.IsStr()) {
					ParseError(line,
							"Run-Time Error-Illegal String Type operation");
					return false;
				}
				if (tok.GetToken() == MULT) {
					val = val * rval;
				} else {
					val = val / rval;
				}
			} else {
				PushBackToken(tok);
				break;
			}
		}
	} else {
		PushBackToken(tok);
	}
	retVal = val;
	return true;
}

//Factor := ident | iconst | rconst | sconst | (Expr)
bool Factor(istream &in, int &line, Value &retVal) {
	//cout << "in Factor" << endl;

	LexItem tok = GetNextToken(in, line);

	if (tok == IDENT) {
		//check if variable is defined

		string lexeme = tok.GetLexeme();

		if (!(defVar.find(lexeme)->second)) {
			ParseError(line, "Undefined Variable");
			return false;
		}

		retVal = symbolTable.find(lexeme)->second;
		return true;
	} else if (tok.GetToken() == ICONST) {
		//convert the string of digits to an integer number

		string lexeme = tok.GetLexeme();
		int val;
		val = stoi(lexeme);
		//cout << "integer const: " << val << endl;

		//create a Value object for ICONST that will enter into symbol table container
		Value newval(val);
		retVal = newval;
		return true;
	} else if (tok.GetToken() == SCONST) {
		string lexeme = tok.GetLexeme();
		//cout << "string const: " << lexeme << endl;

		//create a Value object for SCONST that will enter into symbol table container
		Value newval(lexeme);
		retVal = newval;
		return true;
	} else if (tok.GetToken() == RCONST) {
		//convert the string of digits to a real number

		string lexeme = tok.GetLexeme();
		float val;
		val = stof(lexeme);
		//cout << "real const: " << val << endl;

		//create a Value object for RCONST that will enter into symbol table container
		Value newval(val);
		retVal = newval;
		return true;
	} else if (tok.GetToken() == LPAREN) {
		Value val;
		bool status = Expr(in, line, val);
		retVal = val;

		if (!status) {
			ParseError(line, "Missing expression after (");
			return false;
		}
		tok = GetNextToken(in, line);

		if (tok.GetToken() == RPAREN) {
			return status;
		} else {
			ParseError(line, "Missing ) after expression");
			return false;
		}
		return false;
	} else if (tok.GetToken() == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	ParseError(line, "Unrecognized input");
	return 0;
}
