#ifndef AST_ENVIRONMENT_HH
#define AST_ENVIRONMENT_HH

#include <unordered_map>
#include <vector>
#include "Ret.hh"

class Environment
{
public:
	Environment() = default;
	~Environment();

	Entry* read(std::string key);
	void write(std::string key, Entry* entry);
	bool hasKey(std::string key) const;

	unsigned int getLength() const;

	// --------- DEBUG ---------
	void print(unsigned depth) const;
	std::string entryToString(Entry* entry, unsigned depth) const;
	// -------------------------

private:
	std::unordered_map<std::string, Entry*> dictionary;
};

struct Entry
{
	Entry() : key("NO_KEY") {}
	Entry(const std::string& key) : key(key) {}
	std::string key;
	std::vector<Ret> returns;
};

#endif