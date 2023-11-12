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
    AbstractSpatialGridItem.h
    CustomPaintWidget.h
    DarkTitleBar.h
    EditAnonymousNPC.cpp EditAnonymousNPC.h EditAnonymousNPC.ui
    EditBaddy.cpp EditBaddy.h EditBaddy.ui
    EditChestDialog.cpp EditChestDialog.h EditChestDialog.ui
    EditExternalNPC.cpp EditExternalNPC.h EditExternalNPC.ui
    EditLinkDialog.cpp EditLinkDialog.h EditLinkDialog.ui
    EditSignsDialog.cpp EditSignsDialog.h EditSignsDialog.ui
    EditTilesetDialog.cpp EditTilesetDialog.h EditTilesetDialog.ui
    EditorTabWidget.cpp EditorTabWidget.h EditorTabWidget.ui
    EntitySpatialGrid.h
    ExternalNPCInspector.cpp ExternalNPCInspector.h ExternalNPCInspector.ui
    FileFormatManager.cpp FileFormatManager.h
    FixMapNamesDialog.cpp FixMapNamesDialog.h FixMapNamesDialog.ui
    GraphicsView.cpp GraphicsView.h
    IEntitySpatialMap.h
    IFileRequester.h
    ILevelFormat.h
    ISpatialMapItem.h
    IWorld.h
    Image.cpp Image.h
    ImageDimensions.cpp ImageDimensions.h
    Level.cpp Level.h
    LevelChest.cpp LevelChest.h
    LevelCommands.cpp LevelCommands.h
    LevelConverter.cpp LevelConverter.h LevelConverter.ui
    LevelEntityType.h
    LevelFactory.h
    LevelFormatGraal.cpp LevelFormatGraal.h
    LevelFormatLVL.cpp LevelFormatLVL.h
    LevelFormatNW.cpp LevelFormatNW.h
    LevelGraalBaddy.cpp LevelGraalBaddy.h
    LevelLink.cpp LevelLink.h
    LevelNPC.cpp LevelNPC.h
    LevelSign.cpp LevelSign.h
    ListLinksDialog.cpp ListLinksDialog.h ListLinksDialog.ui
    MainFileSystem.cpp MainFileSystem.h
    MainWindow.cpp MainWindow.h MainWindow.ui
    NewLevelDialog.cpp NewLevelDialog.h NewLevelDialog.ui
    NewOverworldDialog.cpp NewOverworldDialog.h NewOverworldDialog.ui
    ObjectFactory.cpp ObjectFactory.h
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
    ScreenshotDialog.cpp ScreenshotDialog.h ScreenshotDialog.ui
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
    "icons/baddy.png"
    "icons/chest.png"
    "icons/fugue/application-sidebar-collapse.png"
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
    "icons/fugue/document--pencil.png"
    "icons/fugue/document-copy.png"
    "icons/fugue/document-import.png"
    "icons/fugue/document-medium.png"
    "icons/fugue/document.png"
    "icons/fugue/eye--pencil.png"
    "icons/fugue/eye-close.png"
    "icons/fugue/eye.png"
    "icons/fugue/folder-open-document.png"
    "icons/fugue/folder-open.png"
    "icons/fugue/function.png"
    "icons/fugue/image-export.png"
    "icons/fugue/information-octagon.png"
    "icons/fugue/layer-shade.png"
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
    "icons/grid-snap.png"
    "icons/grid.png"
    "icons/npc.png"
    "icons/opps.png"
    "icons/sign.png"
    "rule_cpp.xml"
)

qt_add_resources(TilesEditor "MainWindow"
    PREFIX
        "/MainWindow"
    FILES
        ${MainWindow_resource_files}
)
set(darkstyle_resource_files
    "qdarkstyle/dark/rc/arrow_down.png"
    "qdarkstyle/dark/rc/arrow_down@2x.png"
    "qdarkstyle/dark/rc/arrow_down_disabled.png"
    "qdarkstyle/dark/rc/arrow_down_disabled@2x.png"
    "qdarkstyle/dark/rc/arrow_down_focus.png"
    "qdarkstyle/dark/rc/arrow_down_focus@2x.png"
    "qdarkstyle/dark/rc/arrow_down_pressed.png"
    "qdarkstyle/dark/rc/arrow_down_pressed@2x.png"
    "qdarkstyle/dark/rc/arrow_left.png"
    "qdarkstyle/dark/rc/arrow_left@2x.png"
    "qdarkstyle/dark/rc/arrow_left_disabled.png"
    "qdarkstyle/dark/rc/arrow_left_disabled@2x.png"
    "qdarkstyle/dark/rc/arrow_left_focus.png"
    "qdarkstyle/dark/rc/arrow_left_focus@2x.png"
    "qdarkstyle/dark/rc/arrow_left_pressed.png"
    "qdarkstyle/dark/rc/arrow_left_pressed@2x.png"
    "qdarkstyle/dark/rc/arrow_right.png"
    "qdarkstyle/dark/rc/arrow_right@2x.png"
    "qdarkstyle/dark/rc/arrow_right_disabled.png"
    "qdarkstyle/dark/rc/arrow_right_disabled@2x.png"
    "qdarkstyle/dark/rc/arrow_right_focus.png"
    "qdarkstyle/dark/rc/arrow_right_focus@2x.png"
    "qdarkstyle/dark/rc/arrow_right_pressed.png"
    "qdarkstyle/dark/rc/arrow_right_pressed@2x.png"
    "qdarkstyle/dark/rc/arrow_up.png"
    "qdarkstyle/dark/rc/arrow_up@2x.png"
    "qdarkstyle/dark/rc/arrow_up_disabled.png"
    "qdarkstyle/dark/rc/arrow_up_disabled@2x.png"
    "qdarkstyle/dark/rc/arrow_up_focus.png"
    "qdarkstyle/dark/rc/arrow_up_focus@2x.png"
    "qdarkstyle/dark/rc/arrow_up_pressed.png"
    "qdarkstyle/dark/rc/arrow_up_pressed@2x.png"
    "qdarkstyle/dark/rc/base_icon.png"
    "qdarkstyle/dark/rc/base_icon@2x.png"
    "qdarkstyle/dark/rc/base_icon_disabled.png"
    "qdarkstyle/dark/rc/base_icon_disabled@2x.png"
    "qdarkstyle/dark/rc/base_icon_focus.png"
    "qdarkstyle/dark/rc/base_icon_focus@2x.png"
    "qdarkstyle/dark/rc/base_icon_pressed.png"
    "qdarkstyle/dark/rc/base_icon_pressed@2x.png"
    "qdarkstyle/dark/rc/branch_closed.png"
    "qdarkstyle/dark/rc/branch_closed@2x.png"
    "qdarkstyle/dark/rc/branch_closed_disabled.png"
    "qdarkstyle/dark/rc/branch_closed_disabled@2x.png"
    "qdarkstyle/dark/rc/branch_closed_focus.png"
    "qdarkstyle/dark/rc/branch_closed_focus@2x.png"
    "qdarkstyle/dark/rc/branch_closed_pressed.png"
    "qdarkstyle/dark/rc/branch_closed_pressed@2x.png"
    "qdarkstyle/dark/rc/branch_end.png"
    "qdarkstyle/dark/rc/branch_end@2x.png"
    "qdarkstyle/dark/rc/branch_end_disabled.png"
    "qdarkstyle/dark/rc/branch_end_disabled@2x.png"
    "qdarkstyle/dark/rc/branch_end_focus.png"
    "qdarkstyle/dark/rc/branch_end_focus@2x.png"
    "qdarkstyle/dark/rc/branch_end_pressed.png"
    "qdarkstyle/dark/rc/branch_end_pressed@2x.png"
    "qdarkstyle/dark/rc/branch_line.png"
    "qdarkstyle/dark/rc/branch_line@2x.png"
    "qdarkstyle/dark/rc/branch_line_disabled.png"
    "qdarkstyle/dark/rc/branch_line_disabled@2x.png"
    "qdarkstyle/dark/rc/branch_line_focus.png"
    "qdarkstyle/dark/rc/branch_line_focus@2x.png"
    "qdarkstyle/dark/rc/branch_line_pressed.png"
    "qdarkstyle/dark/rc/branch_line_pressed@2x.png"
    "qdarkstyle/dark/rc/branch_more.png"
    "qdarkstyle/dark/rc/branch_more@2x.png"
    "qdarkstyle/dark/rc/branch_more_disabled.png"
    "qdarkstyle/dark/rc/branch_more_disabled@2x.png"
    "qdarkstyle/dark/rc/branch_more_focus.png"
    "qdarkstyle/dark/rc/branch_more_focus@2x.png"
    "qdarkstyle/dark/rc/branch_more_pressed.png"
    "qdarkstyle/dark/rc/branch_more_pressed@2x.png"
    "qdarkstyle/dark/rc/branch_open.png"
    "qdarkstyle/dark/rc/branch_open@2x.png"
    "qdarkstyle/dark/rc/branch_open_disabled.png"
    "qdarkstyle/dark/rc/branch_open_disabled@2x.png"
    "qdarkstyle/dark/rc/branch_open_focus.png"
    "qdarkstyle/dark/rc/branch_open_focus@2x.png"
    "qdarkstyle/dark/rc/branch_open_pressed.png"
    "qdarkstyle/dark/rc/branch_open_pressed@2x.png"
    "qdarkstyle/dark/rc/checkbox_checked.png"
    "qdarkstyle/dark/rc/checkbox_checked@2x.png"
    "qdarkstyle/dark/rc/checkbox_checked_disabled.png"
    "qdarkstyle/dark/rc/checkbox_checked_disabled@2x.png"
    "qdarkstyle/dark/rc/checkbox_checked_focus.png"
    "qdarkstyle/dark/rc/checkbox_checked_focus@2x.png"
    "qdarkstyle/dark/rc/checkbox_checked_pressed.png"
    "qdarkstyle/dark/rc/checkbox_checked_pressed@2x.png"
    "qdarkstyle/dark/rc/checkbox_indeterminate.png"
    "qdarkstyle/dark/rc/checkbox_indeterminate@2x.png"
    "qdarkstyle/dark/rc/checkbox_indeterminate_disabled.png"
    "qdarkstyle/dark/rc/checkbox_indeterminate_disabled@2x.png"
    "qdarkstyle/dark/rc/checkbox_indeterminate_focus.png"
    "qdarkstyle/dark/rc/checkbox_indeterminate_focus@2x.png"
    "qdarkstyle/dark/rc/checkbox_indeterminate_pressed.png"
    "qdarkstyle/dark/rc/checkbox_indeterminate_pressed@2x.png"
    "qdarkstyle/dark/rc/checkbox_unchecked.png"
    "qdarkstyle/dark/rc/checkbox_unchecked@2x.png"
    "qdarkstyle/dark/rc/checkbox_unchecked_disabled.png"
    "qdarkstyle/dark/rc/checkbox_unchecked_disabled@2x.png"
    "qdarkstyle/dark/rc/checkbox_unchecked_focus.png"
    "qdarkstyle/dark/rc/checkbox_unchecked_focus@2x.png"
    "qdarkstyle/dark/rc/checkbox_unchecked_pressed.png"
    "qdarkstyle/dark/rc/checkbox_unchecked_pressed@2x.png"
    "qdarkstyle/dark/rc/line_horizontal.png"
    "qdarkstyle/dark/rc/line_horizontal@2x.png"
    "qdarkstyle/dark/rc/line_horizontal_disabled.png"
    "qdarkstyle/dark/rc/line_horizontal_disabled@2x.png"
    "qdarkstyle/dark/rc/line_horizontal_focus.png"
    "qdarkstyle/dark/rc/line_horizontal_focus@2x.png"
    "qdarkstyle/dark/rc/line_horizontal_pressed.png"
    "qdarkstyle/dark/rc/line_horizontal_pressed@2x.png"
    "qdarkstyle/dark/rc/line_vertical.png"
    "qdarkstyle/dark/rc/line_vertical@2x.png"
    "qdarkstyle/dark/rc/line_vertical_disabled.png"
    "qdarkstyle/dark/rc/line_vertical_disabled@2x.png"
    "qdarkstyle/dark/rc/line_vertical_focus.png"
    "qdarkstyle/dark/rc/line_vertical_focus@2x.png"
    "qdarkstyle/dark/rc/line_vertical_pressed.png"
    "qdarkstyle/dark/rc/line_vertical_pressed@2x.png"
    "qdarkstyle/dark/rc/radio_checked.png"
    "qdarkstyle/dark/rc/radio_checked@2x.png"
    "qdarkstyle/dark/rc/radio_checked_disabled.png"
    "qdarkstyle/dark/rc/radio_checked_disabled@2x.png"
    "qdarkstyle/dark/rc/radio_checked_focus.png"
    "qdarkstyle/dark/rc/radio_checked_focus@2x.png"
    "qdarkstyle/dark/rc/radio_checked_pressed.png"
    "qdarkstyle/dark/rc/radio_checked_pressed@2x.png"
    "qdarkstyle/dark/rc/radio_unchecked.png"
    "qdarkstyle/dark/rc/radio_unchecked@2x.png"
    "qdarkstyle/dark/rc/radio_unchecked_disabled.png"
    "qdarkstyle/dark/rc/radio_unchecked_disabled@2x.png"
    "qdarkstyle/dark/rc/radio_unchecked_focus.png"
    "qdarkstyle/dark/rc/radio_unchecked_focus@2x.png"
    "qdarkstyle/dark/rc/radio_unchecked_pressed.png"
    "qdarkstyle/dark/rc/radio_unchecked_pressed@2x.png"
    "qdarkstyle/dark/rc/toolbar_move_horizontal.png"
    "qdarkstyle/dark/rc/toolbar_move_horizontal@2x.png"
    "qdarkstyle/dark/rc/toolbar_move_horizontal_disabled.png"
    "qdarkstyle/dark/rc/toolbar_move_horizontal_disabled@2x.png"
    "qdarkstyle/dark/rc/toolbar_move_horizontal_focus.png"
    "qdarkstyle/dark/rc/toolbar_move_horizontal_focus@2x.png"
    "qdarkstyle/dark/rc/toolbar_move_horizontal_pressed.png"
    "qdarkstyle/dark/rc/toolbar_move_horizontal_pressed@2x.png"
    "qdarkstyle/dark/rc/toolbar_move_vertical.png"
    "qdarkstyle/dark/rc/toolbar_move_vertical@2x.png"
    "qdarkstyle/dark/rc/toolbar_move_vertical_disabled.png"
    "qdarkstyle/dark/rc/toolbar_move_vertical_disabled@2x.png"
    "qdarkstyle/dark/rc/toolbar_move_vertical_focus.png"
    "qdarkstyle/dark/rc/toolbar_move_vertical_focus@2x.png"
    "qdarkstyle/dark/rc/toolbar_move_vertical_pressed.png"
    "qdarkstyle/dark/rc/toolbar_move_vertical_pressed@2x.png"
    "qdarkstyle/dark/rc/toolbar_separator_horizontal.png"
    "qdarkstyle/dark/rc/toolbar_separator_horizontal@2x.png"
    "qdarkstyle/dark/rc/toolbar_separator_horizontal_disabled.png"
    "qdarkstyle/dark/rc/toolbar_separator_horizontal_disabled@2x.png"
    "qdarkstyle/dark/rc/toolbar_separator_horizontal_focus.png"
    "qdarkstyle/dark/rc/toolbar_separator_horizontal_focus@2x.png"
    "qdarkstyle/dark/rc/toolbar_separator_horizontal_pressed.png"
    "qdarkstyle/dark/rc/toolbar_separator_horizontal_pressed@2x.png"
    "qdarkstyle/dark/rc/toolbar_separator_vertical.png"
    "qdarkstyle/dark/rc/toolbar_separator_vertical@2x.png"
    "qdarkstyle/dark/rc/toolbar_separator_vertical_disabled.png"
    "qdarkstyle/dark/rc/toolbar_separator_vertical_disabled@2x.png"
    "qdarkstyle/dark/rc/toolbar_separator_vertical_focus.png"
    "qdarkstyle/dark/rc/toolbar_separator_vertical_focus@2x.png"
    "qdarkstyle/dark/rc/toolbar_separator_vertical_pressed.png"
    "qdarkstyle/dark/rc/toolbar_separator_vertical_pressed@2x.png"
    "qdarkstyle/dark/rc/transparent.png"
    "qdarkstyle/dark/rc/transparent@2x.png"
    "qdarkstyle/dark/rc/transparent_disabled.png"
    "qdarkstyle/dark/rc/transparent_disabled@2x.png"
    "qdarkstyle/dark/rc/transparent_focus.png"
    "qdarkstyle/dark/rc/transparent_focus@2x.png"
    "qdarkstyle/dark/rc/transparent_pressed.png"
    "qdarkstyle/dark/rc/transparent_pressed@2x.png"
    "qdarkstyle/dark/rc/window_close.png"
    "qdarkstyle/dark/rc/window_close@2x.png"
    "qdarkstyle/dark/rc/window_close_disabled.png"
    "qdarkstyle/dark/rc/window_close_disabled@2x.png"
    "qdarkstyle/dark/rc/window_close_focus.png"
    "qdarkstyle/dark/rc/window_close_focus@2x.png"
    "qdarkstyle/dark/rc/window_close_pressed.png"
    "qdarkstyle/dark/rc/window_close_pressed@2x.png"
    "qdarkstyle/dark/rc/window_grip.png"
    "qdarkstyle/dark/rc/window_grip@2x.png"
    "qdarkstyle/dark/rc/window_grip_disabled.png"
    "qdarkstyle/dark/rc/window_grip_disabled@2x.png"
    "qdarkstyle/dark/rc/window_grip_focus.png"
    "qdarkstyle/dark/rc/window_grip_focus@2x.png"
    "qdarkstyle/dark/rc/window_grip_pressed.png"
    "qdarkstyle/dark/rc/window_grip_pressed@2x.png"
    "qdarkstyle/dark/rc/window_minimize.png"
    "qdarkstyle/dark/rc/window_minimize@2x.png"
    "qdarkstyle/dark/rc/window_minimize_disabled.png"
    "qdarkstyle/dark/rc/window_minimize_disabled@2x.png"
    "qdarkstyle/dark/rc/window_minimize_focus.png"
    "qdarkstyle/dark/rc/window_minimize_focus@2x.png"
    "qdarkstyle/dark/rc/window_minimize_pressed.png"
    "qdarkstyle/dark/rc/window_minimize_pressed@2x.png"
    "qdarkstyle/dark/rc/window_undock.png"
    "qdarkstyle/dark/rc/window_undock@2x.png"
    "qdarkstyle/dark/rc/window_undock_disabled.png"
    "qdarkstyle/dark/rc/window_undock_disabled@2x.png"
    "qdarkstyle/dark/rc/window_undock_focus.png"
    "qdarkstyle/dark/rc/window_undock_focus@2x.png"
    "qdarkstyle/dark/rc/window_undock_pressed.png"
    "qdarkstyle/dark/rc/window_undock_pressed@2x.png"
)

qt_add_resources(TilesEditor "darkstyle"
    PREFIX
        "/qss_icons/dark"
    BASE
        "qdarkstyle/dark"
    FILES
        ${darkstyle_resource_files}
)
set(darkstyle1_resource_files
    "qdarkstyle/dark/darkstyle.qss"
)

qt_add_resources(TilesEditor "darkstyle1"
    PREFIX
        "/qdarkstyle/dark"
    BASE
        "qdarkstyle/dark"
    FILES
        ${darkstyle1_resource_files}
)

install(TARGETS TilesEditor
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
add_subdirectory(cmake/mingw64)
