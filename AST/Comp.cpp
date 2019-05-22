#include "Comp.hh"

#include <sstream>
#include "Symbols.hh"
#include "Data.hh"

#define min(a, b) (a<b?a:b)

void ThreeAd::dump()
{
	std::cout << name << " := ";
	std::cout << lhs << " " << op << " " << rhs << std::endl;
}

std::string ThreeAd::cfg()
{
	return this->name + " := " + this->lhs + " " + this->op + " " + this->rhs + "\\n";
}

std::string ThreeAd::toTarget(Symbols* symbols)
{
    auto findN = [](const std::string& s)->int {
        int p, p2 = 100000;
        bool a = false;
        p = s.find('\n');
        if(p != std::string::npos) { p2 = min(p2, p); a = true; }
        p = s.find('\t');
        if(p != std::string::npos) { p2 = min(p2, p); a = true; }
        p = s.find(' ');
        if(p != std::string::npos) { p2 = min(p2, p); a = true; }
        if(!a)  p2 = s.size();
        return p2;
    };

    auto cut = [&](std::string s)->std::string {
        if(s[0] == '$') s = s.substr(1);
        if(s[0] == '#') s = s.substr(1);
        return s.substr(0, findN(s));
    };

    auto getType = [this]()->std::string {
        if(this->type == Data::Type::NUMBER)
            return "double ";
        if(this->type == Data::Type::STRING)
            return "char* ";
        if(this->type == Data::Type::BOOL)
            return "bool ";
        return "double ";
    };

    // -------------------- Arthmetic operators --------------------
	if(this->op == "+")
	{
		return getType() + this->name + " = " + cut(this->lhs) + " + " + cut(this->rhs) + ";\n";
	}
	else if(this->op == "-")
	{
		return getType() + this->name + " = " + cut(this->lhs) + " - " + cut(this->rhs) + ";\n";
	}
	else if(this->op == "*")
	{
		return getType() + this->name + " = " + cut(this->lhs) + " * " + cut(this->rhs) + ";\n";
	}
	else if(this->op == "/")
	{
		return getType() + this->name + " = " + cut(this->lhs) + " / " + cut(this->rhs) + ";\n";
	}
    else if(this->op == "^")
	{
		return getType() + this->name + " = pow(" + cut(this->lhs) + ", " + cut(this->rhs) + ");\n";
	}
    // -------------------- Comparison operators -------------------- 
	else if(this->op == "==")
	{
		return "if(" + cut(this->lhs) + " == " + cut(this->rhs) + ")\n";
	}
    else if(this->op == "~=")
	{
		return "if(" + cut(this->lhs) + " != " + cut(this->rhs) + ")\n";
	}
    else if(this->op == "<=")
	{
		return "if(" + cut(this->lhs) + " <= " + cut(this->rhs) + ")\n";
	}
    else if(this->op == ">=")
	{
		return "if(" + cut(this->lhs) + " >= " + cut(this->rhs) + ")\n";
	}
    else if(this->op == "<")
	{
		return "if(" + cut(this->lhs) + " < " + cut(this->rhs) + ")\n";
	}
    else if(this->op == ">")
	{
		return "if(" + cut(this->lhs) + " > " + cut(this->rhs) + ")\n";
	}
    // --------------------------------------------------------------------------
    else if(this->op == "cpy")
	{
		return getType() + this->name + " = " + cut(this->lhs) + ";\n";
	}
    else if(this->op == "storeAt")
    {
        return this->name + "[" + cut(this->rhs) + "] = " + cut(this->lhs) + ";\n";
    }
    else if(this->op == "loadAt")
    {
        return getType() + this->name + " = " + cut(this->rhs) + "[" + cut(this->lhs) + "];\n";
    }
    else if(this->op == "call") // Function call.
	{
        if(cut(this->lhs) == "printf")
        {
            std::string l = this->rhs;
            if(l[0] == '#') // Variable
            {
                if(l.size() > 2 && l[1] == 't' && l[2] == '_') // Temp variable
                {
                    std::string s = "";
                    std::string l2 = l.substr(1); 
                    int p = findN(l2);
                    if(p != l2.size())
                        s = s.substr(0, p);
                    // Assume only floats as temp variables in printf.
                    return "printf(\"%f" + s + "\", " + cut(l) + ");\n";
                }
                else // Need to fetch from symbol table.
                {
                    Symbol sym = symbols->get(cut(l));
                    std::string typeStr = "f";
                    if(sym.type == Symbol::Type::STRING)
                        typeStr = "s";
                    else if(sym.type == Symbol::Type::BOOL)
                        typeStr = "d";
                    
                    std::string s = "";
                    std::string l2 = l.substr(1); 
                    int p = findN(l2);
                    if(p != l2.size())
                        s = s.substr(0, p);
                    
                    return "printf(\"%" + typeStr + s + "\", " + cut(l) + ");\n";
                }
            }
            else if(l[0] == '$') // Number
            {
                std::string s = "";
                std::string l2 = l.substr(1); 
                int p = findN(l2);
                if(p != l2.size())
                    s = s.substr(0, p);
                return "printf(\"%f" + s + "\", " + cut(l) + ");\n";
            } 
        }
        else if(cut(this->lhs) == "scanf")
        {
            return getType() + this->name + ";\n\tscanf(\"%f\", " + this->name + ");\n";
        }
        else // User-created functions
        {
            if(this->rhs == "NIL")
                return this->lhs + "();\n";
            else
                return this->lhs + "(" + cut(this->rhs) + ");\n";
        }
    }
}

// ---------------------------------------------------------------------------------------------------------

int BBlock::nCounter = 0;

void BBlock::dump()
{
	std::cout << "BBlock @ " << this << std::endl;
	std::cout << name << std::endl;
	for(auto i : instructions)
			i.dump();
	std::cout << "True:  " << tExit << std::endl;
	std::cout << "False: " << fExit << std::endl;
}

std::string BBlock::cfg()
{
	std::string s;
	std::stringstream ss;
	ss << "BBlock @ " << this << "\\n";
	ss << name << "\\n";
	for(auto i : instructions)
			ss << i.cfg();
	ss << "True:  " << tExit << "\\n";
	ss << "False: " << fExit << "\\n";
	return ss.str();
}

std::string BBlock::toTarget(Symbols* symbols)
{
	std::string out;
	out += this->name + ":\n";
	
    for(auto i : this->instructions)
        out += "\t" + i.toTarget(symbols);

	// If it can branch to two blocks.
	if(this->tExit && this->fExit)
	{
		out += "\t\tgoto " + this->tExit->name + ";\n";
		out += "\telse\n";
		out += "\t\tgoto " + this->fExit->name + ";\n";
	}

	// If it can only flow to one block.
	if(this->tExit && !this->fExit)
	{
		out += "\tgoto " + this->tExit->name + ";\n";
	}

	out += "\t// End of block.\n";

	// It is the termenated block.
	if(!this->tExit && !this->fExit)
	{
		out += "\t// Finished!\n";
	}
	return out;
}

// ---------------------------------------------------------------------------------------------------------

void dumpToTarget(BBlock* start, std::vector<BBlock*> funcBlocks)
{
    std::ofstream file;
    file.open("target.cc");
    file << "#include <iostream>" << std::endl; // printf
    file << "#include <cmath>" << std::endl;    // pow

    // Functions.


    file << "int main()\n{\n";

    // Initialize the variables.
    file << "\t// Initialize symbols." << std::endl;
    for(auto& e : start->symbols->map)
    {
        Symbol& sym = e.second;
        if(sym.type == Symbol::Type::NUMBER)
            file << "\tdouble " << e.first << ";" << std::endl;
        if(sym.type == Symbol::Type::STRING)
            file << "\tchar " << e.first << "[] = \"" + sym.s + "\";" << std::endl;
        if(sym.type == Symbol::Type::BOOL)
            file << "\tbool " << e.first << ";" << std::endl;
    }

    file << std::endl;

    // The magic.
    bool dbg_wasT = false;
    std::set<BBlock *> done, todo;
    todo.insert(start);
    while(todo.size()>0)
    {
        // Pop an arbitrary element from todo set
        auto first = todo.begin();
        BBlock *next = *first;
        todo.erase(first);

        std::string code = next->toTarget(start->symbols);

        // Insert code block in file.
        file << code;

        if(dbg_wasT)
        {
            file << "// This is the false-branch of the if in the test-case\n";
            dbg_wasT = false;
        }
        
        bool dbg_hasT = false;
        bool dbg_hasF = false;
        done.insert(next);
        if(next->tExit!=NULL && done.find(next->tExit)==done.end())
        {
            dbg_hasT = true;
            todo.insert(next->tExit);
        }
        if(next->fExit!=NULL && done.find(next->fExit)==done.end())
        {
            dbg_hasF = true;
            todo.insert(next->fExit);
        }

        // Check if it was If.
        if(dbg_hasT && dbg_hasF)
        {
            file << "// This is the true-branch of the if in the test-case\n";
            dbg_wasT = true;
        }
    }

    // End
    file << "\treturn 0;\n}\n";
    file.close();
}