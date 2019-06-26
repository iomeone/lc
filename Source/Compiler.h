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


class Context;


//class Arg(LocalType) :
//	def __init__(self, idx) :
//	self.idx = r_uint(idx)
//
//	def emit(self, ctx) :
//	ctx.push_arg(self.idx)

class Arg
{
public:
	Arg(int index): idx(index){}

	void emit(Context& ctx);

	int idx{ 0 };
};






class Context {

public:

	Context(uint32 argc) : sp(argc + 3)  // the saved context ocupy 3 spaces.
	{

	}

	Code* to_code()
	{
		return (new Code(this->bytecode, this->consts));
	
	}

	void push_const(TObj obj)
	{
		jassert(obj.is<TInt*>());

		int idx = consts.size();

		consts.push_back(obj.get<TInt*>()->_val);

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
		int len = locals.size();
		if (len > 1)
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
		}
	}

	std::vector<uint32> bytecode;
	std::vector<uint32> consts;
	std::vector< std::map<String, Arg>> locals;


	uint32 sp;
	bool can_tail_call{ false };

};

class CompileInfo
{
public:
	String log;
	int	   indent;
};

void compile_(TObj&  obj, Context & ctx, CompileInfo& compileInfo);
Code compile(TObj& obj, CompileInfo& compileInfo);
