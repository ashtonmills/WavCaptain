

/*
  ==============================================================================

	PositionOverlay.h
	Created: 5 Feb 2020 4:14:18pm
	Author:  ashton_mills

  ==============================================================================
*/



//Component that handles interfacing with the waveform
#pragma once
#include "MainComponent.h"


//Component for selection regions of the waveform

class SelectionRegion : public Component
{
public:
	SelectionRegion()
	{
		this->setAlpha(0.25);
	}
	~SelectionRegion()
	{

	}
	void paint(Graphics& g) override
	{
		g.setColour(Colours::teal);
		g.fillAll();
	}

};


class FadeoutLookAndFeel : public LookAndFeel_V4
{
public:
	FadeoutLookAndFeel()
	{
	}
	Font getTextButtonFont(TextButton&, int buttonHeight) override
	{
		return Font("Segoe UI Symbol", 25, Font::bold); 
	}
	Font getLabelFont(Label&) 
	{
		return Font("Segoe UI Symbol", 25, Font::bold);
	}
};

//==============================================================================

class FadeoutWidget : public Component
{
public:
	FadeoutWidget()
	{
		addAndMakeVisible(symbolLabel);
		symbolLabel.setName("fadeOut Widget");
		symbolLabel.setText(symbol, dontSendNotification);
		symbolLabel.setLookAndFeel(&unicodeLF);
		//symbolLabel.setAlpha(0.5);
	}
	~FadeoutWidget()
	{
		symbolLabel.setLookAndFeel(nullptr);
	}
	void resized()
	{
		symbolLabel.setBounds(getLocalBounds());
	}


private:
	Label symbolLabel;
	FadeoutLookAndFeel unicodeLF;
	String symbol  = CharPointer_UTF8("\xe2\x97\xba");
};

//==============================================================================

class PositionOverlay : public Component, public Timer, public ComponentDragger, public ValueTree::Listener
{
public:
	PositionOverlay(AudioTransportSource& transportSourcetoUse, ValueTree vt)
		: transportSource(transportSourcetoUse), mainVT(vt)
	{
		startTimer(1);
		addAndMakeVisible(fadeoutWidget);
		fadeoutWidget.addMouseListener(this,true);
		mainVT.addListener(this);
	}

	void paint(Graphics& g) override
	{
		auto duration = (float)transportSource.getLengthInSeconds();

		if (duration > 0.0)
		{
			float audioPosition = (float)transportSource.getCurrentPosition();  //why here do he have (float) in brackets? 
			//update the valuetree with audioposition.
			//This is not really a good place to do this, inside a paint function, TODO refector to make it tidier. 

			auto transportTime = floatToTimecode(audioPosition);
			mainVT.setProperty(ValTreeIDs::timeCode, transportTime, nullptr);

			drawPosition = (audioPosition / duration) * getWidth();
			g.setColour(Colours::black);
			g.drawLine(drawPosition, 0.0f, (float)drawPosition, getHeight(), 1.0);
		}
	}
	String floatToTimecode(float rawTime)
	{
		int rawMilSecs, rawSecs, rawMins;
		String milSecs, secs, mins;
		float fRawSecs;
		float fRawMilSec = std::modf(rawTime, &fRawSecs);
		rawMilSecs = fRawMilSec * 100;
		rawMilSecs %= 100;
		milSecs = zeroFormat(rawMilSecs);
		rawSecs = fRawSecs;
		rawSecs %= 60;
		secs = zeroFormat(rawSecs);
		rawMins = rawTime / 60;
		mins = zeroFormat(rawMins);

		String timeCode = mins + ":" + secs + ":" + milSecs;
		return timeCode;
	}
	//helper function to return an int as string with a zero at the front if it's less than 10, or without if not
	String zeroFormat(int input)
	{
		String formattedNum = (input < 10) ? ("0" + std::to_string(input)) : std::to_string(input);
		return formattedNum;
	}

	void mouseEnter(const MouseEvent& event) override
	{
		MouseCursor mc(MouseCursor::StandardCursorType::IBeamCursor);
		this->setMouseCursor(mc);
	}
	void mouseExit(const MouseEvent& event) override
	{
		MouseCursor mc(MouseCursor::StandardCursorType::NormalCursor);
		this->setMouseCursor(mc);
	}

	void mouseUp(const MouseEvent& event) override
	{
		if (event.originalComponent != &fadeoutWidget)
		{
			auto duration = transportSource.getLengthInSeconds();

			if (duration > 0.0f)
			{
				auto clickPosition = event.position.x;
				auto audioPosition = (clickPosition / getWidth()) * duration;

				transportSource.setPosition(audioPosition);
			}
		}
	}
	void mouseDown(const MouseEvent& event)
	{
		if (event.originalComponent->getName() == "fadeOut Widget")
		{
			startDraggingComponent(&fadeoutWidget, event);
		}
	}

	void mouseDrag(const MouseEvent& event) override
	{
	
		if (event.originalComponent->getName() == "fadeOut Widget")
		{
			//int newX = event.getPosition().getX();
			//fadeoutWidget.setBounds(newX, 0, 30, 15);
			//auto bc = std::make_unique<ComponentBoundsConstrainer>();
			//bc->setMinimumOnscreenAmounts(10, 10, 30, 30);
			dragComponent(&fadeoutWidget, event,nullptr);
			//if (fadeoutWidget.getBounds().getY() > -3 || fadeoutWidget.getBounds().getY() < -3)
			//{
			//	fadeoutWidget.setBounds(fadeoutWidget.getBounds().getX(), -3, 30, 15);
			//}
			//if (fadeoutWidget.getBounds().getX() < 0)
			//{
			//	fadeoutWidget.setBounds(0, -3, 30, 15);
			//}
			//if (fadeoutWidget.getBounds().getX() > getParentWidth()-20)
			//{
			//	fadeoutWidget.setBounds(getParentWidth()-20, -3, 30, 15);
			//}
			mainVT.setProperty(ValTreeIDs::fadeoutXPosition, event.getPosition().getX(), nullptr);
			mainVT.setProperty(ValTreeIDs::fadeoutYPosition, event.getPosition().getY(), nullptr);
		}
		//else
		//{
		////	this->removeAllChildren();
		//	if (this->isEnabled())
		//	{
		//		addAndMakeVisible(selectionRegion);
		//		auto selectionStartPosition = event.mouseDownPosition.x;
		//		if (event.getOffsetFromDragStart().x > 0)
		//		{
		//			selectionRegion.setBounds(selectionStartPosition, 0, event.getOffsetFromDragStart().x, getHeight());
		//		}
		//		if (event.getOffsetFromDragStart().x < 0)
		//		{
		//			selectionRegion.setBounds(event.getPosition().x, 0, event.getOffsetFromDragStart().x * -1, getHeight());
		//		}
		//	}
		//}
	}
	void valueTreePropertyChanged(ValueTree& tree, const Identifier& property)
	{
		if (property == ValTreeIDs::fadeoutXPosition)
		{
			int x = mainVT.getProperty(ValTreeIDs::fadeoutXPosition);
			if (x < 0)
			{
				DBG("x = " + std::to_string(x));
				fadeoutWidget.setBounds(0, -3, 30, 15);
				mainVT.setProperty(ValTreeIDs::fadeoutXPosition,0 , nullptr);
				mainVT.setProperty(ValTreeIDs::fadeoutYPosition, -3, nullptr);
			}
			if ( x > getWidth() - 20)
			{
				DBG("x = " + std::to_string(x));
				fadeoutWidget.setBounds(getWidth() - 20, -3, 30, 15);
				mainVT.setProperty(ValTreeIDs::fadeoutXPosition, getParentWidth() - 20, nullptr);
				mainVT.setProperty(ValTreeIDs::fadeoutYPosition, -3, nullptr);
			}

		}
		if (property == ValTreeIDs::fadeoutYPosition)
		{
	
			int y = mainVT.getProperty(ValTreeIDs::fadeoutYPosition);
			DBG("y = " + std::to_string(y));
			if (y > -3 || y < -3)
			{
				fadeoutWidget.setBounds(fadeoutWidget.getBounds().getX(), -3, 30, 15);
				mainVT.setProperty(ValTreeIDs::fadeoutYPosition, -3, nullptr);
				mainVT.setProperty(ValTreeIDs::fadeoutXPosition, fadeoutWidget.getBounds().getX(), nullptr);
			}
		}
	}

	bool getLooping()
	{
		return shouldLoop;
	}
	void setLooping(bool shallItLoop)
	{
		shouldLoop = shallItLoop;
	}
	void resized()
	{
		fadeoutWidget.setBounds(getWidth() - 20, -3, 30, 15);
	}


private:
	//Looping is very primitive at the moment.
	//TODO improve the looping and region selection with float precision. 
	void timerCallback()override
	{
		repaint();
		//do looping if you made a region
		if (selectionRegion.isVisible() && drawPosition >= selectionRegion.getRight() && shouldLoop == true)
		{
			DBG("should Loop");
			transportSource.setPosition((selectionRegion.getX() * transportSource.getLengthInSeconds()) / getWidth());
		}

	}
	AudioTransportSource& transportSource;
	ValueTree mainVT;
	SelectionRegion selectionRegion;
	float drawPosition = 0.0;
	bool shouldLoop = false;
	Label timeLabel;
	FadeoutWidget fadeoutWidget;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionOverlay)
};

