WavCaptain (1_1_2) by BioMannequin (Ashton Mills) -- made with JUCE


WavCaptain is .wav file player and asset managemnet tool. It was originally designed for handling game audio assests to improve workflow. 
I wanted to create single application that could
1) Audition the files I have created quickly and with a waveform view that you can click on to naviagte through the track. 
2) Batch downsample files so they are they right sample rate for the game engine (at time of writing the game I work on only takes 22.05k wav files)
3) Batch deploy (copy) files to the directory that the game engine reads them from, so I don't have to manually copy-paste assets around with Windows Explorer.

Audio Player

At it's most basic you can use WavCaptain as a simple audio player. It has a waveform view that you can use for navigation which I find really useful. You can load wavs into it by browsing for them clicking he 'Source Directory' button, by dragging files in (and it will open up the whole directory) or my doing open with and searching for WavCaptain exe file on yoru machine (whihc also brings up the whole directory of the file you loaded in ). It displays any folders in the loaded directoroes as well so you can navigate that way if you like. 


Asset Managment 
WavCaptain is based on the practice of having a local directory for assets you've created (Source Directory), and a directory (possibly linked to your source verson control) that you game engine reads your audio files from (Destination Directory). Click the respective buttons to browse for these directories. One they have been seleced, or anytime to you use the program to make any changes it will save your directories, so you.
For sample rate conversion, use the drop down menu to select your target sample rate (YOU CAN DOWNSAMPLE BUT NOT UPSAMPLE) and use the tickboxes to select which files you want to change. You can then decide whether you want to back up the original files in a subdirectory of the current source directory, or just overwrite the existing files with the new sampel rate versions (clicking out of the popup menu will cancel the operation).
Deploying means copying files from the left panel (source Directory) to the right panel (Destination Directory). You can either select whihc ones you want to deploy or deploy the whole contents of the folder (not including any folders or non-wav files). You can choose to not copy any files that are already in the destination folder, or to overwrite existing ones with the ones you deploy. 

That's about it for now. 


I've currently only made this work for .wav files because that's all I need at the moment, and it is only available for Windows. 
If people are interested I could potentially look into widening it to Mac and other file types. 

The app is open source: feel free to grab the source code: https://github.com/ashtonmills/WavCaptain  
DISCLAIMER - The code is a bit of pigsty at the time of writing. I need to give it a bit of a clean up.

This is my first C++ project. It may very well be a bit buggy!
If you have any feedback,question, requests or bug reports - please email me at biomannequinaudio@gmail.com
This is just a side project for me so I may not have to to respond.