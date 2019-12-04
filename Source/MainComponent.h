/*

  ==============================================================================

	This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "localTableList.h"

class buttonPanel;
//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/

// child component that controls what we paint in the thumnbnail box.
//or telling us no file is loaded is there is no file opened. 


class ThumbnailComponent : public Component, private ChangeListener
{
public : 
	ThumbnailComponent(int sourceSamplesPerThumbnailSample,
		AudioFormatManager& formatManager,
		AudioThumbnailCache& thumbnailCache) : 
		thumbnail (sourceSamplesPerThumbnailSample, formatManager,thumbnailCache)
	{
		thumbnail.addChangeListener(this);
	}

	void setFile(const File& file)
	{
		thumbnail.setSource(new FileInputSource(file));
	}

	void paint(Graphics& g) override
	{
		if (thumbnail.getNumChannels() == 0)
			paintIfNoFileLoaded(g);
		else
			paintIfFileLoaded(g);
	}

	void paintIfNoFileLoaded(Graphics& g)
	{
		g.setColour(Colours::darkgrey);
		g.fillAll();
		g.setColour(Colours::white);
		g.drawFittedText("No file loaded", getLocalBounds(), Justification::centred, 1.0f);
	}

	void paintIfFileLoaded(Graphics& g)
	{
		g.setColour(Colours::white);
		g.fillAll();
		g.setColour(Colours::darkgrey);

		thumbnail.drawChannels(g,
			getLocalBounds(),
			0.0f,
			thumbnail.getTotalLength(),
			1.0f);
		//g.setColour(Colours::black);
		//auto audioPosition(transportSource.getCurrentPosition());
		//auto drawPosition((audioPosition / audioLength) * thumbnailBounds.getWidth()
		//	+ thumbnailBounds.getX());
	//	g.drawLine(drawPosition, thumbnailBounds.getY(), drawPosition, thumbnailBounds.getBottom(), 2.0);
	}

	void changeListenerCallback(ChangeBroadcaster* source) override
	{
		if (source == &thumbnail)
			thumbnailChanged();
	}

private:
	void thumbnailChanged()
	{
		repaint();
	}

	AudioThumbnail thumbnail;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThumbnailComponent)
};

//______________________________________________________________________________________________________________________

//child component that handles the playback position line

class PositionOverlay : public Component, public Timer
{
public:
	PositionOverlay(AudioTransportSource& transportSourcetoUse)
		: transportSource(transportSourcetoUse)
	{
		startTimer(40);
	}

	void paint(Graphics& g) override
	{
		auto duration = (float)transportSource.getLengthInSeconds();

		if (duration > 0.0)
		{
			auto audioPosition = (float) transportSource.getCurrentPosition();  //why here do he have (float) in brackets? 
			auto drawPosition =  (audioPosition / duration) * getWidth();
			g.setColour(Colours::black);
			g.drawLine(drawPosition, 0.0f, (float)drawPosition, getHeight(), 1.0);
		}
	}


	void mouseDown(const MouseEvent& event) override
	{
		auto duration = transportSource.getLengthInSeconds();

		if (duration > 0.0f)
		{
			auto clickPosition = event.position.x;
			auto audioPosition = (clickPosition / getWidth()) * duration;

			transportSource.setPosition(audioPosition);
		}
	}


private:

	void timerCallback()override
	{
		repaint();
	}
	AudioTransportSource& transportSource;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionOverlay)
};

class MainComponent : public AudioAppComponent, public ChangeListener, public Slider::Listener, public FileDragAndDropTarget, private Timer
{
public:
	//==============================================================================
	MainComponent();
	~MainComponent();

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;
	void filesDropped(const StringArray& files, int x, int y) override;
	bool isInterestedInFileDrag(const StringArray& files) override;
	//==============================================================================
	void paint(Graphics& g) override;
	void resized() override;
	void readFile(File myFile);
	void openButtonClicked();
	void setDebugText(String textToDisplay);
	void play();
	void stop();
	void timerCallback() override;


	class ButtonPanel : public Component
	{
	public:
		ButtonPanel(MainComponent& mc) : mainComp(mc)
		{
			setSize(getParentWidth(), 30);

		//	setLookAndFeel(&unicodeLookAndFeel);

			playButton.onClick = [this] {playButtonClicked(); };
			playButton.setLookAndFeel(&unicodeLookAndFeel);
			addAndMakeVisible(&playButton);
			playButton.setEnabled(false);
			playButton.addShortcut(keyPressPlay);
		

			stopButton.onClick = [this] {stopButtonClicked(); };
			stopButton.setLookAndFeel(&unicodeLookAndFeel);
			addAndMakeVisible(&stopButton);
			stopButton.setEnabled(false);

			rewindButton.onClick = [this] {rewindButtonClicked(); };
			rewindButton.setLookAndFeel(&unicodeLookAndFeel);
			addAndMakeVisible(&rewindButton);
			rewindButton.setEnabled(false);
			rewindButton.addShortcut(keyPressRewind);

	
			deployButton.onClick = [this] {deployButtonClicked(); };
			addAndMakeVisible(&deployButton);

			deployAllButton.onClick = [this] {deployAllButtonClicked(); };
			addAndMakeVisible(&deployAllButton);
		}

		void stopButtonClicked()
		{
			mainComp.stop();
		}

		void playButtonClicked()
		{
			mainComp.play();
		}

		void rewindButtonClicked()
		{
			mainComp.transportSource.setPosition(0.0);
		}

		void deployButtonClicked()
		{
			mainComp.localTableList.deploySelectedFiles(false);
		}
		
		void deployAllButtonClicked()
		{
			mainComp.localTableList.deploySelectedFiles(true);
		}

		void resized() override
		{
			auto panelBounds = getLocalBounds();
			playButton.setBounds(panelBounds.removeFromLeft(100));
			stopButton.setBounds(panelBounds.removeFromLeft(100));
			rewindButton.setBounds(panelBounds.removeFromLeft(100));
			deployButton.setBounds(panelBounds.removeFromLeft(100));
			deployAllButton.setBounds(panelBounds.removeFromLeft(100));
		}

		class UnicodeSymbolsLookAndFeel : public LookAndFeel_V4
		{
		public:
			UnicodeSymbolsLookAndFeel()
			{
			}
			Font getTextButtonFont(TextButton&, int buttonHeight) override
			{
				return Font("Segoe UI Symbol", 20, Font::plain);
			}
		};

		String stopSymbol = CharPointer_UTF8("\xe2\x96\xa0");
		String playSymbol = CharPointer_UTF8("\xe2\x96\xb6");
		TextButton playButton{ playSymbol};
		TextButton stopButton{ stopSymbol };
		TextButton rewindButton{ CharPointer_UTF8("\xe2\x8f\xae") };
		TextButton deployButton{ "Deploy Selected" };
		TextButton deployAllButton{ "Deploy All" };
		KeyPress keyPressPlay{ KeyPress::spaceKey };
		KeyPress keyPressRewind{ KeyPress::createFromDescription("w") };
		MainComponent& mainComp;
		UnicodeSymbolsLookAndFeel unicodeLookAndFeel;


	};

	LocalTableList localTableList;
	LocalTableList destinationRepoList;


private:
	//==============================================================================
	// Your private member variables go here...
	enum TransportState
	{
		Stopped,
		Starting,
		Stopping,
		Playing,
		Pausing,
		Paused
	};

	TransportState state;

	void transportSourceChanged();

	void changeState(TransportState newState);
	void changeListenerCallback(ChangeBroadcaster* source) override;

	void sliderValueChanged(Slider* slider) override;


	AudioTransportSource transportSource;

	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReaderSource> playSource;

	AudioThumbnailCache thumbnailCache;

	ThumbnailComponent thumbnailComponent;
	PositionOverlay positionOverlay;

	Slider gainSlider;
	float gain;

	Label debugLabel;
	int timerFlashCount;


	ButtonPanel buttonPanel;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};


