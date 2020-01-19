

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "localTableList.h"

//Forward declare the buttonPanel for linking reasons
class buttonPanel;

//==============================================================================
//Component for selection regions of the waveform

class SelectionRegion : public Component
{
public :
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

//===============================================================================
//Component for displaying the waveform

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


//=====================================================================
//Component that handles interfacing with the waveform

class PositionOverlay : public Component, public Timer
{
public:
	PositionOverlay(AudioTransportSource& transportSourcetoUse)
		: transportSource(transportSourcetoUse)
	{
		startTimer(1);
	}

	void paint(Graphics& g) override
	{
		auto duration = (float)transportSource.getLengthInSeconds();

		if (duration > 0.0)
		{
			auto audioPosition = (float) transportSource.getCurrentPosition();  //why here do he have (float) in brackets? 
			drawPosition =  (audioPosition / duration) * getWidth();
			g.setColour(Colours::black);
			g.drawLine(drawPosition, 0.0f, (float)drawPosition, getHeight(), 1.0);
		}
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
		auto duration = transportSource.getLengthInSeconds();

		if (duration > 0.0f)
		{
			auto clickPosition = event.position.x;
			auto audioPosition = (clickPosition / getWidth()) * duration;

			transportSource.setPosition(audioPosition);
		}
	}
	void mouseDrag(const MouseEvent& event) override
	{
		this->removeAllChildren();
		if (this->isEnabled())
		{
			addAndMakeVisible(selectionRegion);
			auto selectionStartPosition = event.mouseDownPosition.x;
			if (event.getOffsetFromDragStart().x > 0)
			{
				selectionRegion.setBounds(selectionStartPosition, 0, event.getOffsetFromDragStart().x, getHeight());
			}
			if (event.getOffsetFromDragStart().x < 0)
			{
				selectionRegion.setBounds(event.getPosition().x, 0, event.getOffsetFromDragStart().x * -1, getHeight());
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
	SelectionRegion selectionRegion;
	float drawPosition = 0.0;
	bool shouldLoop = false;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PositionOverlay)
};



//=========================================================================
// Display the version number and click to check for updates in internet browser

class AboutComponent : public Component
{
public:
	AboutComponent()
	{
		setSize(600, 600);
		addAndMakeVisible(updateButton);
		updateButton.onClick = [this] {updateButtonClicked(); };
	}

	void updateButtonClicked()
	{
		DBG("update Button Clicked");
		URL updateLink("https://www.dropbox.com/sh/gyj376qz42zyiz8/AAC7-i3AwXAr4Aw5yCu8xaota?dl=0");
		updateLink.launchInDefaultBrowser();
	}
	void resized()
	{
		updateButton.setBounds(getLocalBounds());
	}

private:
	String currentVersion = "1.1.3";
	TextButton updateButton{ "WavCaptain by BioMannequin (Ashton Mills)\n\n"+ currentVersion +"\n\nClick in this window to check for updates" };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutComponent)
};

class AboutWindow : public DocumentWindow
{
public:
	AboutWindow(String name) : DocumentWindow(name,
		Desktop::getInstance().getDefaultLookAndFeel()
		.findColour(ResizableWindow::backgroundColourId),
		DocumentWindow::allButtons)
	{
		setUsingNativeTitleBar(false);
		setResizable(true, true);

		setContentOwned(new AboutComponent(), true);
		centreWithSize(600, 600);
		setVisible(true);

	}

	void closeButtonPressed() override
	{
		// This is called when the user tries to close this window. Here, we'll just
		// ask the app to quit when this happens, but you can change this to do
		// whatever you need.
		delete this;

	}

	/* Note: Be careful if you override any DocumentWindow methods - the base
	   class uses a lot of them, so by overriding you might break its functionality.
	   It's best to do all your work in your content component instead, but if
	   you really have to override any DocumentWindow methods, make sure your
	   subclass also calls the superclass's method.
	*/

private:


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutWindow)
};



//Main component
//_________________________________________________________________________________

class MainComponent : public AudioAppComponent, public ChangeListener, private Timer
{
public:
	//==============================================================================
	MainComponent(String commandLineParams);
	~MainComponent();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;
	void paint(Graphics& g) override;
	void resized() override;
	void readFile(File myFile);
	void openButtonClicked();
	void setDebugText(String textToDisplay, bool flash = true);
	void play();
	void stop();
	void timerCallback() override;
	void saveData();
	void aboutButtonClicked();
	int getTargetSampleRate();


	class ButtonPanel : public Component, public Slider::Listener, public MouseListener
	{
	public:
		ButtonPanel(MainComponent& mc) : mainComp(mc)
		{
			setSize(getParentWidth(), 30);

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
			deployButton.addMouseListener(this, false);

			deployAllButton.onClick = [this] {deployAllButtonClicked(); };
			addAndMakeVisible(&deployAllButton);
			deployAllButton.addMouseListener(this, false);

			addAndMakeVisible(convertSRButton);
			convertSRButton.onClick = [this] {convertSRButtonClicked(); };
			convertSRButton.addMouseListener(this, false);

			addAndMakeVisible(SRMenu);
			Array<String> rates = { "22.05Khz","44.1Khz","48Khz","96Khz" };
			SRMenu.addItemList(rates, 1);
			SRMenu.setSelectedId(1);
			SRMenu.addMouseListener(this, false);

			addAndMakeVisible(gainSlider);
			gainSlider.setSliderStyle(Slider::LinearHorizontal);
			gainSlider.setRange(0.0f, 1.0f, 0.01);
			gainSlider.setValue(gain);
			gainSlider.setColour(0x1001300, Colours::white); //slider thumb colour
			gainSlider.setColour(0x1001310, Colours::lightgrey); // slider track colour
			gainSlider.setTextBoxStyle(Slider::NoTextBox, true, 40, 30);
			gainSlider.addListener(this);
			gainSlider.setSkewFactorFromMidPoint(0.25);

			muteButton.setLookAndFeel(&unicodeLookAndFeel);
			addAndMakeVisible(muteButton);
			muteButton.onClick = [this] {muteButtonClicked(); };
			gainSlider.addMouseListener(this, false);

			addAndMakeVisible(loopButton);
			loopButton.setLookAndFeel(&unicodeLookAndFeel);
			loopButton.onClick = [this] {loopButttonClicked(); };

		}

		~ButtonPanel()
		{
			//We need to make sure nothing is using UnicodeLookAndFeel because it gets destroyed
			//before the components that reference it. 
			playButton.setLookAndFeel(nullptr);
			stopButton.setLookAndFeel(nullptr);
			rewindButton.setLookAndFeel(nullptr);
		}

		void stopButtonClicked()
		{
			mainComp.stop();
		}

		void playButtonClicked()
		{
			mainComp.play();
		}

		void loopButttonClicked()
		{
			if (!mainComp.positionOverlay.getLooping())
			{
				mainComp.positionOverlay.setLooping(true);
				loopButton.setButtonText(loopSymbol);
			}
			else
			{
				mainComp.positionOverlay.setLooping(false);
				loopButton.setButtonText(stopAtEndSymbol);
			}
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

		void convertSRButtonClicked()
		{
			mainComp.localTableList.convertSampleRate();
		}

		void sliderValueChanged(Slider* slider) override
		{
			mainComp.transportSource.setGain(slider->getValue());
		}

		void muteButtonClicked()
		{
			if (!isMuted)
			{
				gain = gainSlider.getValue();
				gainSlider.setValue(0);
				muteButton.setButtonText(muteSymbol);
				isMuted = true;
			}
			else
			{
				gainSlider.setValue(gain);
				muteButton.setButtonText(gainLabelSymbol);
				isMuted = false;
			}
		}
		//Mouse hover text for the buttons
		void mouseEnter(const MouseEvent& event) override
		{
			if (event.originalComponent == &deployAllButton)
			{
				mainComp.setDebugText("Copy contents of source directory to destination directory", false);
			}
			if (event.originalComponent == &deployButton)
			{
				mainComp.setDebugText("Copy selected files to destination directory", false);
			}
			if (event.originalComponent == &convertSRButton)
			{
				mainComp.setDebugText("Downsample selected files to chosen sample rate", false);
			}
			if (event.originalComponent == &SRMenu)
			{
				mainComp.setDebugText("Choose sample rate to convert to", false);
			}
			if (event.originalComponent == &muteButton)
			{
				mainComp.setDebugText("Mute", false);
			}
		}

		//Delete the mouse hover text 
		void mouseExit(const MouseEvent& event) override
		{
			if (event.originalComponent == &deployAllButton ||
				event.originalComponent == &deployButton ||
				event.originalComponent == &muteButton ||
				event.originalComponent == &SRMenu ||
				event.originalComponent == &convertSRButton)
			{
				mainComp.setDebugText("", false);
			}
		}

		void resized() override
		{
			auto panelBounds = getLocalBounds();
			playButton.setBounds(panelBounds.removeFromLeft(70));
			stopButton.setBounds(panelBounds.removeFromLeft(70));
			rewindButton.setBounds(panelBounds.removeFromLeft(70));
			loopButton.setBounds(panelBounds.removeFromLeft(70));
			deployButton.setBounds(panelBounds.removeFromLeft(100));
			deployAllButton.setBounds(panelBounds.removeFromLeft(100));
			convertSRButton.setBounds(panelBounds.removeFromLeft(100));
			SRMenu.setBounds(panelBounds.removeFromLeft(150));
			gainSlider.setBounds(panelBounds.removeFromRight(150));
			muteButton.setBounds(panelBounds.removeFromRight(30));
			editModeButton.setBounds(panelBounds.removeFromRight(100));
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

		//Unicode symbols for buttons
		String stopSymbol = CharPointer_UTF8("\xe2\x96\xa0");
		String playSymbol = CharPointer_UTF8("\xe2\x96\xb6");
		String loopSymbol = CharPointer_UTF8("\xe2\x88\x9e");
		String stopAtEndSymbol = CharPointer_UTF8("\xe2\x87\xa5");
		String gainLabelSymbol = CharPointer_UTF8("\xf0\x9f\x94\x8a");

		//Buttons
		TextButton playButton{ playSymbol};
		TextButton stopButton{ stopSymbol };
		TextButton rewindButton{ CharPointer_UTF8("\xe2\x8f\xae") };
		TextButton deployButton{ "Deploy Selected" };
		TextButton deployAllButton{ "Deploy All" };
		TextButton convertSRButton{ "Convert Sample Rate" };
		TextButton editModeButton{ "Edit" };

		KeyPress keyPressPlay{ KeyPress::spaceKey };
		KeyPress keyPressRewind{ KeyPress::createFromDescription("w") };

		UnicodeSymbolsLookAndFeel unicodeLookAndFeel;
		ComboBox SRMenu;

		MainComponent& mainComp;

		TextButton muteButton{gainLabelSymbol};

		TextButton loopButton{ stopAtEndSymbol };
		String muteSymbol = CharPointer_UTF8("\xf0\x9f\x94\x87");
		Slider gainSlider;
		float gain = 1;
		bool isMuted = false;
		bool isInEditMode = false;
		

	};

	LocalTableList localTableList;
	LocalTableList destinationRepoList;


private:

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

	AudioTransportSource transportSource;

	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReaderSource> playSource;

	AudioThumbnailCache thumbnailCache;

	ThumbnailComponent thumbnailComponent;
	PositionOverlay positionOverlay;

	TextButton aboutButton{ "About" };

	Label debugLabel;
	int timerFlashCount;

	ButtonPanel buttonPanel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};


