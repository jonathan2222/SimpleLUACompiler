#include "Comp.hh"

#include <algorithm>
#include <sstream>
#include "Symbols.hh"
#include "Data.hh"
#include "AST.hh"

#define min(a, b) (a<b?a:b)

CompileLevel compLevel;

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
        if(s[0] == '$') 
        {
            s = s.substr(1);
            if(s.find('.') == s.npos)
                s += ".0"; 
        }
        if(s[0] == '#') s = s.substr(1);
        return s.substr(0, findN(s));
    };

    auto toS = [](std::string s)->std::string {
		std::string::size_type pos = 0;
		while((pos = s.find("\n", pos)) != s.npos)
			s.replace(pos, 2, "\\n");
        pos = 0;
        while((pos = s.find("\t", pos)) != s.npos)
			s.replace(pos, 2, "\\t");
		return s;
    };

    auto getPrefix = [&](std::string s)->std::string {
        int p = findN(s);
        if(p != s.size())
        {
            s = s.substr(p);
            s = toS(s);
        }
        else s = "";
        return s;
    };

    // -------------------- Arthmetic operators --------------------
	if(this->op == "+")
	{
        if(compLevel == CompileLevel::E)
		    return "\t" + this->name + " = " + cut(this->lhs) + " + " + cut(this->rhs) + ";\n";
        else if(compLevel == CompileLevel::D)
        {
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " + " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[a], %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %[b], %%xmm1\\n\\t\"\n";
            s += "\t\t\"addsd %%xmm0, %%xmm1\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm1, %[" + this->name + "]\\n\\t\"\n";

            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [a] \"x\" (" + cut(this->lhs) + "),\n";
            s += "\t\t  [b] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;    
        }
        else if(compLevel == CompileLevel::C)
        {
            bool lhsEqRhs = this->lhs == this->rhs;
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " + " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[" + this->lhs + "], %%xmm0\\n\\t\"\n";
            if(lhsEqRhs)
                s += "\t\t\"movsd %%xmm0, %%xmm1\\n\\t\"\n";
            else
                s += "\t\t\"movsd %[" + this->rhs + "], %%xmm1\\n\\t\"\n";
            s += "\t\t\"addsd %%xmm0, %%xmm1\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm1, %[" + this->name + "]\\n\\t\"\n";
            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [" + this->lhs + "] \"x\" (" + cut(this->lhs) + ")" + (lhsEqRhs?"":",") + "\n";
            if(!lhsEqRhs)
                s += "\t\t  [" + this->rhs + "] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;
        }
	}
	else if(this->op == "-")
	{
        if(compLevel == CompileLevel::E)
		    return "\t" + this->name + " = " + cut(this->lhs) + " - " + cut(this->rhs) + ";\n";
        else if(compLevel == CompileLevel::D)
        {
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " - " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[a], %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %[b], %%xmm1\\n\\t\"\n";
            s += "\t\t\"subsd %%xmm1, %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";

            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [a] \"x\" (" + cut(this->lhs) + "),\n";
            s += "\t\t  [b] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;    
        }
        else if(compLevel == CompileLevel::C)
        {
            bool lhsEqRhs = this->lhs == this->rhs;
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " - " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[" + this->lhs + "], %%xmm0\\n\\t\"\n";
            if(lhsEqRhs)
                s += "\t\t\"movsd %%xmm0, %%xmm1\\n\\t\"\n";
            else
                s += "\t\t\"movsd %[" + this->rhs + "], %%xmm1\\n\\t\"\n";
            s += "\t\t\"subsd %%xmm1, %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";
            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [" + this->lhs + "] \"x\" (" + cut(this->lhs) + ")" + (lhsEqRhs?"":",") + "\n";
            if(!lhsEqRhs)
                s += "\t\t  [" + this->rhs + "] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;
        }
	}
	else if(this->op == "*")
	{
        if(compLevel == CompileLevel::E)
		    return "\t" + this->name + " = " + cut(this->lhs) + " * " + cut(this->rhs) + ";\n";
        else if(compLevel == CompileLevel::D)
        {
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " * " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[a], %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %[b], %%xmm1\\n\\t\"\n";
            s += "\t\t\"mulsd %%xmm1, %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";

            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [a] \"x\" (" + cut(this->lhs) + "),\n";
            s += "\t\t  [b] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;    
        }
        else if(compLevel == CompileLevel::C)
        {
            bool lhsEqRhs = this->lhs == this->rhs;
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " * " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[" + this->lhs + "], %%xmm0\\n\\t\"\n";
            if(lhsEqRhs)
                s += "\t\t\"movsd %%xmm0, %%xmm1\\n\\t\"\n";
            else
                s += "\t\t\"movsd %[" + this->rhs + "], %%xmm1\\n\\t\"\n";
            s += "\t\t\"mulsd %%xmm1, %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";
            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [" + this->lhs + "] \"x\" (" + cut(this->lhs) + ")" + (lhsEqRhs?"":",") + "\n";
            if(!lhsEqRhs)
                s += "\t\t  [" + this->rhs + "] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;
        }
	}
	else if(this->op == "/")
	{
        if(compLevel == CompileLevel::E)
		    return "\t" + this->name + " = " + cut(this->lhs) + " / " + cut(this->rhs) + ";\n";
        else if(compLevel == CompileLevel::D)
        {
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " / " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[a], %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %[b], %%xmm1\\n\\t\"\n";
            s += "\t\t\"divsd %%xmm1, %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";

            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [a] \"x\" (" + cut(this->lhs) + "),\n";
            s += "\t\t  [b] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;    
        }
        else if(compLevel == CompileLevel::C)
        {
            bool lhsEqRhs = this->lhs == this->rhs;
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " / " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[" + this->lhs + "], %%xmm0\\n\\t\"\n";
            if(lhsEqRhs)
                s += "\t\t\"movsd %%xmm0, %%xmm1\\n\\t\"\n";
            else
                s += "\t\t\"movsd %[" + this->rhs + "], %%xmm1\\n\\t\"\n";
            s += "\t\t\"divsd %%xmm1, %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";
            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [" + this->lhs + "] \"x\" (" + cut(this->lhs) + ")" + (lhsEqRhs?"":",") + "\n";
            if(!lhsEqRhs)
                s += "\t\t  [" + this->rhs + "] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;
        }
	}
    else if(this->op == "^")
	{
        if(compLevel == CompileLevel::E || compLevel == CompileLevel::D)
		    return "\t" + this->name + " = pow(" + cut(this->lhs) + ", " + cut(this->rhs) + ");\n";
        else if(compLevel == CompileLevel::C)
        {
            bool lhsEqRhs = this->lhs == this->rhs;
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " ^ " + this->rhs + "\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[" + this->lhs + "], %%xmm0\\n\\t\"\n";
            if(lhsEqRhs)
                s += "\t\t\"movsd %%xmm0, %%xmm1\\n\\t\"\n";
            else
                s += "\t\t\"movsd %[" + this->rhs + "], %%xmm1\\n\\t\"\n";
            s += "\t\t\"call pow\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";
            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [" + this->lhs + "] \"x\" (" + cut(this->lhs) + ")" + (lhsEqRhs?"":",") + "\n";
            if(!lhsEqRhs)
                s += "\t\t  [" + this->rhs + "] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"cc\"\n";
            s += "\t);\n";
            return s;
        }
	}
    else if(this->op == "%")
	{
        if(compLevel == CompileLevel::E)
		    return "\t" + this->name + " = (double)((long int)" + cut(this->lhs) + "% (long int)" + cut(this->rhs) + ");\n";
        else if(compLevel == CompileLevel::D)
        {
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " % " + this->rhs + " -> " + 
                this->name + " := " + this->lhs + " - " + this->rhs + " * (long int)(" + this->lhs + " / " + this->rhs + ")\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[a], %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %[b], %%xmm1\\n\\t\"\n";     
            s += "\t\t\"movsd %%xmm0, %%xmm2\\n\\t\"\n";   
            s += "\t\t\"divsd %%xmm1, %%xmm2\\n\\t\"\n";       // xmm2 = xmm0 / xmm1
            s += "\t\t\"cvttsd2siq %%xmm2, %%rax\\n\\t\"\n";   // rax = (long int)xmm2
            s += "\t\t\"cvtsi2sdq %%rax, %%xmm2\\n\\t\"\n";    // xmm2 = rax
            s += "\t\t\"mulsd %%xmm1, %%xmm2\\n\\t\"\n";       // xmm2 = xmm1 * xmm2
            s += "\t\t\"subsd %%xmm2, %%xmm0\\n\\t\"\n";       // xmm0 = xmm0 - xmm2
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";

            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [a] \"x\" (" + cut(this->lhs) + "),\n";
            s += "\t\t  [b] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"%xmm2\", \"%rax\", \"cc\"\n";
            s += "\t);\n";
            return s;    
        }
        else if(compLevel == CompileLevel::C)
        {
            bool lhsEqRhs = this->lhs == this->rhs;
            // May need to change rax to some other register.
            std::string s;
            s += "\t// Expand " + this->name + " := " + this->lhs + " % " + this->rhs + " -> " + 
                this->name + " := " + this->lhs + " - " + this->rhs + " * (long int)(" + this->lhs + " / " + this->rhs + ")\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[" + this->lhs + "], %%xmm0\\n\\t\"\n";
            if(lhsEqRhs)
                s += "\t\t\"movsd %%xmm0, %%xmm1\\n\\t\"\n";
            else
                s += "\t\t\"movsd %[" + this->rhs + "], %%xmm1\\n\\t\"\n";
            s += "\t\t\"movsd %%xmm0, %%xmm2\\n\\t\"\n";
            s += "\t\t\"divsd %%xmm1, %%xmm2\\n\\t\"\n";       // xmm2 = xmm0 / xmm1
            s += "\t\t\"cvttsd2siq %%xmm2, %%rax\\n\\t\"\n";   // rax = (long int)xmm2
            s += "\t\t\"cvtsi2sdq %%rax, %%xmm2\\n\\t\"\n";    // xmm2 = rax
            s += "\t\t\"mulsd %%xmm1, %%xmm2\\n\\t\"\n";       // xmm2 = xmm1 * xmm2
            s += "\t\t\"subsd %%xmm2, %%xmm0\\n\\t\"\n";       // xmm0 = xmm0 - xmm2
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";
            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [" + this->lhs + "] \"x\" (" + cut(this->lhs) + ")" + (lhsEqRhs?"":",") + "\n";
            if(!lhsEqRhs)
                s += "\t\t  [" + this->rhs + "] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"%xmm2\", \"%rax\", \"cc\"\n";
            s += "\t);\n";
            return s;
        }
	}
    // -------------------- Comparison operators -------------------- 
	else if(this->op == "==")
	{
		//if(compLevel == CompileLevel::E || compLevel == CompileLevel::D)
	    return "\tif(" + cut(this->lhs) + " == " + cut(this->rhs) + ")\n";
        /*else if(this->type == Data::Type::STRING)
        {
            std::string s;
            s += "\t\t\"movsd %[" + this->lhs + "], %%xmm0\\n\\t\"\n";
            s += "\t\t\"movsd %[" + this->rhs + "], %%xmm1\\n\\t\"\n";
            s += "\t\t\"compsd %%xmm1, %%xmm0\\n\\t\"\n";            // Not sure if this works for floating-point numbers 
            s += "\t\t\"je ----\\n\\t\"\n";
            return s;
        }*/
	}
    else if(this->op == "~=")
	{
		return "\tif(" + cut(this->lhs) + " != " + cut(this->rhs) + ")\n";
	}
    else if(this->op == "<=")
	{
		return "\tif(" + cut(this->lhs) + " <= " + cut(this->rhs) + ")\n";
	}
    else if(this->op == ">=")
	{
		return "\tif(" + cut(this->lhs) + " >= " + cut(this->rhs) + ")\n";
	}
    else if(this->op == "<")
	{
		return "\tif(" + cut(this->lhs) + " < " + cut(this->rhs) + ")\n";
	}
    else if(this->op == ">")
	{
		return "\tif(" + cut(this->lhs) + " > " + cut(this->rhs) + ")\n";
	}
    // --------------------------------------------------------------------------
    else if(this->op == "cpy")
	{
        if(compLevel == CompileLevel::E || compLevel == CompileLevel::D)
		    return "\t" + this->name + " = " + cut(this->lhs) + ";\n";
        else if(compLevel == CompileLevel::C)
        {
            if(this->type == Data::Type::NUMBER || this->type == Data::Type::BOOL)
            {
                std::string s;
                s += "\t// Expand " + this->name + " := " + this->lhs + "\n";
                s += "\tasm __volatile__(\n";
                s += "\t\t\"movsd %[" + this->lhs + "], %%xmm0\\n\\t\"\n";
                s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";
                // Output
                s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
                // Input
                s += "\t\t: [" + this->lhs + "] \"x\" (" + cut(this->lhs) + ")\n";
                // Clobbered registers
                s += "\t\t: \"%xmm0\", \"cc\"\n";
                s += "\t);\n";
                return s;
            }
            else if(this->type == Data::Type::STRING)
            {
                // This might be wrong.
                std::string s;
                s += "\t// Expand " + this->name + " := " + this->lhs + "\n";
                s += "\tasm __volatile__(\n";
                s += "\t\t\"movq %[" + this->lhs + "], %%rax\\n\\t\"\n";
                s += "\t\t\"movq %%rax, %[" + this->name + "]\\n\\t\"\n";
                // Output
                s += "\t\t: [" + this->name + "] \"=g\" (" + this->name + ")\n";
                // Input
                s += "\t\t: [" + this->lhs + "] \"g\" (" + cut(this->lhs) + ")\n";
                // Clobbered registers
                s += "\t\t: \"%rax\", \"cc\"\n";
                s += "\t);\n";
                return s;
            }
        }
	}
    else if(this->op == "storeAt")
    {
        if(compLevel == CompileLevel::E || compLevel == CompileLevel::D)
            return "\t" + this->name + "[(long int)" + cut(this->rhs) + " - 1] = " + cut(this->lhs) + ";\n";
        else if(compLevel == CompileLevel::C)
        {
            std::string s;
            s += "\t// Expand " + this->name + "[(long int)" + cut(this->rhs) + " - 1] = " + cut(this->lhs) + ";\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[" + this->rhs + "], %%xmm0\\n\\t\"\n";                 // xmm0 = rhs
            s += "\t\t\"cvttsd2siq %%xmm0, %%rax\\n\\t\"\n";                           // rax = (long int)xmm0
            s += "\t\t\"subq $1, %%rax\\n\\t\"\n";                                     // rax = rax - 1
            s += "\t\t\"movsd %[" + this->lhs + "], %%xmm1\\n\\t\"\n";                 // xmm1 = lhs
            s += "\t\t\"lea %[" + this->name + "], %%rdx\\n\\t\"\n";                   // rbx = &name
            s += "\t\t\"movsd %%xmm1, (%%rdx, %%rax, 8)\\n\\t\"\n";                    // rbx[rax] = xmm1 
            // Output
            s += "\t\t: [" + this->name + "] \"=g\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [" + this->lhs + "] \"x\" (" + cut(this->lhs) + "),\n";
            s += "\t\t  [" + this->rhs + "] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%xmm1\", \"%rax\", \"%rdx\", \"cc\"\n";
            s += "\t);\n";
            return s;
        }
    }
    else if(this->op == "loadAt")
    {
        if(compLevel == CompileLevel::E || compLevel == CompileLevel::D)
            return "\t" + this->name + " = " + cut(this->lhs) + "[(long int)" + cut(this->rhs) + " - 1];\n";
        else if(compLevel == CompileLevel::C)
        {
            std::string s;
            s += "\t// Expand " + this->name + " = " + cut(this->lhs) + "[(long int)" + cut(this->rhs) + " - 1];\n";
            s += "\tasm __volatile__(\n";
            s += "\t\t\"movsd %[" + this->rhs + "], %%xmm0\\n\\t\"\n";                 // xmm0 = rhs
            s += "\t\t\"cvttsd2siq %%xmm0, %%rax\\n\\t\"\n";                           // rax = (long int)xmm0
            s += "\t\t\"subq $1, %%rax\\n\\t\"\n";                                     // rax = rax - 1
            //s += "\t\t\"lea %[" + this->lhs + "], %%rdx\\n\\t\"\n";                    // rbx = &lhs
            s += "\t\t\"movsd (%[" + this->lhs + "], %%rax, 8), %%xmm0\\n\\t\"\n";                    // xmm0 = rbx[rax]
            s += "\t\t\"movsd %%xmm0, %[" + this->name + "]\\n\\t\"\n";                // name = xmm0
            // Output
            s += "\t\t: [" + this->name + "] \"=x\" (" + this->name + ")\n";
            // Input
            s += "\t\t: [" + this->lhs + "] \"g\" (" + cut(this->lhs) + "),\n";
            s += "\t\t  [" + this->rhs + "] \"x\" (" + cut(this->rhs) + ")\n";
            // Clobbered registers
            s += "\t\t: \"%xmm0\", \"%rax\", \"%rdx\", \"cc\"\n";
            s += "\t);\n";
            return s;
        }
    }
    else if(this->op == "call") // Function call.
	{
        if(cut(this->lhs) == "printf")
        {
            std::string l = this->rhs;
            if(l[0] == '#') // Variable
            {
                if(l.size() > 2 && l[1] == '_' && l[2] == 't') // Temp variable
                {
                    std::string l2 = getPrefix(l);
                    // Assume only floats as temp variables in printf.
                    return "\tprintf(\"%.1lf" + l2 + "\", " + cut(l) + ");\n";
                }
                else // Need to fetch from symbol table.
                {
                    Symbol sym = symbols->get(cut(l));
                    std::string typeStr = ".1lf";
                    if(sym.data.type == Data::Type::STRING)
                        typeStr = "s";
                    else if(sym.data.type == Data::Type::BOOL)
                        typeStr = "d";
                    
                    std::string l2 = getPrefix(l);
                    return "\tprintf(\"%" + typeStr + l2 + "\", " + cut(l) + ");\n";
                }
            }
            else if(l[0] == '$') // Number
            {
                std::string l2 = getPrefix(l);
                return "\tprintf(\"%.1lf" + l2 + "\", " + cut(l) + ");\n";
            } 
        }
        else if(cut(this->lhs) == "scanf")
        {
            return "\tscanf(\"%lf\", &" + this->name + ");\n";
        }
        else // User-created functions
        {
            if(this->rhs == "NIL")
                return "\t" + this->lhs + "();\n";
            else
                return "\t" + this->name + " = " + this->lhs + "(" + cut(this->rhs) + ");\n";
        }
    }
    else if(this->op == "ret")
    {
        if(cut(this->lhs) == "NIL")
            return "\treturn;\n";
        else
        {
            return "\treturn " + cut(this->lhs) + ";\n";
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
        out += i.toTarget(symbols);

	// If it can branch to two blocks.
	if(this->tExit && this->fExit)
	{
        if(compLevel != CompileLevel::B && compLevel != CompileLevel::A)
        {
            out += "\t\tgoto " + this->tExit->name + ";\n";
            out += "\telse\n";
            out += "\t\tgoto " + this->fExit->name + ";\n";
        }
    }

	// If it can only flow to one block.
	if(this->tExit && !this->fExit)
	{
        if(compLevel != CompileLevel::B && compLevel != CompileLevel::A)
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

bool BBlock::hasReturn() const
{
    // Return ture if a return operator was found.
    for(ThreeAd i : this->instructions)
        if(i.op == "ret")
            return true;
    return false;
}

void BBlock::fetchVars(VMap& vmap, Symbols* symbols)
{
    for(ThreeAd& i : this->instructions)
    {
        if(i.type != Data::Type::NIL && symbols->map.find(i.name) == symbols->map.end() && i.op != "ret")
        {
            if(vmap.find(i.type) == vmap.end())
            {
                VSet vSet;
                vmap.insert({i.type, vSet});
            }
            VSet& vset = vmap[i.type];
            vset.insert(i.name);
        }
    }
}

BBlock* BBlock::getLastBlock()
{
    BBlock* end = nullptr;
    std::set<BBlock *> done, todo;
    todo.insert(this);
    while(todo.size()>0)
    {
        // Pop an arbitrary element from todo set
        auto first = todo.begin();
        BBlock *next = *first;
        todo.erase(first);

        if(next->tExit == NULL && next->fExit == NULL)
        {
            end = next;
            break;
        }

        done.insert(next);
        if(next->tExit!=NULL && done.find(next->tExit)==done.end())
            todo.insert(next->tExit);

        if(next->fExit!=NULL && done.find(next->fExit)==done.end())
            todo.insert(next->fExit);
    }
    return end;
}

// ---------------------------------------------------------------------------------------------------------

std::string getType(Data::Type type)
{
    if(type == Data::Type::NUMBER)
        return "double";
    if(type == Data::Type::STRING)
        return "char*";
    if(type == Data::Type::BOOL)
        return "long int";
    return Data::typeToString(type);
}

// ---------------------------------------------------------------------------------------------------------

VMap fetchVars(BBlock* start)
{
    VMap varMap;

    std::set<BBlock *> done, todo;
    todo.insert(start);
    while(todo.size()>0)
    {
        // Pop an arbitrary element from todo set
        auto first = todo.begin();
        BBlock *next = *first;
        todo.erase(first);

        // Fetch this block's variables.
        next->fetchVars(varMap, start->symbols);

        done.insert(next);
        if(next->tExit!=NULL && done.find(next->tExit)==done.end())
            todo.insert(next->tExit);

        if(next->fExit!=NULL && done.find(next->fExit)==done.end())
            todo.insert(next->fExit);
    }

    return varMap;
}

void initVariables(std::ofstream& file, BBlock* start, std::vector<Symbol> exclude)
{
    file << "\t// Initialize symbols." << std::endl;
    for(auto& e : start->symbols->map)
    {
        Symbol& sym = e.second;
        std::vector<Symbol>::iterator it = std::find(exclude.begin(), exclude.end(), sym);
        if(it == exclude.end())
        {
            std::string cStr = (e.first.size() > 2 ? (e.first[0] == '_' || e.first[2] == '_' ? "const " : "") : "");
            if(sym.data.type == Data::Type::NUMBER)
                file << "\t" << cStr << "double " << e.first << " = " << sym.data.f << ";" << std::endl;
            if(sym.data.type == Data::Type::STRING)
                file << "\t" << cStr << "char " << e.first << "[] = \"" + sym.data.s + "\";" << std::endl;
            if(sym.data.type == Data::Type::BOOL)
                file << "\t" << cStr << "long int " << e.first << " = " << (long int)sym.data.b << ";" << std::endl;
            if(sym.data.type == Data::Type::TABLE)
                file << "\t" << cStr << "double " << e.first << "[" << (long int)(sym.size/sizeof(double)) << "];" << std::endl;
        }
    }
    file << std::endl;
}

// ---------------------------------------------------------------------------------------------------------

void initTmpVariables(std::ofstream& file, BBlock* start)
{
    auto varTypeToFile = [&file](VMap::iterator it)->void {
        file << "\t" << getType(it->first) << " ";
        std::set<std::string>::iterator sIt = it->second.begin();
        for(int i = 0; sIt != it->second.end(); sIt++, i++)
            file << (i==0? "" : ", ") << *sIt;
        file << ";" << std::endl;
    };
    
    file << "\t// Initialize temp-variables." << std::endl;
    VMap vMap = fetchVars(start);
    VMap::iterator it = vMap.begin();
    for(;it != vMap.end(); it++)
        varTypeToFile(it);
    file << std::endl;
}

// ---------------------------------------------------------------------------------------------------------

void dumpCFGInstructions(std::ofstream& file, BBlock* start)
{
    BBlock* endBlock = start->getLastBlock();

    // The dump each block of code.
    file << "\t// The code." << std::endl;
    bool dbg_wasT = false;
    std::set<BBlock *> done, todo;
    todo.insert(start);
    while(todo.size()>0)
    {
        // Pop an arbitrary element from todo set
        auto first = todo.begin();
        BBlock *next = *first;
        todo.erase(first);
        
        if(next != endBlock)
        {
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
    }

    // Dump end block.
    file <<  endBlock->toTarget(start->symbols);
}

// ---------------------------------------------------------------------------------------------------------

std::vector<std::pair<Symbol, BBlock*>> getFunctionMap(Symbols* symbols, std::vector<BBlock*>& functionBlocks)
{
    std::vector<std::pair<Symbol, BBlock*>> functionMap;

    for(auto s : symbols->map)
    {
        std::vector<BBlock*>::iterator it = std::find(functionBlocks.begin(), functionBlocks.end(), s.second.funcBlock);
        if(it != functionBlocks.end())
            functionMap.push_back({s.second, *it});
    }

    return functionMap;
}

// ---------------------------------------------------------------------------------------------------------

void dumpToTarget(BBlock* start, std::vector<BBlock*> funcBlocks)
{
    std::ofstream file;
    std::string fileName = "target.cc";
    if(compLevel == CompileLevel::D || compLevel == CompileLevel::C) 
        fileName = "target.c";
    file.open(fileName);
    file << "#include <stdio.h>" << std::endl;   // printf
    file << "#include <math.h>" << std::endl;    // pow
    file << std::endl;
    
    // Functions.
    std::vector<std::pair<Symbol, BBlock*>> functionMap = getFunctionMap(start->symbols, funcBlocks);
    for(auto f : functionMap)
    {
        Symbol fArgs1 = f.second->funcArgs[0];
        file << "double " << f.first.data.name << "("<< getType(fArgs1.data.type) << " " << fArgs1.data.name << ")\n{" << std::endl;
        // Initialize the variables.
        initVariables(file, f.second, f.second->funcArgs);

        // Initialize temp-variables.
        initTmpVariables(file, f.second);

        // Dump all block in the CFG.
        dumpCFGInstructions(file, f.second);

        file << "}" << std::endl << std::endl; 
    }

    file << "int main()\n{\n";

    // Initialize the variables.
    initVariables(file, start);

    // Initialize temp-variables.
    initTmpVariables(file, start);

    // Dump all block in the CFG.
    dumpCFGInstructions(file, start);

    // End
    file << "\treturn 0;\n}\n";

    file.close();
}