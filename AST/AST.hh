#ifndef AST_HH
#define AST_HH

#include <list>
#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <fstream>

#include "Data.hh"
#include "Ret.hh"
#include "Environment.hh"
#include "Comp.hh"
#include "Symbols.hh"

std::string strSides(const std::string str, std::string c = "\\\"");

class Node
{
public:
	Node(const std::string& tag);
	Node(const std::string& tag, const Data& data);
	virtual ~Node();

	virtual std::string makeName(const std::string& prefix = "t_");
	std::string makeId();
	
	void dump(int depth=0);
	std::string dot();
	void toCfg(BBlock* start);
	void symbolsTocfg(std::ofstream& file, Node* node);
	void fillCfg(std::ofstream& file, BBlock* start);
	
	virtual std::pair<BBlock*, std::string> convert(BBlock* out);

	virtual Entry& eval() { return this->entry; };
	
	void addChild(Node* child, bool isTopBlock = false);

	/*
		Convert the data to return data.
	*/
	void toRet();

	/*
		Set the ret to another one.
	*/
	void setRet(const Ret& other, unsigned index = 0);

	/*
		Like setRet but do not touch the pointers.
	*/
	void setData(const Ret& other, unsigned index = 0);

	void addSymbol(const std::string& key, Symbol& sym);
	Symbol getSymbol(const std::string& key);
	bool hasSymbol(const std::string& key);

	/*
		Combine symbols which is in the same cfg.
	*/
	void compressSymbols(Node* n = nullptr);

	Node* getTop();

	// ---------- DEBUG ----------
	bool haveChildrenSymbols() const;
	void printAllSymbols(unsigned depth = 0) const;
	std::string toString() const;
	void printAllEnv(unsigned depth = 0) const;
	unsigned printParents() const;
	// ---------------------------
	
	std::string id;
	std::string name;
	std::list<Node*> children;

	Node* parent = nullptr;

	// Type of node.
	std::string tag;
	// Data for each node.
	Data data;
	Data ret;
	// All nodes have return object. 
	Entry entry;
	// Environment
	Environment* scope = nullptr;
	bool isTopBlock;

	Symbols* symbols = nullptr;

	std::vector<BBlock*> functions;

private:
	static unsigned counterId;
	static unsigned counterName;

protected:
	bool hasName = false;
};

// ---------------------------------------------------------------------------------------------------------

class Expression : public Node
{
public:
	Expression(const std::string& tag) : Node(tag) {}
	Expression(const std::string& tag, const Data& data) : Node(tag, data) {}
	virtual ~Expression() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Constant : public Expression
{
public:
	Constant() : Expression("Constant", Data("NIL")) {}
	Constant(bool b) : Expression("Constant", Data("Bool", b)) {}
	Constant(double f) : Expression("Constant", Data("Number", f)) {}
	Constant(const std::string& s) : Expression("Constant", Data("String", s)) {}
	virtual ~Constant() {}

	std::string makeName(const std::string& prefix = "t_") override;
	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Var : public Expression
{
public:
	Var(const std::string& name) : Expression("Var", Data("Var", name)) { }
	virtual ~Var() {}

	std::string makeName(const std::string& prefix = "t_") override;
};

// ---------------------------------------------------------------------------------------------------------

class Paren : public Expression
{
public:
	Paren() : Expression("Paren") {}
	virtual ~Paren() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Operation : public Expression
{
public:
	Operation(const std::string& tag, const std::string& op) : Expression(tag), op(op) {}
	virtual ~Operation() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;

	std::string op;
};

// ---------------------------------------------------------------------------------------------------------

class Statement : public Node
{
public:
	Statement(const std::string tag) : Node(tag) {}
	Statement(const std::string tag, const Data& data) : Node(tag, data) {}
	virtual ~Statement() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Block : public Statement
{
public:
	Block() : Statement("Block", Data(Data::Type::SCOPE)) {}
	virtual ~Block() {}
};

// ---------------------------------------------------------------------------------------------------------

class Assignment : public Statement
{
public:
	Assignment() : Statement("Assignment") {}
	virtual ~Assignment() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class If : public Statement
{
public:
	If() : Statement("If") {}
	virtual ~If() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class For : public Statement
{
public:
	For() : Statement("For", Data(Data::Type::SCOPE)) {}
	virtual ~For() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Repeat : public Statement
{
public:
	Repeat() : Statement("Repeat") {}
	virtual ~Repeat() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class FunctionCall : public Expression
{
public:
	FunctionCall() : Expression("FunctionCall", Data(Data::Type::FUNCTIONCALL)) {}
	virtual ~FunctionCall() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Table : public Expression
{
public:
	Table() : Expression("Table", Data(Data::Type::TABLE)) {}
	virtual ~Table() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
	virtual std::pair<BBlock*, std::string> convert(BBlock* out, const Symbol& sym);
	std::vector<double> arr;
};

// ---------------------------------------------------------------------------------------------------------

class TableIndex : public Expression
{
public:
	TableIndex() : Expression("TableIndex") {}
	virtual ~TableIndex() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Hash : public Expression
{
public:
	Hash() : Expression("Hash") {}
	virtual ~Hash() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Function : public Statement
{
public:
	Function() : Statement("Function", Data(Data::Type::FUNCTION)) {}
	virtual ~Function() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Return : public Statement
{
public:
	Return() : Statement("Return") {}
	virtual ~Return() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class USubN : public Node
{
public:
	USubN(const std::string& tag) : Node(tag) {}
	virtual ~USubN() {}
};

#endif