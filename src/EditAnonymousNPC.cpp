#include <QFileDialog>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QMenu>
#include <QStyleHints>

#include "EditAnonymousNPC.h"
#include "ImageDimensions.h"
#include "Level.h"
#include "LevelNPC.h"
#include "LevelCommands.h"
#include "gs1/GS1Converter.h"
#include <QCXXHighlighter.hpp>
#include <QSyntaxStyle.hpp>
#include "IEngine.h"


namespace TilesEditor
{
	void EditAnonymousNPC::accept()
	{
		if (m_modified)
		{
			
			auto undoCommand = new QUndoCommand("Modify NPC");

			auto oldRect = m_npc->toQRectF();
			auto newRect = QRectF(ui.xText->text().toDouble() * m_npc->getUnitWidth(), ui.yText->text().toDouble() * m_npc->getUnitHeight(), m_npc->getWidth(), m_npc->getHeight());

			if (oldRect != newRect)
			{
				QMap<AbstractLevelEntity*, QRectF> oldPosition = { {m_npc, oldRect} };
				QMap<AbstractLevelEntity*, QRectF> newPosition = { {m_npc, newRect} };
				new CommandMoveEntities(m_world, oldPosition, newPosition, undoCommand);
			}

			if(ui.imageText->text() != m_npc->getImageName())
				new CommandSetEntityProperty(m_world, m_npc, "image", ui.imageText->text(), m_npc->getImageName(), undoCommand);

			if (ui.plainTextEdit->document()->isModified())
				new CommandSetEntityProperty(m_world, m_npc, "code", ui.plainTextEdit->toPlainText(), m_npc->getCode(), undoCommand);
			m_world->addUndoCommand(undoCommand);

			m_world->setModified(m_npc->getLevel());
		}
		QDialog::accept();
	}

	void EditAnonymousNPC::reject()
	{
		if (m_modified)
		{
			if (QMessageBox::question(nullptr, "Changes made", "You have made changes to the NPC. Are you sure you want to close?") == QMessageBox::Yes)
			{
				QDialog::reject();
			}
		} else QDialog::reject();
	}

	void EditAnonymousNPC::imageBrowseClicked(bool checked)
	{
		auto fileName = m_world->getResourceManager()->getOpenFileName("Select Image", m_world->getResourceManager()->getConnectionString(), "Image Files (*.png *.gif)");
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);
			auto directory = fi.absolutePath() + "/";

			if(m_world->getResourceManager()->getType() == "FileSystem")
				static_cast<ResourceManagerFileSystem*>(m_world->getResourceManager())->addSearchDir(directory);

			ui.imageText->setText(fi.fileName());

			int width, height;

			if (ImageDimensions::getImageFileDimensions(fileName, &width, &height))
			{
				ui.widthText->setText(QString::number(width));
				ui.heightText->setText(QString::number(height));
			}
			m_modified = true;

		}
		
	}

	void EditAnonymousNPC::testPressed()
	{
		QString error = "";
		if (!m_world->getEngine()->testCodeForErrors(ui.plainTextEdit->toPlainText(), &error, getSelectedLanguage()))
		{
			ui.testEdit->setPlainText(error);
		} else ui.testEdit->setPlainText("No syntax errors found.");
	}

	void EditAnonymousNPC::positionChanged()
	{
		m_modified = true;

		auto posX = std::floor((ui.xText->text().toDouble() * m_npc->getUnitWidth()) * 100.0) / 100.0;
		auto posY = std::floor((ui.yText->text().toDouble() * m_npc->getUnitHeight()) * 100.0) / 100.0;

		ui.localXText->setText(QString::number((posX - m_npc->getLevel()->getX()) / m_npc->getUnitWidth(), 'f', 2));
		ui.localYText->setText(QString::number((posY - m_npc->getLevel()->getY()) / m_npc->getUnitHeight(), 'f', 2));
	}

	void EditAnonymousNPC::deletePressed()
	{
		if (QMessageBox::question(nullptr, "Warning", "Are you sure you want to delete this NPC?") == QMessageBox::Yes)
		{
			m_modified = false;
			m_world->deleteEntity(m_npc);
			QDialog::reject();
			
		}
	}

	void EditAnonymousNPC::convertPressed()
	{
		
		auto originalCode = ui.plainTextEdit->toPlainText();

		//Clear
		auto doc = ui.plainTextEdit->document();
		QTextCursor curs(doc);
		curs.select(QTextCursor::Document);
		curs.removeSelectedText();

		curs.insertText(QString::fromStdString(GS1Converter::convert3(originalCode.toStdString())));
		m_modified = true;

		
		
	}

	void EditAnonymousNPC::revertPressed()
	{
		auto originalCode = ui.plainTextEdit->toPlainText();
		//Clear
		auto doc = ui.plainTextEdit->document();
		QTextCursor curs(doc);
		curs.select(QTextCursor::Document);
		curs.removeSelectedText();

		curs.insertText(QString::fromStdString(GS1Converter::revert(originalCode.toStdString())));
		m_modified = true;

	}

	void EditAnonymousNPC::textEdited(const QString& text)
	{
		m_modified = true;
	}

	void EditAnonymousNPC::textChanged()
	{
		m_modified = true;
	}

	QByteArray EditAnonymousNPC::savedGeometry;

	EditAnonymousNPC::EditAnonymousNPC(LevelNPC* npc, IWorld* world, QWidget* parent)
		: QDialog(parent), m_testScriptTypesGroup(&m_testScriptTypes)
	{
		m_modified = false;

		m_npc = npc;
		m_world = world;
		ui.setupUi(this);

		this->setWindowFlag(Qt::Window);
		this->setWindowFlag(Qt::WindowMaximizeButtonHint);

		/*
		// ## MainWindow::MainWindow
		static QCodeEditorDesign design;


		// modify design ...
		static QList<QSyntaxRule> rules =
			QSyntaxRules::loadFromFile(":/MainWindow/rule_cpp.xml", design);

		ui.plainTextEdit->setRules(rules);
		*/

		QFile fl(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark ? ":/styles/drakula.xml" : ":/default_style.xml");

		if (fl.open(QIODevice::ReadOnly))
		{
			if (m_editorStyle.load(fl.readAll()))
			{
				ui.plainTextEdit->setSyntaxStyle(&m_editorStyle);
			}
		}

		ui.plainTextEdit->setHighlighter(&m_cppHighlighterClient);



		ui.plainTextEdit->setHighlighter(new QCXXHighlighter());

		ui.plainTextEdit->setFrameShape(QFrame::StyledPanel);

		ui.plainTextEdit->setPlainText(m_npc->getCode());
		ui.imageText->setText(m_npc->getImageName());

		ui.xText->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, this));
		ui.yText->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, this));


		ui.xText->setText(QString::number(std::floor((npc->getX() / npc->getUnitWidth()) * 100.0) / 100.0));
		ui.yText->setText(QString::number(std::floor((npc->getY() / npc->getUnitHeight()) * 100.0) / 100.0));

		ui.localXText->setText(QString::number((npc->getX() - npc->getLevel()->getX()) / npc->getUnitWidth(), 'f', 2));
		ui.localYText->setText(QString::number((npc->getY() - npc->getLevel()->getY()) / npc->getUnitHeight(), 'f', 2));

		ui.widthText->setText(QString::number(npc->getWidth()));
		ui.heightText->setText(QString::number(npc->getHeight()));

		ui.plainTextEdit->setWordWrapMode(QTextOption::NoWrap);
		connect(ui.plainTextEdit, &QCodeEditor::textChanged, this, &EditAnonymousNPC::textChanged);
		connect(ui.imageBrowse, &QPushButton::clicked, this, &EditAnonymousNPC::imageBrowseClicked);

		connect(ui.xText, &QLineEdit::textEdited, this, &EditAnonymousNPC::textEdited);
		connect(ui.yText, &QLineEdit::textEdited, this, &EditAnonymousNPC::textEdited);
		connect(ui.imageText, &QLineEdit::textEdited, this, &EditAnonymousNPC::textEdited);
		connect(ui.deleteButton, &QPushButton::pressed, this, &EditAnonymousNPC::deletePressed);
		connect(ui.convertButton, &QPushButton::pressed, this, &EditAnonymousNPC::convertPressed);
		connect(ui.revertButton, &QPushButton::pressed, this, &EditAnonymousNPC::revertPressed);
		connect(ui.testButton, &QPushButton::pressed, this, &EditAnonymousNPC::testPressed);

		
		m_testScriptTypesGroup.addAction(m_testScriptTypes.addAction("SG Script"))->setCheckable(true);
		m_testScriptTypesGroup.addAction(m_testScriptTypes.addAction("GS1"))->setCheckable(true);

		
		auto language = m_npc->getScriptingLanguage();
		if (language == ScriptingLanguage::SCRIPT_UNDEFINED)
		{
			if (m_npc->getLevel())
				language = m_npc->getLevel()->getDefaultScriptingLanguage();
		}

		if (language == ScriptingLanguage::SCRIPT_UNDEFINED)
			language = ScriptingLanguage::SCRIPT_SGSCRIPT;


		if (language >= 0 && language < m_testScriptTypes.actions().size())
			m_testScriptTypes.actions().at(language)->setChecked(true);

		ui.testButton->setMenu(&m_testScriptTypes);

		//connect(ui.convertButton, &QToolButton::triggered, this, &EditAnonymousNPC::convertPressed);
		if (!savedGeometry.isNull())
			restoreGeometry(savedGeometry);

	}

	EditAnonymousNPC::~EditAnonymousNPC()
	{
		auto language = getSelectedLanguage();
		m_npc->setScriptingLanguage(language);

		//Dno why but have to do this (this control still sends signals after this class is destroyed)
		ui.plainTextEdit->blockSignals(true);

		savedGeometry = saveGeometry();
		
	}

	ScriptingLanguage EditAnonymousNPC::getSelectedLanguage()
	{
		int index = 0;
		for (auto action : m_testScriptTypes.actions())
		{
			if (action->isChecked())
			{
				return (ScriptingLanguage)index;
			}
			++index;
		}
		return ScriptingLanguage::SCRIPT_SGSCRIPT;
	}
	
};