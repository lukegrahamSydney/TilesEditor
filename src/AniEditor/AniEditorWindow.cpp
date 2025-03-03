#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include "AniEditorWindow.h"
#include "AniEditor.h"
#include "EditScriptForm.h"

namespace TilesEditor
{

	AniEditorWindow::AniEditorWindow(QApplication& app, QSettings& settings, AbstractResourceManager* resourceManager, QWidget* parent)
		: QMainWindow(parent), m_app(app), m_settings(settings)
	{ 
		ui.setupUi(this);

		
		connect(ui.actionNew, &QAction::triggered, this, &AniEditorWindow::actionNewClicked);
		connect(ui.actionOpen, &QAction::triggered, this, &AniEditorWindow::actionOpenClicked);
		connect(ui.actionSave, &QAction::triggered, this, &AniEditorWindow::actionSaveClicked);
		connect(ui.actionSaveAs, &QAction::triggered, this, &AniEditorWindow::actionSaveAsClicked);
		connect(ui.actionSaveAll, &QAction::triggered, this, &AniEditorWindow::actionSaveAllClicked);
		connect(ui.actionCloseAll, &QAction::triggered, this, &AniEditorWindow::actionCloseAllClicked);
		connect(ui.actionBackgroundColor, &QAction::triggered, this, &AniEditorWindow::actionSetBackgroundColour);
		connect(ui.actionSetWorkingDirectory, &QAction::triggered, this, &AniEditorWindow::actionSetWorkingDirectoryClicked);
		connect(ui.tabWidgetMain, &QTabWidget::tabCloseRequested, this, &AniEditorWindow::tabClose);

		connect(ui.tabWidgetMain->tabBar(), &QTabBar::customContextMenuRequested, this, &AniEditorWindow::tabContextMenuRequested);
		
		ui.tabWidgetMain->tabBar()->installEventFilter(this);

		ui.tabWidgetMain->tabBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

		if (resourceManager == nullptr)
		{
			QString rootDir = "./";
	
			if (settings.contains("GaniEditor/WorkingDirectory"))
				rootDir = settings.value("GaniEditor/WorkingDirectory").toString();
			else {
				auto folder = QFileDialog::getExistingDirectory(nullptr, "Select working directory (example, the graal.exe folder)");
				if (!folder.isEmpty())
				{
					rootDir = folder;
				}
			}

			m_resourceManager = new ResourceManagerFileSystem(rootDir, nullptr);
			m_resourceManager->incrementRef();



			m_resourceManager->addSearchDir("./sounds/");
			m_resourceManager->addSearchDirRecursive(m_resourceManager->getConnectionString(), 6);



			m_openFileDirectory = rootDir;
		}
		else {
			resourceManager->incrementRef();
			m_openFileDirectory = resourceManager->getConnectionString();
		}

		if (settings.contains("scriptEditorGeometry"))
			EditScriptForm::savedGeometry = settings.value("scriptEditorGeometry").toByteArray();

		if (m_settings.contains("GaniEditor/WindowGeometry"))
			this->restoreGeometry(m_settings.value("GaniEditor/WindowGeometry").toByteArray());

		if (m_settings.contains("GaniEditor/WindowState"))
			this->restoreState(m_settings.value("GaniEditor/WindowState").toByteArray());

		if (m_settings.contains("GaniEditor/BackColor"))
			m_backgroundColor = QColor::fromString(m_settings.value("GaniEditor/BackColor").toString());

	}

	void AniEditorWindow::actionNewClicked(bool checked)
	{
		auto currentEditorTab = static_cast<AniEditor*>(ui.tabWidgetMain->currentWidget());

		auto editor = new AniEditor("", m_resourceManager, m_settings, currentEditorTab);
		editor->setBackgroundColour(m_backgroundColor);

		connect(editor, &AniEditor::changeTabText, this, &AniEditorWindow::changeTabText);
		connect(editor, &AniEditor::setStatusText, this, &AniEditorWindow::setStatusText);

		editor->initNewAni();
		ui.tabWidgetMain->addTab(editor, "new");
	}

	void AniEditorWindow::actionOpenClicked(bool checked)
	{

		auto fileNames = m_resourceManager->getOpenFileNames("Open Files", m_openFileDirectory, "*.gani");

		AniEditor* lastTab = nullptr;
		for (auto& fullPath : fileNames)
		{
			QFileInfo fi(fullPath);

			m_openFileDirectory = fi.absolutePath();


			auto currentEditorTab = static_cast<AniEditor*>(ui.tabWidgetMain->currentWidget());
			lastTab = new AniEditor(fi.fileName(), m_resourceManager, m_settings, currentEditorTab);
			lastTab->setBackgroundColour(m_backgroundColor);
			connect(lastTab, &AniEditor::changeTabText, this, &AniEditorWindow::changeTabText);
			connect(lastTab, &AniEditor::setStatusText, this, &AniEditorWindow::setStatusText);

			lastTab->loadFile(fi.fileName(), fullPath);
			ui.tabWidgetMain->addTab(lastTab, fi.fileName());
			
		}

		if (lastTab)
			ui.tabWidgetMain->setCurrentWidget(lastTab);
	}

	void AniEditorWindow::actionSaveClicked(bool checked)
	{
		auto editor = static_cast<AniEditor*>(ui.tabWidgetMain->currentWidget());
		if (editor)
		{
			saveTab(editor, false);
		}
	}

	void AniEditorWindow::actionSaveAsClicked(bool checked)
	{
		auto editor = static_cast<AniEditor*>(ui.tabWidgetMain->currentWidget());
		if (editor)
		{
			saveTab(editor, true);
		}
	}

	void AniEditorWindow::actionSaveAllClicked(bool checked)
	{
		for (int i = ui.tabWidgetMain->count() - 1; i >= 0; --i)
		{
			auto editor = static_cast<AniEditor*>(ui.tabWidgetMain->widget(i));

			saveTab(editor, false);
		}
	}

	void AniEditorWindow::actionCloseAllClicked(bool checked)
	{
		auto currentEditor = static_cast<AniEditor*>(ui.tabWidgetMain->currentWidget());
		if (currentEditor)
			currentEditor->saveSettings(m_settings);

		for (int i = ui.tabWidgetMain->count() - 1; i >= 0; --i)
		{
			closeTabIndex(i);
		}
	}

	void AniEditorWindow::actionSetBackgroundColour(bool checked)
	{
		QColorDialog dialog(m_backgroundColor);

		if (dialog.exec())
		{
			m_backgroundColor = dialog.selectedColor();
			m_settings.setValue("GaniEditor/BackColor", m_backgroundColor.name());

			for (int i = 0; i < ui.tabWidgetMain->count(); ++i)
			{
				auto editor = static_cast<AniEditor*>(ui.tabWidgetMain->widget(i));
				editor->setBackgroundColour(m_backgroundColor);
			}
		}

	}

	void AniEditorWindow::actionSetWorkingDirectoryClicked(bool checked)
	{
		auto folder = QFileDialog::getExistingDirectory(nullptr, "Select working directory (example, the graal.exe folder)", m_resourceManager->getConnectionString());
		if (!folder.isEmpty())
		{
			m_resourceManager->setRootDir(folder);
			m_resourceManager->addSearchDirRecursive(folder, 3);
		}
	}

	void AniEditorWindow::tabClose(int index)
	{
		
		closeTabIndex(index);
	}

	void AniEditorWindow::tabContextMenuRequested(const QPoint& pos)
	{
		closeTabIndex(ui.tabWidgetMain->tabBar()->tabAt(pos));


	}

	void AniEditorWindow::changeTabText(const QString& text)
	{
		auto sourceTab = static_cast<AniEditor*>(this->sender());

		auto index = ui.tabWidgetMain->indexOf(sourceTab);
		if (index >= 0)
		{
			ui.tabWidgetMain->setTabText(index, text);
		}
	}

	void AniEditorWindow::setStatusText(const QString& text)
	{
		ui.statusBar->showMessage(text);
	}

	void AniEditorWindow::themeTriggered(bool checked)
	{
		auto theme = static_cast<AbstractTheme*>(this->sender());
		theme->applyTheme(this, m_app);

		m_settings.setValue("theme", theme->text());
	}

	AniEditorWindow::~AniEditorWindow()
	{
		m_resourceManager->decrementAndDelete();
	}

	bool AniEditorWindow::eventFilter(QObject * object, QEvent * event)
	{
		if (object == ui.tabWidgetMain->tabBar())
		{
			if (event->type() == QEvent::MouseButtonPress)
			{
				auto pos = dynamic_cast<QMouseEvent*>(event)->pos();
				auto index = ui.tabWidgetMain->tabBar()->tabAt(pos);
				
				if (index != ui.tabWidgetMain->currentIndex())
				{
					auto editor = static_cast<AniEditor*>(ui.tabWidgetMain->currentWidget());
					if (editor)
					{
						editor->stop();
					}
				}
			}
		}
		return false;
	}
	
	void AniEditorWindow::addThemesButton()
	{
		QAction* themeAction = new QAction(QIcon(":/MainWindow/icons/tinycolor/icons8-theme-16.png"), "Select Theme");
		m_menuThemesGroup = new QActionGroup(&m_menuThemes);

		themeAction->setMenu(&m_menuThemes);


		ui.mainToolBar->insertAction(ui.actionBackgroundColor, themeAction);
		static_cast<QToolButton*>(ui.mainToolBar->widgetForAction(themeAction))->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
	}

	void AniEditorWindow::addTheme(AbstractTheme* theme)
	{
		theme->setCheckable(true);
		m_menuThemes.addAction(theme);
		m_menuThemesGroup->addAction(theme);
		connect(theme, &QAction::triggered, this, &AniEditorWindow::themeTriggered);

	}

	void AniEditorWindow::saveTab(AniEditor * editor, bool forceSaveAs)
	{
		auto& ani = editor->getAni();

		if (ani.getFullPath().isEmpty() || forceSaveAs)
		{
			auto fullPath = m_resourceManager->getSaveFileName("Save File As", "", "*.gani");

			if (!fullPath.isEmpty())
			{
				QFileInfo fi(fullPath);
				editor->getAni().setFullPath(fullPath);
				editor->getAni().setFileName(fi.fileName());
				
			}
		}

		if (!ani.getFullPath().isEmpty())
		{
			QString fullPath = ani.getFullPath();
			QFile file(fullPath);
			if (file.open(QIODeviceBase::WriteOnly))
			{
				auto index = ui.tabWidgetMain->indexOf(editor);
				QFileInfo fi(fullPath);
				Ani::saveGraalAni(&ani, &file);

				editor->setUnmodified();

			}			
		}

	}

	bool AniEditorWindow::closeTabIndex(int index)
	{
		auto tab = static_cast<AniEditor*>(ui.tabWidgetMain->widget(index));
		if (tab)
		{
			if (tab->isModified())
			{

				switch (QMessageBox::question(nullptr, tab->getAni().getFileName(), "This file has not been saved. Do you want to save it?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
				{
				case QMessageBox::Yes:
					saveTab(tab, false);
					break;
				case QMessageBox::Cancel:
					return false;
				}
			}

			tab->deleteLater();
		}
		return true;
	}

	void AniEditorWindow::closeEvent(QCloseEvent* event)
	{
		auto currentEditor = static_cast<AniEditor*>(ui.tabWidgetMain->currentWidget());
		if (currentEditor)
			currentEditor->saveSettings(m_settings);

		
		for (int i = ui.tabWidgetMain->count(); i >= 0; --i)
		{
			if (!closeTabIndex(i))
			{
				event->ignore();
				return;
			}
		}

		QFileInfo fileInfo("./");
		if (m_resourceManager->getConnectionString() == fileInfo.absoluteFilePath() + "/")
			m_settings.remove("GaniEditor/WorkingDirectory");
		else 
			m_settings.setValue("GaniEditor/WorkingDirectory", m_resourceManager->getConnectionString());
		
		m_settings.setValue("GaniEditor/WindowGeometry", this->saveGeometry());
		m_settings.setValue("GaniEditor/WindowState", this->saveState());

		if (!EditScriptForm::savedGeometry.isNull())
			m_settings.setValue("scriptEditorGeometry", EditScriptForm::savedGeometry);

		QMainWindow::closeEvent(event);
	}
};
