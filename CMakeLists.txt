cmake_minimum_required(VERSION 3.16)
project(TilesEditor VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets)

qt_add_executable(TilesEditor WIN32 MACOSX_BUNDLE
    AboutDialog.cpp AboutDialog.h AboutDialog.ui
    AbstractFileSystem.h
    AbstractLevelEntity.cpp AbstractLevelEntity.h
    AbstractSelection.cpp AbstractSelection.h
    CustomPaintWidget.h
    EditAnonymousNPC.cpp EditAnonymousNPC.h EditAnonymousNPC.ui
    EditExternalNPC.cpp EditExternalNPC.h EditExternalNPC.ui
    EditLinkDialog.cpp EditLinkDialog.h EditLinkDialog.ui
    EditSignsDialog.cpp EditSignsDialog.h EditSignsDialog.ui
    EditTilesetDialog.cpp EditTilesetDialog.h
    EditorTabWidget.cpp EditorTabWidget.h EditorTabWidget.ui
    EntitySpatialGrid.cpp EntitySpatialGrid.h
    GraphicsView.cpp GraphicsView.h
    IEntitySpatialMap.h
    ISpatialMapItem.h
    IWorld.h
    Image.cpp Image.h
    ImageDimensions.cpp ImageDimensions.h
    Level.cpp Level.h
    LevelCommands.cpp LevelCommands.h
    LevelEntityType.h
    LevelLink.cpp LevelLink.h
    LevelNPC.cpp LevelNPC.h
    LevelSign.cpp LevelSign.h
    ListLinksDialog.cpp ListLinksDialog.h ListLinksDialog.ui
    MainFileSystem.cpp MainFileSystem.h
    MainWindow.cpp MainWindow.h MainWindow.ui
    ObjectListModel.cpp ObjectListModel.h
    ObjectSelection.cpp ObjectSelection.h
    ObjectsWidget.cpp ObjectsWidget.h ObjectsWidget.ui
    Overworld.cpp Overworld.h
    QCodeEditor/include/KGL/Design/QCodeEditorDesign.hpp
    QCodeEditor/include/KGL/Design/QCodeEditorSheets.hpp
    QCodeEditor/include/KGL/Design/QLineColumnPadding.hpp
    QCodeEditor/include/KGL/Design/QSyntaxRule.hpp
    QCodeEditor/include/KGL/Design/XmlHelper.hpp
    QCodeEditor/include/KGL/KGLConfig.hpp
    QCodeEditor/include/KGL/Widgets/QCodeEditor.hpp
    QCodeEditor/include/KGL/Widgets/QCodeEditorHighlighter.hpp
    QCodeEditor/include/KGL/Widgets/QCodeEditorLineWidget.hpp
    QCodeEditor/include/KGL/Widgets/QCodeEditorPopup.hpp
    QCodeEditor/src/Design/QCodeEditorDesign.cpp
    QCodeEditor/src/Design/QCodeEditorSheets.cpp
    QCodeEditor/src/Design/QLineColumnPadding.cpp
    QCodeEditor/src/Design/QSyntaxRule.cpp
    QCodeEditor/src/Design/XmlHelper.cpp
    QCodeEditor/src/Widgets/QCodeEditor.cpp
    QCodeEditor/src/Widgets/QCodeEditorEvents.cpp
    QCodeEditor/src/Widgets/QCodeEditorHighlighter.cpp
    QCodeEditor/src/Widgets/QCodeEditorLineWidget.cpp
    QCodeEditor/src/Widgets/QCodeEditorPopup.cpp
    QCodeEditor/src/Widgets/QCodeEditorSlots.cpp
    Rectangle.h
    RefCounter.h
    Resource.h
    ResourceManager.cpp ResourceManager.h
    ResourceType.h
    SaveOverworldDialog.cpp SaveOverworldDialog.h SaveOverworldDialog.ui
    SelectionType.h
    Selector.h
    StringTools.h
    TileGroupListModel.cpp TileGroupListModel.h
    TileGroupModel.cpp TileGroupModel.h
    TileObject.cpp TileObject.h
    TileObjectsWidget.cpp TileObjectsWidget.h TileObjectsWidget.ui
    TileSelection.cpp TileSelection.h
    Tilemap.cpp Tilemap.h
    Tileset.cpp Tileset.h
    TilesetsWidget.cpp TilesetsWidget.h TilesetsWidget.ui
    cJSON/JsonHelper.h
    cJSON/cJSON.c cJSON/cJSON.h
    main.cpp
)
target_include_directories(TilesEditor PRIVATE
    /
    QCodeEditor/include
    QCodeEditor/include/KGL/Widgets
)

target_compile_definitions(TilesEditor PRIVATE
    KGL_STATIC
)

target_link_libraries(TilesEditor PRIVATE
    # Remove: L"."
    Qt::Core
    Qt::Gui
    Qt::Widgets
)


# Resources:
set(MainWindow_resource_files
    "icons/fugue/arrow-circle-315.png"
    "icons/fugue/arrow-curve-180-left.png"
    "icons/fugue/arrow-curve.png"
    "icons/fugue/block.png"
    "icons/fugue/chain--pencil.png"
    "icons/fugue/chain--plus.png"
    "icons/fugue/clipboard-paste.png"
    "icons/fugue/cross-script.png"
    "icons/fugue/cursor.png"
    "icons/fugue/disk--pencil.png"
    "icons/fugue/disk.png"
    "icons/fugue/document-copy.png"
    "icons/fugue/document-import.png"
    "icons/fugue/document-medium.png"
    "icons/fugue/document.png"
    "icons/fugue/eye-close.png"
    "icons/fugue/eye.png"
    "icons/fugue/folder-open-document.png"
    "icons/fugue/function.png"
    "icons/fugue/information-octagon.png"
    "icons/fugue/layers.png"
    "icons/fugue/magnifier.png"
    "icons/fugue/paint-can--pencil.png"
    "icons/fugue/paint-can--plus.png"
    "icons/fugue/paint-can.png"
    "icons/fugue/pencil.png"
    "icons/fugue/scissors.png"
    "icons/fugue/script--pencil.png"
    "icons/fugue/script--plus.png"
    "icons/fugue/ui-progress-bar.png"
    "icons/npc.png"
    "icons/sign.png"
    "rule_cpp.xml"
)

qt_add_resources(TilesEditor "MainWindow"
    PREFIX
        "/MainWindow"
    FILES
        ${MainWindow_resource_files}
)

install(TARGETS TilesEditor
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
add_subdirectory(cmake/mingw64)
