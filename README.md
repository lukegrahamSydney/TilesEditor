# TilesEditor
A level editor that will support multiple level formats

Windows users can download this:
[Windows Release]([https://github.com/lukegrahamSydney/TilesEditor/blob/main/release.zip](https://github.com/lukegrahamSydney/TilesEditor/releases/tag/2.0))

# Features
1. Supports .gmap and .nw/.graal file formats
2. A json-based format (.world for overworld, .lvl for inidividual levels). WIP
3. Select multiple objects at once (hold shift key to select another object)
4. Grab,move and resize links and signs
5. Select tiles and resize the selection.
6. On overworlds (gmap), if you add a link that spans multiple levels, it will automatically and correctly duplicate the link across the multiple levels it spans.
7. Zoom in and out
8. Preload all the levels within the gmap (use this if you're reading from a hard disk instead of SSD)
9. Search for all npcs within the current view
10. Change the tile set
11. Import tile objects from the folder "tileobjects" from graal.
12. When you choose to edit signs/links on an overworld, you can then select which level to edit.
13. Supports tile layers
14. Move objects around with accuracy by holding "Control" while dragging.
15. Tile flood fill
16. undo/redo
17. Pattern tile flood
18. Preview tile flood before doing it
19. Generate an image of the entire level/gmap. Customize the zoom level to create maps and mini-map. Save individual levels to a folder (for gmap)
20. Bulk level format converter


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


![Alt text](https://user-images.githubusercontent.com/132313681/265017810-9a0d8758-eddc-473c-b6c5-65e1301539da.png  "Optional Title")
