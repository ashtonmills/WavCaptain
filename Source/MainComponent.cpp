/*
  ==============================================================================

	This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#define PLAYTEXT CharPointer_UTF8("\xe2\x96\xb6")
#define STOPTEXT CharPointer_UTF8("\xe2\x96\xa0")
#define PAUSETEXT CharPointer_UTF8 ("\xe2\x8f\xb8")
//==============================================================================
MainComponent::MainComponent() :state(Stopped),thumbnailCache(5), thumbnailComponent(1024,formatManager,thumbnailCache), positionOverlay(transportSource),gain(0.5), localTableList(*this,"Source Directory"), destinationRepoList(*this,"Destination Repo Directory"),buttonPanel(*this)
{
	// Make sure you set the size of the component after
	// you add any child components.
	setSize(1200, 800);

	transportSource.addChangeListener(this);

	formatManager.registerBasicFormats();

	addAndMakeVisible(thumbnailComponent);

	addAndMakeVisible(positionOverlay);

	addAndMakeVisible(gainSlider);
	gainSlider.setSliderStyle(Slider::LinearVertical);
	gainSlider.setRange(0.0f, 1.0f, 0.01);
	gainSlider.setValue(gain);
	gainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 30);
	gainSlider.addListener(this);
	gainSlider.setSkewFactorFromMidPoint(0.25);

	addAndMakeVisible(debugLabel);
	debugLabel.setText("This little area could display hints in the future but for now it displays debug messages. ", dontSendNotification);
	
	addAndMakeVisible(localTableList);
	addAndMakeVisible(destinationRepoList);

	addAndMakeVisible(buttonPanel);

	// Some platforms require permissions to open input channels so request that here
	if (RuntimePermissions::isRequired(RuntimePermissions::recordAudio)
		&& !RuntimePermissions::isGranted(RuntimePermissions::recordAudio))
	{
		RuntimePermissions::request(RuntimePermissions::recordAudio,
			[&](bool granted) { if (granted)  setAudioChannels(2, 2); });
	}
	else
	{
		// Specify the number of input and output channels that we want to open
		setAudioChannels(0, 2);
	}
}

MainComponent::~MainComponent()
{
	// This shuts down the audio device and clears the audio source.
	shutdownAudio();
}

//==================================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
	if (playSource.get() == nullptr)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}

	transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
	transportSource.releaseResources();
}

//==============================================================================
void MainComponent::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	//g.setFont(Font("Arial", 12, Font::plain));
	// You can add your drawing code here!
//	Rectangle<int> thumbnailBounds(10, 100, getWidth() - 20, getHeight() - 120);

}


void MainComponent::readFile(File myFile)
{
		AudioFormatReader* reader = formatManager.createReaderFor(myFile);
		//get the file ready to play
		if (reader != nullptr)
		{
			changeState(Stopped);
			std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
			transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
			buttonPanel.playButton.setEnabled(true);
			playSource.reset(newSource.release());
			buttonPanel.playButton.setButtonText(PLAYTEXT);
			thumbnailComponent.setFile(myFile);
			setDebugText("You loaded a file, arean't you clever?");
		}
	
}

void MainComponent::openButtonClicked()
{

	//choose a file
	FileChooser chooser("Choose a wav file",
		{},
		"*.wav");
	//if the user chooses a file
	if (chooser.browseForFileToOpen())
	{
		//what did the user choose? 
		File myFile = chooser.getResult();
		//read the file
		readFile(myFile);
	}

}

void MainComponent::setDebugText(String textToDisplay)
{
	timerFlashCount = 0;
	startTimer(50);
	debugLabel.setText(textToDisplay, dontSendNotification);
}

void MainComponent::play()
{
	if ((state == Stopped) || (state == Paused))
		changeState(Starting);
	else
		changeState(Pausing);
}

void MainComponent::stop()
{
	if (state == Paused)
		changeState(Stopped);
	else
		changeState(Stopping);
}

void MainComponent::timerCallback()
{
	timerFlashCount++;
	if (timerFlashCount % 2 != 0)
	{
		debugLabel.setColour(0x1000280,Colours::darkgrey);
	}
	else
	{
		Colour defaultColour = Colour(0x1005700);
		debugLabel.setColour(0x1000280, defaultColour);
	}
	if (timerFlashCount == 8)
	{
		timerFlashCount = 0;
		stopTimer();
	}
}

void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == &transportSource)
		transportSourceChanged();
}


void MainComponent::transportSourceChanged()
{
	if (transportSource.isPlaying())
		changeState(Playing);
	else if ((state == Playing) || (state == Stopping))
		changeState(Stopped);
	else if (state == Pausing)
		changeState(Paused);
}


void MainComponent::changeState(TransportState newState)
{
	if (newState != state)
	{
		state = newState;

		switch (state)
		{
		case Stopped:
			buttonPanel.stopButton.setEnabled(false);
			buttonPanel.playButton.setEnabled(true);
			buttonPanel.rewindButton.setEnabled(false);
			buttonPanel.playButton.setButtonText(PLAYTEXT);
			buttonPanel.stopButton.setButtonText(STOPTEXT);
			transportSource.setPosition(0.0);
			break;

		case Starting:
			transportSource.start();
			break;

		case Playing:
			buttonPanel.rewindButton.setEnabled(true);
			buttonPanel.stopButton.setEnabled(true);
			buttonPanel.playButton.setButtonText(PAUSETEXT);
			buttonPanel.stopButton.setButtonText(STOPTEXT);
			break;

		case Stopping:
			transportSource.stop();

			break;

		case Pausing:
			transportSource.stop();
			break;

		case Paused:
			buttonPanel.playButton.setButtonText(PLAYTEXT);
			buttonPanel.stopButton.setButtonText(STOPTEXT);
			break;
		}
	}
}

//try using the lambda version of this instead 

void MainComponent::sliderValueChanged(Slider* slider)
{
	if (slider == &gainSlider)
	{
	//	debugLabel.setText("volume slider changed",dontSendNotification);
		transportSource.setGain(slider->getValue());
	}
}
//
//void MainComponent::filesDropped(const StringArray& files, int x, int y)
//{
//	 (isInterestedInFileDrag (files))
//	
//	readFile(files[0]);
//	debugLabel.setText("you tried to drop a file in, that's adventurous!",dontSendNotification);
//}
//
//bool MainComponent::isInterestedInFileDrag(const StringArray& files)
//
//{
//	return true;
//}



void MainComponent::resized()
{
	
	int border = 10;
	Rectangle<int> area (0,border,getWidth(),getHeight()-border);
	thumbnailComponent.setBounds(area.removeFromTop(100));
	positionOverlay.setBounds(thumbnailComponent.getBounds());
	buttonPanel.setBounds(area.removeFromTop(20));
	debugLabel.setBounds(area.removeFromBottom(30));
	localTableList.setBounds(area.removeFromLeft(getWidth() / 2));
	destinationRepoList.setBounds(area.removeFromLeft(localTableList.getWidth()));
	//auto wavPlayerBounds = area.removeFromLeft(getWidth() / 3);
//	openButton.setBounds(wavPlayerBounds.getX() , wavPlayerBounds.getY() + border, wavPlayerBounds.getWidth(), wavPlayerBounds.getHeight() / 8 );
	/*playButton.setBounds(wavPlayerBounds.getX() , wavPlayerBounds.getY() + border + (wavPlayerBounds.getHeight() / 6), wavPlayerBounds.getWidth(), wavPlayerBounds.getHeight() / 8 );
	stopButton.setBounds(wavPlayerBounds.getX() , wavPlayerBounds.getY() + border + ((wavPlayerBounds.getHeight() / 6)*2), wavPlayerBounds.getWidth(), wavPlayerBounds.getHeight() / 8);*/

	//Rectangle<int>thumbnailBounds(10, 100, getWidth() - 60, getHeight() - 160);


	//gainSlider.setBounds(thumbnailBounds.getWidth() + 20, thumbnailBounds.getHeight() - 150, 40, thumbnailBounds.getHeight() + 10);


	

}
