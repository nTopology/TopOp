/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "NTLookAndFeel.h"
#include "TopOpt.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class TopOpComp;
class MainContentComponent   : 
	public Component,
	public HighResolutionTimer,
	public Button::Listener
{
public:
    //==============================================================================
    MainContentComponent();
    ~MainContentComponent();

    void paint (Graphics&);
	void buttonClicked(Button* button)override;

    void resized();
	void hiResTimerCallback() override;
private:
	std::shared_ptr<NTLookAndFeel> mLookAndFeel;
	bool mRun;
	float mCompliance, mVolume, mChange;
	ScopedPointer<TextButton> mRunButton;
	ScopedPointer<Slider> mMaxChangeSlider;
	ScopedPointer<Slider> mElXSlider;
	ScopedPointer<Slider> mElYSlider;
	ScopedPointer<Slider> mVolFracSlider;
	ScopedPointer<Slider> mRMinSlider;
	ScopedPointer<Slider> mPenalSlider;

private:
	Value maxChange;// = 0.01;
	Value nelx;// = 120;
	Value nely;// = 40;
	Value volfrac;// = 0.5;
	Value rmin;// = 3.0;
	Value penal;// = 3.0;


	std::shared_ptr<TopOpt> mTopOp;
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
