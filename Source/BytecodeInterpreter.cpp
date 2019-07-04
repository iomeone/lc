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
#include <list>

//mapbox::util::recursive_wrapper<Code*>,
//mapbox::util::recursive_wrapper<SavedClosure*>,
class Frame
{
public:
	Frame(const TExpr& code_obj) : _code_obj(code_obj)
	{
		ip = 0;
		sp = 0;
		unpack_code_obj();
		closed_overs.clear();
	}

	void unpack_code_obj()
	{
		if (_code_obj.is< Code*>())
		{
			this->_consts = _code_obj.get<Code*>()->_consts;
			this->_bytecode = _code_obj.get<Code*>()->_bytecode;
		}
		else if(_code_obj.is<SavedClosure*>())
		{
			this->_consts = _code_obj.get<SavedClosure*>()->_code._consts;
			this->_bytecode = _code_obj.get<SavedClosure*>()->_code._bytecode;
			this->closed_overs = _code_obj.get<SavedClosure*>()->closed_overs;
		}
		else
		{
			jassertfalse;
		}
			
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
		jassert(code_obj.is<Code*>()  || 
				code_obj.is<SavedClosure*>());

		TExpr  c;
		if (code_obj.is<Code*>())
		{
			c = new Code(*code_obj.get<Code*>());
		}
		else if (code_obj.is<SavedClosure*>())
		{
			c = new SavedClosure(*code_obj.get<SavedClosure*>());
		}
		else
		{
			jassertfalse;
			msg("the save data must be code or closure!");
		}

		push(this->_code_obj);
		push(new TInt(ip));
		push(new TInt(args));

		_code_obj = code_obj;// *code_obj.get<Code*>();
		unpack_code_obj();
		ip = 0;

	}


	bool ascend(TExpr& ret_expr)
	{
		TExpr ret_val = pop();
		if (sp == 0)
        {
            ret_expr = ret_val;
            return true;
        }
		TExpr w_args = pop();
		jassert(w_args.is<TInt*>());

		TExpr w_ip = pop();
		jassert(w_ip.is<TInt*>());

		TExpr code_obj = pop();
		jassert(code_obj.is<Code*>() || code_obj.is<SavedClosure*>());
		_code_obj = code_obj;
		//delete code_obj.get<Code*>();

		for (int i = 0; i < w_args.get<TInt*>()->_val; i++)
		{
			pop();
		}

		unpack_code_obj();
		
		push(ret_val);

		ip = w_ip.get<TInt*>()->_val;
        
        return false;
	}

	void push_const(uint32 idx)
	{
		push(_consts[idx]);
	}

	void jump_rel(uint32 delta)
	{
		ip += delta - 1;
	}


	TExpr _code_obj; // 

	std::vector<uint32> _bytecode;
	std::vector<TExpr> _consts;


	uint32 sp;
	uint32 ip;
	TExpr stack[24];

	std::vector<TExpr> closed_overs;
	//std::stack<TExpr, std::vector<TExpr>> stack;
};

class Numbers
{
public:
	static TExpr add(const TExpr& a, const TExpr b)
	{
		jassert(a.is<TInt*>());

		jassert(b.is<TInt*>());

		int ia = a.get<TInt*>()->_val;
		int ib = b.get<TInt*>()->_val;

		return new TInt(ia + ib);
	}
    
    static TExpr eq(const TExpr& a, const TExpr b)
    {
        jassert(a.is<TInt*>());
        
        jassert(b.is<TInt*>());
        
        int ia = a.get<TInt*>()->_val;
        int ib = b.get<TInt*>()->_val;
        
        return new TBool(ia == ib);
    }
};

TExpr interpret(const Code & code_obj)
{

	Frame frame(new Code(code_obj));

	while (true)
	{
		uint32 inst = frame.get_inst();
		
		if(inst == INS::LOAD_CONST)
		{
			uint32 arg = frame.get_inst();   // arg is the offset of consts, so always be an integer. 
			frame.push_const(arg);           // get arg from const according offset, then put to the stack.
			continue;
		}
		else if(inst == INS::ADD)
		{
			TExpr a = frame.pop(); 
		
			TExpr b = frame.pop();
			
			frame.push(Numbers::add(a, b));
			continue;
		}
		else if(inst == INS::INVOKE)
		{
			uint32 args = frame.get_inst();  // the count of args of the function have.
			TExpr fn = frame.nth(args - 1);             // we get the fn.(type is TExpr), which is pushed by the LOAD_CONST instruction.
			jassert(fn.is<Code*>());

			frame.descend(fn, args);          // save current context, and call the subroutinue
			continue;
		}
		else if(inst == INS::TAIL_CALL)
		{
			uint32 args = frame.get_inst();
			std::vector<TExpr> tmp_args;

			for (uint32 i = 0; i < args; i++)
			{
				tmp_args.push_back(frame.pop()); // tmp_args now is ... arg4 arg3 arg2 argFun
			}
			TExpr code_obj_exp = tmp_args[args - 1];
			jassert(code_obj_exp.is<Code*>() || code_obj_exp.is<SavedClosure*>());

			TExpr code_obj = code_obj_exp;


			TExpr old_args_w = frame.pop();
			jassert(old_args_w.is<TInt*>());

			TExpr old_ip = frame.pop();
			TExpr old_code = frame.pop();

			for (int i = 0; i < old_args_w.get<TInt*>()->_val; i++)
				frame.pop();

			for (int i = args - 1; i >= 0; i--)
			{
				frame.push(tmp_args[i]);
			}

			frame.push(old_code);

			frame.push(old_ip);
			frame.push(new TInt(args));

			frame._code_obj = code_obj;
			frame.unpack_code_obj();
			frame.ip = 0;
			continue;
		}
        else if(inst == INS::DUP_NTH)
        {
            uint32 arg = frame.get_inst();
            frame.push_nth(arg);
            continue;
        }
        else if(inst == INS::RETURN)
        {
            TExpr expr;
            if(frame.ascend(expr))
                return expr;
            else
                continue;
        }
        else if(inst == INS::COND_BR)
        {
            TExpr tst = frame.pop();
            uint32 loc = frame.get_inst();  // loc is the else loc
			if (tst.is<TBool*>())
			{
				if (tst.get<TBool*>()->_b != false)
				{
					continue;
				}
				else
				{
					frame.jump_rel(loc);
				}
					
			}
            continue;
        }
        else if(inst == INS::JMP)
        {
            uint32 ip = frame.get_inst();
            frame.jump_rel(ip);
            continue;
        }
        else if(inst == INS::EQ)
        {
            TExpr eq_ret = Numbers::eq(frame.pop(), frame.pop());
            frame.push(eq_ret);
            continue;
        }
		else if(inst == INS::CLOSED_OVER)
		{
			jassert(frame._code_obj.is<SavedClosure*>());
			int idx = frame.get_inst();
			frame.push(frame.closed_overs[idx]);
		}
		else if(inst == INS::MAKE_CLOSURE)
		{
			int argc = frame.get_inst();
			std::vector<TExpr> theVarOfClosureNeedSaved;

			for (int i = 0; i < argc; i++)
			{
				theVarOfClosureNeedSaved.push_back(frame.pop());
			}
			reverse(theVarOfClosureNeedSaved.begin(), theVarOfClosureNeedSaved.end());

			TExpr cobj = frame.pop();
			jassert(cobj.is<Code*>() );

			SavedClosure * sc = new SavedClosure(*cobj.get<Code*>(), theVarOfClosureNeedSaved);
			frame.push(sc);
		}
        else
        {
            char errorInfo[256];
           
            sprintf(errorInfo, "unknow instruction. ip: %d  inst: %d.", frame.ip, inst);
            
            throw std::runtime_error(errorInfo);
        }
        
	}


	return new TInt(-1);
}
