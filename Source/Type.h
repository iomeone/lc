/*
  ==============================================================================

    Type.h
    Created: 25 Jun 2019 11:20:32am
    Author:  user

  ==============================================================================
*/

#pragma once


#include <mapbox/variant.hpp>


enum INS
{
	LOAD_CONST = 0,
	ADD,
	EQ,
	INVOKE,
	TAIL_CALL,
	DUP_NTH,
	RETURN,
	COND_BR,
	JMP,
	CLOSED_OVER,
	MAKE_CLOSURE,
	POP
};



struct TNil {  };
struct TInt
{
	TInt(int v)
	{
		_val = v;
	}
	int _val;

};

struct TBool
{
    TBool(bool b) : _b(b)
    {
        
    }
    bool _b;
};

struct TSymbol
{
	TSymbol(String s)
	{
		_sym = s;
	}
	String _sym;
};

struct TClosure
{
	TClosure(int index) : idx(index) {}

	int idx{ 0 };
};

struct TCons;
struct Code;
struct SavedClosure;
using TExpr = mapbox::util::variant<TNil*, TInt*, TBool*, TSymbol*, 
	mapbox::util::recursive_wrapper<Code*>, 
	mapbox::util::recursive_wrapper<SavedClosure*>,
	mapbox::util::recursive_wrapper<TCons*>>;


struct Code
{
public:
    Code(std::vector<uint32> bytecode, std::vector<TExpr> consts);
	
	Code(const Code & c)
	{
		this->_bytecode = c._bytecode;
		this->_consts = c._consts;
	}

    std::vector<uint32> _bytecode;
    std::vector<TExpr> _consts;
};


struct SavedClosure
{
public:
	SavedClosure(const SavedClosure& other):_code(other._code), closed_overs(other.closed_overs)
	{
	 
	}
	SavedClosure(Code& code, std::vector<TExpr> otherClosedOvers) : _code(code), closed_overs(otherClosedOvers)
	{

	}
	std::vector<TExpr> closed_overs;
	Code _code;
};






struct TCons
{
	TCons(TExpr h, TExpr t)
	{
		_head = h;
		_tail = t;
	}
	TExpr _head;
	TExpr _tail;
};

