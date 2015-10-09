/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#include "Matrix.h"

//==============================================================================
MainContentComponent::MainContentComponent():
mRun{false},
mTopOp{nullptr},
maxChange{.01},
nelx{120},
nely{40},
volfrac{.5},
rmin{3.0},
penal{3.0}
{
    setSize (1000, 500);
	mLookAndFeel = std::make_shared<NTLookAndFeel>();
	setLookAndFeel(mLookAndFeel.get());
	addAndMakeVisible(mRunButton = new TextButton("Run Top Op"));
	mRunButton->addListener(this);

	addAndMakeVisible(mMaxChangeSlider = new Slider("maxChange"));
	mMaxChangeSlider->setTextBoxStyle(Slider::TextBoxLeft, false, 50, 20);
	mMaxChangeSlider->setRange(.001, 1, 0.001);
	mMaxChangeSlider->setSliderStyle(Slider::LinearHorizontal);
	mMaxChangeSlider->getValueObject().referTo(maxChange);

	addAndMakeVisible(mElXSlider = new Slider("nelx"));
	mElXSlider->setTextBoxStyle(Slider::TextBoxLeft, false, 50, 20);
	mElXSlider->setRange(10, 500, 1);
	mElXSlider->setSliderStyle(Slider::LinearHorizontal);
	mElXSlider->getValueObject().referTo(nelx);
	
	addAndMakeVisible(mElYSlider = new Slider("nely"));
	mElYSlider->setTextBoxStyle(Slider::TextBoxLeft, false, 50, 20);
	mElYSlider->setRange(10, 500, 1);
	mElYSlider->setSliderStyle(Slider::LinearHorizontal);
	mElYSlider->getValueObject().referTo(nely);
	
	addAndMakeVisible(mVolFracSlider = new Slider("volfrac"));
	mVolFracSlider->setTextBoxStyle(Slider::TextBoxLeft, false, 50, 20);
	mVolFracSlider->setRange(.1, 1, 0.01);
	mVolFracSlider->setSliderStyle(Slider::LinearHorizontal);
	mVolFracSlider->getValueObject().referTo(volfrac);

	addAndMakeVisible(mRMinSlider = new Slider("rmin"));
	mRMinSlider->setTextBoxStyle(Slider::TextBoxLeft, false, 50, 20);
	mRMinSlider->setRange(.1, 10, 0.01);
	mRMinSlider->setSliderStyle(Slider::LinearHorizontal);
	mRMinSlider->getValueObject().referTo(rmin);
	
	addAndMakeVisible(mPenalSlider = new Slider("penal"));
	mPenalSlider->setTextBoxStyle(Slider::TextBoxLeft, false, 50, 20);
	mPenalSlider->setRange(.1, 10, 0.01);
	mPenalSlider->setSliderStyle(Slider::LinearHorizontal);
	mPenalSlider->getValueObject().referTo(penal);


	mRunButton->setBounds(10, 10, 250, 24);
	mMaxChangeSlider->setBounds(10, mRunButton->getBottom() + 1, 250, 24);
	mElXSlider->setBounds(10, mMaxChangeSlider->getBottom() + 1, 250, 24);
	mElYSlider->setBounds(10, mElXSlider->getBottom() + 1, 250, 24);
	mVolFracSlider->setBounds(10, mElYSlider->getBottom() + 1, 250, 24);
	mRMinSlider->setBounds(10, mVolFracSlider->getBottom() + 1, 250, 24);
	mPenalSlider->setBounds(10, mRMinSlider->getBottom() + 1, 250, 24);

	mRunButton->setClickingTogglesState(true);
}

MainContentComponent::~MainContentComponent()
{
	if (isTimerRunning()){
		stopTimer();
	}
}

void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour (0xff001F36));

    g.setFont (Font (16.0f));
    g.setColour (Colours::white);

	auto rect = juce::Rectangle<float>(4, 4);
	int xMax = nelx.getValue();
	int yMax = nely.getValue();
	if (mTopOp != nullptr)
	{
		auto& x = mTopOp->getMatrix();
		for (int j = 0; j < x.getColumns(); j++){
		for (int i = 0; i < x.getRows(); i++){
			rect.setCentre(j *4 + 500, i * 4 + 200);
				auto val = (float)x.get(i, j);
				g.setColour(juce::Colour(val,val,val,1.0f));
				g.fillRect(rect);
			}
		}
		for (int j = 0; j < x.getColumns(); j++){
			for (int i = 0; i < x.getRows(); i++){
				rect.setCentre(j * -4 + 500, i * 4 + 200);
				auto val = (float)x.get(i, j);
				g.setColour(juce::Colour(val, val, val, 1.0f));
				g.fillRect(rect);
			}
		}
		g.setFont(12.0);
		g.setColour(juce::Colours::white);
		g.drawText("Compliance: " + juce::String(mCompliance, 3), 300, 10, 150, 24, Justification::left);
		g.drawText("Volume: " + juce::String(mVolume, 3), 460, 10, 150, 24, Justification::left);
		g.drawText("Change: " + juce::String(mChange, 3), 620, 10, 150, 24, Justification::left);

	}
}

void MainContentComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainContentComponent::hiResTimerCallback()
{
	if (mRun){
		if (mTopOp->loop(mCompliance, mVolume, mChange)){
			const MessageManagerLock lock(Thread::getCurrentThread());

			if (lock.lockWasGained()) {
				repaint();
			}
		}
		else{
			mRun = false;
		}
	}
	else{
		stopTimer();
	}

}

void MainContentComponent::buttonClicked(Button* button)
{
	if (button == mRunButton){
		if (mRun == false){
			tfloat maxChangeGo = maxChange.getValue();
			int nelxGo = nelx.getValue();
			int nelyGo = nely.getValue();
			tfloat volfracGo = volfrac.getValue();
			tfloat rminGo = rmin.getValue();
			tfloat penalGo = penal.getValue();

			mTopOp = std::make_shared<TopOpt>(nelxGo, nelyGo, volfracGo, penalGo, rminGo, maxChangeGo);
			mRun = true;
			startTimer(18);
		}
		else{
			mRun = false;
		}
	}
}
