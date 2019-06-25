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



//def compile_platform_eq(form, ctx):
//    form = form.next()
//    assert count(form) == 2
//    while form is not nil:
//        compile_form(form.first(), ctx)
//        form = form.next()
//
//    ctx.bytecode.append(code.EQ)
//    ctx.sp -= 1
//    return ctx



std::function<TObj()> getHander(String& c)
{
	if (c == "platform+")
	{
		//std::function<TObj()> h = [this]()
		//{
		//	return this->ListReader();
		//};
		//return h;
		return nullptr;
	}
	else if (c == "fn")
	{
		//std::function<TObj()> h = [this]()
		//{
		//	char errorInfo[256];
		//	sprintf(errorInfo, "unexpected ')', line: %ld coloum: %ld", _line, _col);
		//	throw std::runtime_error(errorInfo);
		//	return (new TNil);
		//};
		//return h;
		return nullptr;
	}
	else if (c == "if")
	{
		return nullptr;
	}
	else if (c == "platform=")
	{
		return nullptr;
	}
	else
		return nullptr;
}



String getSpace(int n)
{
	return String::repeatedString("   ", n);
}


//def compile_platform_plus(form, ctx) :
//	form = form.next()
//	while form is not nil :
//		compile_form(form.first(), ctx)
//		form = form.next()
//
//		ctx.bytecode.append(code.ADD)
//		ctx.sp -= 1
//		return ctx




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





