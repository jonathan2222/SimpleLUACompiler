#include "Symbols.hh"

#include <iostream>
#include "Comp.hh"

std::string Symbol::toString() const
{
	return "(Type: " + TypeNames[this->type] + ", size: " + std::to_string(this->size) + 
		(this->type == Type::STRING? ", s: \\'" + this->s + "\\'" : 
			(this->type == Type::FUNCTION? ", block: " + this->funcBlock->name : "")) + ")";
}

bool Symbols::hasKey(std::string key)
{
	return this->map.find(key) != this->map.end();
}

void Symbols::insert(std::string key, Symbol sym)
{
	this->map[key] = sym;
	this->map[key].name = key;
}

Symbol Symbols::get(std::string key)
{
	return this->map[key];
}

void Symbols::removeKey(std::string key)
{
	this->map.erase(key);
}

void Symbols::print(unsigned depth) const
{
	std::string padding = "";
	for(int i = 0; i < depth; i++) padding += "\t";

	std::cout << padding << "Symbols: \n";
	auto it = this->map.begin();
	for(;it != this->map.end(); it++)
		std::cout << padding << "\t" << it->first << ": " << it->second.toString() << std::endl;
}