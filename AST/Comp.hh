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

#define VSet std::set<std::string>
#define VMap std::unordered_map<Data::Type, VSet>

/************* Three Address Instructions *************/
class Symbols;
class ThreeAd
{
public:
    std::string name,lhs,rhs;
    std::string op;
    Data::Type type;

    ThreeAd(std::string name, std::string op, std::string lhs, std::string rhs, Data::Type type) :
        name(name), op(op), lhs(lhs), rhs(rhs), type(type) { }

    void dump();
    std::string cfg();

    std::string toTarget(Symbols* symbols);
};


/* Basic Blocks */
class BBlock
{
private:
    static int nCounter;
public:
    std::list<ThreeAd> instructions;
    BBlock *tExit, *fExit;
    std::string name;

    std::set<std::string> vars;

    BBlock() : tExit(NULL), fExit(NULL), name("blk" + std::to_string(nCounter++)) { }

    void dump();
    std::string cfg();

    std::string toTarget(Symbols* symbols);
    Symbols* symbols = nullptr;

    void fetchVars(VMap& vmap, Symbols* symbols);

    BBlock* getLastBlock();
};

std::string getType(Data::Type type);

VMap fetchVars(BBlock* start);

void dumpToTarget(BBlock* start, std::vector<BBlock*> funcBlocks);

#endif
