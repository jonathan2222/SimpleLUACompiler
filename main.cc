#include <iostream>
#include <fstream>
#include "binary.tab.hh"
#include "AST/AST.hh"
extern Node* root;
extern FILE* yyin;
extern CompileLevel compLevel;

void yy::parser::error(std::string const&err)
{
    std::cout << err << std::endl;
}

void showHelp()
{
    std::cout << "Usage: comp [options], comp <lua-file>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "\t-c <level>\tSet the compile level to use. level can be E, D, C, B or A." << std::endl;
    std::cout << "\t-h, --help\tShow this help message." << std::endl;    
}

int processOptions(int argc, char** argv)
{
    // Default comp-level
    compLevel = CompileLevel::D;

    int fileIndex = -1;
    for(unsigned i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if(arg == "-c")
        {
            if(argc-i > 1)
            {
                i++;
                arg = argv[i];
                if(arg == "E" || arg == "e") compLevel = CompileLevel::E;
                else if(arg == "D" || arg == "d") compLevel = CompileLevel::D;
                else if(arg == "C" || arg == "c") compLevel = CompileLevel::C;
                else if(arg == "B" || arg == "b") compLevel = CompileLevel::B;
                else if(arg == "A" || arg == "a") compLevel = CompileLevel::A;                
            }
            else
                std::cout << "-c option requires one argument." << std::endl;
        }
        else if(arg == "-h" || arg == "--help") showHelp();
        else if(arg[0] == '-')
            std::cout << "comp: invalid option '" << arg << "'" << std::endl;
        else
            fileIndex = i;
    }
    return fileIndex;
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
    int fileIndex = processOptions(argc, argv); 
    if(fileIndex != -1)
    {
        yyin = fopen(argv[fileIndex], "r");
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
    }

    delete root;
    return 0;
}