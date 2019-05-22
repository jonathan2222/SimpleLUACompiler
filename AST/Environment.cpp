#include "Environment.hh"

#include <iostream>
#include "AST.hh"

Environment::~Environment()
{
	for(auto& e : this->dictionary)
		if(e.second != nullptr)
			delete e.second;
	this->dictionary.clear();	
}

Entry* Environment::read(std::string key)
{
	auto it = this->dictionary.find(key);
	if(it != this->dictionary.end())
		return it->second;
	std::cout << "[ERROR] Could not read data [" << key << "] from environment!" << std::endl;
	return nullptr;
}

void Environment::write(std::string key, Entry* entry)
{
	if(hasKey(key))
		if(this->dictionary[key] != nullptr)
		{
			Entry* old = this->dictionary[key];
			delete old; 
		}
	this->dictionary[key] = entry;
}

bool Environment::hasKey(std::string key) const
{
	return this->dictionary.find(key) != this->dictionary.end();
}

unsigned int Environment::getLength() const
{
	return this->dictionary.size();
}

void Environment::print(unsigned depth) const
{
	std::string padding = "";
	for(int i = 0; i < depth; i++) padding += "\t";

	std::cout << padding << "Environment: \n";
	auto it = this->dictionary.begin();
	for(;it != this->dictionary.end(); it++)
		std::cout << padding << "\t" << it->first << ": " << entryToString(it->second, depth);
}

std::string Environment::entryToString(Entry* entry, unsigned depth) const
{	
	std::string s = "";
	std::string padding = "";
	for(int i = 0; i <= depth; i++) padding += "\t";

	std::string sp = entry->returns.size() > 1 ? "\n" + padding : "";
	for(Ret& ret : entry->returns)
	{
		if(ret.self->scope == nullptr)
			s += sp + ret.data.toStringEx() + "\n";
		else
		{
			Environment* env = ret.self->scope;
			std::string si = "Environment\n";
			auto it = env->dictionary.begin();
			for(;it != env->dictionary.end(); it++)
				si += padding + "\t" + it->first + ": " + entryToString(it->second, depth);
			s += si;
		}
	}
	return s;
}