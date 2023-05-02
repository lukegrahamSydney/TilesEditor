# TilesEditor
A level editor that will support multiple level formats

# Features
1. Supports .gmap and .nw file formats
2. Select multiple objects at once (hold shift key to select another object)
3. Grab,move and resize links and signs
4. Select tiles and resize the selection.
5. On overworlds (gmap), if you add a link that spans multiple levels, it will automatically and correctly duplicate the link across the multiple levels it spans.
6. Zoom in and out
7. Preload all the levels within the gmap (use this if you're reading from a hard disk instead of SSD)
8. Search for all npcs within the current view
9. Change the tile set
10. Import tile objects from the folder "tileobjects" from graal.
11. When you choose to edit signs/links on an overworld, you can then select which level to edit.
12. Supports tile layers

# Compiling on windows:
You need Qt. Set that up then use this command line:

qmake -tp vc TilesEditor.pro

This will generate a visual studio project file which you can open in visual studio community and build it.

![Alt text](/screenshot1.png "Optional Title")
