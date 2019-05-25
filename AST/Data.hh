#ifndef AST_DATA_HH
#define AST_DATA_HH

#include <string>

class Data
{
public:
	struct TypeHash
	{
		template<typename T>
		std::size_t operator()(T t) const { return static_cast<std::size_t>(t); }
	};
	enum Type {NUMBER, STRING, BOOL, NIL, FUNCTION, FUNCTIONCALL, TABLE, SCOPE, EMPTY, MAX, MAX2 };
	static std::string TypeNames[Type::MAX2];

	// Empty data, no name, no type.
	Data() : type(Type::EMPTY), name("NO_NAME") { }
	// Type data.
	Data(Type type) : type(type), name("NO_NAME") { }
	// Nil data.
	Data(const std::string& name) : name(name) { setNil(); }
	// Number data.
	Data(const std::string& name, double f) : name(name) { setVal(f); }
	// Boolean data.
	Data(const std::string& name, bool b) : name(name) { setVal(b); }
	// String data.
	Data(const std::string& name, const std::string& s) : name(name) { setVal(s); }

	void setVal(const Data& data);
	void setVal(double f);
	void setVal(bool b);
	void setVal(const std::string& s);
	void setNil();

	// -------- Debug --------
	std::string toString() const;
	std::string toStringEx() const;
	std::string valToString(Type type = Type::MAX) const;
	static std::string typeToString(Data::Type type);
	// -----------------------

	Type type = Type::EMPTY;
	double f;
	bool b;
	std::string s;
	std::string name;

private:
	static unsigned int counter;
};

#endif