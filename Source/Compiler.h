/*
  ==============================================================================

    Compiler.h
    Created: 25 Jun 2019 12:02:58pm
    Author:  user

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "Type.h"



struct Arg
{
	Arg(int index) : idx(index) 
	{
	}
	int idx{ 0 };
};


struct Closure
{
	Closure(Arg a) : local(a.idx)
	{
	}
	Arg local;
};

using ArgOrClosure = mapbox::util::variant<Arg, Closure>;

class Context {

public:

	Context(uint32 argc) : sp(argc + 3)  // the saved context ocupy 3 spaces.
	{

	}

	Code* to_code()
	{
		return new Code(this->bytecode, this->consts);
	
	}

	void push_const(TExpr obj)
	{
		jassert(obj.is<TInt*>() || obj.is<Code*>() || obj.is<TBool*>());

		size_t idx = consts.size();

		consts.push_back(obj);

		bytecode.push_back(INS::LOAD_CONST);

		bytecode.push_back(idx);

		sp += 1;
	}


	//def push_arg(self, idx) :
	//	self.bytecode.append(code.DUP_NTH)
	//	self.bytecode.append(r_uint(self.sp - idx))
	//	self.sp += 1

	void push_arg(int idx)
	{
		bytecode.push_back(INS::DUP_NTH);
		bytecode.push_back(sp - idx);     // sp is 3 + x
		sp += 1;
		

		// saved args count  I.E. how many args of current calling function have.                   // current status (stack grwow upwards)
		// saved ip
		// saved code_obj
		// arg 3  and so on
		// arg 2
		// arg 1 


		// how do we get the arg we want!
		// eg: if we want the second argument, we get the stack[ curStackIndex - (sp-2) ]  is the arg2 stack pos!
	}


	void add_local(String& argName, Arg arg) //test  Arg&  or  Arg ?
	{
		locals.push_back(Symbol_ArgOrClosure(argName, arg));
		/*size_t len = locals.size();
		if (len >= 1)
		{
			auto lastItem = locals.back();
			lastItem.emplace(argName, arg);
			locals.push_back(lastItem);
		}
		else
		{
			std::map<String, Arg> argMap;
			argMap.emplace(argName, arg);
			locals.push_back(argMap);
		}*/
	}
    
    uint32 label()
    {
        uint32 lbl = bytecode.size();
        bytecode.push_back(99);
        return lbl;
    }
    
    void mark(uint32 lbl)
    {
        bytecode[lbl] = bytecode.size() - lbl;
    }

	bool get_local(String s_name, ArgOrClosure& arg_out)
	{
		bool res = false;

		for_each(locals.begin(), locals.end(), [&arg_out, &res, &s_name](Symbol_ArgOrClosure sa)
		{
			if (s_name == sa._sym)
			{
				res = true;
				arg_out = sa._arg;
			}
			
		});
		return res;

	/*	if (locals.size() > 0)
		{
			std::map<String, Arg>::iterator iter = locals.back().find(s_name);

			if (iter == locals.back().end())
			{
				return false;
			}
			else
			{
				arg_out = iter->second;
				return true;
			
			}
		}
		else 
			return false;*/
	}
	struct Symbol_ArgOrClosure
	{
		Symbol_ArgOrClosure(String sym, Arg arg) :_sym(sym), _arg(arg)
		{

		}

		String _sym;
		ArgOrClosure  _arg;
	};



	std::vector<uint32> bytecode;
	std::vector<TExpr> consts;
	std::vector<Symbol_ArgOrClosure> locals;

	std::vector<ArgOrClosure> closed_overs;


	uint32 sp;
	bool can_tail_call{ false };

};

class CompileInfo
{
public:
	String log;
	int	   indent;
	int _line;
	int _col;
};

void compile_(TExpr&  obj, Context & ctx, CompileInfo& compileInfo);
Code compile(TExpr& obj, CompileInfo& compileInfo);


void ClosureEmit(Context& ctx, int idx);

void ArgEmit(Context& ctx, int idx);