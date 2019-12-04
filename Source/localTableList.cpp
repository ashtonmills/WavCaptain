/*
  ==============================================================================

    localTableList.cpp
    Created: 2 Dec 2019 12:21:43pm
    Author:  ashton_mills

  ==============================================================================
*/

#include "MainComponent.h"
#include "localTableList.h"


LocalTableList::LocalTableList(MainComponent& mc, String chooseButtonText) : mainComp (mc), loadDirButton(chooseButtonText)
{
	setSize(500, 750);
	addAndMakeVisible(loadDirButton);
	loadDirButton.onClick = [this] {chooseDir(); };

	addAndMakeVisible(table);
	//loadData();

	table.setRowHeight(25);
	formatManager.registerBasicFormats();
}


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
	auto dirButtonHeight = 30;
	loadDirButton.setBounds(area.removeFromTop(dirButtonHeight));
	table.setBounds(area.removeFromTop(getHeight() - dirButtonHeight));
}

void LocalTableList::debugLabelMsg(String message)
{
	debugLabel.setText(message, dontSendNotification);
}

File LocalTableList::makeXml(File& localDir)
{
	XmlElement localDirXml("LOCALDIR");

	//Generate the table headers

	XmlElement* header = new XmlElement("HEADERS");
	localDirXml.addChildElement(header);

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

	XmlElement* column5 = new XmlElement("COLUMN");
	column5->setAttribute("columnId", "5");
	column5->setAttribute("name", "select");
	column5->setAttribute("width", "50");
	header->addChildElement(column5);


	//iterate through the files in directory and add data to Xml


	XmlElement* data = new XmlElement("DATA");
	localDirXml.addChildElement(data);


	 localDirWavs = localDir.findChildFiles(File::TypesOfFileToFind::findFiles, false, "*.wav");

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

	File localDirDataFile = File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory().getChildFile("Resources").getChildFile("localDirData.xml");
	localDirXml.writeTo(localDirDataFile);

	return localDirDataFile;
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
				localDirWavs[row].copyFileTo(mainComp.destinationRepoList.directory.getChildFile(getText(1,row)));
				filesCopied++;
			}

		}
	}
	if (filesCopied == 0)
	{
		if (!bDeployingAll) { mainComp.setDebugText("No files selected"); }
		else { mainComp.setDebugText("No files in local directory to copy");}

	}
	else
	{
		//String message = message.formatted("Copied %s files to %s.", filesCopied, mainComp.destinationRepoList.directory.getFileName());

		String message = "Copied ";
		message += filesCopied;
		message += " files to ";
		message += mainComp.destinationRepoList.directory.getFullPathName();
		mainComp.setDebugText(message);
		mainComp.destinationRepoList.loadData();
	}

	
	//if Repo directory is not selected
		//run the choose directoy function on the repo
		

	// For loop cells in table
		//if checkbox is selected
			//copy to the repo directory
	
			//if number of checkboxes ticked on lical list is > 0 
		//popup no files selected to deploy
		//return
}

//TODO make a function for copying ALL files, not just selected ones. 

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

void LocalTableList::loadData()
{
	int numTries = 0;

	//	while (!dir.getChildFile("Resources").exists() && numTries++ < 15)
	//		dir = dir.getParentDirectory();

	//	auto tableFile = dir.getChildFile("Resources").getChildFile("localDirData.xml");
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
}


void LocalTableList::chooseDir()
{
	File initDir = File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory);
	FileChooser directoryChooser("Choose local directory", initDir);
	if (directoryChooser.browseForDirectory())
		directory = directoryChooser.getResult();
	loadData();
}