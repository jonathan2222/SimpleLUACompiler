#include <iostream>
#include <fstream>
#include "binary.tab.hh"
#include "AST/AST.hh"
extern Node* root;
extern FILE* yyin;

void yy::parser::error(std::string const&err)
{
    std::cout << err << std::endl;
}

void toDot()
{
    std::ofstream file;
    file.open("parse.dot");
    if(file.is_open())
    {
        std::string s = "digraph {\ngraph [bgcolor=black]\nedge [color=white]\n";
        s += root->dot();
        s += "}";
        file << s;
        file.close();
    }   
}

int main(int argc, char **argv)
{
    yyin = fopen(argv[1], "r");
    yy::parser parser;
    if(!parser.parse()) {
        toDot();
        BBlock* start = new BBlock();
        start->symbols = root->children.front()->symbols;
        root->convert(start);
        //root->printAllSymbols();
        root->compressSymbols();
        root->toCfg(start);
        
        //printf("----------------------------------------------------------------\n");
        //root->printAllSymbols();

        //printf("----------------------------------------------------------------\n");
        dumpToTarget(start, root->functions);
        
        delete start;
    }

    delete root;

    return 0;
}