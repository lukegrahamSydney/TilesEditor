#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <gs1/parse/Parser.hpp>
#include <gs1/parse/Lexer.hpp>
#include <gs1/parse/Source.hpp>
#include <gs1/parse/Diag.hpp>
#include <gs1/GS1Prototypes.h>

#include "MainWindow.h"
#include "EditorTabWidget.h"
#include "AboutDialog.h"
#include "cJSON/JsonHelper.h"
#include "EditAnonymousNPC.h"
#include "LevelConverter.h"
#include "FileFormatManager.h"
#include "NewLevelDialog.h"
#include "EditExternalNPC.h"
#include "LevelFormatNW.h"
#include "LevelFormatGraal.h"
#include "LevelFormatLVL.h"
#include "EditorObject.h"
#include "NewCustomTheme.h"
#include "Level.h"
#include "Tilemap.h"
#include "AniEditor/AniEditorWindow.h"
#include "ResourceManagerFileSystem.h"
#include "EditTileDefs.h"

namespace TilesEditor
{

    MainWindow::MainWindow(QApplication& app, QSettings& settings, QWidget* parent)
        : QMainWindow(parent), m_app(app), m_settings(settings), m_menuThemes(this)
    {
        ui.setupUi(this);

        initScriptEngine();

        QString objectsFolder = "./objects/";
        if (settings.contains("objectsFolder"))
        {
            objectsFolder = settings.value("objectsFolder").toString();
            m_objectFolderChanged = true;
        }

        m_objectManager = new ObjectManager(ui.objectTree, objectsFolder);
        m_resourceManager = new ResourceManagerFileSystem("./", m_objectManager);
       
        m_resourceManager->incrementRef();

        m_defaultPalette = app.palette();

        QMainWindow::setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

        // Do multipart status bar
        m_statusLeft = new QLabel("", this);

        m_statusMiddle = new QLabel("", this);

        m_statusMiddle->setStyleSheet("{color: red}");
        statusBar()->addPermanentWidget(m_statusLeft, 0);
        statusBar()->addPermanentWidget(m_statusMiddle, 1);



        this->restoreGeometry(settings.value("geometry").toByteArray());

        //if(settings.value("windowState").toBool())
        //    m_framelessContainer->showMaximized();

        restoreState(settings.value("windowState").toByteArray());
        EditAnonymousNPC::savedGeometry = settings.value("anonymousNPCGeometry").toByteArray();
        EditorObject::savedGeometry = settings.value("editorObjectGeometry").toByteArray();

        m_resourceManager->addSearchDirRecursive("./", 4);

        auto tilesets = settings.value("tilesets").toStringList();
        for (auto& tilesetName : tilesets)
        {
            auto tileset = Tileset::loadTileset(tilesetName, m_resourceManager);
            if (tileset)
                m_tilesetList.appendRow(tileset);
        }

        loadTileObjects();


        connect(ui.actionOpen, &QAction::triggered, this, &MainWindow::openFile);
        connect(ui.actionNew, &QAction::triggered, this, &MainWindow::newLevel);
        connect(ui.actionActionNewCustom, &QAction::triggered, this, &MainWindow::newCustomLevel);
        connect(ui.actionAbout, &QAction::triggered, this, &MainWindow::aboutClicked);
        connect(ui.actionLevelConverter, &QAction::triggered, this, &MainWindow::levelConvert);
        connect(ui.actionActionAniEditor, &QAction::triggered, this, &MainWindow::actionLaunchAniEditor);
        connect(ui.actionCloseAll, &QAction::triggered, this, &MainWindow::actionCloseAll);
        connect(ui.actionPreferences, &QAction::triggered, this, &MainWindow::preferencesClicked);
        
        connect(ui.levelsTab, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);
        connect(ui.levelsTab, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTabIndexSlot);
        connect(ui.objectsFolderBrowseBtn, &QAbstractButton::clicked, this, &MainWindow::objectsFolderBrowseClicked);

        connect(ui.levelsTab->tabBar(), &QTabBar::customContextMenuRequested, this, &MainWindow::tabContextMenuRequested);

        ui.levelsTab->tabBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

        //objectsFolderBrowseBtn
        FileFormatManager::instance()->registerLevelExtension("lvl", new LevelFormatLVL());
        FileFormatManager::instance()->registerLevelExtension("nw", new LevelFormatNW());
        FileFormatManager::instance()->registerLevelExtension(QStringList({ "graal", "zelda", "editor"}), new LevelFormatGraal());


         

        QFileInfo fileInfo(objectsFolder);
        objectsFolder = fileInfo.absoluteFilePath();

        ui.objectsFolderEdit->setText(objectsFolder);


        m_objectManager->mergeResourceManager(m_resourceManager);

        m_objectManager->populateDirectory(ui.objectTree->invisibleRootItem(), objectsFolder);
            
        QAction* themeAction = new QAction(QIcon(":/MainWindow/icons/tinycolor/icons8-theme-16.png"), "Select Theme");
        m_menuThemesGroup = new QActionGroup(&m_menuThemes);

        themeAction->setMenu(&m_menuThemes);


        ui.mainToolBar->insertAction(ui.actionAbout, themeAction);


        if (settings.contains("viewPositionsMax"))
            m_maxScrollPositions = settings.value("viewPositionsMax").toInt();

        auto scrollPositions = settings.value("viewPositions").toStringList();
        for (auto& item : scrollPositions)
        {
            QStringList parts = item.split(';');
            if (parts.size() >= 3)
                m_scrollPositionsCache.push_back(QPair<QString, QPointF>(parts[0], QPointF(parts[1].toFloat(), parts[2].toFloat())));
        }

        if (settings.contains("tileDefs"))
        {
            auto tileDefs = settings.value("tileDefs").toStringList();
            for (auto& line : tileDefs)
            {
                auto parts = line.split(":", Qt::SplitBehaviorFlags::SkipEmptyParts);
                if (parts.size() >= 4)
                {
                    addTileDef2(parts[0], parts[1], parts[2].toInt(), parts[3].toInt(), true);
                }
            }
        }

        static_cast<QToolButton*>(ui.mainToolBar->widgetForAction(themeAction))->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
        connect(ui.objectTree, &QTreeWidget::itemExpanded, this, &MainWindow::objectFolderExpanded);
        connect(ui.objectTree, &QTreeWidget::itemPressed, this, &MainWindow::objectSelectionPressed);
        connect(ui.objectTree, &QTreeWidget::itemSelectionChanged, this, &MainWindow::objectSelectionChanged);
        connect(ui.objectTree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::objectSelectionDoubleClicked);
 
        ui.objectTree->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        connect(ui.objectTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::objectsTreeContextMenuRequested);

        ui.labelObjectPreview->installEventFilter(this);
    }

    MainWindow::~MainWindow()
    {
        m_resourceManager->decrementAndDelete();
    }

    void MainWindow::initScriptEngine()
    {

        m_sgsContext = sgs_CreateEngine();
        sgs_CreateMap(m_sgsContext, &m_cppOwnedObjects, 0);
        sgs_SetGlobalByName(m_sgsContext, "$__CPPOBJECTS", m_cppOwnedObjects);

        Tilemap::registerScriptClass(this);
        Level::registerScriptClass(this);
        AbstractLevelEntity::registerScriptClass(this);
        EditorTabWidget::registerScriptClass(this);

        auto guid = [](sgs_Context* C) -> int
        {
            auto uid = QUuid::createUuid().toString(QUuid::WithoutBraces);
            sgs_PushString(C, uid.toUtf8().data());
            return 1;
        };

        auto makeTile = [](sgs_Context* C) -> int
        {

            sgs_Int left, top, type = 0;

            if (sgs_LoadArgs(C, "ii|i", &left, &top, &type))
            {
                sgs_PushInt(C, Tilemap::MakeTile(left, top, type));
                return 1;
            }

            return 0;
        };

        auto makeInvisibleTile = [](sgs_Context* C) -> int
            {

                sgs_Int type = 0;

                if (sgs_LoadArgs(C, "i", &type))
                {
                    sgs_PushInt(C, Tilemap::MakeInvisibleTile(type));
                    return 1;
                }

                return 0;
            };

        auto tileGetType = [](sgs_Context* C) -> int {

            sgs_Int tile;

            if (sgs_LoadArgs(C, "i", &tile))
            {


                sgs_PushInt(C, Tilemap::GetTileType(tile));
                return 1;
            }
            return 0;
            };

        auto tileGetLeft = [](sgs_Context* C) -> int {

            sgs_Int tile;

            if (sgs_LoadArgs(C, "i", &tile))
            {


                sgs_PushInt(C, Tilemap::GetTileX(tile));
                return 1;
            }
            return 0;
            };


        auto tileGetTop = [](sgs_Context* C) -> int {

            sgs_Int tile;

            if (sgs_LoadArgs(C, "i", &tile))
            {
                sgs_PushInt(C, Tilemap::GetTileY(tile));
                return 1;
            }
            return 0;
         };

        auto tileGetTranslucency = [](sgs_Context* C) -> int {

            sgs_Int tile;

            if (sgs_LoadArgs(C, "i", &tile))
            {
                sgs_PushInt(C, Tilemap::GetTileTranslucency(tile));
                return 1;
            }
            return 0;
         };

        sgs_RegFuncConst func[]{
            {"guid", guid},
            {"makeTile", makeTile},
            {"makeInvisibleTile", makeInvisibleTile},
            {"tileGetType", tileGetType},
            {"tileGetLeft", tileGetLeft},
            {"tileGetTop", tileGetTop},
            {"tileGetTranslucency", tileGetTranslucency},
        };

        sgs_RegFuncConsts(m_sgsContext, func, sizeof(func) / sizeof(func[0]));

        sgs_SetGlobalByName(m_sgsContext, "LOAD_STATE_LOADED", sgs_MakeInt(LoadState::STATE_LOADED));
        sgs_SetGlobalByName(m_sgsContext, "LOAD_STATE_FAILED", sgs_MakeInt(LoadState::STATE_FAILED));
        sgs_SetGlobalByName(m_sgsContext, "LOAD_STATE_LOADING", sgs_MakeInt(LoadState::STATE_LOADING));
        sgs_SetGlobalByName(m_sgsContext, "LOAD_STATE_NOT_LOADED", sgs_MakeInt(LoadState::STATE_NOT_LOADED));
    }


    void MainWindow::closeEvent(QCloseEvent* event)
    {

        while (ui.levelsTab->count() > 0)
        {
            if (!closeTabIndex(ui.levelsTab->count() - 1))
            {
                event->ignore();
                return;
            }
        }

        takeWidgetIntoDock(ui.tilesetsPanelContents, nullptr);
        takeWidgetIntoDock(ui.tileObjectsContainer, nullptr);
        takeWidgetIntoDock(ui.objectsContainer, nullptr);

        QSettings settings("settings.ini", QSettings::IniFormat);

        settings.setValue("geometry", this->saveGeometry());


        settings.setValue("windowState", this->saveState());
        settings.setValue("anonymousNPCGeometry", EditAnonymousNPC::savedGeometry);
        settings.setValue("editorObjectGeometry", EditorObject::savedGeometry);
        
        QStringList scrollPositions;
        for (auto& item : m_scrollPositionsCache)
            scrollPositions.push_back(QString("%1;%2;%3").arg(item.first).arg(item.second.x()).arg(item.second.y()));

        
        settings.setValue("viewPositionsMax", m_maxScrollPositions);
        settings.setValue("viewPositions", scrollPositions);
        if (m_objectFolderChanged)
        {
            settings.setValue("objectsFolder", ui.objectsFolderEdit->text());
        }


        for (auto action : m_menuThemes.actions())
        {
            if (action->isChecked())
            {
                settings.setValue("theme", action->text());
                break;
            }
        }
        QStringList tilesets;

        for (auto i = 0; i < m_tilesetList.rowCount(); ++i)
            tilesets.append(m_tilesetList.item(i)->text());
        settings.setValue("tilesets", tilesets);

        QStringList tileDefs;
        for (auto& tileDef : m_tileDefs)
        {
            if(tileDef.saves)
                tileDefs.push_back(QString("%1:%2:%3:%4").arg(tileDef.imageName).arg(tileDef.prefix).arg(tileDef.x).arg(tileDef.y));
        }
        settings.setValue("tiledefs", tileDefs);
        


        auto jsonRoot = cJSON_CreateObject();


        auto jsonRootArray = cJSON_CreateArray();
        for (auto group : m_tileGroupsList)
        {
            auto jsonGroup = cJSON_CreateObject();
            cJSON_AddStringToObject(jsonGroup, "name", group->getName().toLocal8Bit().data());

            auto jsonGroupArray = cJSON_CreateArray();
            for (auto tileObject: *group)
            {
                auto jsonObject = tileObject->serializeJSON();
                cJSON_AddItemToArray(jsonGroupArray, jsonObject);
            }

            cJSON_AddItemToObject(jsonGroup, "objects", jsonGroupArray);

            cJSON_AddItemToArray(jsonRootArray, jsonGroup);


        }

        cJSON_AddItemToObject(jsonRoot, "tileGroups", jsonRootArray);

        auto tileObjectsText = cJSON_Print(jsonRoot);

        QFile fileObjectsHandle("tileObjects.json");
        if (fileObjectsHandle.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text))
        {
            QTextStream stream(&fileObjectsHandle);
            stream << tileObjectsText;
        }

        free(tileObjectsText);
        cJSON_Delete(jsonRoot);

       // event->ignore();
        QMainWindow::closeEvent(event);


    }

    EditorTabWidget* MainWindow::createNewTab(AbstractResourceManager* resourceManager, bool mergeResourceManager)
    {
        auto tabPage = new EditorTabWidget(this, resourceManager);

        if(mergeResourceManager)
            tabPage->getResourceManager()->mergeResourceManager(m_resourceManager);
        

        tabPage->init(&m_tilesetList, &m_tileGroupsList);
        connect(tabPage, &EditorTabWidget::openLevel, this, &MainWindow::openLevel);
        connect(tabPage, &EditorTabWidget::changeTabText, this, &MainWindow::changeTabText);
        connect(tabPage, &EditorTabWidget::setStatusBar, this, &MainWindow::setStatusText);
      

        return tabPage;
    }

    void MainWindow::addTheme(AbstractTheme* theme)
    {
        theme->setCheckable(true);
        m_menuThemes.addAction(theme);
        m_menuThemesGroup->addAction(theme);
        connect(theme, &QAction::triggered, this, &MainWindow::themeTriggered);
    }

    bool MainWindow::eventFilter(QObject* obj, QEvent* event)
    {
        if (obj == ui.labelObjectPreview)
        {
            if (event->type() == QEvent::MouseButtonPress)
            {
                auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
                auto tabWidget = static_cast<EditorTabWidget*>(ui.levelsTab->currentWidget());

                auto items = ui.objectTree->selectedItems();
                if (items.size() > 0)
                {
                    auto item = items.first();
                    if (tabWidget && item && item->type() == 1)
                    {
                        auto objectClass = static_cast<ObjectClass*>(item);
                        tabWidget->addNewObjectClass(objectClass);
                    }
                }
            }
        }
        return QMainWindow::eventFilter(obj, event);
    }

    void MainWindow::sgsMsgFunc(void* userVal, sgs_Context* ctx, int b, const char* text)
    {
        auto self = static_cast<MainWindow*>(userVal);
        self->setScriptEngineLastError(QString(text));
    }


    EditorTabWidget* MainWindow::openLevelFilename(const QString & fileName)
    {
        AbstractResourceManager* resourceManager = nullptr;
        QFileInfo fi(fileName);

        auto rootDirectory = fi.absolutePath();
        if (!rootDirectory.endsWith('/'))
            rootDirectory += '/';

        //Check existing tabs to see if they share the same root dir
        //If they are, then inherit the sub-folder list from that tab instead of performing
        //a new search

        bool mergeResourceManager = false;
        for (int i = 0; i < ui.levelsTab->count(); ++i)
        {
            auto tab = static_cast<EditorTabWidget*>(ui.levelsTab->widget(i));

            
            if (tab->getResourceManager()->getConnectionString() == rootDirectory)
            {
                resourceManager = tab->getResourceManager();
                break;
            }
        }

        if (resourceManager == nullptr)
        {
            auto newResourceManager = new ResourceManagerFileSystem(rootDirectory, m_objectManager);
            mergeResourceManager = true;
            newResourceManager->addSearchDirRecursive(rootDirectory, 1);
            m_resourceManager->mergeResourceManager(newResourceManager);

            m_objectManager->mergeResourceManager(newResourceManager);
            resourceManager = newResourceManager;

        }

        auto tabPage = createNewTab(resourceManager, mergeResourceManager);

        ui.levelsTab->addTab(tabPage, fi.fileName());
        //ui.levelsTab->setCurrentWidget(tabPage);


        if (fi.suffix() == "gmap" || fi.suffix() == "world" || fi.suffix() == "txt")
            tabPage->loadOverworld(fi.fileName(), fileName);
        else tabPage->loadLevel(fi.fileName(), fileName);

        for (auto& item : m_scrollPositionsCache)
        {
            if (item.first == fileName)
            {
                tabPage->centerView(item.second);
                break;
            }
        }
        
        m_objectManager->loadAllExpanded();
        return tabPage;

    }

    QToolBar* MainWindow::getToolBar()
    {
        return ui.mainToolBar;
    }

    QString MainWindow::parseInlineString(const QString& expression)
    {
        if (expression.startsWith("#"))
        {
            return parseExpression(expression.mid(1));
        }
        return expression;
    }

    QString MainWindow::parseExpression(const QString& expression)
    {
        QString newExpression = QString("return %1;").arg(expression);

        auto startSize = sgs_StackSize(m_sgsContext);
        if (sgs_EvalString(m_sgsContext, newExpression.toUtf8().data()))
        {
            
            int count = sgs_StackSize(m_sgsContext) - startSize;
            if (count >= 1)
            {
                sgs_Variable var;
                sgs_StoreVariable(m_sgsContext, &var);
                QString retval = sgs_ToStringBufFastP(m_sgsContext, &var, nullptr);
                sgs_Release(m_sgsContext, &var);
                
                if (count >= 2)
                    sgs_Pop(m_sgsContext, count - 1);
                return retval;
            }
        }

        return expression;
    }

    void MainWindow::addSearchDir(const QString& dir)
    {
        m_resourceManager->addSearchDirRecursive(dir, 3);
    }

    bool MainWindow::testCodeForErrors(const QString& code, QString* errorOutput, ScriptingLanguage language)
    {
        if (language == ScriptingLanguage::SCRIPT_SGSCRIPT)
        {
            char* output = nullptr;
            size_t outputLen = 0;

            sgs_SetMsgFunc(m_sgsContext, sgsMsgFunc, this);

            if (sgs_Compile(m_sgsContext, code.toUtf8().data(), code.size(), &output, &outputLen) == SGS_SUCCESS)
            {
                sgs_Free(m_sgsContext, output);
                *errorOutput = "";
                return true;
            }
            *errorOutput = m_scriptEngineLastError;
        }
        else if(language == ScriptingLanguage::SCRIPT_GS1)
        {
            
            auto observer = [](const gs1::Diag& d, void* userPointer) ->void
            {
                    if (d.severity == gs1::Diag::Error)
                    {
                        auto self = static_cast<MainWindow*>(userPointer);

                        auto error = std::format("[line {0}] {1}", d.pos.line + 1, d.message);
                        self->setScriptEngineLastError(QString::fromStdString(error));
                    }

            };

            setScriptEngineLastError("");
            gs1::DiagBuilder diagBuilder(observer, this);

            std::string code2 = code.toStdString();
            gs1::MemorySource source(code2.c_str(), code2.length());
            gs1::Lexer lexer(diagBuilder, source);

            gs1::Parser parser(diagBuilder, lexer, gs1::prototypes_cmds, gs1::prototypes_funcs);

            parser.Parse();

            auto error = m_scriptEngineLastError;

            *errorOutput = error;
            return m_scriptEngineLastError.isEmpty();

        }
        return false;
    }

    QString MainWindow::escapeString(const QString& text, ScriptingLanguage language)
    {
        if (language == SCRIPT_SGSCRIPT)
            return "\"" + QString(text).replace("\"", "\\\"") + "\"";
        return text;
    }

    void MainWindow::addCPPOwnedObject(sgs_Variable& var)
    {
        sgs_SetIndex(m_sgsContext, m_cppOwnedObjects, var, sgs_MakeBool(1), false);
    }

    void MainWindow::removeCPPOwnedObject(sgs_Variable& var)
    {
        sgs_Unset(m_sgsContext, m_cppOwnedObjects, var);
    }

    void MainWindow::setErrorText(const QString& text, int seconds)
    {
        setStatusText(text, 1, seconds * 1000);
    }


    void MainWindow::addTileDef2(const QString& image, const QString& levelStart, int x, int y, bool saved)
    {
        for (auto& tileDef : m_tileDefs)
        {
            if (tileDef.prefix == levelStart && tileDef.imageName == image)
                return;
        }

        TileDef tileDef;
        tileDef.imageName = image;
        tileDef.prefix = levelStart;
        tileDef.x = x;
        tileDef.y = y;
        tileDef.saves = saved;

        m_tileDefs.push_back(tileDef);

        for (auto i = 0; i < ui.levelsTab->count(); ++i)
        {
            auto tab = static_cast<EditorTabWidget*>(ui.levelsTab->widget(i));
            tab->applyTileDef2(tileDef);
        }

        auto currentTab = static_cast<EditorTabWidget*>(ui.levelsTab->currentWidget());
        if (currentTab)
        {
            auto levels = currentTab->getLevelsInRect(currentTab->getViewRect());
            for (auto level : levels)
            { 
                if (level->getName().startsWith(levelStart))
                {
                    currentTab->redrawScene();
                    break;
                }
            }
        }
    }

    void MainWindow::applyTileDefs(Level* level)
    {
        for (auto& tileDef : m_tileDefs)
        {
            if (level->getName().startsWith(tileDef.prefix))
            {
                level->addTileDef(tileDef);
            }

        }
    }

    void MainWindow::openFile(bool checked)
    {
        auto allLevelExtensions = FileFormatManager::instance()->getAllLevelLoadExtensions();
        for (auto& ext : allLevelExtensions)
            ext = "*." + ext;

        auto allSupportedFilesFilter = QString("All Supported Files (*.gmap *.world *.txt %1)").arg(allLevelExtensions.join(" "));
        auto filters = QStringList({ allSupportedFilesFilter, "Overworld Files (*.gmap *.world *.txt)", FileFormatManager::instance()->getLevelLoadFilters(), "All Files (*.*)"}).join(";;");
        auto fileNames = QFileDialog::getOpenFileNames(nullptr, "Select Levels", QString(), filters);
        
        EditorTabWidget* lastTab = nullptr;
        if (!fileNames.isEmpty())
        {
            for(auto& fileName: fileNames)
                lastTab = openLevelFilename(fileName);

        }

        if (lastTab)
            ui.levelsTab->setCurrentWidget(lastTab);
    }

    void MainWindow::newLevel(bool checked)
    {
        auto resourceManager = new ResourceManagerFileSystem(m_resourceManager->getConnectionString(), m_objectManager);

        auto tabPage = createNewTab(resourceManager, true);
        tabPage->newLevel("nw", 64, 64);

        ui.levelsTab->addTab(tabPage, "new");

        m_objectManager->mergeResourceManager(tabPage->getResourceManager());
        m_objectManager->loadAllExpanded();
    }

    void MainWindow::newCustomLevel(bool checked)
    {
        
        NewLevelDialog* form = new NewLevelDialog(this);

       
        if (form->exec() == QDialog::Accepted)
        {
            auto resourceManager = new ResourceManagerFileSystem(m_resourceManager->getConnectionString(), m_objectManager);

            auto tabPage = createNewTab(resourceManager, true);

            tabPage->newLevel(form->getFormat(), form->getHCount(), form->getVCount());
           
           
            ui.levelsTab->addTab(tabPage, "new");
            m_objectManager->mergeResourceManager(tabPage->getResourceManager());
            m_objectManager->loadAllExpanded();
        }
        delete form;
    }

    void MainWindow::actionCloseAll(bool checked)
    {
        int currentTabIndex = ui.levelsTab->currentIndex();
        bool resetCurrentTab = false;
        {
            //Block signals from levelTab to stop the signal "QTabWidget::currentChanged" which will cause slow
            //Down. We only need to call this once at the end
            const QSignalBlocker blocker1(ui.levelsTab);
            for (int i = ui.levelsTab->count() - 1; i >= 0; --i)
            {
                if (!closeTabIndex(i))
                    break;

                if (i == currentTabIndex)
                    resetCurrentTab = true;
            }
        }
        
        //If our tab was closed and there are still tabs open, set current tab to the last
        if (ui.levelsTab->count() && resetCurrentTab)
            ui.levelsTab->setCurrentIndex(ui.levelsTab->count() - 1);
    }


    void MainWindow::openLevel(const QString& levelName)
    {

        for (auto i = 0; i < ui.levelsTab->count(); ++i)
        {
            auto tabPage = static_cast<EditorTabWidget*>(ui.levelsTab->widget(i));

            if (tabPage->containsLevel(levelName))
            {
                tabPage->centerLevel(levelName);
                ui.levelsTab->setCurrentWidget(tabPage);
                return;
            }
        }

        auto sourceTab = static_cast<EditorTabWidget*>(this->sender());

        QString fullPath;
        if (sourceTab->getResourceManager()->locateFile(levelName, &fullPath))
        {
            auto newTab = openLevelFilename(fullPath);
            if (newTab)
                ui.levelsTab->setCurrentWidget(newTab);
        }
        
    }

    void MainWindow::changeTabText(const QString& text)
    {
        auto sourceTab = static_cast<EditorTabWidget*>(this->sender());

        auto index = ui.levelsTab->indexOf(sourceTab);
        if (index >= 0)
        {
            ui.levelsTab->setTabText(index, text);
        }
    }

    void MainWindow::setStatusText(const QString& text, int section, int msecs)
    {
        if (section == 0)
            m_statusLeft->setText(text);
        else if (section == 1)
            m_statusMiddle->setText(text);

        //ui.statusBar->showMessage(text, msecs);
    }

    void MainWindow::tabChanged(int index)
    {

        if (index == -1)
        {
            takeWidgetIntoDock(ui.tilesetsPanelContents, nullptr);
            takeWidgetIntoDock(ui.tileObjectsContainer, nullptr);
            takeWidgetIntoDock(ui.objectsContainer, nullptr);
        }
        else {

            auto tabPage = static_cast<EditorTabWidget*>(ui.levelsTab->currentWidget());


            takeWidgetIntoDock(ui.tilesetsPanelContents, tabPage->getTilesetsContainer());
            takeWidgetIntoDock(ui.tileObjectsContainer, tabPage->getTileObjectsContainer());
            takeWidgetIntoDock(ui.objectsContainer, tabPage->getObjectsContainer());
        }
    }

    void MainWindow::tabContextMenuRequested(const QPoint& pos)
    {
        closeTabIndex(ui.levelsTab->tabBar()->tabAt(pos));

    }

    void MainWindow::aboutClicked(bool checked)
    {
        AboutDialog frm;
        frm.exec();
    }

    void MainWindow::closeTabIndexSlot(int index)
    {
        closeTabIndex(index);
        
    }

    void MainWindow::levelConvert(bool checked)
    {
        LevelConverter converter(this, &m_tilesetList);
        converter.exec();
    }

    void MainWindow::preferencesClicked(bool checked)
    {
        EditTileDefs form(m_tileDefs, this);

        if (form.exec())
        {
            for (int i = 0; i < ui.levelsTab->count(); ++i)
            {
                auto tab = static_cast<EditorTabWidget*>(ui.levelsTab->widget(i));
                tab->removeTileDefs("");
            }

            m_tileDefs.clear();
            for (auto& tileDef : form.getTileDefs())
            {
                addTileDef2(tileDef.imageName, tileDef.prefix, tileDef.x, tileDef.y, tileDef.saves);
            }
        }
    }

    void MainWindow::actionLaunchAniEditor(bool checked)
    {
        if (m_settings.value("GaniEditor/WorkingDirectory").toString() == m_resourceManager->getConnectionString())
        {
            auto w = new AniEditorWindow(m_app, m_settings, m_resourceManager, nullptr);
            w->show();
        }
        else {
            auto w = new AniEditorWindow(m_app, m_settings, nullptr, nullptr);
            w->show();
        }
    }



    void MainWindow::objectsFolderBrowseClicked(bool checked)
    {
        QString newDirectory = QFileDialog::getExistingDirectory();

        if (!newDirectory.isEmpty())
        {
            m_objectFolderChanged = true;
            ui.objectsFolderEdit->setText(newDirectory);
           
            m_objectManager->changeFolder(newDirectory);

            auto tabWidget = static_cast<EditorTabWidget*>(ui.levelsTab->currentWidget());

            if (tabWidget)
            {
                m_objectManager->loadAllExpanded();
            }

        }
    }

    void MainWindow::objectFolderExpanded(QTreeWidgetItem* item)
    {
        

        m_objectManager->loadAllExpanded(item);
        
    }

    void MainWindow::objectSelectionChanged()
    {
        if (ui.objectTree->selectedItems().size() > 0)
        {
            auto widgetItem = ui.objectTree->selectedItems().first();
            if (widgetItem->type() == 1)
            {
                auto objectClass = static_cast<ObjectClass*>(widgetItem);

                objectClass->load(m_objectManager->getResourceManager(), false);
                auto image = objectClass->getImage();
                if (!image.isNull())
                {
                    ui.labelObjectPreview->setPixmap(image);
                    ui.labelObjectPreview->setFixedSize(image.width() + 32, image.height() + 32);
                }
                else ui.labelObjectPreview->setPixmap(QPixmap());

            }
        }
    }

    void MainWindow::objectSelectionPressed(QTreeWidgetItem* item, int column)
    {
        if (QApplication::mouseButtons().testFlag(Qt::MouseButton::LeftButton))
        {
            auto tabWidget = static_cast<EditorTabWidget*>(ui.levelsTab->currentWidget());

            if (tabWidget && item && item->type() == 1)
            {


                auto objectClass = static_cast<ObjectClass*>(item);
                tabWidget->addNewObjectClass(objectClass);



            }
        }
        
    }

    void MainWindow::objectSelectionDoubleClicked(QTreeWidgetItem* item, int column)
    {
        if (QApplication::mouseButtons().testFlag(Qt::MouseButton::LeftButton))
        {
            auto tabWidget = static_cast<EditorTabWidget*>(ui.levelsTab->currentWidget());
            AbstractResourceManager* resourceManager = tabWidget == nullptr ? m_resourceManager : tabWidget->getResourceManager();

            if (item && item->type() == 1)
            {
                auto objectClass = static_cast<ObjectClass*>(item);
                auto form = new EditorObject(this, objectClass->getName(), objectClass->getFullPath(), resourceManager);

                form->exec();
                //Changes were made and saved. let's reload this object
                if (form->changesSaved())
                {
                    objectClass->load(resourceManager, false);
                    objectClass->markInstancesModified();
                }
                delete form;

                if (tabWidget)
                    tabWidget->setSelection(nullptr);

            }
        }
    }

    void MainWindow::objectsTreeContextMenuRequested(const QPoint& pos)
    {
        auto tabWidget = static_cast<EditorTabWidget*>(ui.levelsTab->currentWidget());
        AbstractResourceManager* resourceManager = tabWidget == nullptr ? m_resourceManager : tabWidget->getResourceManager();

        QMenu menu(this);

        auto actionNewObjectClass = menu.addAction("New Object Class");
        auto actionNewFolder = menu.addAction("New Folder");
        auto actionImportGraalNPC = menu.addAction("Import Graal NPC");
        auto actionEditObject = menu.addAction("Edit Object Class");
        auto actionDeleteObject = menu.addAction("Delete Object Class");

        actionEditObject->setEnabled(false);
        actionDeleteObject->setEnabled(false);

        auto item = ui.objectTree->itemAt(pos);

        if (item)
        {
            if (item->type() == 1)
            {
                actionEditObject->setEnabled(true);
                actionDeleteObject->setEnabled(true);
            }
        }

        auto result = menu.exec(ui.objectTree->mapToGlobal(pos));
        if (result)
        {
            if(result == actionEditObject && item && item->type() == 1)
            {
                auto objectClass = static_cast<ObjectClass*>(item);
                auto form = new EditorObject(this, objectClass->getName(), objectClass->getFullPath(), resourceManager);

                form->exec();
                //Changes were made and saved. let's reload this object
                if (form->changesSaved())
                {
                    objectClass->load(resourceManager, false);
                    objectClass->markInstancesModified();
                }
                delete form;
            }
            else if (result == actionDeleteObject && item && item->type() == 1)
            {
                auto objectClass = static_cast<ObjectClass*>(item);
                m_objectManager->deleteObjectClass(objectClass);
            }
            else if (result == actionNewFolder)
            {
                QTreeWidgetItem* parent = nullptr;

                if (item)
                {
                    if (item->type() == 0)
                        parent = item;
                    else parent = item->parent();
                }

                if (parent == nullptr)
                {
                    parent = ui.objectTree->invisibleRootItem();
                }

                QString directory = (item && item->type() == 0) ? m_objectManager->getFolderFullPath(item) : m_objectManager->getRootDirectory();
                auto folderName = QInputDialog::getText(this, "New Folder", "Folder Name:");

                if (!folderName.isNull())
                {
                    QDir dir(directory);
                    if (dir.mkdir(folderName))
                    {
                        m_objectManager->addFolder(parent, folderName);

                        if (parent)
                            parent->setExpanded(true);
                    }
                }

            }
            else if (result == actionNewObjectClass)
            {
                QTreeWidgetItem* parent = nullptr;
                
                if (item)
                {
                    if (item->type() == 0)
                        parent = item;
                    else parent = item->parent();
                }

                if (parent == nullptr)
                {
                    parent = ui.objectTree->invisibleRootItem();
                }

                QString directory = (item && item->type() == 0) ? m_objectManager->getFolderFullPath(item) : m_objectManager->getRootDirectory();
                
                auto newClassName = QInputDialog::getText(this, "New Object Class", "Class Name:");
                if (!newClassName.isEmpty())
                {
                    QString fullPath = QString("%1%2.npc").arg(directory, newClassName);

                    QFile newFile(fullPath);
                    if (newFile.open(QIODeviceBase::WriteOnly))
                    {
                        QTextStream stream(&newFile);

                        stream << "NPC001" << Qt::endl;
                        newFile.close();

                        static_cast<QTreeView*>(ui.objectTree)->selectionModel()->clearSelection();

                        auto newClass = m_objectManager->addNewClass(parent, newClassName, fullPath);

                        parent->setExpanded(true);
                        newClass->setSelected(true);

                        ui.objectTree->scrollTo(ui.objectTree->indexFromItem(newClass));
                    }
                }

            }
            else if (result == actionImportGraalNPC)
            {
                QTreeWidgetItem* parent = nullptr;

                if (item)
                {
                    if (item->type() == 0)
                        parent = item;
                    else parent = item->parent();
                }

                if (parent == nullptr)
                    parent = ui.objectTree->invisibleRootItem();

                QString directory = (item && item->type() == 0) ? m_objectManager->getFolderFullPath(item) : m_objectManager->getRootDirectory();


                auto fileNames = QFileDialog::getOpenFileNames(this, "Select predefined NPC files", QString(), "*.txt");
                if (fileNames.size() > 0)
                {
                    for (auto& fileName: fileNames)
                    {
                        QFile textFile(fileName);
                        if (textFile.open(QIODeviceBase::ReadOnly))
                        {
                            QTextStream textStream(&textFile);
                            QString code = "";
                            QList<QPair<QString, QString>> params;
                            while (!textStream.atEnd())
                            {
                                QString line = textStream.readLine();
                                if (line.isNull())
                                    break;
                                

                                if (line.startsWith("//#EDIT "))
                                {
                                    auto thing = line.mid(strlen("//#EDIT "));
                                    auto pos = thing.indexOf(' ');
                                    QString name = pos == -1 ? thing.trimmed() : thing.mid(0, pos).trimmed();
                                    QString defaultValue = pos == -1 ? "" : thing.mid(pos + 1).trimmed();

                                    params.push_back(QPair<QString, QString>(name, defaultValue));
                                }
                                else code += line + "\n";
                            }
                            textFile.close();

                            int index = 1;
                            for (auto& pair : params)
                            {
                                code = code.replace(pair.first, QString("param%1").arg(index++));
                            }

                            
                            QFileInfo fi(fileName);
                            auto newClassName = fi.baseName();

                            if (newClassName.startsWith("defnpc"))
                                newClassName = newClassName.mid(strlen("defnpc"));

                            QString fullPath = QString("%1%2.npc").arg(directory, newClassName);

                            if (!QFile::exists(fullPath))
                            {
                                QFile newFile(fullPath);
                                if (newFile.open(QIODeviceBase::WriteOnly))
                                {
                                    QTextStream stream(&newFile);

                                    stream << "NPC001" << Qt::endl;

                                    auto match = LevelNPC::RegExSetImg.match(code);
                                    if (match.hasMatch())
                                        stream << "IMAGE " << match.captured(1).trimmed() << Qt::endl;

                                    match = LevelNPC::RegExImgPart.match(code);
                                    if (match.hasMatch())
                                        stream << "IMAGESHAPE " << match.captured(1).trimmed() << " " << match.captured(2).trimmed() << " " << match.captured(3).trimmed() << " " << match.captured(4).trimmed() << Qt::endl;

                                    

                                    auto pos = code.indexOf("//#CLIENTSIDE", 0, Qt::CaseInsensitive);
                                    if (pos != -1)
                                    {
                                        auto clientCode = code.mid(pos);
                                        code = code.mid(0, pos).trimmed();

                                        stream << "CLIENTCODE" << Qt::endl;
                                        stream << clientCode;
                                        if (!clientCode.endsWith("\n"))
                                            stream << Qt::endl;
                                        stream << "CLIENTCODEEND" << Qt::endl;

                                    }

                                    if (!code.isEmpty())
                                    {
                                        stream << "SERVERCODE" << Qt::endl;
                                        stream << code;
                                        if (!code.endsWith("\n"))
                                            stream << Qt::endl;
                                        stream << "SERVERCODEEND" << Qt::endl;
                                    }

                                    stream << "SCRIPTINGLANGUAGE gs1script" << Qt::endl;

                                    for (auto& param : params)
                                    {
                                        stream << "PARAM EXPRESSION " << param.first << Qt::endl;
                                        if (!param.second.isEmpty())
                                            stream << "DEFAULT " << param.second << Qt::endl;
                                        stream << "PARAMEND" << Qt::endl;
                                    }
                                    newFile.close();

                                    static_cast<QTreeView*>(ui.objectTree)->selectionModel()->clearSelection();
                                    auto newClass = m_objectManager->addNewClass(parent, newClassName, fullPath);

                                    parent->setExpanded(true);
                                    newClass->setSelected(true);

                                    ui.objectTree->scrollTo(ui.objectTree->indexFromItem(newClass));
                                }
                            }
                            else QMessageBox::critical(nullptr, "Error", "File Name already exists");
                        }
                    }
                }
            }
        }
    }

    void MainWindow::themeTriggered(bool checked)
    {
        m_app.setPalette(m_defaultPalette);
        auto theme = static_cast<AbstractTheme*>(this->sender());
        theme->applyTheme(this, m_app);
    }

    void MainWindow::editCustomThemesTriggered(bool checked)
    {
        NewCustomTheme form(m_menuThemes, m_menuThemesGroup);

        auto theme = new GenericTheme("LOL", "test.qqs", false);
        form.addTheme(theme);
        form.exec();
    }

    void MainWindow::takeWidgetIntoDock(QWidget* dockContainer, QWidget* target)
    {
  
        QLayoutItem* child = NULL;
        //Remove any existing widgets in this dock
        while ((child = dockContainer->layout()->itemAt(0)) != NULL) {
            child->widget()->hide();
            child->widget()->setParent(NULL);
        }


        if (target != nullptr) {
            dockContainer->layout()->addWidget(target);
            target->show();
        }
    }

    void MainWindow::loadTileObjects()
    {
        static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        QFile fileObjectsHandle("tileObjects.json");
        if (fileObjectsHandle.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text))
        {
            QTextStream stream(&fileObjectsHandle);

            auto text = stream.readAll();

            auto jsonRoot = cJSON_Parse(text.toLocal8Bit().data());


            if (jsonRoot->type == cJSON_Object)
            {
                auto jsonGroups = cJSON_GetObjectItem(jsonRoot, "tileGroups");
                if (jsonGroups)
                {
                    for (auto i = 0; i < cJSON_GetArraySize(jsonGroups); ++i)
                    {
                        auto jsonGroup = cJSON_GetArrayItem(jsonGroups, i);
                        if (jsonGroup)
                        {
                            if (jsonGroup->type == cJSON_Object)
                            {
                                auto groupName = jsonGetChildString(jsonGroup, "name");

                                auto tileGroup = new TileGroupModel();
                                tileGroup->setName(groupName);

                                auto jsonObjects = cJSON_GetObjectItem(jsonGroup, "objects");
                                if (jsonObjects)
                                {
                                    for (auto ii = 0; ii < cJSON_GetArraySize(jsonObjects); ++ii)
                                    {
                                        auto jsonTileObject = cJSON_GetArrayItem(jsonObjects, ii);
                                        if (jsonTileObject)
                                        {
                                            if (jsonTileObject->type == cJSON_Object)
                                            {
                                                auto objectName = jsonGetChildString(jsonTileObject, "name");
                                                auto hcount = jsonGetChildInt(jsonTileObject, "hcount");
                                                auto vcount = jsonGetChildInt(jsonTileObject, "vcount");

                                                auto tileObject = new TileObject(hcount, vcount);
                                                tileObject->setName(objectName);

                                                auto tileArray = cJSON_GetObjectItem(jsonTileObject, "tiles");
                                                if (tileArray)
                                                {
                                                    for (int y = 0; y < cJSON_GetArraySize(tileArray); ++y)
                                                    {
                                                        auto arrayItem = cJSON_GetArrayItem(tileArray, y);
                                                        if (arrayItem->type == cJSON_String)
                                                        {
                                                            QString line(arrayItem->valuestring);

                                                            auto parts = line.split(' ', Qt::SkipEmptyParts);
                                                            for (auto x = 0U; x < parts.size(); ++x)
                                                            {
                                                                int tile = 0;
                                                                auto& part = parts[x];
                                                                int bitcount = 0;


                                                                for (auto i = part.length() - 1; i >= 0; --i) {
                                                                    auto value = base64.indexOf(part[i]);

                                                                    tile |= value << bitcount;
                                                                    bitcount += 6;

                                                                }
                                                                tileObject->setTile(x, y, tile);
                                                            }

                                                        }
                                                    }
                                                }

                                                tileGroup->addTileObject(tileObject);

                                            }
                                        }
                                    }
                                }

                                m_tileGroupsList.addTileGroup(tileGroup);
                            }
                        }
                    }
                }
            }
        }

    }

    bool MainWindow::closeTabIndex(int index)
    {
        auto tab = static_cast<EditorTabWidget*>(ui.levelsTab->widget(index));


        if (tab->getModified())
        {

            switch (QMessageBox::question(nullptr, tab->getName(), "This file has not been saved. Do you want to save it?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
            {
            case QMessageBox::Yes:
                tab->saveClicked(false);
                break;
            case QMessageBox::Cancel:
                return false;
            }
        }

        auto fileName = tab->getFileName();

        if (!fileName.isEmpty())
        {
            for (auto it = m_scrollPositionsCache.begin(); it != m_scrollPositionsCache.end(); ++it)
            {
                auto& item = *it;
                if (item.first == fileName)
                {
                    m_scrollPositionsCache.erase(it);
                    break;
                }
            }

            while (m_scrollPositionsCache.size() > m_maxScrollPositions)
                m_scrollPositionsCache.pop_front();
            m_scrollPositionsCache.push_back(QPair<QString, QPointF>(fileName, tab->getCenterPoint()));
        }

        ui.levelsTab->removeTab(index);

        tab->deleteLater();
        return true;
    }
};

