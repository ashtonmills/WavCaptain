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

		addAndMakeVisible(digitsSelection);
		digitsSelection.addItem("1", 1);
		digitsSelection.addItem("2", 2);
		digitsSelection.addItem("3", 3);
		digitsSelection.setSelectedId(2);
		digitsSelection.onChange = [this] {onLabelFieldTextChange(); };

		addAndMakeVisible(digitSelectionLabel);
		digitSelectionLabel.setText("Number of digits in increment", dontSendNotification);
		digitSelectionLabel.attachToComponent(&digitsSelection, true);


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

	void okButtonClicked()
	{
		//Proceed with labelling assets
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
