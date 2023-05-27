#include <QFileDialog>
#include <QMessageBox>
#include <QDoubleValidator>
#include "EditAnonymousNPC.h"
#include "ImageDimensions.h"
#include "Level.h"
#include "LevelNPC.h"
#include "LevelCommands.h"

namespace TilesEditor
{
	void EditAnonymousNPC::accept()
	{
		if (m_modified)
		{
			

			m_npc->setX(ui.xText->text().toDouble() * m_npc->getUnitWidth());
			m_npc->setY(ui.yText->text().toDouble() * m_npc->getUnitHeight());

			m_npc->setImageName(ui.imageText->text());
			m_npc->setCode(ui.plainTextEdit->toPlainText());
			m_world->setModified(m_npc->getLevel());

			m_world->updateMovedEntity(m_npc);
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

	void EditAnonymousNPC::imageBrowsePressed()
	{
		auto fileName = m_world->getResourceManager().getFileSystem()->getOpenFileName("Select Image", m_world->getResourceManager().getRootDir(), "Image Files (*.png *.gif)");
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);
			auto directory = fi.absolutePath() + "/";

			m_world->getResourceManager().addSearchDir(directory);

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

	void EditAnonymousNPC::textEdited(const QString& text)
	{
		m_modified = true;
	}

	QByteArray EditAnonymousNPC::savedGeometry;
	EditAnonymousNPC::EditAnonymousNPC(LevelNPC* npc, IWorld* world, QWidget* parent)
		: QDialog(parent)
	{
		using namespace kgl;
		m_modified = false;

		m_npc = npc;
		m_world = world;
		ui.setupUi(this);

		this->setWindowFlag(Qt::Window);
		this->setWindowFlag(Qt::WindowMaximizeButtonHint);

		// ## MainWindow::MainWindow
		static QCodeEditorDesign design;
		// modify design ...
		static QList<QSyntaxRule> rules =
			QSyntaxRules::loadFromFile(":/MainWindow/rule_cpp.xml", design);

		ui.plainTextEdit->setRules(rules);

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
		connect(ui.plainTextEdit, &QCodeEditor::lineChanged, this, &EditAnonymousNPC::editorLineChanged);
		connect(ui.imageBrowse, &QPushButton::pressed, this, &EditAnonymousNPC::imageBrowsePressed);

		connect(ui.xText, &QLineEdit::textEdited, this, &EditAnonymousNPC::textEdited);
		connect(ui.yText, &QLineEdit::textEdited, this, &EditAnonymousNPC::textEdited);
		connect(ui.imageText, &QLineEdit::textEdited, this, &EditAnonymousNPC::textEdited);
		connect(ui.deleteButton, &QPushButton::pressed, this, &EditAnonymousNPC::deletePressed);

		if (!savedGeometry.isNull())
			restoreGeometry(savedGeometry);

	}

	EditAnonymousNPC::~EditAnonymousNPC()
	{
		//Dno why but have to do this (this control still sends signals after this class is destroyed)
		ui.plainTextEdit->blockSignals(true);

		savedGeometry = saveGeometry();
	}

	void EditAnonymousNPC::editorLineChanged(QTextBlock block)
	{
		m_modified = true;
	}
};