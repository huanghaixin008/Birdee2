// Birdee.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>
#include "Tokenizer.h"
#include <iostream>

using namespace Birdee;

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

Tokenizer tokenizer(fopen("test.txt", "r"));

namespace Birdee{

	void formatprint(int level) {
		for (int i = 0; i < level; i++)
			std::cout << "\t";
	}

	/// StatementAST - Base class for all expression nodes.
	class StatementAST {
	public:
		SourcePos Pos{ tokenizer.GetLine(),tokenizer.GetPos() };
		virtual ~StatementAST() = default;
		virtual void print(int level) {
			for (int i = 0; i < level; i++)
				std::cout << "\t";
		}
	};

	class ExprAST : public StatementAST {
	public:
		virtual ~ExprAST() = default;
	};

	/// NumberExprAST - Expression class for numeric literals like "1.0".
	class NumberExprAST : public ExprAST {
		NumberLiteral Val;
	public:
		NumberExprAST(const NumberLiteral& Val) : Val(Val) {}
		void print(int level) {
			ExprAST::print(level); 
			switch (Val.type)
			{
			case const_int:
				std::cout << "const int "<<Val.v_int << "\n";
				break;
			case const_long:
				std::cout << "const long " << Val.v_long << "\n";
				break;
			case const_uint:
				std::cout << "const uint " << Val.v_uint << "\n";
				break;
			case const_ulong:
				std::cout << "const ulong " << Val.v_ulong << "\n";
				break;
			case const_float:
				std::cout << "const float " << Val.v_double << "\n";
				break;
			case const_double:
				std::cout << "const double " << Val.v_double << "\n";
				break;
			}
		}
	};

	class ReturnAST : public StatementAST {
		std::unique_ptr<ExprAST> Val;
	public:
		ReturnAST(std::unique_ptr<ExprAST>&& val, SourcePos pos) : Val(std::move(val)) { Pos = pos; };
		void print(int level) {
			StatementAST::print(level); std::cout<<"Return\n";
			Val->print(level + 1);
		}
	};

	class StringLiteralAST : public ExprAST {
		std::string Val;
	public:
		StringLiteralAST(const std::string& Val) : Val(Val) {}
		void print(int level) { ExprAST::print(level); std::cout << "\"" <<Val << "\"\n"; }
	};

	/// IdentifierExprAST - Expression class for referencing a variable, like "a".
	class IdentifierExprAST : public ExprAST {
		std::string Name;
	public:
		IdentifierExprAST(const std::string &Name) : Name(Name) {}
		void print(int level) { ExprAST::print(level); std::cout << "Identifier:" << Name << "\n"; }
	};
	/// IfBlockAST - Expression class for If block.
	class IfBlockAST : public StatementAST {
		std::unique_ptr<ExprAST> cond;
		std::vector<std::unique_ptr<StatementAST>> iftrue;
		std::vector<std::unique_ptr<StatementAST>> iffalse;
	public:
		IfBlockAST(std::unique_ptr<ExprAST>&& cond,
		std::vector<std::unique_ptr<StatementAST>>&& iftrue,
		std::vector<std::unique_ptr<StatementAST>>&& iffalse,
		SourcePos pos)
			: cond(std::move(cond)), iftrue(std::move(iftrue)), iffalse(std::move(iffalse)) {
			Pos = pos;
		}

		void print(int level) {
			StatementAST::print(level);	std::cout << "If" << "\n";
			cond->print(level + 1);
			StatementAST::print(level+1);	std::cout << "Then" << "\n";
			for (auto&& s : iftrue)
				s->print(level + 2);
			StatementAST::print(level + 1);	std::cout << "Else" << "\n";
			for (auto&& s : iffalse)
				s->print(level + 2);
		}
	};

	/// BinaryExprAST - Expression class for a binary operator.
	class BinaryExprAST : public ExprAST {
		Token Op;
		std::unique_ptr<ExprAST> LHS, RHS;
	public:
		BinaryExprAST(Token Op, std::unique_ptr<ExprAST>&& LHS,
			std::unique_ptr<ExprAST>&& RHS)
			: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
		BinaryExprAST(Token Op, std::unique_ptr<ExprAST>&& LHS,
			std::unique_ptr<ExprAST>&& RHS,SourcePos pos)
			: Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {
			Pos = pos;
		}
		void print(int level) {
			ExprAST::print(level);
			std::cout << "OP:" << tokenizer.tok_names[Op] << "\n";
			LHS->print(level + 1);
			ExprAST::print(level+1); std::cout << "----------------\n";
			RHS->print(level + 1);
		}
	};
	/// BinaryExprAST - Expression class for a binary operator.
	class IndexExprAST : public ExprAST {
		std::unique_ptr<ExprAST> Expr, Index;
	public:
		IndexExprAST(std::unique_ptr<ExprAST>&& Expr,
			std::unique_ptr<ExprAST>&& Index,SourcePos Pos)
			: Expr(std::move(Expr)), Index(std::move(Index)) {
			this->Pos = Pos;
		}
		IndexExprAST(std::unique_ptr<ExprAST>&& Expr,
			std::unique_ptr<ExprAST>&& Index)
			: Expr(std::move(Expr)), Index(std::move(Index)) {}
		void print(int level) {
			ExprAST::print(level);
			std::cout << "Index\n";
			Expr->print(level + 1);
			ExprAST::print(level + 1); std::cout << "----------------\n";
			Index->print(level + 1);
		}
	};
	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		std::unique_ptr<ExprAST> Callee;
		std::vector<std::unique_ptr<ExprAST>> Args;

	public:

		void print(int level)
		{
			ExprAST::print(level); std::cout << "Call\n";
			Callee->print(level + 1);
			formatprint(level+1); std::cout << "---------\n";
			for (auto&& n : Args)
				n->print(level + 1);

		}
		CallExprAST(std::unique_ptr<ExprAST> &&Callee,
			std::vector<std::unique_ptr<ExprAST>>&& Args)
			: Callee(std::move(Callee)), Args(std::move(Args)) {}
	};



	class Type {
		
	public:
		Token type;
		int index_level = 0;
		virtual ~Type() = default;
		Type(Token _type):type(_type){}
		virtual void print(int level)
		{
			formatprint(level);
			std::cout << "Type " << type << " index: "<<index_level;
		}
	};

	class IdentifierType :public Type {
		std::string name;
	public:
		IdentifierType(const std::string&_name):Type(tok_identifier), name(_name){}
		void print(int level)
		{
			Type::print(level);
			std::cout << " Name: " << name;
		}
	};

	class VariableDefAST : public StatementAST {
	};



	class VariableSingleDefAST : public VariableDefAST {
		std::string name;
		std::unique_ptr<Type> type;
		std::unique_ptr<ExprAST> val;
	public:
		VariableSingleDefAST(const std::string& _name, std::unique_ptr<Type>&& _type, std::unique_ptr<ExprAST>&& _val) : name(_name),type(std::move(_type)), val(std::move(_val)){}
		VariableSingleDefAST(const std::string& _name, std::unique_ptr<Type>&& _type, std::unique_ptr<ExprAST>&& _val,SourcePos Pos) : name(_name), type(std::move(_type)), val(std::move(_val)) {
			this->Pos = Pos;
		}
		void print(int level) {
			VariableDefAST::print(level);
			std::cout << "Variable:" << name << " ";
			type->print(0);
			std::cout << "\n";
			if(val)
				val->print(level + 1);
		}
	};

	class VariableMultiDefAST : public VariableDefAST {
		std::vector<std::unique_ptr<VariableSingleDefAST>> lst;
	public:
		VariableMultiDefAST(std::vector<std::unique_ptr<VariableSingleDefAST>>&& vec) :lst(std::move(vec)) {}
		VariableMultiDefAST(std::vector<std::unique_ptr<VariableSingleDefAST>>&& vec, SourcePos pos) :lst(std::move(vec)) {
			Pos = pos;
		}
		void print(int level) {
			//VariableDefAST::print(level);
			for (auto& a : lst)
				a->print(level);
		}
	};


	/// PrototypeAST - This class represents the "prototype" for a function,
	/// which captures its name, and its argument names (thus implicitly the number
	/// of arguments the function takes).
	class PrototypeAST {
	protected:
		std::string Name;
		std::unique_ptr<VariableDefAST> Args;
		std::unique_ptr<Type> RetType;

	public:
		PrototypeAST(const std::string &Name, std::unique_ptr<VariableDefAST>&& Args, std::unique_ptr<Type>&& RetType)
			: Name(Name), Args(std::move(Args)), RetType(std::move(RetType)) {}

		const std::string &getName() const { return Name; }
		void print(int level)
		{
			formatprint(level);
			std::cout << "Function Proto: " << Name << std::endl;
			if(Args)
				Args->print(level+1);
			else
			{
				formatprint(level + 1); std::cout << "No arg\n";
			}
			RetType->print(level + 1); std::cout << "\n";
		}
	};

	/// FunctionAST - This class represents a function definition itself.
	class FunctionAST : public ExprAST {
		std::unique_ptr<PrototypeAST> Proto;
		std::vector<std::unique_ptr<StatementAST>> Body;

	public:
		FunctionAST(std::unique_ptr<PrototypeAST> Proto,
			std::vector<std::unique_ptr<StatementAST>>&& Body)
			: Proto(std::move(Proto)), Body(std::move(Body)) {}
		FunctionAST(std::unique_ptr<PrototypeAST> Proto,
			std::vector<std::unique_ptr<StatementAST>>&& Body, SourcePos pos)
			: Proto(std::move(Proto)), Body(std::move(Body)) {
			Pos = pos;
		}
		void print(int level)
		{
			ExprAST::print(level);
			std::cout << "Function def\n";
			Proto->print(level+1);
			for (auto&& node : Body)
			{
				node->print(level+1);
			}
		}
	};


	class MemberDef
	{
	public:
		enum AccessModifier
		{
			access_public,
			access_private
		}access;
		enum FunctionOrVariable
		{
			member_function,
			member_variable
		}kind;
		std::unique_ptr<StatementAST> decl;
		void print(int level)
		{
			formatprint(level);
			std::cout << "Member ";
			if (access == access_private)
				std::cout << "private ";
			else
				std::cout << "public ";
			if (kind == member_function)
				std::cout << "function ";
			else
				std::cout << "variable ";
			decl->print(level);
		}
		MemberDef(AccessModifier access, FunctionOrVariable kind, std::unique_ptr<StatementAST>&& decl):access(access),kind(kind),decl(std::move(decl)) {}
	};

	class ClassAST : public StatementAST {
		std::string name;
		std::vector<MemberDef> Body;

	public:


		ClassAST(const std::string& name,
			std::vector<MemberDef>&& Body,SourcePos pos)
			: name(name), Body(std::move(Body)) {
			Pos = pos;
		}
		void print(int level)
		{
			StatementAST::print(level);
			std::cout << "Class def: "<<name<<std::endl;
			for (auto& node : Body)
			{
				node.print(level + 1);
			}
		}
	};

	class CompileError {
		int linenumber;
		int pos;
		std::string msg;
	public:
		CompileError(int _linenumber, int _pos, const std::string& _msg): linenumber(_linenumber),pos(_pos),msg(_msg){}
		CompileError(const std::string& _msg) : linenumber(tokenizer.GetLine()), pos(tokenizer.GetPos()), msg(_msg) {}
		void print()
		{
			printf("Compile Error at line %d, postion %d : %s", linenumber, pos, msg.c_str());
		}
	};
}




inline void CompileExpect(Token expected_tok, const std::string& msg)
{
	if (tokenizer.CurTok != expected_tok)
	{
		throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), msg);
	}

	tokenizer.GetNextToken();
}


inline void CompileAssert(bool a, const std::string& msg)
{
	if (!a)
	{
		throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), msg);
	}
}

inline void CompileExpect(std::initializer_list<Token> expected_tok, const std::string& msg)
{

	for(Token t :expected_tok)
	{
		if (t == tokenizer.CurTok)
		{
			tokenizer.GetNextToken();
			return;
		}
	}
	throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), msg);
}

template<typename T>
T CompileExpectNotNull(T&& p, const std::string& msg)
{
	if(!p)
		throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), msg);
	return std::move(p);
}

/////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr<ExprAST> ParseExpressionUnknown();
std::unique_ptr<ExprAST> ParseFunction();
std::unique_ptr<IfBlockAST> ParseIf();
////////////////////////////////////////////////////////////////////////////////////


std::unique_ptr<Type> ParseType()
{
	//CompileExpect(tok_as, "Expected \'as\'");
	if(tokenizer.CurTok!= tok_as)
		return std::make_unique<Type>(tok_auto);
	tokenizer.GetNextToken(); //eat as
	std::unique_ptr<Type> type;
	switch (tokenizer.CurTok)
	{
	case tok_identifier:
		type = std::make_unique<IdentifierType>(tokenizer.IdentifierStr);
		break;
	case tok_int:
		type = std::make_unique<Type>(tok_int);
		break;
	default:
		throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), "Expected an identifier or basic type name");
	}
	tokenizer.GetNextToken();
	while (tokenizer.CurTok == tok_left_index)
	{
		type->index_level++;
		tokenizer.GetNextToken();//eat [
		CompileExpect(tok_right_index, "Expected  \']\'");
	}
	return std::move(type);
}

std::unique_ptr<VariableSingleDefAST> ParseSingleDim()
{
	SourcePos pos = tokenizer.GetSourcePos();
	std::string identifier = tokenizer.IdentifierStr;
	CompileExpect(tok_identifier, "Expected an identifier to define a variable");
	std::unique_ptr<Type> type= ParseType();
	std::unique_ptr<ExprAST> val;
	if (tokenizer.CurTok == tok_assign)
	{
		tokenizer.GetNextToken();
		val = ParseExpressionUnknown();
	}
	
	return std::make_unique<VariableSingleDefAST>(identifier, std::move(type), std::move(val),pos);
}

std::unique_ptr<VariableDefAST> ParseDim()
{
	std::vector<std::unique_ptr<VariableSingleDefAST>> defs;
	auto pos = tokenizer.GetSourcePos();
	auto def = ParseSingleDim();
	defs.push_back(std::move(def));
	for (;;)
	{
		
		switch (tokenizer.CurTok)
		{
		case tok_comma:
			tokenizer.GetNextToken();
			def = ParseSingleDim();
			defs.push_back(std::move(def));
			break;
		case tok_newline:
		case tok_eof:
		case tok_right_bracket:
			goto done;
			break;
		default:
			throw CompileError(tokenizer.GetLine(),tokenizer.GetPos(), "Expected a new line after variable definition");
		}
		
	}
done:
	if (defs.size() == 1)
		return std::move(defs[0]);
	else 
		return std::make_unique<VariableMultiDefAST>(std::move(defs),pos);
}

std::vector<std::unique_ptr<ExprAST>> ParseArguments()
{
	std::vector<std::unique_ptr<ExprAST>> ret;
	if (tokenizer.CurTok == tok_right_bracket)
	{
		tokenizer.GetNextToken();
		return std::move(ret);
	}
	for (;;)
	{
		ret.push_back(ParseExpressionUnknown());
		if (tokenizer.CurTok == tok_right_bracket)
		{
			tokenizer.GetNextToken();
			return ret;
		}
		else if (tokenizer.CurTok == tok_comma)
		{
			tokenizer.GetNextToken();
		}
		else
		{
			throw CompileError("Unexpected Token, expected \')\'");
		}
	}
	
}

std::unique_ptr<ExprAST> ParsePrimaryExpression()
{
	std::unique_ptr<ExprAST> firstexpr;
	switch (tokenizer.CurTok)
	{
	case tok_identifier:
		firstexpr = std::make_unique<IdentifierExprAST>(tokenizer.IdentifierStr);
		break;
	case tok_number:
		firstexpr = std::make_unique<NumberExprAST>(tokenizer.NumVal);
		break;
	case tok_string_literal:
		firstexpr = std::make_unique<StringLiteralAST>(tokenizer.IdentifierStr);
		break;
	case tok_left_bracket:
		tokenizer.GetNextToken();
		firstexpr = ParseExpressionUnknown();
		CompileAssert(tok_right_bracket == tokenizer.CurTok, "Expected \')\'");
		break;
	case tok_func:
		tokenizer.GetNextToken(); //eat function
		firstexpr = ParseFunction();
		CompileAssert(tok_newline == tokenizer.CurTok, "Expected newline after function definition");
		return firstexpr; //don't eat the newline token!
		break;
	case tok_eof:
		return nullptr;
		break;
	default:
		throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), "Expected an expression");
	}
	tokenizer.GetNextToken(); //eat token
	auto parse_tail_token = [&firstexpr]()
	{
		auto pos = tokenizer.GetSourcePos();
		switch (tokenizer.CurTok)
		{
		case tok_left_index:
			tokenizer.GetNextToken();//eat [
			firstexpr = std::make_unique<IndexExprAST>(std::move(firstexpr), CompileExpectNotNull(ParseExpressionUnknown(), "Expected an expression for index"), pos);
			CompileExpect(tok_right_index, "Expected  \']\'");
			return true;
		case tok_left_bracket:
			tokenizer.GetNextToken();//eat [
			firstexpr = std::make_unique<CallExprAST>(std::move(firstexpr), ParseArguments());
			return true;
		default:
			return false;
		}
	};
	while (parse_tail_token()) //check for index/function call
	{	}
	return firstexpr;
}




std::map<Token, int> tok_precedence = {
{tok_mul,15},
{tok_div,15},
{tok_mod,15},
{ tok_add,14 },
{ tok_minus,14 },
{tok_lt,11},
{tok_gt,11},
{tok_le,11},
{tok_ge,11},
{tok_equal,10},
{tok_ne,10},
{tok_cmp_equal,10},
{tok_and,9},
{tok_xor,8},
{tok_or,7},
{tok_logic_and,6},
{tok_logic_or,5},
{tok_assign,4},
};


int GetTokPrecedence()
{
	Token tok = tokenizer.CurTok;
	switch (tokenizer.CurTok)
	{
	case tok_eof:
	case tok_newline:
	case tok_right_bracket:
	case tok_right_index:
	case tok_comma:
	case tok_then:
		return -1;
		break;
	default:
		auto f = tok_precedence.find(tok);
		if (f != tok_precedence.end())
		{
			return f->second;
		}
		throw CompileError(tokenizer.GetLine(), tokenizer.GetPos(), "Unknown Token");
	}
	return -1;
}

/*
Great code to handle binary operators. Copied from:
https://llvm.org/docs/tutorial/LangImpl02.html
*/
std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
	std::unique_ptr<ExprAST> LHS) {
	// If this is a binop, find its precedence.
	
	while (true) {
		SourcePos pos = tokenizer.GetSourcePos();
		int TokPrec = GetTokPrecedence();

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (TokPrec < ExprPrec)
			return LHS;

		// Okay, we know this is a binop.
		Token BinOp = tokenizer.CurTok;
		tokenizer.GetNextToken(); // eat binop

						// Parse the primary expression after the binary operator.
		auto RHS = CompileExpectNotNull(ParsePrimaryExpression(), "Expected an expression");

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = CompileExpectNotNull(
				ParseBinOpRHS(TokPrec + 1, std::move(RHS)),
				"Expected an expression");			
		}

		// Merge LHS/RHS.
		LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
			std::move(RHS),pos);
	}
}

/*
Parse an expression when we don't know what kind of it is. Maybe an identifier? A function def?
*/
std::unique_ptr<ExprAST> ParseExpressionUnknown()
{
	
	return ParseBinOpRHS(0, 
		CompileExpectNotNull(ParsePrimaryExpression(),"Expected an expression"));
}




void ParseBasicBlock(std::vector<std::unique_ptr<StatementAST>>& body,Token optional_tok)
{
	std::unique_ptr<ExprAST> firstexpr;
	SourcePos pos(0, 0);
	while (true)
	{

		switch (tokenizer.CurTok)
		{
		case tok_newline:
			tokenizer.GetNextToken(); //eat newline
			continue;
			break;
		case tok_dim:
			tokenizer.GetNextToken(); //eat dim
			body.push_back(std::move(ParseDim()));
			CompileExpect({ tok_newline,tok_eof }, "Expected a new line after variable definition");
			break;
		case tok_return:
			tokenizer.GetNextToken(); //eat return
			pos = tokenizer.GetSourcePos();
			body.push_back(std::make_unique<ReturnAST>(ParseExpressionUnknown(),pos));
			CompileExpect({ tok_newline,tok_eof }, "Expected a new line");
			break;
		case tok_func:
			tokenizer.GetNextToken(); //eat function
			body.push_back(std::move(ParseFunction()));
			CompileExpect({ tok_newline,tok_eof }, "Expected a new line after function definition");
			break;
		case tok_end:
			tokenizer.GetNextToken(); //eat end
			if (optional_tok>=0 && tokenizer.CurTok == optional_tok) //optional: end function/if/for...
				tokenizer.GetNextToken(); //eat it
			goto done;
			break;
		case tok_else:
			//don't eat else here
			goto done;
			break;
		case tok_eof:
			throw CompileError("Unexpected end of file, missing \"end\"");
			break;
		case tok_if:
			tokenizer.GetNextToken(); //eat if
			body.push_back(std::move(ParseIf()));
			CompileExpect({ tok_newline,tok_eof }, "Expected a new line after if-block");
			break;
		default:
			firstexpr = ParseExpressionUnknown();
			CompileExpect({ tok_eof,tok_newline }, "Expect a new line after expression");
			CompileAssert(firstexpr != nullptr, "Compiler internal error: firstexpr=null");
			body.push_back(std::move(firstexpr));
		}
	}
done:
	return;
}
std::unique_ptr<IfBlockAST> ParseIf()
{
	SourcePos pos = tokenizer.GetSourcePos();
	std::unique_ptr<ExprAST> cond= ParseExpressionUnknown();
	CompileExpect(tok_then, "Expected \"then\" after the condition of \"if\"");
	CompileExpect(tok_newline, "Expected a newline after \"then\"");
	std::vector<std::unique_ptr<StatementAST>> true_block;
	std::vector<std::unique_ptr<StatementAST>> false_block;
	ParseBasicBlock(true_block, tok_if);
	if (tokenizer.CurTok == tok_else)
	{
		tokenizer.GetNextToken(); //eat else
		if (tokenizer.CurTok == tok_if) //if it is else-if
		{
			tokenizer.GetNextToken(); //eat if
			false_block.push_back(std::move(ParseIf()));
			//if-else don't need to eat the "end" in the end
		}
		else
		{
			CompileExpect(tok_newline, "Expected a newline after \"else\"");
			ParseBasicBlock(false_block, tok_if);
		}
	}
	CompileAssert(tokenizer.CurTok == tok_newline, "Expected a new line after \"if\" block");
	return std::make_unique<IfBlockAST>(std::move(cond), std::move(true_block), std::move(false_block), pos);
}

std::unique_ptr<ExprAST> ParseFunction()
{
	auto pos = tokenizer.GetSourcePos();
	std::string name;
	if (tokenizer.CurTok == tok_identifier)
	{
		name = tokenizer.IdentifierStr;
		tokenizer.GetNextToken();
	}
	CompileExpect(tok_left_bracket, "Expected \'(\'");
	std::unique_ptr<VariableDefAST> args;
	if (tokenizer.CurTok!= tok_right_bracket) 
		args = ParseDim();
	CompileExpect(tok_right_bracket, "Expected \')\'");
	auto rettype = ParseType();
	if (rettype->type == tok_auto)
		rettype->type = tok_void;
	auto funcproto = std::make_unique<PrototypeAST>(name,std::move(args),std::move(rettype));

	std::vector<std::unique_ptr<StatementAST>> body;
	
	//parse function body
	ParseBasicBlock(body,tok_func);
	return std::make_unique<FunctionAST>(std::move(funcproto), std::move(body),pos);
}

std::unique_ptr<ClassAST> ParseClass()
{
	auto pos = tokenizer.GetSourcePos();
	std::string name = tokenizer.IdentifierStr;
	CompileExpect(tok_identifier, "Expected an identifier for class name");
	CompileExpect(tok_newline, "Expected an newline after class name");
	std::vector<MemberDef> body; 
	while (true)
	{
		MemberDef::AccessModifier access;
		switch (tokenizer.CurTok)
		{
		case tok_newline:
			tokenizer.GetNextToken(); //eat newline
			continue;
			break;
		case tok_dim:
			throw CompileError("You should use public/private to define a member variable in a class");
			break;
		case tok_private:
		case tok_public:
			access = tokenizer.CurTok==tok_private? MemberDef::AccessModifier::access_private: MemberDef::AccessModifier::access_public;
			tokenizer.GetNextToken(); //eat access modifier
			if (tokenizer.CurTok == tok_identifier)
			{
				body.push_back(MemberDef(access, MemberDef::member_variable,ParseDim()));
				CompileExpect(tok_newline, "Expected a new line after variable definition");
			}
			else if (tokenizer.CurTok == tok_func)
			{
				tokenizer.GetNextToken(); //eat function
				body.push_back(MemberDef(access, MemberDef::member_function, ParseFunction()));
				CompileExpect(tok_newline, "Expected a new line after function definition");
			}
			else
			{
				throw CompileError("After public/private, only accept function/variable definitions");
			}
			break;
		case tok_eof:
			throw CompileError("Unexpected end of file, missing \"end\"");
			break;
		case tok_func:
			tokenizer.GetNextToken(); //eat function
			body.push_back(MemberDef(MemberDef::access_private, MemberDef::member_function, ParseFunction()));
			CompileExpect({ tok_newline }, "Expected a new line after function definition");
			break;
		case tok_end:
			tokenizer.GetNextToken(); //eat end
			if (tokenizer.CurTok == tok_class) //optional: end function
				tokenizer.GetNextToken(); //eat function
			goto done;
			break;
		default:
			throw CompileError("Expected member declarations");
		}
	}
done:
	return std::make_unique<ClassAST>(name, std::move(body),pos);
}

int ParseTopLevel(std::vector<std::unique_ptr<StatementAST>>& out)
{
	std::unique_ptr<ExprAST> firstexpr;
	tokenizer.GetNextToken();
	try {
		while (tokenizer.CurTok != tok_eof && tokenizer.CurTok != tok_error)
		{
			
			switch (tokenizer.CurTok)
			{
			case tok_newline:
				tokenizer.GetNextToken(); //eat newline
				continue;
				break;
			case tok_dim:
				tokenizer.GetNextToken(); //eat dim
				out.push_back(std::move(ParseDim()));
				CompileExpect({ tok_newline,tok_eof }, "Expected a new line after variable definition");
				break;
			case tok_eof:
				return 0;
				break;
			case tok_func:
				tokenizer.GetNextToken(); //eat function
				out.push_back(std::move(ParseFunction()));
				CompileExpect({ tok_newline,tok_eof }, "Expected a new line after function definition");
				break;
			case tok_if:
				tokenizer.GetNextToken(); //eat if
				out.push_back(std::move(ParseIf()));
				CompileExpect({ tok_newline,tok_eof }, "Expected a new line after if-block");
				break;
			case tok_class:
				tokenizer.GetNextToken(); //eat class
				out.push_back(std::move(ParseClass()));
				CompileExpect({ tok_newline,tok_eof }, "Expected a new line after class definition");
				break;
			default:
				firstexpr = ParseExpressionUnknown();
				CompileExpect({ tok_eof,tok_newline }, "Expect a new line after expression");
				//if (!firstexpr)
				//	break;
				CompileAssert(firstexpr != nullptr, "Compiler internal error: firstexpr=null");
				out.push_back(std::move(firstexpr));
			}
		}
	}
	catch (CompileError e)
	{
		e.print();
		return 1;
	}
	catch (TokenizerError e)
	{
		e.print();
		return 1;
	}
	return 0;
}

int main()
{
	std::vector<std::unique_ptr<StatementAST>> s;
	ParseTopLevel(s);
	for (auto&& node : s)
	{
		node->print(0);
	}
    return 0;
}

