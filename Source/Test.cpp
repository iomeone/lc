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

bool testOnce(String sourceCode, int expectedVal)
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
		if (e.is<TInt*>())
		{
			if (e.get<TInt*>()->_val == expectedVal)
			{
				return true;
			}
		}
	}
	catch (std::runtime_error& e)
	{
		msg(e.what());
		return false;
	}
	return false;
}

std::vector<std::pair<String, int>> source = {
	{"(platform+ 3 7)", 10}


};


void test()
{
	for_each(source.begin(), source.end(), [](std::pair<String, int> p)
	{
			if (!testOnce(p.first, p.second))
				msg("fail to test : " + p.first);
	});
}