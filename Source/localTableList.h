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
		setSize(600, 800);
		addAndMakeVisible(loadDirButton);
		loadDirButton.onClick = [this] {loadData(); };
		
		addAndMakeVisible(table);
		//loadData();
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
		if (columnId == 4)
		{
			auto* mySelectionBox = dynamic_cast<SelectionColumnCustomComponent*>(existingComponentToUpdate);

			if (mySelectionBox == nullptr)
			{
				mySelectionBox = new SelectionColumnCustomComponent(*this);
			}

			mySelectionBox->setRowAndColumn(rowNumber, columnId);
			return mySelectionBox;
		}

		if (columnId == 3)
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
		table.setBounds(5, 50, 590, 745);
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



		//data

		XmlElement* data = new XmlElement("DATA");
		plantList.addChildElement(data);

		XmlElement* pumpkin1 = new XmlElement("PUMPKIN");
		pumpkin1->setAttribute("name", "Pumpkin");
		pumpkin1->setAttribute("family", "cucurbit");
		pumpkin1->setAttribute("colour", "orange");
		pumpkin1->setAttribute("type", "veg");
		data->addChildElement(pumpkin1);

		XmlElement* pumpkin2 = new XmlElement("PUMPKIN");
		pumpkin2->setAttribute("name", "Pumpkin");
		pumpkin2->setAttribute("family", "halloweeny");
		pumpkin2->setAttribute("colour", "green");
		pumpkin2->setAttribute("type", "zombie");
		data->addChildElement(pumpkin2);

		XmlElement* holly = new XmlElement("HOLLY");
		holly->setAttribute("name", "Holly");
		holly->setAttribute("family", "aquifoliaceae");
		holly->setAttribute("colour", "green");
		holly->setAttribute("type", "tree");
		data->addChildElement(holly);

		File plantListFile = File::getCurrentWorkingDirectory().getChildFile("plantList.xml");
		plantList.writeTo(plantListFile);

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
		makeTreeListXml();
		FileChooser directoryChooser("Choose local directory");
		File dir;
		if (directoryChooser.browseForDirectory())
			 dir = directoryChooser.getResult();

		int numTries = 0;

		while (!dir.getChildFile("Resources").exists() && numTries++ < 15)
			dir = dir.getParentDirectory();

		auto tableFile = dir.getChildFile("Resources").getChildFile("plantList.xml");
	

		if (tableFile.exists())
		{
			playlistData = XmlDocument::parse(tableFile);
			dataList = playlistData->getChildByName("DATA");
			columnList = playlistData->getChildByName("HEADERS");

			numRows = dataList->getNumChildElements();
		}
		else
		{
			debugLabelMsg("no such file you nob");
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


		//addAndMakeVisible(debugLabel);
		//debugLabel.setText("default debug message", dontSendNotification);
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



