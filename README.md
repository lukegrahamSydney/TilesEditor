# TilesEditor
A level editor that will support multiple level formats

#Features
*Supports .gmap and .nw file formats
*Select multiple objects at once (hold shift key to select another object)
*Grab,move and resize links and signs
*Select tiles and resize the selection.
*On overworlds (gmap), if you add a link that spans multiple levels, it will automatically and correctly duplicate the link across the multiple levels it spans.
*Zoom in and out
*Preload all the levels within the gmap (use this if you're reading from a hard disk instead of SSD)
*Search for all npcs within the current view
*Change the tile set
*Import tile objects from the folder "tileobjects" from graal.
*When you choose to edit signs/links on an overworld, you can then select which level to edit.

# Compiling on windows:
You need Qt. Set that up then use this command line:

qmake -tp vc TilesEditor.pro

This will generate a visual studio project file which you can open in visual studio community and build it.

![Alt text](/screenshot1.png "Optional Title")
