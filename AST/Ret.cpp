#include "Ret.hh"

#include "AST.hh"
#include "Ret.hh"
#include "Environment.hh"
#include <iostream>

void Ret::to(Node* node)
{
	this->data = node->data;
	this->parent = node->parent;
	this->self = node;
}

void Ret::convert(Node* node)
{
	this->data = data;
	this->from = this->self;
	this->self = node;
	this->parent = node->parent;
}

bool Ret::hasEnvironment() const
{
	return this->data.hasEnvironment();
}

Environment* Ret::findLocal(bool skipRedirects)
{
	if(this->data.hasEnvironment())
		return this->self->scope;

	if(this->from != nullptr && !skipRedirects)
	{
		Ret& retFrom = this->from->entry.returns[0];
		return retFrom.findLocal(skipRedirects || retFrom.skipRedirects);
	}

	if(this->parent != nullptr)
	{
		Ret& retParent = this->parent->entry.returns[0];
		return retParent.findLocal(skipRedirects || retParent.skipRedirects);
	}
	
	return nullptr;
}
	
Environment* Ret::findGlobal(bool skipRedirects)
{
	if(this->data.hasEnvironment() && this->self->isTopBlock)
		return this->self->scope;

	if(this->from != nullptr && !skipRedirects)
	{
		Ret& retFrom = this->from->entry.returns[0];
		return retFrom.findGlobal(skipRedirects || retFrom.skipRedirects);
	}
	
	if(this->parent != nullptr)
	{
		Ret& retParent = this->parent->entry.returns[0];
		return retParent.findGlobal(skipRedirects || retParent.skipRedirects);
	}
	
	return nullptr;
}

void Ret::writeGlobal(const std::string& key, Entry* entry, bool skipRedirects)
{
	Environment* global = findGlobal(skipRedirects);
	if(global != nullptr)
		global->write(key, entry);
	else
		std::cout << "[Error] Could not write data [k=" << key << ", v=" << entry->returns[0].data.valToString() << (entry->returns.size()>1?", ...":"") << "] to global scope!" << std::endl;
}

void Ret::writeLocal(const std::string& key, Entry* entry, bool skipRedirects)
{
	Environment* local = findLocal(skipRedirects);
	if(local != nullptr)
		local->write(key, entry);
	else
		std::cout << "[Error] Could not write data [k=" << key << ", v=" << entry->returns[0].data.valToString() << (entry->returns.size()>1?", ...":"") << "] to local scope!" << std::endl;
}

Entry* Ret::readGlobal(const std::string& key, bool skipRedirects)
{
	Environment* global = findGlobal(skipRedirects);
	if(global != nullptr)
		return global->read(key);
	std::cout << "[Error] Could not read global scope!" << std::endl;
	return nullptr;
}
	
Entry* Ret::readLocal(const std::string& key, bool skipRedirects)
{
	Environment* local = findLocal(skipRedirects);
	if(local != nullptr)
		return local->read(key);
	std::cout << "[Error] Could not read local scope!" << std::endl;
	return nullptr;
}

Entry* Ret::findEntry(const std::string& key, Data::Type searchType, bool skipRedirects, std::string depth)
{
	if(this->data.hasEnvironment())
	{
		if(searchType == Data::Type::MAX || this->data.type == searchType)
		{
			if(this->self->scope->hasKey(key))
			{
				return this->self->scope->read(key);
			}
		}
	}

	if(this->from != nullptr && !skipRedirects)
	{
		Ret& retFrom = this->from->entry.returns[0];
		return retFrom.findEntry(key, searchType, skipRedirects || retFrom.skipRedirects, depth+"\t");
	}

	if(this->parent != nullptr)
	{
		Ret& retParent = this->parent->entry.returns[0];
		return retParent.findEntry(key, searchType, skipRedirects || retParent.skipRedirects, depth+"\t");
	}

	return nullptr;
}

void Ret::writeToEntry(const std::string& key, Entry* entry, Data::Type searchType, bool skipRedirects)
{
	if(this->data.hasEnvironment())
	{
		if(searchType == Data::Type::MAX || this->data.type == searchType)
		{
			if(this->self->scope->hasKey(key))
			{
				this->self->scope->write(key, entry);	
				return;
			}
		}
	}

	if(this->from != nullptr && !skipRedirects)
	{
		Ret& retFrom = this->from->entry.returns[0]; 
		retFrom.writeToEntry(key, entry, searchType, skipRedirects || retFrom.skipRedirects);
		return;
	}

	if(this->parent != nullptr)
	{
		Ret& retParent = this->parent->entry.returns[0]; 
		retParent.writeToEntry(key, entry, searchType, skipRedirects || retParent.skipRedirects);
		return;
	}

	std::cout << "[Error] Could not write to entry [k=" << key << ", v=" << entry->returns[0].data.valToString() << (entry->returns.size()>1?", ...":"") << "]" << std::endl;
}

void Ret::print() const
{
	printf("[Ret] {%s}[skipRe: %s]", this->data.toString().c_str(), this->skipRedirects?"True":"False");
	if(this->self != nullptr)
		printf(", self: %p(%s)", (void*)this->self, this->self->data.typeToString().c_str());
	if(this->parent != nullptr)
		printf(", parent: %p(%s)", (void*)this->parent, this->parent->data.typeToString().c_str());
	if(this->from != nullptr)
		printf(", from: %p(%s)", (void*)this->from, this->from->data.typeToString().c_str());
	printf("\n");
}