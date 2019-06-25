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




void compile_(TObj&  obj, Context & ctx, String& str, int indent = 0)
{
	obj.match([&str, &indent](TNil* a)
	{
		str += "   Nil";
	}
	, [&str, &indent](TInt* e)
	{
		str += String(e->_val) + ":Int";
	}
		, [&str, indent](TSymbol* e)
	{
		str += String(e->_sym);
	}
		, [&str, &indent, &ctx](TCons* e)
	{
		if (indent == 0)
			str += "(";
		else
			str += "\n" + getSpace(indent) + "(";

		++indent;
		compile_(e->_head, ctx, str, indent);
		compile_(e->_tail, ctx, str, indent);

		str += ")";
	}

	);
}



Code compile(TObj& obj)
{
	Context ctx{ 0 };
	compile_(obj, ctx, String(""));

	ctx.bytecode.push_back(INS::RETURN);
	Code c = ctx.to_code();
	return c;


}

Code::Code(Context * ctx)
{
	_bytecode = ctx->bytecode;
	_consts = ctx->consts;
}