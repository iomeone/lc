/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include "LogComponent.h"
#include "Parser.h"
#include "Compiler.h"
//==============================================================================
MainComponent::MainComponent()
{
    _logWin.reset (new LogComponent());
    addAndMakeVisible(_logWin.get());
    
    _EditorWin.reset (new LogComponent());
    addAndMakeVisible(_EditorWin.get());
    
    auto e = dynamic_cast<LogComponent*>(_EditorWin.get());
    
    if(e)
    {
        e->getEditor()->addListener(this);
		e->getEditor()->setText("(platform+ 6 7)");
    }
     setSize (1200, 800);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setFont (Font (16.0f));
    g.setColour (Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), Justification::centred, true);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    Rectangle<int> r = getLocalBounds();
    

    _logWin->setBounds(r.removeFromBottom(roundToInt(proportionOfWidth(0.2000f))));
    
    _EditorWin->setBounds(r.removeFromTop(roundToInt(proportionOfWidth(0.8000f))));
    
   
}

void MainComponent::textEditorTextChanged(juce::TextEditor & e) {

	auto edtLog = dynamic_cast<LogComponent*>(_logWin.get());
	try
	{
		String src = e.getText();
		Lan lan(src);
		TObj o = lan.compile();

		String s;
		lan.getASTStr(o, s);

		CompileInfo compileInfo;
		Code c =  compile(o, compileInfo);

		juce::String strByteCode, strConsts;
		for_each(c._bytecode.begin(), c._bytecode.end(), [&strByteCode](uint32 bytecode){strByteCode += String::toHexString(bytecode) + " ";});
		
        
//        for_each(c._consts.begin(), c._consts.end(), [&strConsts](uint32 consts){strConsts += String::toHexString(consts) + " ";});
 
		edtLog->getEditor()->setText(s + "\n" + strByteCode + "\n" + strConsts + "\n" + compileInfo.log);
	}
	catch (std::runtime_error& e)
	{
		if (edtLog)
			edtLog->getEditor()->setText(e.what());
	}
}

