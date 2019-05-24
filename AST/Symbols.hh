#ifndef SYMBOLS_HH
#define SYMBOLS_HH

#include <unordered_map>
#include <string>
#include "Data.hh"

class BBlock;
struct Symbol
{
	Symbol() {}
	Symbol(Data::Type type) { data.type = type; }
	unsigned int size = 0;

	std::string toString() const;

	Data data;
	BBlock* funcBlock = nullptr;
};

class Node;
class Symbols
{
public:
	Symbols(Node* parent) : parent(parent) {}
	virtual ~Symbols() {}

	bool hasKey(std::string key);
	void insert(std::string key, Symbol sym);
	Symbol get(std::string key);
	void removeKey(std::string key);

	void print(unsigned depth) const;

	std::unordered_map<std::string, Symbol> map;
	Node* parent = nullptr;
};

#endif