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
	JMP
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

struct TSymbol
{
	TSymbol(String s)
	{
		_sym = s;
	}
	String _sym;
};




struct TCons;
struct Code;

using TObj = mapbox::util::variant<TNil*, TInt*, TSymbol*, mapbox::util::recursive_wrapper<Code*>, mapbox::util::recursive_wrapper<TCons*>>;


struct Code
{
public:
    Code(std::vector<uint32> bytecode, std::vector<TObj> consts);
    
    std::vector<uint32> _bytecode;
    std::vector<TObj> _consts;
};

struct TCons
{
	TCons(TObj h, TObj t)
	{
		_head = h;
		_tail = t;
	}
	TObj _head;
	TObj _tail;
};



