#ifndef MAINWINDOWH
#define MAINWINDOWH

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include "ui_MainWindow.h"
#include "TileGroupListModel.h"
#include "ResourceManager.h"
#include "EditorTabWidget.h"

namespace TilesEditor
{
    class MainWindow : public QMainWindow
    {
        Q_OBJECT
    public slots:
        void openFile(bool checked);
        void newLevel(bool checked);
        void openLevel(const QString& levelName);
        void changeTabText(const QString& text);
        void setStatusText(const QString& text, int section, int msecs);
        void tabChanged(int index);
        void aboutClicked(bool checked);
        void closeTabIndexSlot(int index);

    public:
        MainWindow(QWidget* parent = nullptr);
        ~MainWindow();

        EditorTabWidget* openLevelFilename(const QString& fileName);

    private:
        Ui::MainWindowClass ui;

        ResourceManager m_resourceManager;
        QStandardItemModel m_tilesetList;
        TileGroupListModel m_tileGroupsList;

        QLabel* m_statusLeft;
        QLabel* m_statusMiddle;

        //Brings a widget into the specified dock
        void takeWidgetIntoDock(QWidget* dockContainer, QWidget* target);
        void loadTileObjects();
        bool closeTabIndex(int index);
        void closeEvent(QCloseEvent* event) override;
        EditorTabWidget* createNewTab();

    };
}

#endif
