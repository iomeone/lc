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

int coutOfCons(TObj & s)
{
	int count = 0;
	TObj& t = s;

	while (!t.is<TNil*>())
	{
		++count;
		jassert(t.is<TCons*>());  // arg list must be list of TCons type.
		t = t.get<TCons*>()->_tail;
	}
	return count;
}

std::function<void(TObj&, Context&, CompileInfo&)> compile_fn = [](TObj&obj, Context&ctx, CompileInfo& compileInfo) {

	jassert(obj.is<TCons*>());             //  obj must be a list, i.e. an Cons struct

	TObj& nxt = obj.get<TCons*>()->_tail;  // obj->head is an fun string,  just an identify, we skip it.

	jassert(nxt.is<TCons*>());             // nxt must also be an cons.  eg.  fun (x y) (+ x y),  nxt now represent (x y) (+ x y) . cert

	TObj& nameObj = nxt.get<TCons*>()->_head;
	String name ;
	if (nameObj.is<TSymbol*>())             // the function has a name.  eg.  fun myadd(x y) (+ x y)
	{
		name = nameObj.get<TSymbol*>()->_sym;
	}
	else
	{
		name = "";                         // the function do not have a name.
		TObj args = nameObj;			   // the nameObj actually is args list;

		jassert(args.is<TCons*>() || args.is<TNil*>());   // arg list must be an none empty list (i.e. TCons object)  or empty list (i.e. TNil object)

		TObj body = nxt.get<TCons*>()->_tail;

		Context new_ctx(coutOfCons(args));

	}










};




std::function<void(TObj&, Context&, CompileInfo& compileInfo)> builtins(String& key)
{
	if (key == "platform+")
		return compile_platform_plus;


	return nullptr;
}



void compile_(TObj&  obj, Context & ctx, CompileInfo& compileInfo)
{
	obj.match(
		
		
	[&compileInfo](TNil* a)
	{
		//compileInfo.log += "   Nil";
	}
	
		
	, [&compileInfo, &ctx, &obj](TInt* e)
	{
		compileInfo.log +=  "\nLOAD_CONST index:" + String(ctx.consts.size())+ String(" val:") + String(e->_val);

		ctx.push_const(obj);
	}
	
		
	, [&compileInfo](TSymbol* e)
	{
		//compileInfo.log += String(e->_sym);
	}
	
		
	, [&compileInfo, &ctx, &obj](TCons* e)
	{
		// if is builtins function.
		if ( e->_head.is<TSymbol*>())   // extreamly cool !!!!!!!!!!
		{
			String& sym = e->_head.get<TSymbol*>()->_sym;
			auto fn = builtins(sym);
			if (fn)
			{
				fn(obj, ctx, compileInfo);
				return;
			}
				
		}

		// not build in function.



		//if (indent == 0)
		//	str += "(";
		//else
		//	str += "\n" + getSpace(indent) + "(";


		//++indent;
		//compile_(e->_head, ctx, str, indent);
		//compile_(e->_tail, ctx, str, indent);

		//str += ")";
	}


	);
}



Code compile(TObj& obj, CompileInfo& compileInfo)
{
	Context ctx{ 0 };
	auto s = String("");
	compile_(obj, ctx, compileInfo);

	compileInfo.log += "\nreturn";
	ctx.bytecode.push_back(INS::RETURN);
	Code c = ctx.to_code();
	return c;


}

Code::Code(Context * ctx)
{
	_bytecode = ctx->bytecode;
	_consts = ctx->consts;
}





