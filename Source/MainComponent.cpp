
#include "MainComponent.h"

#define PLAYTEXT CharPointer_UTF8("\xe2\x96\xb6")
#define STOPTEXT CharPointer_UTF8("\xe2\x96\xa0")
#define PAUSETEXT CharPointer_UTF8 ("\xe2\x8f\xb8")

//==============================================================================
MainComponent::MainComponent(String commandLineParam) :state(Stopped),
thumbnailCache(5),
thumbnailComponent(1024,formatManager,thumbnailCache), 
positionOverlay(transportSource,mainVT),
localTableList(*this,"Source Directory",true,commandLineParam),
destinationRepoList(*this,"Destination Repo Directory",false,""),
buttonPanel(*this,mainVT,coreData),tc(2,mainVT)
{
	setSize(1200, 900);

	transportSource.addChangeListener(this);

	formatManager.registerBasicFormats();

	addAndMakeVisible(thumbnailComponent);

	addAndMakeVisible(positionOverlay);

	addAndMakeVisible(tc);
	addAndMakeVisible(debugLabel);
	debugLabel.setText("Display debug messages here", dontSendNotification);

	addAndMakeVisible(localTableList);
	addAndMakeVisible(destinationRepoList);

	addAndMakeVisible(buttonPanel);

	addAndMakeVisible(aboutButton);
	aboutButton.onClick = [this] {aboutButtonClicked(); 

	};


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

void MainComponent::paint(Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

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

void MainComponent::setDebugText(String textToDisplay,bool flash)
{
	if (flash)
	{
		timerFlashCount = 0;
		startTimer(50);
	}
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
	}

}

void MainComponent::aboutButtonClicked()
{
	auto aboutWindow = new AboutWindow ("About");
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

double MainComponent::getTargetSampleRate()
{

		int SRSelection = buttonPanel.SRMenu.getSelectedItemIndex() +1;

		switch (SRSelection)
		{
		case 1: return 22050.00;
		case 2: return 44100.00;
		case 3: return 48000.00;
		case 4: return 96000.00;
		}
	
}


void MainComponent::resized()
{
	int border = 10;
	int bottomPanelHeight = 30;
	Rectangle<int> area (0,border,getWidth(),(getHeight()-border)-bottomPanelHeight);
	Rectangle<int> bottomPanel(0, area.getHeight()+border, getWidth(), bottomPanelHeight);
	thumbnailComponent.setBounds(area.removeFromTop(100));
	positionOverlay.setBounds(thumbnailComponent.getBounds());
	buttonPanel.setBounds(area.removeFromTop(20));
	aboutButton.setBounds(bottomPanel.removeFromRight(75));
	debugLabel.setBounds(bottomPanel.removeFromLeft(getWidth() - aboutButton.getWidth()));
	localTableList.setBounds(area.removeFromLeft(getWidth() / 2));
	destinationRepoList.setBounds(area.removeFromLeft(localTableList.getWidth()));
}
