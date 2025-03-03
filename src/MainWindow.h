#ifndef MAINWINDOWH
#define MAINWINDOWH

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QMap>
#include <QSettings>
#include <QVector>
#include "ui_MainWindow.h"
#include "TileGroupListModel.h"
#include "EditorTabWidget.h"
#include "ObjectManager.h"
#include "AbstractTheme.h"
#include "sgscript/sgscript.h"
#include "IEngine.h"
#include "ResourceManagerFileSystem.h"
#include "TileDefs.h"

namespace TilesEditor
{
    class MainWindow : public QMainWindow,
        public IEngine
    {
        Q_OBJECT
    private slots:
        void openFile(bool checked);
        void newLevel(bool checked);
        void newCustomLevel(bool checked);
        void actionCloseAll(bool checked);
        void openLevel(const QString& levelName);
        void changeTabText(const QString& text);
        void setStatusText(const QString& text, int section, int msecs);
        void tabChanged(int index);
        void tabContextMenuRequested(const QPoint& pos);
        void aboutClicked(bool checked);
        void closeTabIndexSlot(int index);
        void levelConvert(bool checked);
        void preferencesClicked(bool checked);
		void actionLaunchAniEditor(bool checked);

        void objectsFolderBrowseClicked(bool checked);
        void objectFolderExpanded(QTreeWidgetItem* item);
        void objectSelectionChanged();
        void objectSelectionPressed(QTreeWidgetItem* item, int column);
        void objectSelectionDoubleClicked(QTreeWidgetItem* item, int column);
        void objectsTreeContextMenuRequested(const QPoint& pos);
        
        void themeTriggered(bool checked = false);
        void editCustomThemesTriggered(bool checked = false);

    private:
        QApplication& m_app;
        QSettings& m_settings;
        QPalette m_defaultPalette;

        Ui::MainWindowClass ui;
        QMenu m_objectsLibraryContextMenu;

        bool m_objectFolderChanged = false;
        ResourceManagerFileSystem* m_resourceManager;
        ObjectManager* m_objectManager;
        QStandardItemModel m_tilesetList;
        TileGroupListModel m_tileGroupsList;

        int m_maxScrollPositions = 10;
        QList<QPair<QString, QPointF> > m_scrollPositionsCache;

        QLabel* m_statusLeft;
        QLabel* m_statusMiddle;

        QMenu m_menuThemes;
        QActionGroup* m_menuThemesGroup;

        sgs_Context* m_sgsContext = nullptr;
        sgs_Variable m_cppOwnedObjects;
        QString m_scriptEngineLastError;


        QVector<TileDef> m_tileDefs;

        //Brings a widget into the specified dock
        void takeWidgetIntoDock(QWidget* dockContainer, QWidget* target);
        void loadTileObjects();
        bool closeTabIndex(int index);
        void closeEvent(QCloseEvent* event) override;
        EditorTabWidget* createNewTab(AbstractResourceManager* resourceManager, bool mergeResourceManager);



        bool eventFilter(QObject* obj, QEvent* event) override;

        static void sgsMsgFunc(void* userVal, sgs_Context* ctx, int b, const char* text);
        void setScriptEngineLastError(const QString& text) { m_scriptEngineLastError = text; }
        void initScriptEngine();

    public:
        MainWindow(QApplication& app, QSettings& settings, QWidget* parent = nullptr);
        ~MainWindow();

        EditorTabWidget* openLevelFilename(const QString& fileName);

        QToolBar* getToolBar();
        QString parseInlineString(const QString& expression) override;
        QString parseExpression(const QString& expression) override;
        AbstractResourceManager* getResourceManager() override { return m_resourceManager; }
        ObjectManager* getObjectManager() override { return m_objectManager; }
        bool testCodeForErrors(const QString& code, QString* errorOutput, ScriptingLanguage language) override;
        QString escapeString(const QString& text, ScriptingLanguage language) override;
        sgs_Context* getScriptContext() override { return m_sgsContext; }
        void addCPPOwnedObject(sgs_Variable& var) override;
        void removeCPPOwnedObject(sgs_Variable& var) override;

        void setErrorText(const QString& text, int seconds = 10) override;
        void addTileDef2(const QString& image, const QString& levelStart, int x, int y, bool saved) override;
        void applyTileDefs(Level* level) override;

        void addTheme(AbstractTheme* theme);
    

    };
}

#endif
