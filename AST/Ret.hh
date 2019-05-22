#ifndef AST_RET_HH
#define AST_RET_HH

#include "Data.hh"

class Node;
class Environment;
struct Entry;
struct Ret
{
	/*
		Empty return data.
	*/
	Ret() = default;
	Ret(const Ret& other) = default;
	/*
		Copy data of node, and set 'parent' and 'self'.
	*/
	void to(Node* node);
	/*
		Copy data of node, and set 'parent', 'self' and set 'from' to the old 'self'.
	*/
	void convert(Node* node);

	bool hasEnvironment() const;

	/*
		Find the closest environment. Time complexity is O(n) in worst case.
		Return:	Closest environment, nullptr if not found.
	*/
	Environment* findLocal(bool skipRedirects = false);
	
	/*
		Find the global environment. Time complexity is O(n) in worst case.
		Return:	Global environment, nullptr if not found (This should never happen!).
	*/
	Environment* findGlobal(bool skipRedirects = false);

	/*
		Try to write (entry) as (key) to the global environment. Time complexity is O(n) in worst case.
	*/
	void writeGlobal(const std::string& key, Entry* ret, bool skipRedirects = false);
	
	/*
		Try to write (entry) as (key) to the local environment. Time complexity is O(n) in worst case.
	*/
	void writeLocal(const std::string& key, Entry* ret, bool skipRedirects = false);
	
	/*
		Try to read (key) from the global environment. Time complexity is O(n) in worst case.
		Return:	Entry, nullptr if not found.
	*/
	Entry* readGlobal(const std::string& key, bool skipRedirects = false);
	
	/*
		Try to read (key) from the local environment. Time complexity is O(n) in worst case.
		Return:	Entry, nullptr if not found.
	*/
	Entry* readLocal(const std::string& key, bool skipRedirects = false);

	/*
		Try to find the entry which was stored with the 'key' in the environment of type 'searchType'.
		If 'searchType' is of type MAX. it will look in any entry which has an environment.
		Time complexity is O(n) in worst case.
		Return: Entry, nullptr if not found. 
	*/
	Entry* findEntry(const std::string& key, Data::Type searchType = Data::Type::MAX, bool skipRedirects = false, std::string depth="");

	void writeToEntry(const std::string& key, Entry* entry, Data::Type searchType = Data::Type::MAX, bool skipRedirects = false);

	void print() const;

	Data data;
	Node* parent = nullptr;
	Node* from = nullptr;
	Node* self = nullptr;
	bool skipRedirects = false;
};

#endif