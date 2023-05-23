#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include "MainWindow.h"
#include "EditorTabWidget.h"
#include "AboutDialog.h"
#include "cJSON/JsonHelper.h"
#include "EditAnonymousNPC.h"
#include "MainFileSystem.h"
#include "LevelConverter.h"

namespace TilesEditor
{

    MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent), m_resourceManager(&m_mainFileSystem)
    {
        ui.setupUi(this);
        QMainWindow::setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

        // Do multipart status bar
        m_statusLeft = new QLabel("", this);
        m_statusLeft->setFrameStyle(QFrame::Panel | QFrame::Sunken);

        m_statusMiddle = new QLabel("", this);
        m_statusMiddle->setFrameStyle(QFrame::Panel | QFrame::Sunken);

        statusBar()->addPermanentWidget(m_statusLeft, 0);
        statusBar()->addPermanentWidget(m_statusMiddle, 1);



        QSettings settings("settings.ini", QSettings::IniFormat);

        restoreGeometry(settings.value("geometry").toByteArray());
        restoreState(settings.value("windowState").toByteArray());
        EditAnonymousNPC::savedGeometry = settings.value("anonymousNPCGeometry").toByteArray();


        m_resourceManager.addSearchDir("./");
        m_resourceManager.addSearchDir("./levels/");
        m_resourceManager.addSearchDir("./world/");

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
        connect(ui.actionAbout, &QAction::triggered, this, &MainWindow::aboutClicked);
        connect(ui.actionLevelConverter, &QAction::triggered, this, &MainWindow::levelConvert);

        connect(ui.levelsTab, &QTabWidget::currentChanged, this, &MainWindow::tabChanged);
        connect(ui.levelsTab, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTabIndexSlot);
       
    }

    MainWindow::~MainWindow()
    {}

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
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        settings.setValue("anonymousNPCGeometry", EditAnonymousNPC::savedGeometry);


        QStringList tilesets;

        for (auto i = 0; i < m_tilesetList.rowCount(); ++i)
            tilesets.append(m_tilesetList.item(i)->text());
        
        settings.setValue("tilesets", tilesets);


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

    EditorTabWidget* MainWindow::createNewTab()
    {
        auto tabPage = new EditorTabWidget(nullptr, &m_mainFileSystem);
        tabPage->getResourceManager().mergeSearchDirectories(m_resourceManager);

        tabPage->init(&m_tilesetList, &m_tileGroupsList);
        connect(tabPage, &EditorTabWidget::openLevel, this, &MainWindow::openLevel);
        connect(tabPage, &EditorTabWidget::changeTabText, this, &MainWindow::changeTabText);
        connect(tabPage, &EditorTabWidget::setStatusBar, this, &MainWindow::setStatusText);
        
        return tabPage;
    }

    EditorTabWidget* MainWindow::openLevelFilename(const QString & fileName)
    {
        auto tabPage = createNewTab();



        QFileInfo fi(fileName);

        ui.levelsTab->addTab(tabPage, fi.fileName());
        ui.levelsTab->setCurrentWidget(tabPage);

        if (fi.suffix() == "gmap" || fi.suffix() == "world" || fi.suffix() == "txt")
            tabPage->loadOverworld(fi.fileName(), fileName);
        else tabPage->loadLevel(fi.fileName(), fileName);

        return tabPage;

    }

    void MainWindow::openFile(bool checked)
    {
        auto fileName = QFileDialog::getOpenFileName(nullptr, "Select level", QString(), "All supported files (*.nw *.graal *.zelda *.gmap *.lvl *.world *.txt)");
        
        if (!fileName.isEmpty())
        {
            openLevelFilename(fileName);

        }
    }

    void MainWindow::newLevel(bool checked)
    {
        auto tabPage = createNewTab();
        tabPage->newLevel(64, 64);

        ui.levelsTab->addTab(tabPage, "new");
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
        if (sourceTab->getResourceManager().locateFile(levelName, &fullPath))
        {
            auto tab = openLevelFilename(fullPath);

            if (tab && tab != sourceTab)
            {
                tab->getResourceManager().mergeSearchDirectories(sourceTab->getResourceManager());
            }
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
        LevelConverter converter;
        converter.exec();
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
        ui.levelsTab->removeTab(index);

        tab->deleteLater();
        return true;
    }
};

