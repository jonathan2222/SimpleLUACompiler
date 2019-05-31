#ifndef AST_HH
#define AST_HH

#include <list>
#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <fstream>

#include "Data.hh"
#include "Comp.hh"
#include "Symbols.hh"

std::string strSides(const std::string str, std::string c = "\\\"");

class Node
{
public:
	Node(const std::string& tag);
	Node(const std::string& tag, const Data& data);
	virtual ~Node();

	virtual std::string makeName(const std::string& prefix = "_t_");
	std::string makeId();
	
	void dump(int depth=0);
	std::string dot();
	void toCfg(BBlock* start);
	void symbolsTocfg(std::ofstream& file, Node* node);
	void fillCfg(std::ofstream& file, BBlock* start);
	
	virtual std::pair<BBlock*, std::string> convert(BBlock* out);
	
	void addChild(Node* child, bool isTopBlock = false);

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
	
	bool isTopBlock;

	Symbols* symbols = nullptr;

	std::vector<BBlock*> functions;

private:
	static unsigned counterId;
	static unsigned counterName;

protected:
	bool isUserVar(const std::string& s) const;
	int getArg(const std::string& s);
	std::string getNameASM(const std::string& s);

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
	Constant() : Expression("Constant", Data("NO_NAME")) {}
	Constant(bool b) : Expression("Constant", Data("NO_NAME", b)) {}
	Constant(double f) : Expression("Constant", Data("NO_NAME", f)) {}
	Constant(const std::string& s) : Expression("Constant", Data("NO_NAME", s)) {}
	virtual ~Constant() {}

	std::string makeName(const std::string& prefix = "_t_") override;
	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Var : public Expression
{
public:
	Var(const std::string& name) : Expression("Var", Data(name)) { }
	virtual ~Var() {}

	std::string makeName(const std::string& prefix = "_t_") override;
	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
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

class AssignmentOne : public Statement
{
public:
	AssignmentOne() : Statement("AssignmentOne") {}
	virtual ~AssignmentOne() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out, bool shouldAddSym);
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
	std::vector<Symbol> arr;
};

// ---------------------------------------------------------------------------------------------------------

class TableIndex : public Expression
{
public:
	TableIndex() : Expression("TableIndex", Data(Data::Type::NUMBER)) {}
	virtual ~TableIndex() {}

	virtual std::pair<BBlock*, std::string> convert(BBlock* out) override;
};

// ---------------------------------------------------------------------------------------------------------

class Hash : public Expression
{
public:
	Hash() : Expression("Hash", Data(Data::Type::NUMBER)) {}
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