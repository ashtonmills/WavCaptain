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


//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/


class LocalTableList : public Component,
	public TableListBoxModel
{
public:
	LocalTableList() : loadDirButton ("choose local directory")
	{
		setSize(400, 750);
		addAndMakeVisible(loadDirButton);
		loadDirButton.onClick = [this] {loadData(); };
		
		addAndMakeVisible(table);
		//loadData();

		table.setRowHeight(25);
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
			g.fillAll(Colours::lightblue);
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
		loadDirButton.setBounds(5, 5, getWidth()-10, 30);
	//	debugLabel.setBounds(100, 500, 500, 30);
		table.setBounds(0, 50, 400, 700);
	}

	void debugLabelMsg(String message)
	{
		debugLabel.setText(message, dontSendNotification);
	}

	void makeTreeListXml() 
	{
		XmlElement plantList ("PLANTS");

		//headers

	    XmlElement* header = new XmlElement("HEADERS");
		plantList.addChildElement(header);

		XmlElement* column1 = new XmlElement("COLUMN");
		column1->setAttribute("columnId", "1");
		column1->setAttribute("name", "ID");
		column1->setAttribute("width", "50");
		header->addChildElement(column1);

		XmlElement* column2 = new XmlElement("COLUMN");
		column2->setAttribute("columnId", "2");
		column2->setAttribute("name", "NAME");
		column2->setAttribute("width", "150");
		header->addChildElement(column2);

		XmlElement* column3 = new XmlElement("COLUMN");
		column3->setAttribute("columnId", "3");
		column3->setAttribute("name", "family");
		column3->setAttribute("width", "150");
		header->addChildElement(column3);

		XmlElement* column4 = new XmlElement("COLUMN");
		column4->setAttribute("columnId", "4");
		column4->setAttribute("name", "colour");
		column4->setAttribute("width", "150");
		header->addChildElement(column4);

		XmlElement* column5 = new XmlElement("COLUMN");
		column5->setAttribute("columnId", "5");
		column5->setAttribute("name", "type");
		column5->setAttribute("width", "150");
		header->addChildElement(column5);

		XmlElement* column6 = new XmlElement("COLUMN");
		column6->setAttribute("columnId", "6");
		column6->setAttribute("name", "select");
		column6->setAttribute("width", "50");
		header->addChildElement(column6);



		//data

		XmlElement* data = new XmlElement("DATA");
		plantList.addChildElement(data);

		XmlElement* pumpkin1 = new XmlElement("PUMPKIN");
		pumpkin1->setAttribute("NAME", "Pumpkin");
		pumpkin1->setAttribute("family", "cucurbit");
		pumpkin1->setAttribute("colour", "orange");
		pumpkin1->setAttribute("type", "veg");
		data->addChildElement(pumpkin1);

		XmlElement* pumpkin2 = new XmlElement("PUMPKIN");
		pumpkin2->setAttribute("NAME", "Pumpkin");
		pumpkin2->setAttribute("family", "halloweeny");
		pumpkin2->setAttribute("colour", "green");
		pumpkin2->setAttribute("type", "zombie");
		data->addChildElement(pumpkin2);

		XmlElement* holly = new XmlElement("HOLLY");
		holly->setAttribute("NAME", "Holly");
		holly->setAttribute("family", "aquifoliaceae");
		holly->setAttribute("colour", "green");
		holly->setAttribute("type", "tree");
		data->addChildElement(holly);

		File plantListFile = File::getCurrentWorkingDirectory().getChildFile("plantList.xml");
		plantList.writeTo(plantListFile);

	}

	File makeLocalXml(File& localDir)
	{
		XmlElement localDirXml("LOCALDIR");

		//headers

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

		//XmlElement* column6 = new XmlElement("COLUMN");
		//column6->setAttribute("columnId", "6");
		//column6->setAttribute("name", "select");
		//column6->setAttribute("width", "50");
		//header->addChildElement(column6);



		//data



		XmlElement* data = new XmlElement("DATA");
		localDirXml.addChildElement(data);

		//XmlElement* file1 = new XmlElement("FILE");
		//file1->setAttribute("FileName", "Whoosh_01");
		//file1->setAttribute("SampleRate", "44100");
		//file1->setAttribute("Channels", "2");
		//file1->setAttribute("DateModified", "30/10/1987");
		//file1->setAttribute("select", "0");
		//data->addChildElement(file1);

		auto localDirFileArray = localDir.findChildFiles(File::TypesOfFileToFind::findFiles, false, "*.wav");

		for (int i = 0; i < localDirFileArray.size(); ++i)
		{
			XmlElement* file  = new XmlElement("FILE");
			file->setAttribute("FileName", localDirFileArray[i].getFileNameWithoutExtension());
			file->setAttribute("DateModified", localDirFileArray[i].getLastModificationTime().toString(true,true,false,true));
			data->addChildElement(file);

		}

		//XmlElement* pumpkin1 = new XmlElement("PUMPKIN");
		//pumpkin1->setAttribute("NAME", "Pumpkin");
		//pumpkin1->setAttribute("family", "cucurbit");
		//pumpkin1->setAttribute("colour", "orange");
		//pumpkin1->setAttribute("type", "veg");
		//data->addChildElement(pumpkin1);

		//XmlElement* pumpkin2 = new XmlElement("PUMPKIN");
		//pumpkin2->setAttribute("NAME", "Pumpkin");
		//pumpkin2->setAttribute("family", "halloweeny");
		//pumpkin2->setAttribute("colour", "green");
		//pumpkin2->setAttribute("type", "zombie");
		//data->addChildElement(pumpkin2);

		//XmlElement* holly = new XmlElement("HOLLY");
		//holly->setAttribute("NAME", "Holly");
		//holly->setAttribute("family", "aquifoliaceae");
		//holly->setAttribute("colour", "green");
		//holly->setAttribute("type", "tree");
		//data->addChildElement(holly);

		File localDirDataFile = File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory().getChildFile("Resources").getChildFile("localDirData.xml");
		localDirXml.writeTo(localDirDataFile);

		return localDirDataFile;
	}

private:
	//member variables
	TableListBox table{ {}, this };
	Font font{ 14.0 };
	std::unique_ptr<XmlElement> playlistData;
	XmlElement* columnList = nullptr;
	XmlElement* dataList = nullptr;
	int numRows = 0;
	Label debugLabel;
	TextButton loadDirButton;
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
		FileChooser directoryChooser("Choose local directory",initDir);
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



