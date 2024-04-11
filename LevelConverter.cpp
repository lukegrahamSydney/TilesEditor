#include <QFileDialog>
#include <QDirIterator>
#include <QTimer>
#include "LevelConverter.h"
#include "Level.h"
#include "MainFileSystem.h"
#include "FileFormatManager.h"
#include "DarkTitleBar.h"
#include "gs1/GS1Converter.h"

namespace TilesEditor
{
	LevelConverter::LevelConverter(QStandardItemModel* tilesetsModel, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		DarkTitleBar::ApplyStyle(this->winId());

		m_tilesetsModel = tilesetsModel;
		m_dummyTab = new EditorTabWidget(nullptr, &m_fileSystem);
		m_dummyTab->hide();

		connect(ui.inputBrowseButton, &QAbstractButton::clicked, this, &LevelConverter::inputBrowseClicked);
		connect(ui.outputBrowseButton, &QAbstractButton::clicked, this, &LevelConverter::outputBrowseClicked);

		auto formats = FileFormatManager::instance()->getRegisteredFormats();
		for (auto format : formats)
		{
			if (format->canSave())
				ui.formatCombo->addItem(format->getPrimaryExtension());
		}
		
		ui.defaultTilesetComboBox->setModel(tilesetsModel);
		this->setFixedSize(this->size());
	}

	LevelConverter::~LevelConverter()
	{
		delete m_dummyTab;
	}


	void LevelConverter::timer()
	{
		for (int i = 0; i < 10 && m_files.size() > 0; ++i)
		{
			auto file = m_files.first();


			m_files.pop_front();
			QFileInfo info(file);

			auto subDir = info.absolutePath().mid(ui.inputEdit->text().length());
			subDir += "/";

			
			auto defaultTileset = ui.defaultTilesetComboBox->currentIndex() >= 0 ? static_cast<Tileset*>(m_tilesetsModel->item(ui.defaultTilesetComboBox->currentIndex())) : nullptr;

			auto level = new Level(m_dummyTab, 0.0, 0.0, 64 * 16, 64 * 16, nullptr, "");
			level->setDefaultTileset(defaultTileset);
			level->setName(info.fileName());
			level->setFileName(file);

			level->loadFile();

			//Go through links and find links for levels that need to be converted and change them
			auto& links = level->getLinks();
			for (auto link : links)
			{
				auto nextLevel = link->getNextLevel();

				if (m_levelNames.contains(nextLevel))
				{
					auto i = nextLevel.lastIndexOf('.');

					if (i >= 0)
					{
						nextLevel = QString("%1.%2").arg(nextLevel.left(i)).arg(ui.formatCombo->currentText());

						link->setNextLevel(nextLevel);
					}
				}
			}

			//Go through NPCs and change any code references to our old level names. Change to the new one.
			for (auto object : level->getObjects())
			{
				if (object->getEntityType() == LevelEntityType::ENTITY_NPC)
				{
					auto npc = static_cast<LevelNPC*>(object);

					auto start = 0;
					auto code = npc->getCode();
					bool changed = false;
					for (auto& filter : m_filters)
					{
						
						for (auto pos = code.indexOf(filter, start); pos >= 0; pos = code.indexOf(filter, start))
						{
							start = pos + 1;
							for (auto i = pos - 1; i >= 0; --i)
							{
								
								if (!code[i].isLetterOrNumber() && code[i] != '_' && code[i] != '-')
								{
									auto startPos = i + 1;
									auto length = pos - startPos + filter.length();

									auto fileName = code.mid(startPos, length);

									if (m_levelNames.contains(fileName))
									{
										auto i = fileName.lastIndexOf('.');

										if (i >= 0)
										{
											fileName = QString("%1.%2").arg(fileName.left(i)).arg(ui.formatCombo->currentText());
											code.replace(startPos, length, fileName);
											qDebug() << "FOUND: " << fileName;
											changed = true;
										}
										
									}

									start = startPos + fileName.length();
									break;
								}
							}
						}
					}

					if (ui.gS1ToSGScriptCheckBox->isChecked())
					{
						if (code.indexOf("//sgscript") == -1)
						{
							code = QString::fromStdString(GS1Converter::convert3(code.toStdString()));
							changed = true;
						}
					}

					if(changed)
						npc->setCodeRaw(code);
					
				}
			}

			auto newName = QString("%1.%2").arg(info.completeBaseName()).arg(ui.formatCombo->currentText());
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
		}

		if (m_files.size() > 0)
			QTimer::singleShot(1, this, &LevelConverter::timer);
		
	}

	void LevelConverter::accept()
	{
		m_filters = ui.inputMaskEdit->text().split(",");
		QDirIterator it(ui.inputEdit->text(), m_filters, QDir::Files, ui.subFoldersCheckBox->checkState() == Qt::CheckState::Checked ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);

		m_files.clear();
		m_levelNames.clear();
		while (it.hasNext())
		{
			auto fileName = it.next();

			QFileInfo fi(fileName);
			m_files.append(fileName);
			m_levelNames.insert(fi.fileName());
		}
		
		ui.progressBar->setValue(0);
		ui.progressBar->setMaximum(m_files.size());

		//Remove the * from filter strings
		for (auto& filter : m_filters)
		{
			while (filter.startsWith('*'))
				filter = filter.mid(1);
		}
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

