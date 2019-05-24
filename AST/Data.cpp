#include "Data.hh"

#include <sstream>
#include <iomanip>

unsigned int Data::counter = 0;
std::string Data::TypeNames[Type::MAX2] = {"NUMBER", "STRING", "BOOL", "NIL", "FUNCTION", "FUNCTIONCALL", "TABLE", "SCOPE", "EMPTY", "MAX"};

void Data::setVal(const Data& data)
{
	switch (data.type)
	{
	case Type::NUMBER:
		setVal(data.f);
		break;
	case Type::STRING:
		setVal(data.s);
		break;
	case Type::BOOL:
		setVal(data.b);
		break;
	case Type::NIL:
		setNil();
		break;
	default:
		break;
	}
}

void Data::setVal(double f)
{
	this->f = f;
	this->b = true;
	this->type = Type::NUMBER;
}

void Data::setVal(bool b)
{
	this->b = b;
	this->type = Type::BOOL;
}

void Data::setVal(const std::string& s)
{
	this->s = s;
	this->b = true;
	this->type = Type::STRING;
}

void Data::setNil()
{
	this->f = 0.0;
	this->b = false;
	this->s = "";
	this->type = Type::NIL;
}

std::string Data::toString() const
{
	return "(" + this->name + ") Type: " + TypeNames[this->type] + ", val: " + valToString(this->type);
}

std::string Data::toStringEx() const
{
	return "(" + this->name + ") Type: " + TypeNames[this->type] + ", val: (f=" + valToString(Type::NUMBER) + ", b=" + valToString(Type::BOOL) + ", s=" + valToString(Type::STRING) + ")";
}

std::string Data::valToString(Type type) const
{
	if(type == Type::MAX)
		type = this->type;
	switch (type)
	{
	case Type::NUMBER:
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision(1) <<  this->f; 
			return ss.str();
		}
		break;
	case Type::BOOL:
		return this->b ? "1" : "0";
		break;
	case Type::STRING:
		return this->s;
		break;
	case Type::NIL:
		return "NIL";
		break;
	default:
		return TypeNames[type];
		break;
	}
}

std::string Data::typeToString(Data::Type type)
{
	return TypeNames[type];
}
