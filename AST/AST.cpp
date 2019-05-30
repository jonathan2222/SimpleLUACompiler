#include "AST.hh"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <queue>

unsigned Node::counterId = 0;
unsigned Node::counterName = 0;

std::string strSides(const std::string str, std::string c) 
{
	return c + str + c;
}

Node::Node(const std::string& tag)
{
	this->tag = tag;
	makeName();
	makeId();
}

Node::Node(const std::string& tag, const Data& data)
{
	this->tag = tag;
	this->data = data;
	makeName();
	makeId();

	if(data.type == Data::Type::SCOPE || data.type == Data::Type::FUNCTION)
		this->symbols = new Symbols(this);
}

Node::~Node()
{
	for(Node*& node : this->children)
		delete node;
	this->children.clear();

	if(this->symbols != nullptr)
		delete this->symbols;
}

std::string Node::makeName(const std::string& prefix)
{
	this->name = prefix + std::to_string(++counterName);
	return this->name;
}

std::string Node::makeId()
{
	this->id = "Node_" + std::to_string(counterId++);
	return this->id;
}

void Node::dump(int depth)
{
	for(int i = 0; i < depth; i++)
		std::cout << "  ";
	std::cout << this->toString() << std::endl;
	for(auto i=this->children.begin(); i!=this->children.end(); i++)
		(*i)->dump(depth+1);
}

std::string Node::dot()
{
	std::string s = this->id + " [color=white, fontcolor=white, label=\"" + 
			this->tag + 
			(this->data.type!=Data::EMPTY?": " + (this->data.name=="NO_NAME"? 
			this->data.valToString(this->data.type): this->data.name) : "" ) +  
			"\"];\n";
	for(auto i=this->children.begin(); i!=this->children.end(); i++)
	{
		s += (*i)->dot();
		s += this->id + " -> " + (*i)->id + ";\n";
	}
	return s;
}

void Node::toCfg(BBlock* start)
{
	std::ofstream file;
    file.open("cfg.dot");
    if(file.is_open())
    {
        std::string s = "digraph {\ngraph [bgcolor=black];\nedge [color=white];node [shape=box]\n";
        file << s;
		
		symbolsTocfg(file, this);

		fillCfg(file, start);

		for(BBlock* f : this->functions)
			fillCfg(file, f);

        file << "}";
        file.close();
    }
}

void Node::symbolsTocfg(std::ofstream& file, Node* node)
{
	if(node->symbols != nullptr)
	{
		static unsigned symbolID = 0;
		file << "\tsubgraph clusterSymbols" << std::to_string(++symbolID) << " {" << std::endl;
		file << "\t\tlabel=\"SymbolTable_" << std::to_string(symbolID) << "\";" << std::endl;
		file << "\t\tcolor=yellow;" << std::endl;

		std::string pre = "";
		for(auto e : node->symbols->map)
		{
			Symbol& s = e.second;
			std::stringstream ss;
			std::string name = "SYM_" + std::to_string(symbolID) + "_" + e.first;  
			ss << "\t\t" << name << " [color=white, fontcolor=white, label=\"";
			ss << s.data.name << ": " << s.toString() << "\"];" << std::endl;
			
			if(pre.empty() == false)
				ss << "\t\t" << pre << " -> " << name << ";" << std::endl;
			pre = name;
			
			file << ss.str();
		}

		file << "\t}" << std::endl;
	}

	for(Node* c : this->children)
		c->symbolsTocfg(file, c);
}

void Node::fillCfg(std::ofstream& file, BBlock* start)
{
	static unsigned clusterID = 0;
	file << "\tsubgraph cluster" << std::to_string(++clusterID) << " {" << std::endl;
	file << "\t\tlabel=\"Cluster_" << std::to_string(clusterID) << "\";" << std::endl;
	file << "\t\tcolor=blue;" << std::endl;
	file << "\t\tfontcolor=white;" << std::endl;

	std::set<BBlock *> done, todo;
	std::queue<std::pair<BBlock*, BBlock*>> edgeQueueTrue;
	std::queue<std::pair<BBlock*, BBlock*>> edgeQueueFalse;
	todo.insert(start);
	while(todo.size()>0)
	{
		// Pop an arbitrary element from todo set
		auto first = todo.begin();
		BBlock *next = *first;
		todo.erase(first);

		// Write content
		file << "\t\t" << next->name + " [color=white, fontcolor=white, label=\"" + 
					next->cfg() +
					"\"];\n";
		
		done.insert(next);
		
		if(next->tExit != nullptr)
			edgeQueueTrue.push(std::pair<BBlock*, BBlock*>(next, next->tExit));
		if(next->tExit!=nullptr && done.find(next->tExit)==done.end())
		{
			todo.insert(next->tExit);
		}
		
		if(next->fExit != nullptr)
			edgeQueueFalse.push(std::pair<BBlock*, BBlock*>(next, next->fExit));
		if(next->fExit!=nullptr && done.find(next->fExit)==done.end())
		{
			todo.insert(next->fExit);
		}
	}

	// Write edges.
	file << "\t\t" << "edge [color=green];" << std::endl;
	while(edgeQueueTrue.empty() == false)
	{
		auto& p = edgeQueueTrue.front();
		file << "\t\t" << p.first->name << " -> " << p.second->name << ";" << std::endl;
		edgeQueueTrue.pop();
	}

	file << "\t\t" << "edge [color=red];" << std::endl;
	while(edgeQueueFalse.empty() == false)
	{
		auto& p = edgeQueueFalse.front();
		file << "\t\t" << p.first->name << " -> " << p.second->name << ";" << std::endl;
		edgeQueueFalse.pop();
	}

	file << "\t}" << std::endl;
}

std::pair<BBlock*, std::string> Node::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");
	for(Node* c : this->children)
	{
		result = c->convert(result.first);
		this->name = result.first->name;
	}
	return result;
}

void Node::addChild(Node* child, bool isTopBlock)
{
	this->children.push_back(child);
	child->parent = this;
	child->isTopBlock = isTopBlock;
}

void Node::addSymbol(const std::string& key, Symbol& sym)
{
	if(this->symbols != nullptr)
	{
		this->symbols->insert(key, sym);
	}
	else
	{
		if(this->parent != nullptr)
		{
			this->parent->addSymbol(key, sym);
		}
		else
		{
			std::cout << "[Error] (addSymbol) Could not find symbol table! Key: " << key << ", Sym: " << sym.toString() << std::endl;
		}
	}
}

Symbol Node::getSymbol(const std::string& key)
{
	if(this->symbols != nullptr)
	{
		if(this->symbols->hasKey(key))
		{
			return this->symbols->get(key);
		} 
	}

	if(this->parent != nullptr)
	{
		return this->parent->getSymbol(key);
	}

	std::cout << "[Error] (getSymbol) Could not find symbol table! Key: " << key << std::endl;
	return Symbol();
}

bool Node::hasSymbol(const std::string& key)
{
	if(this->symbols != nullptr)
	{
		if(this->symbols->hasKey(key)) 
			return true;
	}

	if(this->parent != nullptr)
	{
		if(this->parent->hasSymbol(key)) 
			return true;
	}

	return false;
}

void Node::compressSymbols(Node* n)
{
	if(this->isTopBlock || this->data.type == Data::Type::FUNCTION)
	{
		n = this;
	}
	else if(n != nullptr)
	{
		if(this->symbols != nullptr)
		{
			// Add symbols to n.
			for(auto e : this->symbols->map)
				n->symbols->insert(e.first, e.second);
			delete this->symbols;
			this->symbols = nullptr;
		}
	}

	// Go to children.
	for(Node* c : this->children)
		c->compressSymbols(n);
}

Node* Node::getTop()
{
	if(this->parent != nullptr)
		return this->parent->getTop();
	return this;
}

bool Node::haveChildrenSymbols() const
{
	for(Node* c : this->children)
	{
		if(c->symbols != nullptr)
			return true;
		if(c->haveChildrenSymbols())
			return true;
	}
	return false;
}

void Node::printAllSymbols(unsigned depth) const
{
	std::string padding = "";
	for(int i = 0; i < depth; i++) padding += "\t";
	std::cout << padding << "[" << this->tag << "] " << this->name << (this->isTopBlock?" [GLOBAL]" : "") << ": " << std::endl;
	if(this->symbols != nullptr)
		this->symbols->print(depth+1);

	if(haveChildrenSymbols())
	{
		for(auto& c : this->children)
			c->printAllSymbols(depth+1);
	}
}

std::string Node::toString() const
{
	return "[" + this->tag + "] " + this->data.toString() + (this->isTopBlock ? " GLOBAL BLOCK" : "");
}

unsigned Node::printParents() const
{
	unsigned d = 0;
	bool top = false;
	if(this->parent != nullptr)
		d = this->parent->printParents();
	else
		top = true;
	
	std::string padding = "";
	for(int i = 0; i < d; i++) padding += "\t";
	std::cout << padding << (top?"[TOP]":"") << toString() << std::endl;

	return d+1;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Statement::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");
	for(Node* c : this->children)
		result = c->convert(result.first);
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Expression::convert(BBlock* out)
{
	makeName();
	std::pair<BBlock*, std::string> result(nullptr, this->name);
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::string Constant::makeName(const std::string& prefix)
{
	this->name = data.valToString();
	if(data.type == Data::Type::NUMBER)
		this->name = "$" + this->name;
	return this->name;
}

std::pair<BBlock*, std::string> Constant::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");
	// If the constant is a string => make a symbol for it.
	if(data.type == Data::Type::STRING)
	{
		::Expression::makeName("_s_");
		Symbol sym(Data::Type::STRING);
		sym.size = (this->data.s.size()+1)*sizeof(char);
		sym.data.s = this->data.s;
		addSymbol(this->name, sym);
	}
	else if(data.type == Data::Type::BOOL)
	{
		//makeName();
		::Expression::makeName("_b_");
		Symbol sym(Data::Type::BOOL);
		sym.size = sizeof(long int);
		sym.data.b = this->data.b;
		addSymbol(this->name, sym);
	}
	else if(data.type == Data::Type::NUMBER)
	{
		::Expression::makeName("_n_");
		Symbol sym(Data::Type::NUMBER);
		sym.size = sizeof(double);
		sym.data.f = this->data.f;
		addSymbol(this->name, sym);
	}

	data.name = this->name;
	result.second = this->name;
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::string Var::makeName(const std::string& prefix)
{
	this->name = data.name;
	return this->name;
}

std::pair<BBlock*, std::string> Var::convert(BBlock* out)
{
	makeName();
	std::pair<BBlock*, std::string> result(nullptr, this->name);
	if(hasSymbol(this->name) != false)
	{
		Symbol sym = getSymbol(this->name);
		this->data.type = sym.data.type;
	}
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Operation::convert(BBlock* out)
{
	Expression* lhs = dynamic_cast<Expression*>(this->children.front());
	Expression* rhs = dynamic_cast<Expression*>(this->children.back());
	lhs->convert(out);
	rhs->convert(out);

	if(lhs->data.type != rhs->data.type)
	{
		// Assume other type if one is string.
		if(lhs->data.type == Data::Type::STRING)
			this->data.type = rhs->data.type;
		else
			this->data.type = lhs->data.type;
	}
	else
	{
		this->data.type = lhs->data.type;
	}

	makeName();
	std::pair<BBlock*, std::string> result(out, this->name);
	result.first->instructions.push_back(ThreeAd(this->name, this->op, lhs->name, rhs->name, this->data.type, result.first));

	data.name = this->name;
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Assignment::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");

	Node* var_list = this->children.front();
	Node* exp_list = this->children.back();

	auto vIt = var_list->children.begin();
	auto eIt = exp_list->children.begin();

	for(;vIt != var_list->children.end(); vIt++, eIt++)
	{
		Expression* lhs = dynamic_cast<Expression*>(*vIt);
		Expression* rhs = dynamic_cast<Expression*>(*eIt);
		if(rhs->tag == "Table")
		{
			AssignmentOne* as = new AssignmentOne();
			rhs->convert(out);
			as->addChild(lhs);
			as->addChild(rhs);
			as->parent = this;
			as->convert(out, true);
		}
		else
		{
			// Make a assignment for each 'part'
			AssignmentOne* as = new AssignmentOne();
			Var* v = new Var("_tmp" + lhs->name);
			v->makeName();
			as->addChild(v);
			rhs->convert(out);
			as->addChild(rhs);
			as->parent = this;
			as->convert(out, false);
		}
	}

	vIt = var_list->children.begin();
	eIt = exp_list->children.begin();
	for(; vIt != var_list->children.end(); vIt++, eIt++)
	{
		Expression* lhs = dynamic_cast<Expression*>(*vIt);
		Expression* rhs = dynamic_cast<Expression*>(*eIt);
		if(rhs->tag != "Table")
		{
			AssignmentOne* as = new AssignmentOne();
			as->addChild(lhs);
			Var* e = new Var("_tmp" + lhs->name);
			e->makeName();
			e->convert(out);
			e->data.type = rhs->data.type;
			e->ret = rhs->ret;
			as->addChild(e);
			as->parent = this;
			as->convert(out, true);
		}
	}
	
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> AssignmentOne::convert(BBlock* out, bool shouldAddSym)
{
	std::pair<BBlock*, std::string> result(out, "");

	Expression* lhs = dynamic_cast<Expression*>(this->children.front());
	Expression* rhs = dynamic_cast<Expression*>(this->children.back());
	//rhs->convert(out);

	if(rhs->tag == "Table")
	{
		lhs->convert(out);
		this->name = lhs->name;
		Table* table = dynamic_cast<Table*>(rhs); 
		// Check if symbol exist.
		if(hasSymbol(lhs->name) == false)
		{
			// If not allocate it in .data
			Symbol sym(Data::Type::TABLE);
			sym.size = table->arr.size()*sizeof(double); // Size of array in bytes.
			addSymbol(this->name, sym); // All symbols have allocated space in .data
		}

		// Insert elements into the allocated space.
		Symbol sym = getSymbol(lhs->name);
		table->convert(out, sym);
		this->data.type = Data::Type::TABLE;
	}
	else if(lhs->tag == "TableIndex")
	{
		Node* cVar = lhs->children.front();
		cVar->convert(out);
		std::string tblIndexVar = cVar->name;
		Node* cIndex = lhs->children.back();
		cIndex->convert(out);
		std::string tblIndexVarI = cIndex->name;
		this->data.type = Data::Type::TABLE;
		result.first->instructions.push_back(ThreeAd(tblIndexVar, "storeAt", rhs->name, tblIndexVarI, Data::Type::TABLE, result.first));
	}
	else
	{
		lhs->convert(out);
		this->name = lhs->name;
		this->data.type = rhs->data.type;

		Symbol sym;
		switch (rhs->data.type)
		{
		case Data::Type::NUMBER:
			sym.data.type = Data::Type::NUMBER;
			sym.size = sizeof(double);
			break;
		case Data::Type::STRING:
			{
				sym.data.type = Data::Type::STRING;
				Symbol sym2;
				if(hasSymbol(rhs->name))
					sym2 = getSymbol(rhs->name);
				else
				{ /* In the test cases, this will never happen! */ }
				sym.size = sym2.size;
			}
			break;
		case Data::Type::BOOL:
			sym.data.type = Data::Type::BOOL;
			sym.size = 1; // Only one byte is needed.
			break;
		case Data::Type::FUNCTIONCALL:
			{
				if(rhs->ret.type != Data::Type::EMPTY)
				{
					this->data.type = rhs->ret.type;
					sym.data.type = this->data.type;
					sym.size = sizeof(double); // Assume Number.
				}
			}
			break;
		default:
			break;
		}
		if(shouldAddSym)
			addSymbol(this->name, sym);
		result.first->instructions.push_back(ThreeAd(this->name, "cpy", rhs->name, rhs->name, this->data.type, result.first));	
	}	
	
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Paren::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");

	Expression* lhs = dynamic_cast<Expression*>(this->children.front());
	lhs->convert(out);

	this->name = lhs->name;
	this->data.type = lhs->data.type;
	//makeName();
	//result.first->instructions.push_back(ThreeAd(this->name, "cpy", lhs->name, lhs->name));

	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> If::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");
	auto itIf = this->children.begin();

	// Check if it is an Else block.
	if(this->tag == "Block")
	{
		Node* el = this;
		BBlock* elIn = new BBlock();
		BBlock* elOut = el->convert(elIn).first;
		out->tExit = elIn;	
	
		BBlock* end = new BBlock();
		elOut->tExit = end;

		result.first = end;
		return result;
	}
	
	// Exp
	Expression* exp = dynamic_cast<Expression*>(*(itIf++));
	exp->convert(result.first);
	
	// True Block
	Node*& tr = *(itIf++);
	BBlock* tIn = new BBlock();
	BBlock* tOut = tr->convert(tIn).first;
	out->tExit = tIn;

	BBlock* end = new BBlock();
	if(!tOut->hasReturn())
		tOut->tExit = end;

	// Check if elseif
	if(itIf != this->children.end())
	{
		Node*& fl = *(itIf++);
		BBlock* fIn = new BBlock();
		BBlock* fOut = fl->convert(fIn).first;
		out->fExit = fIn;
		if(!fOut->hasReturn())
			fOut->tExit = end;
	}
	else
	{
		// If the last child is an 'else' block, set the 'false' exit to the 'end' block.
		out->fExit = end;
	}

	result.first = end;
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> For::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");

	double step = 1.0;
	auto itFor = this->children.begin();
	// NAME
	Node*& nVarN = *(itFor++);
	// var
	Node*& varN = *(itFor++);

	BBlock* forBlock = new BBlock();
	out->tExit = forBlock;

	// Create assignment node. To assign var to NAME.
	AssignmentOne* as = new AssignmentOne();
	as->addChild(nVarN);
	varN->convert(forBlock);
	as->addChild(varN);
	as->parent = this;

	// Assignment only fills the forBlock with instructions.
	as->convert(forBlock, true).first;

	// Limit
	Node*& limitN = *(itFor++);

	// Block.
	Node*& block = *(itFor++);
	
	BBlock* ifBlock = new BBlock();
	forBlock->tExit = ifBlock;
	// Make a if node to check if NAME <= limit.
	If* ifN = new If();
	Operation* op = new Operation("L_leq", "<=");
	op->addChild(nVarN);
	op->addChild(limitN);
	ifN->addChild(op);
	// If the comparison is true, goto block else goto end.
	ifN->addChild(block);
	ifN->parent = this;
	BBlock* end = ifN->convert(ifBlock).first;

	// Add 1 to the assigned variable at the end of the block.
	BBlock* e = ifBlock;
	while(e->tExit != end)
		e = e->tExit;
	makeName();
	
	std::string oneSym = "_n_ONE";
	if(hasSymbol(oneSym) == false)
	{
		Symbol symOne(Data::Type::NUMBER);
		symOne.data.f = 1.0;
		symOne.size = sizeof(double);
		addSymbol(oneSym, symOne);
	}
	e->instructions.push_back(ThreeAd(this->name, "+", as->name, oneSym, as->data.type, e));
	e->instructions.push_back(ThreeAd(as->name, "cpy", this->name, this->name, as->data.type, e));
	e->tExit = ifBlock;

	// After each block, go back to start of forBlock.
	//forBlock->tExit->tExit = forBlock;

	/*ifN->children.front()->children.clear(); // Remove nVarN & limitN
	ifN->children.pop_back(); // Remove block
	delete ifN;
	*/
	// Delete assignment nodes.
	/*as->children.front()->children.clear(); // Remove nVarN
	as->children.back()->children.clear(); // Remove varN
	delete as;
	*/	
	// Return 'end' node.
	result.first = end;
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Repeat::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");

	// Exp
	Node*& blockN = this->children.front();
	// Block
	Node*& expN = this->children.back();

	BBlock* repeatBlock = new BBlock();
	// Begin the repeat block.
	out->tExit = repeatBlock;
	BBlock* blockEnd = blockN->convert(repeatBlock).first;

	// Make a if node and attach it to the end of the block.
	If* ifN = new If();
	ifN->addChild(expN);
	ifN->addChild(new Block());
	ifN->parent = this;
	BBlock* end = ifN->convert(blockEnd).first;
	// If the comparison was false, repeat the block, else goto end.
	blockEnd->fExit = repeatBlock;
	
	// Find the block before end and set that to end.
	BBlock* e = blockEnd;
	while(e->tExit != end && e->tExit != nullptr)
		e = e->tExit;
	delete end;
	e->tExit = nullptr;
	e->fExit = nullptr;
	end = e;
	
	// Return new end.
	result.first = end;
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> FunctionCall::convert(BBlock* out)
{
	bool isPredefined = false;
	BBlock* block = out;

	auto addSymStr = [&](const std::string& name, const std::string& cmd) {
		if(hasSymbol(name) == false)
		{
			Symbol sym(Data::Type::STRING);
			std::string s = cmd;
			sym.data.s = s;
			sym.size = sizeof(s);
			addSymbol(name, sym);	
		}
	};

	Var* lhs = dynamic_cast<Var*>(this->children.front());
	lhs->convert(block);
	Constant* str = dynamic_cast<Constant*>(this->children.back()); 
	if(str != nullptr)
	{
		str->convert(block);
		makeName();
		
		// Print when only string as an argument.
		if(lhs->name == "print")
		{
			addSymStr("_STR_S_NL", "%s\\n");
			block->instructions.push_back(ThreeAd(this->name, "call", "printf_vnl", str->name, Data::Type::NIL, block));
			this->ret.type = Data::Type::NIL;
			isPredefined = true;
		}
		
		// Io functions, single string argument.
		if(lhs->name == "io")
		{
			Var* lhs2 = dynamic_cast<Var*>(lhs->children.front());
			lhs2->convert(block);
			
			if(lhs2->name == "write")
			{
				addSymStr("_STR_S", "%s");
				block->instructions.push_back(ThreeAd(this->name, "call", "printf_v", str->name, Data::Type::NIL, block));
				this->ret.type = Data::Type::NIL;
				isPredefined = true;
			}
			else if(lhs2->name == "read")
			{
				// Not used.
				isPredefined = true;
			}
		}

	}
	else
	{	
		Node* args = this->children.back();
		
		if(lhs->name == "print")
		{
			Node* exp_list = args->children.front();
			if(exp_list->tag == "Exp_list")
			{
				unsigned cn = 0;
				for(Node* n : exp_list->children)
				{
					cn++;
					// New name each iteration.
					makeName();
					n->convert(block);
					
					std::string s = n->name;
					if(cn == exp_list->children.size())
					{
						if(s[0] == '_' && s[1] == 't' && s[2] == '_')
						{
							addSymStr("_STR_F_NL", "%.1f\\n");
							block->instructions.push_back(ThreeAd(this->name, "call", "printf_fnl", s, Data::Type::NIL, block));
						}
						else
						{
							addSymStr("_STR_F_NL", "%.1f\\n");
							addSymStr("_STR_S_NL", "%s\\n");
							block->instructions.push_back(ThreeAd(this->name, "call", "printf_vnl", s, Data::Type::NIL, block));
						}
					}
					else
					{
						if(s[0] == '_' && s[1] == 't' && s[2] == '_')
						{
							addSymStr("_STR_F_T", "%.1f\\t");
							block->instructions.push_back(ThreeAd(this->name, "call", "printf_ft", s, Data::Type::NIL, block));
						}
						else
						{
							addSymStr("_STR_F_T", "%.1f\\t");
							addSymStr("_STR_S_T", "%s\\t");
							block->instructions.push_back(ThreeAd(this->name, "call", "printf_vt", s, Data::Type::NIL, block));
						}
					}
					this->ret.type = Data::Type::NIL;
				}
			}
			isPredefined = true;
		}
		else if(lhs->name == "io")
		{
			isPredefined = true;
			Var* lhs2 = dynamic_cast<Var*>(lhs->children.front());
			lhs2->convert(block);

			if(lhs2->name == "write")
			{
				Node* exp_list = args->children.front();
				if(exp_list->tag == "Exp_list")
				{
					unsigned cn = 0;
					for(Node* n : exp_list->children)
					{
						cn++;
						// New name each iteration.
						makeName();
						n->convert(block);
						
						addSymStr("_STR_F", "%.1f");
						addSymStr("_STR_S", "%s");
						block->instructions.push_back(ThreeAd(this->name, "call", "printf_v", n->name, Data::Type::NIL, block));
						this->ret.type = Data::Type::NIL;
					}
				}
			}
			else if(lhs2->name == "read")
			{
				Node* exp_list = args->children.front();
				// Assume one child.
				Node* child = exp_list->children.front();
				
				if(child->data.s == "*number")
				{
					// Read number
					makeName();
					addSymStr("_STR_SCAN_F", "%lf");
					block->instructions.push_back(ThreeAd(this->name, "call", "scanf", "NUM", Data::Type::NUMBER, block));
					this->ret.type = Data::Type::NUMBER;
				}
			}
		}

		// If not predefined, it must be a user-defined function.
		if(!isPredefined)
		{
			if(hasSymbol(lhs->name))
			{
				Symbol sym = getSymbol(lhs->name);

				makeName();
				Node* exp_list = args->children.front();
				if(exp_list->tag == "Exp_list")
				{
					// Assume one or none arguments.
					Node* n = exp_list->children.front();
					n->convert(block);
					
					// Assume NUMBER as return type.
					block->instructions.push_back(ThreeAd(this->name, "call", lhs->name, n->name, Data::Type::NUMBER, block));
				}
				else
				{
					// Assume NUMBER as return type.
					block->instructions.push_back(ThreeAd(this->name, "call", lhs->name, "NIL", Data::Type::NUMBER, block));
				}
			}
			else
			{
				std::cout << "[Error] function symbol not defined! Key: " << lhs->name << std::endl;
				this->ret.type = Data::Type::NIL;
			}
		}
	}
	
	std::pair<BBlock*, std::string> result(block, this->name);
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Table::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");
	// All test-cases uses constants only => I will only uses them too.
	
	// Construct array.
	makeName();
	for (Node* c : this->children)
	{
		Node* c2 = c->children.front();
		c2->convert(out);
		
		this->arr.push_back(c2->getSymbol(c2->name));
	}
	
	result.second = this->name;
	return result;
}

std::pair<BBlock*, std::string> Table::convert(BBlock* out, const Symbol& sym)
{
	std::pair<BBlock*, std::string> result(out, this->name);

	unsigned int index = 1;
	for(Symbol e : this->arr)
	{
		// Insert d into sym at position index.
		Symbol s(Data::Type::NUMBER);
		s.data.f = (double)index;
		s.size = sizeof(double);
		std::string idxName = "_idx" + std::to_string(index);
		this->addSymbol(idxName, s);
		out->instructions.push_back(ThreeAd(sym.data.name, "storeAt", e.data.name, idxName, Data::Type::TABLE, out));
		index += 1; // TODO: Change this when using asm. 
	}
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> TableIndex::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");
	Expression* var = dynamic_cast<Expression*>(this->children.front());
	var->convert(out);
	Expression* index = dynamic_cast<Expression*>(this->children.back());
	index->convert(out);

	if(this->hasName == false)
	{
		this->hasName = true;
		makeName();
	}
	
	out->instructions.push_back(ThreeAd(this->name, "loadAt", var->name, index->name, Data::Type::NUMBER, out));

	// Assume all data from a table are numbers.
	this->data.type = Data::Type::NUMBER;
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Hash::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");
	
	Expression* child = dynamic_cast<Expression*>(this->children.front());
	child->convert(out);

	// If it would have been a more general case, than I would have not fetched the size at runtime, but instead use a separet instruction for it. 
	unsigned int size = getSymbol(child->name).size/sizeof(double);
	std::string sSym = "_n_" + std::to_string(size);
	if(hasSymbol(sSym) == false)
	{
		Symbol symOne(Data::Type::NUMBER);
		symOne.data.f = (double)size;
		symOne.size = sizeof(double);
		addSymbol(sSym, symOne);
	}

	makeName();
	out->instructions.push_back(ThreeAd(this->name, "cpy", sSym, sSym, Data::Type::NUMBER, out));

	this->data.type = Data::Type::NUMBER;
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Function::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");


	Var* funcName = dynamic_cast<Var*>(this->children.front());
	funcName->convert(out);
	if(this->parent->hasSymbol(funcName->name) == false)
	{
		Node* topNode = getTop();
		// This block is not connected to the tree.
		BBlock* funcBlock = new BBlock();
		topNode->functions.push_back(funcBlock);

		Symbol sym(Data::Type::FUNCTION);
		sym.funcBlock = funcBlock; // Every instance of the function, has its own cfg.
		funcBlock->symbols = this->symbols;
		this->parent->addSymbol(funcName->name, sym);

		Node* funcBody = this->children.back();
		Node* nameList = funcBody->children.front();
		Node* block = nameList;
		if(nameList->tag == "Block")
			nameList = nullptr;
		else
			block = funcBody->children.back();
		
		if(nameList != nullptr)
		{
			Node* c = nameList->children.front();
			while(c->children.empty() == false)
			{
				c->convert(funcBlock);
				Symbol sym(Data::Type::NUMBER);
				sym.size = 0;
				addSymbol(c->name, sym);
				sym.data.name = c->name;
				funcBlock->funcArgs.push_back(sym);
				c = c->children.front();
			}
			c->convert(funcBlock);
			Symbol sym(Data::Type::NUMBER);
			sym.size = 0;
			addSymbol(c->name, sym);
			sym.data.name = c->name;
			funcBlock->funcArgs.push_back(sym);

		}
		// Assume return type as Number.
		this->ret.type = Data::Type::NUMBER;

		block->convert(funcBlock);
		
	}	
	return result;
}

// ---------------------------------------------------------------------------------------------------------

std::pair<BBlock*, std::string> Return::convert(BBlock* out)
{
	std::pair<BBlock*, std::string> result(out, "");
	BBlock* block = out;

	// Assume one return expression or none.
	if(this->children.empty() == false)
	{
		Expression* retExp = dynamic_cast<Expression*>(this->children.front()->children.front());
		retExp->convert(out);
		if(out->instructions.empty() == false)
		{
			block = new BBlock();
			out->tExit = block;
		}
		makeName();
		block->instructions.push_back(ThreeAd(this->name, "ret", retExp->name, retExp->name, retExp->data.type, block));
	}
	else
	{
		if(out->instructions.empty() == false)
		{
			block = new BBlock();
			out->tExit = block;
		}
		makeName();
		// Assume NUMBER as return type.
		block->instructions.push_back(ThreeAd(this->name, "ret", "NIL", "NIL", Data::Type::NUMBER, block));
	}
	
	result.first = block;
	return result;
}