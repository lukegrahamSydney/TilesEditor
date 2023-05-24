#include <QFileDialog>
#include <QDirIterator>
#include <QTimer>
#include "LevelConverter.h"
#include "Level.h"
#include "MainFileSystem.h"


namespace TilesEditor
{
	LevelConverter::LevelConverter(QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		m_dummyTab = new EditorTabWidget(nullptr, &m_fileSystem);
		m_dummyTab->hide();

		connect(ui.inputBrowseButton, &QAbstractButton::clicked, this, &LevelConverter::inputBrowseClicked);
		connect(ui.outputBrowseButton, &QAbstractButton::clicked, this, &LevelConverter::outputBrowseClicked);

		this->setFixedSize(this->size());
	}

	LevelConverter::~LevelConverter()
	{
		delete m_dummyTab;
	}


	void LevelConverter::timer()
	{
		if (m_files.size() > 0)
		{
			auto file = m_files.first();

			
			m_files.pop_front();
			QFileInfo info(file);

			auto subDir = info.absolutePath().mid(ui.inputEdit->text().length());
			subDir += "/";

			auto level = new Level(m_dummyTab, 0.0, 0.0, 64 * 16, 64 * 16, nullptr, "");
			level->setName(info.fileName());
			level->setFileName(file);

			level->loadFile();

			//Go through links and find links for levels that need to be converted and change them
			auto& links = level->getLinks();
			for (auto link : links)
			{
				auto nextLevel = link->getNextLevel();

				if (m_filesSet.contains(nextLevel))
				{
					auto i = nextLevel.lastIndexOf('.');

					if (i >= 0)
					{
						nextLevel = nextLevel.left(i) + ui.formatCombo->currentText();

						link->setNextLevel(nextLevel);
					}
				}
			}

			auto newName = info.completeBaseName() + ui.formatCombo->currentText();
			auto newPath = ui.outFolderEdit->text() + subDir;
			auto newFullPath = newPath + newName;

			QDir dir(newPath);
			if (!dir.exists())
				dir.mkpath(".");

			level->setName(newName);
			level->setFileName(newFullPath);

			level->saveFile(nullptr);

			delete level;
			ui.progressBar->setValue(ui.progressBar->value() + 1);
			if (m_files.size() > 0)
				QTimer::singleShot(1, this, &LevelConverter::timer);
		}
	}

	void LevelConverter::accept()
	{
		auto filters = ui.inputMaskEdit->text().split(",");
		QDirIterator it(ui.inputEdit->text(), filters, QDir::Files, ui.subFoldersCheckBox->checkState() == Qt::CheckState::Checked ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

		m_files.clear();
		while (it.hasNext())
		{
			auto fileName = it.next();

			QFileInfo fi(fileName);
			m_files.append(fileName);
			m_filesSet.insert(fi.fileName());
		}
		
		ui.progressBar->setValue(0);
		ui.progressBar->setMaximum(m_files.size());
		timer();

	}

	void LevelConverter::processDirectory(const QString& dir)
	{
		QDir directory(dir);

	}

	void LevelConverter::inputBrowseClicked(bool checked)
	{
		auto folder = QFileDialog::getExistingDirectory(nullptr, "Select folder containing levels");
		if (!folder.isEmpty())
			ui.inputEdit->setText(folder);

	}

	void LevelConverter::outputBrowseClicked(bool checked)
	{
		auto folder = QFileDialog::getExistingDirectory(nullptr, "Select folder to save levels");
		if (!folder.isEmpty())
			ui.outFolderEdit->setText(folder);
	}
};

