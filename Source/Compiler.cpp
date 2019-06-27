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



std::function<void(TObj&, Context&, CompileInfo&)> compile_platform_plus = [](TObj& obj, Context& ctx, CompileInfo& compileInfo) {

	jassert(obj.is<TCons*>());
	TObj& nxt = obj.get<TCons*>()->_tail;   // obj->head is platform+ ,we already know that, so we skip it.

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

int coutOfCons(const TObj & s)
{
	int count = 0;
	TObj t = s;

	while (!t.is<TNil*>())
	{
		++count;
		jassert(t.is<TCons*>());  // arg list must be list of TCons type.
		t = t.get<TCons*>()->_tail;
	}
	return count;
}

void add_args(const TObj & t, Context& ctx, int& count)
{

	TObj args = t;
	int i = 0;
	while (!args.is<TNil*>())
	{
		jassert(args.is<TCons*>());
		TObj & a = args.get<TCons*>()->_head;

		jassert(a.is<TSymbol*>());

		ctx.add_local(a.get<TSymbol*>()->_sym, Arg(i + 1));
		args = args.get<TCons*>()->_tail;
		++i;
	}


}

std::function<void(TObj&, Context&, CompileInfo&)> compile_fn = [](TObj&obj, Context&ctx, CompileInfo& compileInfo) {

	jassert(obj.is<TCons*>());             //  obj must be a list, i.e. an Cons struct

	TObj form = obj.get<TCons*>()->_tail;  // obj->head is an fun string,  just an identify, we skip it.

	jassert(form.is<TCons*>());             // form must also be an cons.  eg.  fun (x y) (+ x y),  nxt now represent (x y) (+ x y) . cert


	bool hasName = false;
	TObj name;
	//TObj argsCons;

	if (form.get<TCons*>()->_head.is<TSymbol*>())             // the function has a name.  eg.  fun myadd(x y) (+ x y)
	{
		//hasName = true;

		name = form.get<TCons*>()->_head;	 // nameObj.get<TSymbol*>()->_sym;

		form = form.get<TCons*>()->_tail;
	}

	jassert(form.is<TCons*>());
	TObj& args = form.get<TCons*>()->_head;
	jassert(args.is<TCons*>() || args.is<TNil*>());

	TObj& body = form.get<TCons*>()->_tail;

	int numOfArgs = coutOfCons(args);
	Context new_ctx(numOfArgs);
	add_args(args, new_ctx, numOfArgs);

	if (hasName)
	{
		jassert(name.is<TSymbol*>());
		new_ctx.add_local(name.get<TSymbol*>()->_sym, Arg(0));
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
	ctx.push_const( TObj( new_ctx.to_code()) );

	

};




std::function<void(TObj&, Context&, CompileInfo& compileInfo)> builtins(String& key)
{
	if (key == "platform+")
		return compile_platform_plus;
	else if (key == "fn")
		return compile_fn;

	return nullptr;
}



void compile_(TObj&  obj, Context & ctx, CompileInfo& compileInfo)
{
	obj.match(
		
		
	[&compileInfo](TNil* a)
	{
		//compileInfo.log += "   Nil";
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
		std::map<String, Arg>::iterator iter = ctx.locals.back().find(e->_sym);

		if (iter == ctx.locals.back().end())
		{
			sprintf(errorInfo, "undefined symbol %s, line: %ld coloum: %ld", e->_sym, compileInfo._line, compileInfo._col); // TSymbol should contain col and line infomation
			throw std::runtime_error(errorInfo);
		}
		else
		{		
			compileInfo.log += "\npush symbol:" + String(e->_sym) + String(" index:") + String::toHexString(iter->second.idx);
			iter->second.emit(ctx);
		
		}

		//sprintf(errorInfo, "can not compile. line: %ld coloum: %ld", compileInfo._line, compileInfo._col); // TSymbol should contain col and line infomation
		//throw std::runtime_error(errorInfo);


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
		TObj form = e;

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


	);
}



Code compile(TObj& obj, CompileInfo& compileInfo)
{
	Context ctx{ 0 };
	auto s = String("");
	compile_(obj, ctx, compileInfo);

	compileInfo.log += "\nRETURN";
	ctx.bytecode.push_back(INS::RETURN);
	Code *c = ctx.to_code();
	return *c;


}

Code::Code(std::vector<uint32> bytecode , std::vector<TObj> consts)
{
	_bytecode = bytecode;
	_consts = consts;
}

void Arg::emit(Context & ctx)
{
	ctx.push_arg(idx);
}
