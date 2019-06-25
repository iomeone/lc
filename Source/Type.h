/*
  ==============================================================================

    Type.h
    Created: 25 Jun 2019 11:20:32am
    Author:  user

  ==============================================================================
*/

#pragma once


#include <mapbox/variant.hpp>


struct TNil { int x; };
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

using TObj = mapbox::util::variant<TNil*, TInt*, TSymbol*, mapbox::util::recursive_wrapper<TCons*>>;

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
