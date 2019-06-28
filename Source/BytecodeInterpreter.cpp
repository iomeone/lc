/*
  ==============================================================================

    BytecodeInterpreter.cpp
    Created: 28 Jun 2019 9:59:03am
    Author:  user

  ==============================================================================
*/
#include "../JuceLibraryCode/JuceHeader.h"
#include "BytecodeInterpreter.h"
#include <stack>  

class Frame
{
public:
	Frame(const Code& code_obj): _code_obj(code_obj)
	{
		
	
	}

	void unpack_code_obj()
	{
		this->_consts = _code_obj._consts;
		this->_bytecode = _code_obj._bytecode;
	}

	uint32 get_inst()
	{
		jassert(ip >= 0 && ip < _bytecode.size());
		uint32 inst = this->_bytecode[ip];
		++ip;
		return inst;
	}

	void push( TExpr val)
	{
		stack[sp] = val;
		++sp;
	}

	TExpr pop()
	{
		--sp;
		return stack[sp];
	}


	TExpr nth(uint32 delta)
	{
		return stack[sp - delta - 1];
	}

	void push_nth(uint32 delta)
	{
		push(nth(delta));
	}


	void descend( TExpr  code_obj, uint32 args)
	{
		jassert(code_obj.is<Code*>());

		push(code_obj);
		push(new TInt(ip));
		push(new TInt(args));

		_code_obj = *code_obj.get<Code*>();
		unpack_code_obj();
		ip = 0;

	}


	TExpr ascend()
	{
		TExpr ret_val = pop();
		if (sp == 0)
			return ret_val;
		TExpr w_args = pop();
		jassert(w_args.is<TInt*>());

		TExpr w_ip = pop();
		jassert(w_ip.is<TInt*>());

		TExpr code_obj = pop();
		jassert(code_obj.is<Code*>());

		for (int i = 0; i < w_args.get<TInt*>()->_val; i++)
		{
			pop();
		}

		unpack_code_obj();
		
		push(ret_val);

		ip = w_ip.get<TInt*>()->_val;
	}

	void push_const(uint32 idx)
	{
		push(_consts[idx]);
	}

	void jump_rel(uint32 delta)
	{
		ip += delta - 1;
	}


	Code _code_obj;

	std::vector<uint32> _bytecode;
	std::vector<TExpr> _consts;


	uint32 sp;
	uint32 ip;
	TExpr stack[24];
	//std::stack<TExpr, std::vector<TExpr>> stack;
};



TExpr interpret(const Code & code_obj)
{




	return new TInt(-1);
}
