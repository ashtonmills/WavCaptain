/*
  ==============================================================================

    localTableList.h
    Created: 27 Nov 2019 1:16:44pm
    Author:  ashton_mills

  ==============================================================================
*/

#pragma once

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ValTreeIds.h"

class MainComponent;


class LocalTableList : public Component, public FileDragAndDropTarget,
	public TableListBoxModel, public MouseListener, public ValueTree::Listener, public DragAndDropContainer
{
public:
	LocalTableList(MainComponent& mc, ValueTree vt, String chooseButtonText, bool isLeftPanel,String sInitFile);
	~LocalTableList();
	
	void populateValTree();
	void loadData(bool isInitLoad);
	void LocalTableList::initDirectoryLoad();
	int getNumRows() override;
	void paintRowBackground(Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;

	void paintCell(Graphics& g, int rowNumber, int columnId,
		int width, int height, bool rowIsSelected) override;


	Component* refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/,
		Component* existingComponentToUpdate) override;

	int getColumnAutoSizeWidth(int columnId) override;
	int getSelection(const int rowNumber) const;
	void setSelection(const int rowNumber, const int newSelection);
	String getText(const int columnNumber, const int rowNumber) const;
	void setText(const int columnNumber, const int rowNumber, const String& newText) const;
	void resized() override;
	File getDirectory();
	void debugLabelMsg(String message);
	File makeXml(File& localDir);
	void convertSampleRate();
	void chooseDir();
	void deploySelectedFiles(bool bDeployingAll);
	String getAttributeNameForColumnId(const int columnId) const;
	void cellClicked(int rowNumber, int columnId, const MouseEvent&);
	void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent&);
	void filesDropped(const StringArray& files, int x, int y) override;
	bool isInterestedInFileDrag(const StringArray& files) override;
	void LocalTableList::refreshButtonClicked();
	void backFolderButtonClicked();
	void openInExplorerButtonClicked();
	void openInExplorerButtonMouseEnter();
	void mouseEnter(const MouseEvent& event) override;
	void mouseExit(const MouseEvent& event) override;
	void valueTreePropertyChanged(ValueTree& tree,
		const Identifier& property);
	void startDragging(const var& sourceDescription,
		Component* sourceComponent,
		Image 	dragImage = Image(),
		bool 	allowDraggingToOtherJuceWindows = false,
		const Point< int >* imageOffsetFromMouse = nullptr,
		const MouseInputSource* inputSourceCausingDrag = nullptr
	);



	
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

private:
	//member variables
	ValueTree mainVT;

	ValueTree sourceFiles{ ValTreeIDs::sourceFilesNode };
	ValueTree repoFiles{ ValTreeIDs::repoFilesNode };
	ValueTree selectedFiles{ ValTreeIDs::selectedFiles };
	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReaderSource> playSource;
	TableListBox table{ {}, this };
	Font font{ 14.0 };
	std::unique_ptr<XmlElement> playlistData;
	std::unique_ptr<XmlElement> savedData;
	XmlElement* columnList = nullptr;
	XmlElement* dataList = nullptr;
	int numRows = 0;
	Label debugLabel;
	TextButton loadDirButton;
	String refreshSymbol = CharPointer_UTF8("\xe2\x9f\xb3");
	TextButton refreshButton{ refreshSymbol };
	String upArrowSymbol = CharPointer_UTF8("\xe2\x86\x91");
	String folderSymbol = CharPointer_UTF8("\xf0\x9f\x93\x81");
	TextButton backFolderButton{ folderSymbol + upArrowSymbol };
	String outArrowSymbol = CharPointer_UTF8("\xe2\xae\xa2");
	TextButton openInExplorerButton{ folderSymbol+outArrowSymbol };
	Array<File> localDirWavs;
	Array<File> repoDirWavs;
	MainComponent& mainComp;
	File directory;
	bool bIsLeftPanel;
	XmlElement* saveDirColumnList = nullptr;
	XmlElement* saveDirDataList = nullptr;
	UnicodeSymbolsLookAndFeel unicodeLookAndFeel;





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

			toggleButton.onClick = [this]
			{ owner.setSelection(row, (int)toggleButton.getToggleState()); };
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



};



