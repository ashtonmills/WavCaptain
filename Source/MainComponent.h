

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "localTableList.h"
#include "LabellingComponent.h"

//Forward declare the buttonPanel for linking reasons
class buttonPanel;
class MainComponent;
class LabellingComponent;

namespace ValTreeIDs
{
//put all the valuetree identifiers in here as static Identfier and then we don't have to fuck abotu all the time. 
}

struct CoreData
{
	CoreData(ValueTree vt) : mainVT(vt)
	{
		mainVT.setProperty(loadSwitch, 0, nullptr);
	}
	Array<File> sourceFiles;
	Array<File> repoFiles;
	const Identifier loadSwitch{ "load" };
	ValueTree mainVT;
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

class TempComponent : public Component
{
public:
	TempComponent(int number,ValueTree vt) : mainVT(vt)
	{
	int numProps=	mainVT.getNumProperties();
	DBG(numProps);
	}
private:
	ValueTree mainVT;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempComponent)
};
//=====================================================================
//Component that handles interfacing with the waveform

class PositionOverlay : public Component, public Timer
{
public:
	PositionOverlay(AudioTransportSource& transportSourcetoUse,ValueTree vt)
		: transportSource(transportSourcetoUse),mainVT(vt)
	{
		startTimer(1);
	}

	void paint(Graphics& g) override
	{
		auto duration = (float)transportSource.getLengthInSeconds();

		if (duration > 0.0)
		{
			float audioPosition = (float) transportSource.getCurrentPosition();  //why here do he have (float) in brackets? 
			//update the valuetree with audioposition.
			//This is not really a good place to do this, inside a paint function, TODO refector to make it tidier. 
			Identifier timeCode("timeCode");
			auto transportTime = floatToTimecode(audioPosition);
			mainVT.setProperty(timeCode, transportTime, nullptr);

			drawPosition =  (audioPosition / duration) * getWidth();
			g.setColour(Colours::black);
			g.drawLine(drawPosition, 0.0f, (float)drawPosition, getHeight(), 1.0);
		}
	}
	String floatToTimecode(float rawTime)
	{
		int rawMilSecs, rawSecs, rawMins;
		String milSecs,secs, mins;
		float fRawSecs;
		float fRawMilSec = std::modf(rawTime, &fRawSecs);
		rawMilSecs = fRawMilSec *100;
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
	ValueTree mainVT;
	SelectionRegion selectionRegion;
	float drawPosition = 0.0;
	bool shouldLoop = false;
	Label timeLabel;

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
	String currentVersion = "1.1.4";
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
	private:


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
		double getTargetSampleRate();
	


		class ButtonPanel : public Component, public Slider::Listener, public MouseListener
		{
		public:
			ButtonPanel(MainComponent& mc,ValueTree vt, CoreData& data) : mainComp(mc), mainVT(vt), coreData(data)
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

				addAndMakeVisible(labelButton);
				labelButton.onClick = [this] {labelButtonClicked(); };

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

				Identifier timeCode("timeCode");
				addAndMakeVisible(timeLabel);
				timeLabel.getTextValue().referTo(mainVT.getPropertyAsValue(timeCode, nullptr));

				addAndMakeVisible(clickPlayButton);
				clickPlayButton.onClick = [this] {clickPlayButtonPressed(); };
				oneClickToggleVT.setProperty(oneClickBoolID, true, nullptr);
				clickPlayButton.setLookAndFeel(&unicodeLookAndFeel);
				String oneClickToggleState = oneClickToggleVT.getProperty(oneClickToggleID);
				DBG("one Click toggle state is " + oneClickToggleState);

				mainVT.addChild(oneClickToggleVT, -1, nullptr);
			}

			~ButtonPanel()
			{
				//We need to make sure nothing is using UnicodeLookAndFeel because it gets destroyed
				//before the components that reference it. 
				playButton.setLookAndFeel(nullptr);
				stopButton.setLookAndFeel(nullptr);
				rewindButton.setLookAndFeel(nullptr);
				clickPlayButton.setLookAndFeel(nullptr);
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

			void labelButtonClicked()
			{
				 auto labellingWindow =new LabellingWindow("Label Selected Assets",coreData);
			}

			void clickPlayButtonPressed()
			{
				String oneClickToggleState = oneClickToggleVT.getProperty(oneClickBoolID);
				DBG("one Click toggle state is " + oneClickToggleState);
				bool isSetToOneClickMode = oneClickToggleVT.getProperty(oneClickBoolID);
				if (isSetToOneClickMode)
				{
					clickPlayButton.setButtonText(twoClickPlay);
					oneClickToggleVT.setProperty(oneClickBoolID, false, nullptr);
				}
				else
				{
					clickPlayButton.setButtonText(oneClickPlay);
					oneClickToggleVT.setProperty(oneClickBoolID, true, nullptr);
				}
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
				clickPlayButton.setBounds(panelBounds.removeFromLeft(70));
				deployButton.setBounds(panelBounds.removeFromLeft(100));
				deployAllButton.setBounds(panelBounds.removeFromLeft(100));
				convertSRButton.setBounds(panelBounds.removeFromLeft(100));
				SRMenu.setBounds(panelBounds.removeFromLeft(150));
				labelButton.setBounds(panelBounds.removeFromLeft(70)); 
				gainSlider.setBounds(panelBounds.removeFromRight(150));
				muteButton.setBounds(panelBounds.removeFromRight(30));
				timeLabel.setBounds(panelBounds.removeFromRight(100));

				//editModeButton.setBounds(panelBounds.removeFromRight(100));

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

			ValueTree mainVT;
			CoreData& coreData;
			const Identifier oneClickToggleID{ "oneClickToggle" };
			const Identifier oneClickBoolID{ "oneClickBool" };
			ValueTree oneClickToggleVT{ oneClickToggleID };

			//Unicode symbols for buttons
			String stopSymbol = CharPointer_UTF8("\xe2\x96\xa0");
			String playSymbol = CharPointer_UTF8("\xe2\x96\xb6");
			String loopSymbol = CharPointer_UTF8("\xe2\x88\x9e");
			String stopAtEndSymbol = CharPointer_UTF8("\xe2\x87\xa5");
			String gainLabelSymbol = CharPointer_UTF8("\xf0\x9f\x94\x8a");
			String mouseSymbol = CharPointer_UTF8("\xf0\x9f\x96\xaf");
			String oneClickPlay = "1" + mouseSymbol + playSymbol;
			String twoClickPlay = "2" + mouseSymbol + playSymbol;

			//Buttons
			TextButton playButton{ playSymbol };
			TextButton stopButton{ stopSymbol };
			TextButton rewindButton{ CharPointer_UTF8("\xe2\x8f\xae") };
			TextButton deployButton{ "Deploy Selected" };
			TextButton deployAllButton{ "Deploy All" };
			TextButton convertSRButton{ "Convert Sample Rate" };
			TextButton editModeButton{ "Edit" };
			TextButton labelButton{ "Label" };
			TextButton clickPlayButton{oneClickPlay};

			Label timeLabel;

			KeyPress keyPressPlay{ KeyPress::spaceKey };
			KeyPress keyPressRewind{ KeyPress::createFromDescription("w") };

			UnicodeSymbolsLookAndFeel unicodeLookAndFeel;
			ComboBox SRMenu;

			MainComponent& mainComp;

			TextButton muteButton{ gainLabelSymbol };

			TextButton loopButton{ stopAtEndSymbol };
			String muteSymbol = CharPointer_UTF8("\xf0\x9f\x94\x87");
			Slider gainSlider;
			float gain = 1;
			bool isMuted = false;
			bool isInEditMode = false;



		};
		const Identifier mainVTType{ "mainVT" };
		ValueTree mainVT{ mainVTType };
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
	
		TextButton aboutButton{ "About" };

		Label debugLabel;
		int timerFlashCount;
		const Identifier mainVTtype{ "mainVT" };


		ButtonPanel buttonPanel;

		CoreData coreData{ mainVT };

		TempComponent tc;
		PositionOverlay positionOverlay;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
	};
