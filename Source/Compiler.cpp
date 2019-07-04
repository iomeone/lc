/*
  ==============================================================================

    Compiler.cpp
    Created: 25 Jun 2019 12:02:58pm
    Author:  user

  ==============================================================================
*/

#include "Compiler.h"


//builtins = { "platform+": compile_platform_plus,
//			"fn" : compile_fn,
//			"if" : compile_if,
//			"platform=" : compile_platform_eq }


String getSpace(int n)
{
	return String::repeatedString("   ", n);
}

std::function<void(TExpr&, Context&, CompileInfo&)> compile_platform_eq = [](TExpr& obj, Context& ctx, CompileInfo& compileInfo) {
    
    jassert(obj.is<TCons*>());
    TExpr& nxt = obj.get<TCons*>()->_tail;   // obj->head is platform+ ,we already know that, so we skip it.
    
    while (!nxt.is<TNil*>())
    {
        jassert(nxt.is<TCons*>());            // if nxt is not nil, it must be an another cons expression.
        
        compile_(nxt.get<TCons*>()->_head, ctx, compileInfo);
        nxt = nxt.get<TCons*>()->_tail;
    }
    
    compileInfo.log += "\nEQ";
    ctx.bytecode.push_back(INS::EQ);
    
    ctx.sp -= 1;   //  pop up two integer, and push the result ,  so overall , the stack need substruct 1.
                   //  question : it seems we need track the number of add argument, the code should be sp -= coutOfArg -1;
};




std::function<void(TExpr&, Context&, CompileInfo&)> compile_platform_plus = [](TExpr& obj, Context& ctx, CompileInfo& compileInfo) {

	jassert(obj.is<TCons*>());
	TExpr& nxt = obj.get<TCons*>()->_tail;   // obj->head is platform+ ,we already know that, so we skip it.

	while (!nxt.is<TNil*>())
	{
		jassert(nxt.is<TCons*>());			// if nxt is not nil, it must be an another cons expression.

		compile_(nxt.get<TCons*>()->_head, ctx, compileInfo);
		nxt = nxt.get<TCons*>()->_tail;
	}

	compileInfo.log += "\nADD";
	ctx.bytecode.push_back(INS::ADD);

	ctx.sp -= 1;   //  pop up two integer, and push the result ,  so overall , the stack need substruct 1.    
                   //  question : it seems we need track the number of add argument, the code should be sp -= coutOfArg -1;
};

int coutOfCons(const TExpr & s)
{
	int count = 0;
	TExpr t = s;

	while (!t.is<TNil*>())
	{
		++count;
		jassert(t.is<TCons*>());  // arg list must be list of TCons type.
		t = t.get<TCons*>()->_tail;
	}
	return count;
}

void add_args(const TExpr & t, Context& ctx, int& count)
{
	TExpr args = t;
	int i = 0;
	while (!args.is<TNil*>())
	{
		jassert(args.is<TCons*>());
		TExpr & a = args.get<TCons*>()->_head;

		jassert(a.is<TSymbol*>());

		ctx.add_local(a.get<TSymbol*>()->_sym, Arg(i + 1, Uuid()));
		args = args.get<TCons*>()->_tail;
		++i;
	}


}

std::function<void(TExpr&, Context&, CompileInfo&)> compile_fn = [](TExpr&obj, Context&ctx, CompileInfo& compileInfo) {

	jassert(obj.is<TCons*>());             //  obj must be a list, i.e. an Cons struct

	TExpr form = obj.get<TCons*>()->_tail;  // obj->head is an fun string,  just an identify, we skip it.

	jassert(form.is<TCons*>());             // form must also be an cons.  eg.  fun (x y) (+ x y),  nxt now represent (x y) (+ x y) . cert


	bool hasName = false;
	TExpr name;
	//TExpr argsCons;

	if (form.get<TCons*>()->_head.is<TSymbol*>())             // the function has a name.  eg.  fun myadd(x y) (+ x y)
	{
		hasName = true;

		name = form.get<TCons*>()->_head;	 // nameObj.get<TSymbol*>()->_sym;

		form = form.get<TCons*>()->_tail;
	}
	
	jassert(form.is<TCons*>());
	TExpr& args = form.get<TCons*>()->_head;
	jassert(args.is<TCons*>() || args.is<TNil*>());

	TExpr& body = form.get<TCons*>()->_tail;

	int numOfArgs = coutOfCons(args);
	Context new_ctx(numOfArgs, ctx.locals);
	add_args(args, new_ctx, numOfArgs);

	if (hasName)
	{
		jassert(name.is<TSymbol*>());
		new_ctx.add_local(name.get<TSymbol*>()->_sym, Arg(0, Uuid()));
	}

	new_ctx.can_tail_call = true;

	jassert(body.is<TCons*>() || body.is<TNil*>());

	while (!body.is<TNil*>())
	{
		compile_(body.get<TCons*>()->_head, new_ctx, compileInfo);
		body = body.get<TCons*>()->_tail;

		jassert(body.is<TCons*>() || body.is<TNil*>());
	}

	new_ctx.bytecode.push_back(INS::RETURN);
	compileInfo.log += "\nRet";

	compileInfo.log += "\nLOAD_CONST index:" + String(new_ctx.consts.size());

	std::vector<ArgOrClosure> closed_overs = new_ctx.closed_overs;

	ctx.push_const(TExpr(new_ctx.to_code()));
	if (closed_overs.size() == 0)
	{
		
	}
	else
	{
		for_each(closed_overs.begin(), closed_overs.end(), [&ctx](ArgOrClosure arg)  
		{
			if (arg.is<Arg>())
			{
				ArgEmit(ctx, arg.get<Arg>().idx);
			}
			else if (arg.is<Closure>())
			{
				jassert(false);   // the item which the closed_overs saved should be type of Arg???  if in nested closure, this could be Closure, so we just need call ClosureEmit ?
			}
			else
			{
				jassert(false);  // shouldn't happen.
			}
			
		});

		ctx.bytecode.push_back(INS::MAKE_CLOSURE);
		ctx.bytecode.push_back(closed_overs.size());
		ctx.sp -= closed_overs.size();
	}
};


std::function<void(const TExpr&, Context&, CompileInfo&)> compile_if = [](const TExpr&obj, Context&ctx, CompileInfo& compileInfo) {
    jassert(obj.is<TCons*>());
    
    TExpr form = obj;
    
    form = form.get<TCons*>()->_tail;
    jassert(coutOfCons(form) == 3);
    jassert(form.is<TCons*>());  // should be TCons type  `(test_expr  (true_expr (false_expr nil)))
    TExpr testExpr = form.get<TCons*>()->_head;
    
    form = form.get<TCons*>()->_tail;
    jassert(form.is<TCons*>());
    TExpr thenExpr = form.get<TCons*>()->_head;
    
    form = form.get<TCons*>()->_tail;
     jassert(form.is<TCons*>());
    TExpr elseExpr = form.get<TCons*>()->_head;
    
    ctx.can_tail_call = false;
    compile_(testExpr, ctx, compileInfo);
    
    ctx.bytecode.push_back(INS::COND_BR);
    ctx.sp -= 1;   // bcz when interpreter COND_BR , we pop the test value and see if it is ture or false.  so the stack must sub 1
    
    uint32 cond_lbl = ctx.label();
    
    
    ctx.can_tail_call = true;
    
    compile_(thenExpr, ctx, compileInfo);
    ctx.bytecode.push_back(INS::JMP);
    ctx.sp -= 1;
    uint32 else_lbl = ctx.label();
    
    ctx.mark(cond_lbl);
    compile_(elseExpr, ctx, compileInfo);
    ctx.mark(else_lbl);
};


std::function<void(TExpr&, Context&, CompileInfo& compileInfo)> builtins(String& key)
{
	if (key == "platform+")
		return compile_platform_plus;
	else if (key == "fn")
		return compile_fn;
    else if (key == "if")
        return compile_if;
    else if (key == "platform=")
        return compile_platform_eq;

	return nullptr;
}



void ArgEmit(Context& ctx, int idx)
{
	ctx.push_arg(idx);
}

void ClosureEmit(Context& ctx, int idx)
{
	ctx.bytecode.push_back(INS::CLOSED_OVER);
	ctx.bytecode.push_back(idx);
	ctx.sp += 1;
}


void compile_(TExpr&  obj, Context & ctx, CompileInfo& compileInfo)
{
	obj.match(
	[&compileInfo](TNil* a)
	{
		//compileInfo.log += "   Nil";
	}
	,[&compileInfo](SavedClosure* e)
	{
		jassertfalse;
	}
		
	, [&compileInfo, &ctx](TInt* e)
	{
		compileInfo.log +=  "\nLOAD_CONST index:" + String(ctx.consts.size())+ String(" val:") + String(e->_val);

		ctx.push_const(e);
	}
	
		
	, [&compileInfo, &ctx](TSymbol* e )
	{
		//compileInfo.log += String(e->_sym);
		char errorInfo[256];
		bool bfind = false;
		ArgOrClosure arg = Arg(-1, Uuid());
		bfind = ctx.get_local(e->_sym, arg);
		if (bfind)
		{
			if (arg.is<Arg>())
			{
				jassert(arg.get<Arg>().idx >= 0);
				compileInfo.log += "\npush symbol:" + String(e->_sym) + String(" index:") + String::toHexString(arg.get<Arg>().idx);

				ArgEmit(ctx, arg.get<Arg>().idx);
				//ctx.push_arg(arg.get<Arg>().idx);
			}
			else if (arg.is<Closure>())
			{
				jassert(false);
				msg("arg.is<Closure>()???");
				
			}
			else if (arg.is<ClosureCell>())
			{

				jassert(arg.get<ClosureCell>()._idx >= 0);
				compileInfo.log += "\npush symbol:" + String(e->_sym) + String(" index:") + String::toHexString(arg.get<ClosureCell>()._idx);
				ClosureEmit(ctx, arg.get<ClosureCell>()._idx);

			}
			else
			{
				msg("???");
			}

		}
		else
		{
			sprintf(errorInfo, "undefined symbol %s, line: %ld coloum: %ld", e->_sym.toRawUTF8(), compileInfo._line, compileInfo._col); // TSymbol should contain col and line infomation
			throw std::runtime_error(errorInfo);
		}
	
	}
    
	
    , [&compileInfo](Code* e)
    {
		compileInfo.log += "\nCode???";
    }
              
              
    , [&compileInfo, &ctx, &obj](TCons* e)
	{
		// if is builtins function.
		if (e->_head.is<TSymbol*>())   // extreamly cool !!!!!!!!!!
		{
			String& sym = e->_head.get<TSymbol*>()->_sym;
			auto fn = builtins(sym);
			if (fn)
			{
				fn(obj, ctx, compileInfo);
				return;
			}

		}
		int cnt = 0;
		TExpr form = e;

		while (form.is<TCons*>())
		{
			compile_(form.get<TCons*>()->_head, ctx, compileInfo);
			form = form.get<TCons*>()->_tail;
			++cnt;
		}
		jassert(form.is<TNil*>());

		if (ctx.can_tail_call)
		{
			ctx.bytecode.push_back(INS::TAIL_CALL);
			compileInfo.log += "\nTAIL_CALL	";
		}

		else
		{
			ctx.bytecode.push_back(INS::INVOKE);
			compileInfo.log += "\nINVOKE	";
		}

		ctx.bytecode.push_back(cnt);
		compileInfo.log += String::toHexString(cnt);
	}
              
              
    , [&compileInfo, &ctx](TBool* e)
    {
		ctx.push_const(e);
		compileInfo.log += "\nLOAD_CONST_BOOL index:" + String(ctx.consts.size()) + String(" val:") + String(e->_b ? "True" : "False");
    }
	);
}



Code compile(TExpr& obj, CompileInfo& compileInfo)
{
	std::vector<Context::Symbol_ArgOrClosure> l;
	Context ctx(0, l);
	auto s = String("");
	compile_(obj, ctx, compileInfo);

	compileInfo.log += "\nRETURN";
	ctx.bytecode.push_back(INS::RETURN);
	Code *c = ctx.to_code();
	return *c;


}

Code::Code(std::vector<uint32> bytecode , std::vector<TExpr> consts)
{
	_bytecode = bytecode;
	_consts = consts;
}

