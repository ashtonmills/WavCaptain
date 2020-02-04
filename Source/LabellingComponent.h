/*
  ==============================================================================

    LabellingComponent.h
    Created: 3 Feb 2020 8:17:03pm
    Author:  User

  ==============================================================================
*/

#pragma once
#include "MainComponent.h"
#include "ValTreeIds.h"

struct CoreData;

//=========================================================================
// Dialog window for labeling assets
class LabellingComponent : public Component
{
public:
	LabellingComponent(ValueTree vt, CoreData& data) : mainVT(vt), coreData(data)
	{
		setSize(600, 600);
		addAndMakeVisible(labelField);
		labelField.setEditable(true);
		labelField.setColour(Label::ColourIds::textColourId, Colours::black);
		labelField.setColour(Label::ColourIds::textWhenEditingColourId, Colours::black);
		labelField.setColour(Label::backgroundColourId, Colours::white);
		labelField.onTextChange = [this] {onLabelFieldTextChange(); };

		addAndMakeVisible(labelLabel);
		labelLabel.attachToComponent(&labelField, true);
		labelLabel.setText("New File Name", dontSendNotification);
		labelLabel.setColour(Label::ColourIds::textColourId, Colours::white);

		if (mainVT.getChildWithName(ValTreeIDs::selectedFiles).getNumProperties() > 1)
		{ 

		addAndMakeVisible(digitsSelection);
		digitsSelection.addItem("1", 1);
		digitsSelection.addItem("2", 2);
		digitsSelection.addItem("3", 3);
		digitsSelection.setSelectedId(2);
		digitsSelection.onChange = [this] {onLabelFieldTextChange(); };

		addAndMakeVisible(digitSelectionLabel);
		digitSelectionLabel.setText("Number of digits in increment", dontSendNotification);
		digitSelectionLabel.attachToComponent(&digitsSelection, true);
		}


		addAndMakeVisible(outputPreview);
		outputPreview.setColour(Label::ColourIds::textColourId, Colours::black);
		labelField.setColour(Label::ColourIds::textWhenEditingColourId, Colours::black);
		outputPreview.setColour(Label::backgroundColourId, Colours::white);

		addAndMakeVisible(outputPreviewLabel);
		outputPreviewLabel.setColour(Label::ColourIds::textColourId, Colours::white);
		outputPreviewLabel.attachToComponent(&outputPreview, true);
		outputPreviewLabel.setText("Output preview", dontSendNotification);

		addAndMakeVisible(okButton);
		okButton.onClick = [this] {okButtonClicked(); };
		okButton.setEnabled(false);

		cancelVT.setProperty(ValTreeIDs::closeLabellingDialog, false, nullptr);

		addAndMakeVisible(cancelButton);
		cancelButton.onClick = [this] {cancelButtonClicked(); };

		mainVT.addChild(cancelVT, -1, nullptr);

	}
	~LabellingComponent()
	{

	}
	void onLabelFieldTextChange()
	{
		if (labelField.getText() == "")
		{
			outputPreview.setText("", dontSendNotification);
			return;
		}
		okButton.setEnabled(true);
		if(mainVT.getChildWithName(ValTreeIDs::selectedFiles).getNumProperties()>1)
		{
			switch (digitsSelection.getSelectedId())
			{
			case 1: outputPreview.setText(labelField.getText() + "1.wav", dontSendNotification);
				break;
			case 2: outputPreview.setText(labelField.getText() + "01.wav", dontSendNotification);
				break;
			case 3: outputPreview.setText(labelField.getText() + "001.wav", dontSendNotification);
				break;
			}
		}
		else
		{
			outputPreview.setText(labelField.getText(),dontSendNotification);
		}
		ValueTree sourceFiles = mainVT.getChildWithName(ValTreeIDs::sourceFilesNode);
		Identifier iId(outputPreview.getText());
		if (sourceFiles.hasProperty(iId))
		{
			okButton.setEnabled(false);
			outputPreview.setText("Name already taken", dontSendNotification);
		}
	}

	void okButtonClicked()
	{
		//do nothing if label is empty
		if (labelField.getText() == "")
		{
			cancelVT.setProperty(ValTreeIDs::closeLabellingDialog, "close", nullptr);
		}
		String newName = labelField.getText();
		String newNameCaseInsensitive = newName.toLowerCase();
		ValueTree files = mainVT.getChildWithName(ValTreeIDs::selectedFiles);
		ValueTree sourceFiles = mainVT.getChildWithName(ValTreeIDs::sourceFilesNode);
		

		for (int file = 1; file != files.getNumProperties(); ++file)
		{	
			//Loop through the source file assets to make sure the name is not going to overwrite anything
			Identifier iId(newName);
			for (int i = 1; i != sourceFiles.getNumProperties(); ++i)
			{
				Identifier iId = sourceFiles.getPropertyName(i);
				DBG("name in source Files: " +sourceFiles.getPropertyName(i).toString());
				DBG("newNAme is : " + newName);
				if ((sourceFiles.getPropertyName(i).toString().toLowerCase() == newNameCaseInsensitive) || 
					(sourceFiles.getPropertyName(i).toString().toLowerCase() == newNameCaseInsensitive + std::to_string(file)) || 
					(sourceFiles.getPropertyName(i).toString().toLowerCase() == newNameCaseInsensitive + "0" + std::to_string(file)) || 
					(sourceFiles.getPropertyName(i).toString().toLowerCase() == newNameCaseInsensitive + "00" + std::to_string(file)))
				{
					//	String msg = "Labelling stopped at iteration number " + std::to_string(file) + " because it would have overwritten files.";
				//	mainVT.setProperty(ValTreeIDs::debugMsg, "Labelling cancelled because it would have overwritten files", nullptr);
				//	auto aw = new AlertWindow("Filename(s) already in use", "Labelling Aborted because it would have overwritten files.", AlertWindow::AlertIconType::WarningIcon, this);
	
					AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Filename(s) already in use", "Labelling Aborted because it would have overwritten files.","ok",this);
					cancelVT.setProperty(ValTreeIDs::closeLabellingDialog, "close", nullptr);

					return;
				}
			}
			//Proceed with labelling assets
			Identifier iFileID = files.getPropertyName(file-1);
			File iFile(files.getProperty(iFileID));
			DBG("existing filename is: " + iFile.getFileName());

			String iPath = iFile.getParentDirectory().getFullPathName() + "\\";

			String newFullPath = iPath + "\\" + newName;
			//sort out the increments based on selected number of digits in the increment

			if (files.getNumProperties() > 1)
			{
				String incr2 = (file < 10) ? "0" + std::to_string(file) : std::to_string(file);
				String incr3;
				if (file < 10)
				{
					incr3 = "00" + std::to_string(file);
				}
				else if (file >= 10 && file < 100)
				{
					incr3 = "0" + std::to_string(file);
				}
				else
				{
					incr3 = std::to_string(file);
				}
				
				switch (digitsSelection.getSelectedId())
					{
						case 1: newFullPath += (file);
							break;
						case 2: newFullPath += incr2;
							break;
						case 3: newFullPath += incr3;
							break;
					}
			}
			newFullPath += ".wav";

			if (iFile.moveFileTo(File(newFullPath)))
			{
				DBG("renamed to: " + newFullPath );
			}

		}

		files.removeAllChildren(nullptr);
		mainVT.setProperty(ValTreeIDs::loadSwitch, true,nullptr);
		mainVT.setProperty(ValTreeIDs::loadSwitch, false, nullptr);
		cancelVT.setProperty(ValTreeIDs::closeLabellingDialog,"close",nullptr);
	}
	void cancelButtonClicked()
	{
		cancelVT.setProperty(ValTreeIDs::closeLabellingDialog, true, nullptr);
	}

	void resized()
	{

		labelField.setBounds(100, 100, 300, 25);
		digitsSelection.setBounds(190, 150, 70, 30);
		outputPreview.setBounds(100, 200, 300, 30);
		okButton.setBounds(200, 400, 70, 40);
		cancelButton.setBounds(300, 400, 70, 40);
	}



private:
	Label labelField;
	Label labelLabel{ "New Filename" }; //I made it so confusing by calling it labelling instead of renaming lol
	ComboBox digitsSelection;
	Label digitSelectionLabel;
	Label outputPreview;
	Label outputPreviewLabel;
	TextButton okButton{ "OK" };
	TextButton cancelButton{ "Cancel" };
	ValueTree mainVT;
	ValueTree cancelVT{ ValTreeIDs::closeLabellingDialog };
	CoreData& coreData;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabellingComponent)
};

class LabellingWindow : public DocumentWindow, ValueTree::Listener
{
public:
	LabellingWindow(String name, CoreData& data, ValueTree vt) : DocumentWindow(name,
		Desktop::getInstance().getDefaultLookAndFeel()
		.findColour(ResizableWindow::backgroundColourId),
		DocumentWindow::allButtons), mainVT (vt)
	{
		setUsingNativeTitleBar(false);
		setResizable(true, true);

		setContentOwned(new LabellingComponent(mainVT, data), true);
		centreWithSize(600, 600);
		setVisible(true);

		mainVT.addListener(this);

	}

	void valueTreePropertyChanged(ValueTree& tree, const Identifier& property) override
	{
		if (property== ValTreeIDs::closeLabellingDialog)
		{
			closeButtonPressed();
		}
	}


	void closeButtonPressed() override
	{
		// This is called when the user tries to close this window. Here, we'll just
		// ask the app to quit when this happens, but you can change this to do
		// whatever you need.
		delete this;

	}

	/* Note: Be careful if you override any DocumentWindow methods - the base
	   class uses a lot of them, so by overriding you might break its functionality.
	   It's best to do all your work in your content component instead, but if
	   you really have to override any DocumentWindow methods, make sure your
	   subclass also calls the superclass's method.
	*/

private:
	ValueTree mainVT;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabellingWindow)
};
