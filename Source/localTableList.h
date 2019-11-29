/*
  ==============================================================================

    localTableList.h
    Created: 27 Nov 2019 1:16:44pm
    Author:  ashton_mills

  ==============================================================================
*/

#pragma once
/*
  ==============================================================================

	This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"


//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/


class LocalTableList : public Component,
	public TableListBoxModel
{
public:
	LocalTableList(String chooseButtonText) : loadDirButton(chooseButtonText)
	{
		setSize(500, 750);
		addAndMakeVisible(loadDirButton);
		loadDirButton.onClick = [this] {loadData(); };

		addAndMakeVisible(table);
		//loadData();

		table.setRowHeight(25);
		formatManager.registerBasicFormats();
	}

	int getNumRows() override
	{
		return numRows;
	}

	void paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
	{
		auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
			.interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
		if (rowIsSelected)
		{
			g.fillAll(Colours::lightblue);
			if (parent)
				parent->readFile(localDirWavs[rowNumber - 1]);
		}
		else if (rowNumber % 2)
			g.fillAll(alternateColour);
	}

	void paintCell(Graphics& g, int rowNumber, int columnId,
		int width, int height, bool rowIsSelected) override
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


	Component* refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/,
		Component* existingComponentToUpdate) override
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

		if (columnId == 1)
		{
			auto* textLabel = dynamic_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

			if (textLabel == nullptr)
			{
				textLabel = new EditableTextCustomComponent(*this);
			}

			textLabel->setRowAndColumn(rowNumber, columnId);
			return textLabel;
		}

		jassert(existingComponentToUpdate == nullptr);
		return nullptr;
	}

	int getColumnAutoSizeWidth(int columnId) override
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


	int getSelection(const int rowNumber) const
	{
		return dataList->getChildElement(rowNumber)->getIntAttribute("Select");
	}

	void setSelection(const int rowNumber, const int newSelection)
	{
		dataList->getChildElement(rowNumber)->setAttribute("Select", newSelection);
	}

	String getText(const int columnNumber, const int rowNumber) const
	{
		return dataList->getChildElement(rowNumber)->getStringAttribute(getAttributeNameForColumnId(columnNumber));
	}

	void setText(const int columnNumber, const int rowNumber, const String& newText) const
	{
		const auto& columnName = table.getHeader().getColumnName(columnNumber);
		dataList->getChildElement(rowNumber)->setAttribute(columnName, newText);
	}

	void resized() override
	{
		auto area = getLocalBounds();
		auto dirButtonHeight = 30;
		loadDirButton.setBounds(area.removeFromTop(dirButtonHeight));
		//loadDirButton.setBounds(5, 5, getWidth()-10, 30);
	//	debugLabel.setBounds(100, 500, 500, 30);
		table.setBounds(area.removeFromTop(getHeight() - dirButtonHeight));
	}

	void debugLabelMsg(String message)
	{
		debugLabel.setText(message, dontSendNotification);
	}

	File makeLocalXml(File& localDir)
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
		column4->setAttribute("width", "200");
		header->addChildElement(column4);

		XmlElement* column5 = new XmlElement("COLUMN");
		column5->setAttribute("columnId", "5");
		column5->setAttribute("name", "select");
		column5->setAttribute("width", "50");
		header->addChildElement(column5);


		//iterate through the files in directory and add data to Xml


		XmlElement* data = new XmlElement("DATA");
		localDirXml.addChildElement(data);


		auto localDirWavs = localDir.findChildFiles(File::TypesOfFileToFind::findFiles, false, "*.wav");

		for (int i = 0; i < localDirWavs.size(); ++i)
		{
			XmlElement* file = new XmlElement("FILE");
			file->setAttribute("FileName", localDirWavs[i].getFileNameWithoutExtension());
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

private:
	//member variables
	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReaderSource> playSource;
	TableListBox table{ {}, this };
	Font font{ 14.0 };
	std::unique_ptr<XmlElement> playlistData;
	XmlElement* columnList = nullptr;
	XmlElement* dataList = nullptr;
	int numRows = 0;
	Label debugLabel;
	TextButton loadDirButton;
	Array<File> localDirWavs;
	Array<File> repoDirWavs;
	MainComponent* parent = dynamic_cast<MainComponent*>(getParentComponent());
	//==========================================================================
	class EditableTextCustomComponent : public Label
	{
	public:
		EditableTextCustomComponent(LocalTableList& td)
			:owner(td)
		{
			setEditable(false, true, false);
		}

		void mouseDown(const MouseEvent& event) override
		{
			owner.table.selectRowsBasedOnModifierKeys(row, event.mods, false);
			Label::mouseDown(event);
		}

		void textWasEdited()override
		{
			owner.setText(columnId, row, getText());
		}

		void setRowAndColumn(const int newRow, const int newColumn)
		{
			row = newRow;
			columnId = newColumn;
			setText(owner.getText(columnId, row), dontSendNotification);
		}

	private:
		LocalTableList& owner;
		int row, columnId;
		Colour textColour;
	};

	class SelectionColumnCustomComponent : public Component
	{
	public:
		SelectionColumnCustomComponent(LocalTableList& td)
			: owner(td)
		{
			addAndMakeVisible(toggleButton);

			toggleButton.onClick = [this] { owner.setSelection(row, (int)toggleButton.getToggleState()); };
		}

		void resized() override
		{
			toggleButton.setBoundsInset(BorderSize<int>(2));
		}

		void setRowAndColumn(int newRow, int newColumn)
		{
			row = newRow;
			columnId = newColumn;
			toggleButton.setToggleState((bool)owner.getSelection(row), dontSendNotification);
		}

	private:
		LocalTableList& owner;
		ToggleButton toggleButton;
		int row, columnId;
	};

	void loadData()
	{
		File initDir = File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory);
		FileChooser directoryChooser("Choose local directory", initDir);
		File dir;
		if (directoryChooser.browseForDirectory())
			dir = directoryChooser.getResult();



		int numTries = 0;

		//	while (!dir.getChildFile("Resources").exists() && numTries++ < 15)
		//		dir = dir.getParentDirectory();

		//	auto tableFile = dir.getChildFile("Resources").getChildFile("localDirData.xml");
		auto tableFile = makeLocalXml(dir);

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

	String getAttributeNameForColumnId(const int columnId) const
	{
		forEachXmlChildElement(*columnList, columnXml)
		{
			if (columnXml->getIntAttribute("columnId") == columnId)
				return columnXml->getStringAttribute("name");
		}
		return{};
	}


};



