/*
  ==============================================================================

	This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"
#define PLAYINIT "Play"

//==============================================================================
MainComponent::MainComponent() : openButton("Open"), playButton("Play"), stopButton("Stop"), state(Stopped),
thumbnailCache(5), thumbnailComponent(512,formatManager,thumbnailCache), positionOverlay(transportSource),gain(0.5)
{
	// Make sure you set the size of the component after
	// you add any child components.
	setSize(600, 400);

	openButton.onClick = [this] {openButtonClicked(); };
	addAndMakeVisible(&openButton);

	playButton.onClick = [this] {playButtonClicked(); };
	addAndMakeVisible(&playButton);
	playButton.setColour(TextButton::buttonColourId, Colours::green);
	playButton.setEnabled(false);

	stopButton.onClick = [this] {stopButtonClicked(); };
	addAndMakeVisible(&stopButton);
	stopButton.setColour(TextButton::buttonColourId, Colours::red);
	stopButton.setEnabled(false);

	transportSource.addChangeListener(this);
	//transportSource.addChangeListener(this);

	formatManager.registerBasicFormats();

	addAndMakeVisible(thumbnailComponent);

	addAndMakeVisible(positionOverlay);

	addAndMakeVisible(gainSlider);
	gainSlider.setSliderStyle(Slider::LinearVertical);
	gainSlider.setRange(-1.0f, 1.0f, 0.01);
	gainSlider.setValue(gain);
	gainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 30);



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

	// You can add your drawing code here!
//	Rectangle<int> thumbnailBounds(10, 100, getWidth() - 20, getHeight() - 120);

}

void MainComponent::resized()
{
	openButton.setBounds(10, 10, getWidth() - 20, 20);
	playButton.setBounds(10, 40, getWidth() - 20, 20);
	stopButton.setBounds(10, 70, getWidth() - 20, 20);

	Rectangle<int>thumbnailBounds(10, 100, getWidth() - 60, getHeight() - 160);
	thumbnailComponent.setBounds(thumbnailBounds);
	positionOverlay.setBounds(thumbnailBounds);

	gainSlider.setBounds(thumbnailBounds.getWidth() + 20, thumbnailBounds.getHeight()-150, 40, thumbnailBounds.getHeight()+10);
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
		AudioFormatReader* reader = formatManager.createReaderFor(myFile);
		//get the file ready to play
		if (reader != nullptr)
		{
			changeState(Stopped);
			std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
			transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
			playButton.setEnabled(true);
			playSource.reset(newSource.release());
			playButton.setButtonText(PLAYINIT);
			playButton.setColour(TextButton::buttonColourId, Colours::green);
			thumbnailComponent.setFile(myFile);
		}


	}

}

void MainComponent::playButtonClicked()
{
	if ((state == Stopped) || (state == Paused))
		changeState(Starting);
	else
		changeState(Pausing);
}

void MainComponent::stopButtonClicked()
{
	if (state == Paused)
		changeState(Stopped);
	else
		changeState(Stopping);
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
			stopButton.setEnabled(false);
			playButton.setEnabled(true);
			playButton.setButtonText(PLAYINIT);
			playButton.setColour(TextButton::buttonColourId, Colours::green);
			stopButton.setButtonText("Stop");
			transportSource.setPosition(0.0);
			break;

		case Starting:
			transportSource.start();
			break;

		case Playing:
			stopButton.setEnabled(true);
			playButton.setButtonText("Pause");
			playButton.setColour(TextButton::buttonColourId, Colours::yellow);
			stopButton.setButtonText("Stop");
			break;

		case Stopping:
			transportSource.stop();

			break;

		case Pausing:
			transportSource.stop();
			break;

		case Paused:
			playButton.setButtonText("Resume");
			playButton.setColour(TextButton::buttonColourId, Colours::green);
			stopButton.setButtonText("Rewind");
			break;
		}
	}
}

//try using the lambda version of this instead 

void MainComponent::sliderValueChanged(Slider* slider)
{
	if (&gainSlider == slider)
	{
		gainSlider.set
		transportSource.setGain(slider->getValue());
	}
}
