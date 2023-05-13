# TilesEditor
A level editor that will support multiple level formats

Windows users can download this:
[Windows Release](https://github.com/lukegrahamSydney/TilesEditor/blob/main/release.zip)

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
13. Move objects around with accuracy by holding "Control" while dragging.
14. Tile flood fill
15. undo/redo
16. Pattern tile flood
17. Preview tile flood before doing it
18. Generate an image of the entire level/gmap. Customize the zoom level to create maps and mini-map. Save individual levels to a folder (for gmap)

# Requirements For compiling
* QT6
* python3 + pip3 (Linux)

# Compiling on windows:
You need Qt6. Set that up then use this command line:

qmake -tp vc TilesEditor.pro

This will generate a visual studio project file to build...or you can just use the one i included.

# Compiling on other:
Well you'll need Qt. Then do something similar like:
qmake TilesEditor.pro

I believe that will create a makefile

OR generate CMakeLists.txt:
```bash
pip3 install qmake2cmake
qmake2cmake_all --min-qt-version 6 ./
cmake -S. -Bbuild/
cmake --build build/ -j $(nproc)
```

![Alt text](https://user-images.githubusercontent.com/132313681/236118575-99d12ea4-4cad-411a-bee6-71d694e72abc.png "Optional Title")
