#ifndef COMP_HH
#define COMP_HH

#include <list>
#include <set>
#include <initializer_list>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "Data.hh"
#include "Symbols.hh"

#define VSet std::set<std::string>
#define VMap std::unordered_map<Data::Type, VSet, Data::TypeHash>

enum CompileLevel { E, D, C, B, A };

/************* Three Address Instructions *************/
class Symbols;
class BBlock;
class ThreeAd
{
public:
    std::string name,lhs,rhs;
    std::string op;
    Data::Type type;
    BBlock* parent = nullptr;

    ThreeAd(std::string name, std::string op, std::string lhs, std::string rhs, Data::Type type, BBlock* parent) :
        name(name), op(op), lhs(lhs), rhs(rhs), type(type), parent(parent) { }

    void dump();
    std::string cfg();

    std::string toTarget(Symbols* symbols, unsigned vSize);
};


/* Basic Blocks */
class BBlock
{
private:
    static int nCounter;
public:
    std::vector<Symbol> funcArgs;
    std::list<ThreeAd> instructions;
    BBlock *tExit, *fExit;
    std::string name;
    unsigned id;
    unsigned vSize = 0; // Size of total local variables

    std::set<std::string> vars;

    BBlock() : tExit(NULL), fExit(NULL) { id = nCounter; name = "blk" + std::to_string(nCounter++); }

    void dump();
    std::string cfg();

    std::string toTarget(Symbols* symbols, unsigned vSize);
    Symbols* symbols = nullptr;

    bool hasReturn() const;
    void fetchVars(VMap& vmap, Symbols* symbols);

    BBlock* getLastBlock();
};

std::string getType(Data::Type type);

VMap fetchVars(BBlock* start);

void initVariables(std::ofstream& file, BBlock* start, std::vector<Symbol> exclude = std::vector<Symbol>(), std::unordered_map<std::string, Symbol>* include = nullptr);
void initTmpVariables(std::ofstream& file, BBlock* start);
unsigned getMaxTmpVSize(BBlock* start);
void dumpCFGInstructions(std::ofstream& file, BBlock* start, unsigned vSize);
std::vector<std::pair<Symbol, BBlock*>> getFunctionMap(Symbols* symbols, std::vector<BBlock*>& functionBlocks);

std::string setup();
std::string cleanup();
void dumpToTarget(BBlock* start, std::vector<BBlock*> funcBlocks);

#endif
