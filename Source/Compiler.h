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
class Code
{
public:
	Code::Code(Context* ctx);

	std::vector<uint32> _bytecode;
	std::vector<uint32> _consts;
};


class Context {

public:

	Context(uint32 argc) : sp(argc + 3)  // the saved context ocupy 3 spaces.
	{

	}

	Code to_code()
	{
		Code c(this);
		return c;
	}

	void push_const(TObj& obj)
	{
		jassert(obj.is<TInt*>());

		int idx = consts.size();

		consts.push_back(obj.get<TInt*>()->_val);

		bytecode.push_back(INS::LOAD_CONST);

		bytecode.push_back(idx);

		sp += 1;
	}

	std::vector<uint32> bytecode;
	std::vector<uint32> consts;
	std::vector< std::map<std::string, int>> locals;


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
