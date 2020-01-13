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
MainComponent::MainComponent(String commandLineParam) :state(Stopped),thumbnailCache(5),
thumbnailComponent(1024,formatManager,thumbnailCache), 
positionOverlay(transportSource),
localTableList(*this,"Source Directory",true,commandLineParam),
destinationRepoList(*this,"Destination Repo Directory",false,""),
buttonPanel(*this)
{
	// Make sure you set the size of the component after
	// you add any child components.

	setSize(1200, 900);

	transportSource.addChangeListener(this);

	formatManager.registerBasicFormats();

	addAndMakeVisible(thumbnailComponent);

	addAndMakeVisible(positionOverlay);

	addAndMakeVisible(debugLabel);
	debugLabel.setText("Display debug messages here", dontSendNotification);

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

	commandLineParam = commandLineParam.unquoted();

	if ((commandLineParam.contains(".wav")) && (File(commandLineParam).existsAsFile()))
	{
		localTableList.filesDropped(commandLineParam, 1, 1);
		setDebugText(commandLineParam + " was loaded");
	}
}

MainComponent::~MainComponent()
{
	// This shuts down the audio device and clears the audio source.
	transportSource.setSource(nullptr);
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
		//	setDebugText("You loaded a file, arean't you clever?");
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

void MainComponent::saveData()
{
	//wrapped this in an if check so it doesn't save empty paths
	if (localTableList.getDirectory().exists()||destinationRepoList.getDirectory().exists())
	{
	//	setDebugText("saveData function called");
		XmlElement saveData("SAVEDATA");

		XmlElement* header = new XmlElement("HEADERS");
		saveData.addChildElement(header);

		XmlElement* column1 = new XmlElement("COLUMN");
		column1->setAttribute("name", "filepath");
		header->addChildElement(column1);

		XmlElement* column2 = new XmlElement("COLUMN");
		column2->setAttribute("name", "LocalOrRepo");
		header->addChildElement(column2);

		XmlElement* data = new XmlElement("DATA");
		saveData.addChildElement(data);

		if (localTableList.getDirectory().exists())
		{
			XmlElement* localPath = new XmlElement("PATH");
			localPath->setAttribute("filepath", localTableList.getDirectory().getFullPathName());
			localPath->setAttribute("LocalOrRepo", "Local");
			data->addChildElement(localPath);
		}

		if (destinationRepoList.getDirectory().exists())
		{
			XmlElement* repoPath = new XmlElement("PATH");
			repoPath->setAttribute("filepath", destinationRepoList.getDirectory().getFullPathName());
			repoPath->setAttribute("LocalOrRepo", "Repo");
			data->addChildElement(repoPath);
		}

		File saveDirFile = File::getCurrentWorkingDirectory().getChildFile("Resources").getChildFile("savedDirectories.xml");
		saveData.writeTo(saveDirFile);

		/*Let's have another think through this one.
		Obviously the best thing to do would be to have a recent files option.
		But for now Iwant the simple option, where it will load up whaterver was last in the direcrtorries when th project loads.
		So let's just ovewrite the file each time for now. It will bring back the same as it was before, even if that was empty.
		(Though at the minute there is no option for it to be selected as empty,
		so when you open one file, with the basic save feature it will always then have a file loaded)

		SO change this function instead of passing in variables, the function itslf just looks at the two directory members and saves the full file path as a string and writes the XML doc.
		Then when we first open the app we parse that document and set thoise members and load the data.

		After that is working we add a recent places feature
		*/
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
int MainComponent::getTargetSampleRate()
{

		int SRSelection = buttonPanel.SRMenu.getSelectedItemIndex() +1;

		switch (SRSelection)
		{
		case 1: return 22050;
		case 2: return 44100;
		case 3: return 48000;
		case 4: return 96000;
		}
	
}


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
