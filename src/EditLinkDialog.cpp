#include <QMessageBox>
#include <QMap>
#include "EditLinkDialog.h"
#include "LevelLink.h"
#include "Level.h"
#include "LevelCommands.h"

namespace TilesEditor
{
	void EditLinkDialog::accept()
	{
		if (m_modified)
		{
			auto undoCommand = new QUndoCommand("Modify Link");

			new CommandSetEntityProperty(m_world, m_link, "nextLevel", ui.nextLevelText->text(), m_link->getNextLevel(), undoCommand);
			new CommandSetEntityProperty(m_world, m_link, "nextX", ui.nextXText->text(), m_link->getNextX(), undoCommand);
			new CommandSetEntityProperty(m_world, m_link, "nextY", ui.nextYText->text(), m_link->getNextY(), undoCommand);
			new CommandSetEntityProperty(m_world, m_link, "nextLayer", ui.nextLayerLineEdit->text().toInt(), m_link->getNextLayer(), undoCommand);

			QMap<AbstractLevelEntity*, QRectF> oldPosition = { {m_link, m_link->toQRectF()} };
			QMap<AbstractLevelEntity*, QRectF> newPosition = { {m_link, QRectF(ui.xSpinBox->value() * m_link->getUnitWidth(), ui.ySpinBox->value() * m_link->getUnitHeight(), ui.widthSpinBox->value() * m_link->getUnitWidth(), ui.heightSpinBox->value() * m_link->getUnitHeight())}};
			
			//new CommandSetEntityProperty(m_)
			new CommandMoveEntities(m_world, oldPosition, newPosition, undoCommand);

			m_world->addUndoCommand(undoCommand);
			m_world->setModified(m_link->getLevel());


		}
		QDialog::accept();
	}

	void EditLinkDialog::reject()
	{
		if (m_modified)
		{
			if (QMessageBox::question(nullptr, "Changes made", "You have made changes to the Link. Are you sure you want to close?") == QMessageBox::Yes)
			{
				QDialog::reject();
			}
		}
		else QDialog::reject();
	}

	void EditLinkDialog::deletePressed()
	{
		if (QMessageBox::question(nullptr, "Warning", "Are you sure you want to delete this link?") == QMessageBox::Yes)
		{
			m_modified = false;

			if(m_allowDelete)
				m_world->deleteEntity(m_link);
			QDialog::reject();
		}
	}


	void EditLinkDialog::textEdited(const QString& text)
	{
		m_modified = true;
	}

	void EditLinkDialog::valueChanged(int value)
	{
		m_modified = true;
	}

	EditLinkDialog::EditLinkDialog(LevelLink* link, IWorld* world, bool allowDelete, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		m_allowDelete = allowDelete;
		m_link = link;
		m_world = world;


		ui.nextLevelText->setText(link->getNextLevel());
		ui.nextXText->setText(link->getNextX());
		ui.nextYText->setText(link->getNextY());
		ui.nextLayerLineEdit->setText(QString::number(link->getNextLayer()));

		ui.xSpinBox->setValue(link->getX() / link->getUnitWidth());
		ui.ySpinBox->setValue(link->getY() / link->getUnitHeight());

		ui.localXText->setText(QString::number((link->getX() - (link->getLevel() ? link->getLevel()->getX() : 0.0)) / link->getUnitWidth(), 'f', 2));
		ui.localYText->setText(QString::number((link->getY() - (link->getLevel() ? link->getLevel()->getY() : 0.0)) / link->getUnitHeight(), 'f', 2));

		ui.widthSpinBox->setValue(link->getWidth() / link->getUnitWidth());
		ui.heightSpinBox->setValue(link->getHeight() / link->getUnitHeight());

		connect(ui.xSpinBox, &QSpinBox::valueChanged, this, &EditLinkDialog::valueChanged);
		connect(ui.ySpinBox, &QSpinBox::valueChanged, this, &EditLinkDialog::valueChanged);
		connect(ui.widthSpinBox, &QSpinBox::valueChanged, this, &EditLinkDialog::valueChanged);
		connect(ui.heightSpinBox, &QSpinBox::valueChanged, this, &EditLinkDialog::valueChanged);
		connect(ui.nextLevelText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.nextXText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.nextYText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.nextLayerLineEdit, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.deleteButton, &QPushButton::pressed, this, &EditLinkDialog::deletePressed);
		this->setFixedSize(this->size());

		m_modified = false;

	}

	EditLinkDialog::~EditLinkDialog()
	{}
};

