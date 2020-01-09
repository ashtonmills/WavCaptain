/*
  ==============================================================================

    localTableList.cpp
    Created: 2 Dec 2019 12:21:43pm
    Author:  ashton_mills

  ==============================================================================
*/

#include "MainComponent.h"
#include "localTableList.h"


LocalTableList::LocalTableList(MainComponent& mc, String chooseButtonText, bool isLeftPanel) : mainComp (mc), loadDirButton(chooseButtonText), bIsLeftPanel(isLeftPanel)
{
	//set the current working directory for the program and add a resources folder. 
	File workingDir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getChildFile("wavCaptain");
	if (!workingDir.isDirectory())
	{
		workingDir.createDirectory();
	}
	workingDir.setAsCurrentWorkingDirectory();
	File resourcesDir = File::getCurrentWorkingDirectory().getChildFile("Resources");
	if (!resourcesDir.isDirectory())
	{
	resourcesDir.createDirectory();
	}

	setSize(500, 750);
	addAndMakeVisible(loadDirButton);
	loadDirButton.onClick = [this] {chooseDir(); };

	refreshButton.onClick = [this] {refreshButtonClicked(); };
	refreshButton.setLookAndFeel(&unicodeLookAndFeel);
	addAndMakeVisible(refreshButton);

	addAndMakeVisible(table);


	table.setRowHeight(25);
	formatManager.registerBasicFormats();

	initDirectoryLoad();
	if (directory.exists())
	{
		loadData(true);
	}
}

LocalTableList::~LocalTableList()
{
	refreshButton.setLookAndFeel(nullptr);
}

void LocalTableList::initDirectoryLoad()
{
	//TODO get ride of these afterwards? make them smart pointers?
	XmlElement* saveDirColumnList = nullptr;
	XmlElement* saveDirDataList = nullptr;
	

	File savedDirFile = File::getCurrentWorkingDirectory().getChildFile("Resources").getChildFile("savedDirectories.xml");
	if (savedDirFile.exists())
	{
		savedData = XmlDocument::parse(savedDirFile);
		saveDirColumnList = savedData->getChildByName("HEADERS");
		saveDirDataList = savedData->getChildByName("DATA");
		numRows = savedData->getNumChildElements();
		forEachXmlChildElement(*saveDirDataList, child)
		{
			String cellData = String(child->getAttributeValue(0));
			if (cellData.isNotEmpty())
			{
				File dirFile = File(cellData);
				if ((child->getAttributeValue(1) == "Local") && (bIsLeftPanel))
				{
					directory = dirFile;
					//loadDirButton.setButtonText(cellData);
					break;
				}
				if ((child->getAttributeValue(1) == "Repo") && (!bIsLeftPanel))
				{
					directory = dirFile;
					//loadData(false);
					break;
				}

			}
		}

	}
}

	void LocalTableList::loadData(bool isInitLoad)
	{
		//	while (!dir.getChildFile("Resources").exists() && numTries++ < 15)
		//		dir = dir.getParentDirectory();

		//	auto tableFile = dir.getChildFile("Resources").getChildFile("localDirData.xml");

		table.getHeader().removeAllColumns();
		auto tableFile = makeXml(directory);

		if (tableFile.exists())
		{
			playlistData = XmlDocument::parse(tableFile);
			columnList = playlistData->getChildByName("HEADERS");
			dataList = playlistData->getChildByName("DATA");


			numRows = dataList->getNumChildElements();
		}

		table.setColour(ListBox::outlineColourId, Colours::lightgrey);
		table.setOutlineThickness(1);

		if (columnList != nullptr)
		{
			forEachXmlChildElement(*columnList, columnXml)
			{
				table.getHeader().addColumn(columnXml->getStringAttribute("name"),
					columnXml->getIntAttribute("columnId"),
					columnXml->getIntAttribute("width"), 50, 400, TableHeaderComponent::defaultFlags);
			}
		}


		/*	addAndMakeVisible(debugLabel);
			debugLabel.setText("default debug message", dontSendNotification);*/
		table.setMultipleSelectionEnabled(true);
		if (!isInitLoad)
		{
			mainComp.saveData();
		}
	}
//	loadData(false);  maybe we want to call load load data or maybe just handle it seperatly as it's the init load

//	Or we overLoad loadData(false)!!  Do a sencind version of load data that takes a direcotry paramter and misses out all of the stuff to do with finding the file. 

//or or we just separate the load data function, picking the directory and loading the data are separate concerns. 

	// I do that anyway lol. don't need to change anything. 


int LocalTableList::getNumRows()
{
	return numRows;
}


void LocalTableList::paintRowBackground(Graphics& g, int rowNumber, int, int, bool rowIsSelected)
{
	auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
		.interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
	if (rowIsSelected)
	{
		g.fillAll(Colours::lightblue);
	}
	else if (rowNumber % 2)
		g.fillAll(alternateColour);
}

void LocalTableList::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) 
{
	g.setColour(rowIsSelected ? Colours::darkblue : getLookAndFeel().findColour(ListBox::textColourId)); // [5]
	g.setFont(font);
	if (auto* rowElement = dataList->getChildElement(rowNumber))
	{
		auto text = rowElement->getStringAttribute(getAttributeNameForColumnId(columnId));
		g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true); // [6]
	}
	g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
	g.fillRect(width - 1, 0, 1, height);                                             // [7]
}

Component* LocalTableList::refreshComponentForCell(int rowNumber, int columnId, bool, Component* existingComponentToUpdate)
{
	if (columnId == 5)
	{
		auto* mySelectionBox = dynamic_cast<SelectionColumnCustomComponent*>(existingComponentToUpdate);

		if (mySelectionBox == nullptr)
		{
			mySelectionBox = new SelectionColumnCustomComponent(*this);
		}

		mySelectionBox->setRowAndColumn(rowNumber, columnId);
		return mySelectionBox;
	}

	//if (columnId == 1)
	//{
	//	auto* textLabel = dynamic_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

	//	if (textLabel == nullptr)
	//	{
	//		textLabel = new EditableTextCustomComponent(*this);
	//	}

	//	textLabel->setRowAndColumn(rowNumber, columnId);
	//	return textLabel;
	//}

	jassert(existingComponentToUpdate == nullptr);
	return nullptr;
}

int LocalTableList::getColumnAutoSizeWidth(int columnId)
{
	if (columnId == 9)
		return 50;

	int widest = 32;

	for (auto i = getNumRows(); --i >= 0;)
	{
		if (auto* rowElement = dataList->getChildElement(i))
		{
			auto text = rowElement->getStringAttribute(getAttributeNameForColumnId(columnId));

			widest = jmax(widest, font.getStringWidth(text));
		}
	}

	return widest + 8;

}

int LocalTableList::getSelection(const int rowNumber) const
{
	return dataList->getChildElement(rowNumber)->getIntAttribute("Select");
}

void LocalTableList::setSelection(const int rowNumber, const int newSelection)
{
	dataList->getChildElement(rowNumber)->setAttribute("Select", newSelection);
}

String LocalTableList::getText(const int columnNumber, const int rowNumber) const
{
	return dataList->getChildElement(rowNumber)->getStringAttribute(getAttributeNameForColumnId(columnNumber));
}

void LocalTableList::setText(const int columnNumber, const int rowNumber, const String& newText) const
{
	const auto& columnName = table.getHeader().getColumnName(columnNumber);
	dataList->getChildElement(rowNumber)->setAttribute(columnName, newText);
}

void LocalTableList::resized()
{
	auto area = getLocalBounds();
	auto buttonPanelHeight = 30;
	auto buttonPanel = area.removeFromTop(buttonPanelHeight);
	loadDirButton.setBounds(buttonPanel.removeFromLeft(getWidth()-30));
	refreshButton.setBounds(buttonPanel.removeFromLeft(30));
	table.setBounds(area.removeFromTop(getHeight() - buttonPanelHeight));
}

File LocalTableList::getDirectory()
{
	return directory;
}

void LocalTableList::debugLabelMsg(String message)
{
	debugLabel.setText(message, dontSendNotification);
}

File LocalTableList::makeXml(File& dir)
{
	XmlElement dirXml("DIR");

	//Generate the table headers

	XmlElement* header = new XmlElement("HEADERS");
	dirXml.addChildElement(header);

	XmlElement* column1 = new XmlElement("COLUMN");
	column1->setAttribute("columnId", "1");
	column1->setAttribute("name", "FileName");
	column1->setAttribute("width", "200");
	header->addChildElement(column1);

	XmlElement* column2 = new XmlElement("COLUMN");
	column2->setAttribute("columnId", "2");
	column2->setAttribute("name", "SampleRate");
	column2->setAttribute("width", "100");
	header->addChildElement(column2);

	XmlElement* column3 = new XmlElement("COLUMN");
	column3->setAttribute("columnId", "3");
	column3->setAttribute("name", "Channels");
	column3->setAttribute("width", "70");
	header->addChildElement(column3);

	XmlElement* column4 = new XmlElement("COLUMN");
	column4->setAttribute("columnId", "4");
	column4->setAttribute("name", "DateModified");
	column4->setAttribute("width", "150");
	header->addChildElement(column4);

	if (bIsLeftPanel)
	{
		XmlElement* column5 = new XmlElement("COLUMN");
		column5->setAttribute("columnId", "5");
		column5->setAttribute("name", "select");
		column5->setAttribute("width", "50");
		header->addChildElement(column5);
	}


	//iterate through the files in directory and add data to Xml


	XmlElement* data = new XmlElement("DATA");
	dirXml.addChildElement(data);


	 localDirWavs = dir.findChildFiles(File::TypesOfFileToFind::findFiles, false, "*.wav");

	for (int i = 0; i < localDirWavs.size(); ++i)
	{
		XmlElement* file = new XmlElement("FILE");
		file->setAttribute("FileName", localDirWavs[i].getFileName());
		file->setAttribute("DateModified", localDirWavs[i].getLastModificationTime().toString(true, true, false, true));
		auto reader = std::unique_ptr<AudioFormatReader>(formatManager.createReaderFor(localDirWavs[i]));
		if (reader)
		{
			float fileSampleRate = reader->sampleRate / 1000;
			String sampleRateString(fileSampleRate);
			sampleRateString += " khz";
			String numChannels;
			if (reader->numChannels > 1) numChannels = "Stereo";
			else numChannels = "Mono";
			file->setAttribute("Channels", numChannels);
			file->setAttribute("SampleRate", sampleRateString);
			data->addChildElement(file);
		}

	}


	if (bIsLeftPanel)
	{
		File localDirDataFile = File::getCurrentWorkingDirectory().getChildFile("Resources").getChildFile("localDirData.xml");
		dirXml.writeTo(localDirDataFile);
		return localDirDataFile;
	}
	else
	{
		File repoDirDataFile = File::getCurrentWorkingDirectory().getChildFile("Resources").getChildFile("repoDirData.xml");
		dirXml.writeTo(repoDirDataFile);
		return repoDirDataFile;
	}
}

void LocalTableList::deploySelectedFiles(bool bDeployingAll)
{
	mainComp.setDebugText("deployFiles() called on local table");
	if (!directory.exists())
	{
		mainComp.setDebugText("You haven't selected a source directory yet mate. Click 'Source Directory' to select one");
		return;
	}

	if (!mainComp.destinationRepoList.directory.exists())
	{
		mainComp.destinationRepoList.chooseDir();
	}
	int filesCopied = 0;

	for (int row = 0; row < table.getNumRows(); ++row)
	{
		if ((getSelection(row) != 0) || (bDeployingAll))
		{
			if (mainComp.destinationRepoList.directory.exists())
			{
				localDirWavs[row].copyFileTo(mainComp.destinationRepoList.directory.getChildFile(getText(1, row)));
				filesCopied++;
			}

		}
	}
	if (filesCopied == 0)
	{
		if (!bDeployingAll) { mainComp.setDebugText("No files selected"); }
		else { mainComp.setDebugText("No files in local directory to copy"); }

	}
	else
	{
		//String message = message.formatted("Copied %s files to %s.", filesCopied, mainComp.destinationRepoList.directory.getFileName());

		String message = "Copied ";
		message += filesCopied;
		message += " files to ";
		message += mainComp.destinationRepoList.directory.getFullPathName();
		mainComp.setDebugText(message);
		mainComp.destinationRepoList.loadData(false);
	}
}




void LocalTableList::convertSampleRate()
{
	//create popup menu to decide whether we overiwite or keep original files
	PopupMenu popup;
	popup.addItem(1, "Overwite Existing Files");
	popup.addItem(2, "Create Subfolder for new files");
	popup.addItem(3, "Append names of new files");
	const int popupResult = popup.show();
	//if user picks nothing from this menu cancel the function
	if (popupResult == 0) return;

	if (!directory.exists())
	{
		mainComp.setDebugText("No source directory selected");
		return;
	}
	int targetSampleRate = mainComp.getTargetSampleRate();

	for (int row = 0; row < table.getNumRows(); ++row)
	{
		if (getSelection(row) != 0)
		{
			//start off just converting first file in the directory, then we'll work out the selection boxes and which ones to convert etc
			File targetFile = localDirWavs[row];
			//make reader to load the file
			AudioFormatReader* reader = formatManager.createReaderFor(targetFile);
			//check the sample rate of the file and skip over this iteration if it's already the sample rate we want
			unsigned int initialSampleRate = reader->sampleRate;
			if (initialSampleRate != targetSampleRate)
			{
				unsigned int numChans = reader->numChannels;
				unsigned int numSamples = reader->lengthInSamples;
				double SRRatio = initialSampleRate / targetSampleRate;
				//create an audiosource that can read from the reader, read from the reader then delete the reader
				AudioFormatReaderSource* newSource = new AudioFormatReaderSource(reader, true);
				//create a resamping audiosource from the audiosource then delete the one that you passed in 
				auto resamplingAudioSource = std::make_unique<ResamplingAudioSource>(newSource, true, reader->numChannels);
			//	ResamplingAudioSource* resamplingAudioSource = new ResamplingAudioSource(newSource, true, reader->numChannels);
				resamplingAudioSource->setResamplingRatio(SRRatio);
				File targetDestination;
				File tempFile = File(File::getCurrentWorkingDirectory().getChildFile("wavCapTemp.wav"));
				switch (popupResult)
				{
				case 0: return;
				case 1: targetDestination = File(localDirWavs[row].getFullPathName());//overwite the file with the new sample rate version
					break;
				case 2: targetDestination = directory.getChildFile("resampled_"+ std::to_string(targetSampleRate)).getChildFile(localDirWavs[row].getFileName()); //create a subfolder in this directory named the sample rate they've been named to 
					break; 
				case 3: targetDestination = File(localDirWavs[row].getParentDirectory().getFullPathName() + "/"+ localDirWavs[row].getFileNameWithoutExtension() + "-" + std::to_string(targetSampleRate) + ".wav"); //make a new file in this directory with the sample rate appending the name
					break;
				}
				auto fos = std::unique_ptr<FileOutputStream>(tempFile.createOutputStream());
				auto writer = std::unique_ptr<AudioFormatWriter>  (WavAudioFormat().createWriterFor(fos.release(), targetSampleRate, numChans, 16, StringPairArray(), 0)); //note here that to be able to use the unique_ptr as a parameter I need to do .get()
				resamplingAudioSource->prepareToPlay(512, targetSampleRate);
				writer->writeFromAudioSource(*resamplingAudioSource, numSamples / SRRatio);
				File moveFile = File(File::getCurrentWorkingDirectory().getChildFile("wavCapTemp.wav"));
				moveFile.moveFileTo(targetDestination.getParentDirectory());
				mainComp.setDebugText("tagetDestination is " + moveFile.getFullPathName());
			}
		}
	}

}




void LocalTableList::cellClicked(int rowNumber, int columnId, const MouseEvent&)
{
	mainComp.readFile(localDirWavs[rowNumber]);
	mainComp.play();
}

String LocalTableList::getAttributeNameForColumnId(const int columnId) const
{
	forEachXmlChildElement(*columnList, columnXml)
	{
		if (columnXml->getIntAttribute("columnId") == columnId)
			return columnXml->getStringAttribute("name");
	}
	return{};
}


void LocalTableList::chooseDir()
{
	File initDir = File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory);
	FileChooser directoryChooser("Choose directory", initDir);
	if (directoryChooser.browseForDirectory())
		directory = directoryChooser.getResult();
	if (directory.exists())
	{
		loadData(false);
	}
}

void LocalTableList::filesDropped(const StringArray& files, int x, int y)
{
	if (isInterestedInFileDrag(files))
	{
		int iterator = 0;
		for (int i = 0; i < files.size(); ++i)
		{
			File iFile = File(files[0]);
			if (iFile.isDirectory())
			{
				directory = iFile;
				loadData(false);
				break;
			}
			else if (iFile.getFileExtension() == ".wav")
			{
				mainComp.readFile(iFile);
				mainComp.play();
				directory = iFile.getParentDirectory();
				loadData(false);
				break;
			}
			iterator++;
		}
		if (iterator == files.size())
		{
			mainComp.setDebugText("No .wav files dropped");
		}
	}
}

bool LocalTableList::isInterestedInFileDrag(const StringArray& files)
{
	for (int i = 0; i < files.size(); ++i)
	{
		File iFile = File(files[i]);
	 if	((iFile.isDirectory()) && (iFile.findChildFiles(File::TypesOfFileToFind::findFiles, false, "*.wav").size()>0))
		{
		 return true;
		}

	 else if (iFile.getFileExtension()==".wav")
	 {
		 return true;
	 }
	 else return false;
	}
}


void LocalTableList::refreshButtonClicked()
{
	loadData(false);
}
