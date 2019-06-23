/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public Component
                        ,public TextEditor::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    virtual void textEditorTextChanged (TextEditor&) override;

    std::unique_ptr<Component>  _logWin;
    
    std::unique_ptr<Component>  _EditorWin;
private:
    //==============================================================================
    // Your private member variables go here...
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
