#ifndef AST_DATA_HH
#define AST_DATA_HH

#include <string>

class Data
{
public:
	enum Type {NUMBER, STRING, BOOL, NIL, FUNCTION, FUNCTIONCALL, TABLE, SCOPE, EMPTY, MAX, MAX2 };
	static std::string TypeNames[Type::MAX2];

	// Empty data, no name, no type.
	Data() : type(Type::EMPTY) { makeName("Empty"); }
	// Type data.
	Data(Type type) : type(type) { makeName(TypeNames[this->type]); }
	// Nil data.
	Data(const std::string& name) { makeName(name); setNil(); }
	// Number data.
	Data(const std::string& name, double f) { makeName(name); setVal(f); }
	// Boolean data.
	Data(const std::string& name, bool b) { makeName(name); setVal(b); }
	// String data.
	Data(const std::string& name, const std::string& s) {makeName(name); setVal(s); }

	void setVal(const Data& data);
	void setVal(double f);
	void setVal(bool b);
	void setVal(const std::string& s);
	void setNil();

	bool hasEnvironment() const;

	// -------- Debug --------
	std::string toString() const;
	std::string toStringEx() const;
	std::string valToString(Type type = Type::MAX) const;
	std::string typeToString() const;
	// -----------------------

	void makeName(const std::string& name);
	
	Type type = Type::EMPTY;
	double f;
	bool b;
	std::string s;
	std::string name;

private:
	static unsigned int counter;
};

#endif