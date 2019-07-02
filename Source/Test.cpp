/*
  ==============================================================================

    Test.cpp
    Created: 1 Jul 2019 6:46:16pm
    Author:  user

  ==============================================================================
*/

#include "Test.h"

#include "../JuceLibraryCode/JuceHeader.h"
#include "BytecodeInterpreter.h"
#include "Parser.h"
#include "Compiler.h"
#include <vector>

bool testOnce(String sourceCode, TExpr expectedVal)
{
	try
	{
		
		Lan lan(sourceCode);
		TExpr o = lan.compile();
		
	/*	String s;
		lan.getASTStr(o, s);*/

		CompileInfo compileInfo;
		Code c = compile(o, compileInfo);

		TExpr e = interpret(c);
		if (e.is<TInt*>() && expectedVal.is<TInt*>())
		{
			int x = e.get<TInt*>()->_val;
			int expected = expectedVal.get<TInt*>()->_val;
			if (x == expected)
			{
				return true;
			}
		}
		else if (e.is<TBool*>() && expectedVal.is<TBool*>())
		{
			bool b = e.get<TBool*>()->_b;
			bool expected = expectedVal.get<TBool*>()->_b;
			if (b == expected)
			{
				return true;
			}
			else
				return false;
		}
	}
	catch (std::runtime_error& e)
	{
		msg(e.what());
		return false;
	}
	return false;
}

std::vector<std::pair<String, TExpr>> source = {
	{"((fn rf (x) (if (platform= x 10)  x (rf (platform+ x 1)))) 0)", new TInt(10)}
	,{"(platform= 1 1)", new TBool(true)}
	,{"(platform= 1 2)",new TBool(false)}
	,{"(if 1 2 3)", new TInt(2)}
	,{"(if false 2 3)" ,new TInt(3)}
	,{"(platform+ 3 7)", new TInt(10)}
	,{"((fn (x y) (platform+ x y)) 1 2)",new TInt(3)}
	,{"(if (platform= 1 2) true false)",new TBool(false)}
	
};


void test()
{
	for_each(source.begin(), source.end(), [](std::pair<String, TExpr> p)
	{
			if (!testOnce(p.first, p.second))
				msg("fail to test : " + p.first);
	});
}