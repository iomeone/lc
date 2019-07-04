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
	Arg(int index, Uuid uid) : idx(index)  , _uid(uid)
	{
	}
	int idx{ 0 };
	Uuid _uid;
};


struct Closure
{
	Closure(Arg a) : local(a.idx, a._uid)
	{
	}
	Arg local;
};

struct ClosureCell
{
	ClosureCell(int index) : _idx(index)
	{

	}

	int _idx;
};

using ArgOrClosure = mapbox::util::variant<Arg, Closure, ClosureCell>;

class Context {
public:
	struct Symbol_ArgOrClosure
	{
		Symbol_ArgOrClosure(String sym, Arg arg) :_sym(sym), _arg(arg)
		{

		}

		String _sym;
		ArgOrClosure  _arg;
	};


public:

	Context(uint32 argc, const std::vector<Symbol_ArgOrClosure>& parentlocals) : sp(argc + 3)  // the saved context ocupy 3 spaces.
	{
		for_each(parentlocals.begin(), parentlocals.end(), [this](Symbol_ArgOrClosure o)
		{
		
			Symbol_ArgOrClosure c = o;
			if (c._arg.is<Arg>())
			{
				c._arg = Closure(c._arg.get<Arg>());
			}
			else if (c._arg.is<Closure>())
			{
				jassert(false);  // if have two nested closure, could happen?
			}
			else if (c._arg.is<ClosureCell>())
			{
				msg("c._arg.is<ClosureCell>()???");
			}
			
			this->locals.push_back(c);
		});
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

		for_each(locals.begin(), locals.end(), [&arg_out, &res, &s_name, this](Symbol_ArgOrClosure sa)
		{
			if (s_name == sa._sym)
			{

				if (sa._arg.is<Closure>())
				{
					int idx = 0;
					for (int i = 0; i < closed_overs.size(); i++)
					{
						jassert(closed_overs[i].is<Arg>());

						if (closed_overs[i].get<Arg>()._uid == sa._arg.get<Closure>().local._uid)
							break;
						else
							++idx;
					}
					closed_overs.push_back(sa._arg.get<Closure>().local);  // returen an arg object, which will push the value which is a closure value to the stack.
					arg_out = ClosureCell(idx);
				}
				else
				{
					res = true;
					arg_out = sa._arg;
				}
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