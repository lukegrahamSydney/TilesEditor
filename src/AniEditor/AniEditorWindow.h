#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QMenu>
#include <QActionGroup>
#include "ui_AniEditorWindow.h"
#include "ResourceManagerFileSystem.h"
#include "AniEditor.h"
#include "AbstractTheme.h"

namespace TilesEditor
{
	class AniEditorWindow : public QMainWindow
	{
		Q_OBJECT

	private slots:
		void actionNewClicked(bool checked);
		void actionOpenClicked(bool checked);
		void actionSaveClicked(bool checked);
		void actionSaveAsClicked(bool checked);
		void actionSaveAllClicked(bool checked);
		void actionCloseAllClicked(bool checked);
		void actionSetBackgroundColour(bool checked);

		void actionSetWorkingDirectoryClicked(bool checked);
		void tabClose(int index);
		void tabContextMenuRequested(const QPoint& pos);
		void changeTabText(const QString& text);
		void setStatusText(const QString& text);
		void themeTriggered(bool checked);

	public:
		AniEditorWindow(QApplication& app, QSettings& settings, AbstractResourceManager* resourceManager, QWidget* parent = nullptr);
		~AniEditorWindow();

		void addThemesButton();
		void addTheme(AbstractTheme* theme);

	protected:
		bool eventFilter(QObject* object, QEvent* event) override;

	private:
		QApplication& m_app;
		Ui::AniEditorWindowClass ui;
		QString m_openFileDirectory;
		QSettings& m_settings;
		QColor m_backgroundColor = QColorConstants::DarkGreen;
		ResourceManagerFileSystem* m_resourceManager;

		QMenu m_menuThemes;
		QActionGroup* m_menuThemesGroup;

		void saveTab(AniEditor* editor, bool forceSaveAs);
		bool closeTabIndex(int index);
		void closeEvent(QCloseEvent* event) override;

	};
};
