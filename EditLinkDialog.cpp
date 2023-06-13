#include <QDoubleValidator>
#include <QMessageBox>
#include "EditLinkDialog.h"
#include "LevelLink.h"
#include "Level.h"

namespace TilesEditor
{
	void EditLinkDialog::accept()
	{
		if (m_modified)
		{
			m_link->setNextLevel(ui.nextLevelText->text());
			m_link->setNextX(ui.nextXText->text());
			m_link->setNextY(ui.nextYText->text());

			m_link->setX(ui.xText->text().toDouble() * m_link->getUnitWidth());
			m_link->setY(ui.yText->text().toDouble() * m_link->getUnitHeight());
			m_link->setWidth(ui.widthText->text().toInt() * m_link->getUnitWidth());
			m_link->setHeight(ui.heightText->text().toInt() * m_link->getUnitHeight());

			m_world->updateMovedEntity(m_link);


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

	EditLinkDialog::EditLinkDialog(LevelLink* link, IWorld* world, bool allowDelete, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		m_allowDelete = allowDelete;
		m_link = link;
		m_world = world;

		ui.xText->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, this));
		ui.yText->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, this));

		ui.widthText->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, this));
		ui.heightText->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, this));

		ui.nextLevelText->setText(link->getNextLevel());
		ui.nextXText->setText(link->getNextX());
		ui.nextYText->setText(link->getNextY());

		ui.xText->setText(QString::number(std::floor((link->getX() / link->getUnitWidth()) * 100.0) / 100.0));
		ui.yText->setText(QString::number(std::floor((link->getY() / link->getUnitHeight()) * 100.0) / 100.0));

		ui.localXText->setText(QString::number((link->getX() - link->getLevel()->getX()) / link->getUnitWidth(), 'f', 2));
		ui.localYText->setText(QString::number((link->getY() - link->getLevel()->getY()) / link->getUnitHeight(), 'f', 2));

		ui.widthText->setText(QString::number(std::floor((link->getWidth() / link->getUnitWidth()) * 100.0) / 100.0));
		ui.heightText->setText(QString::number(std::floor((link->getHeight() / link->getUnitHeight()) * 100.0) / 100.0));

		connect(ui.xText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.yText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.widthText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.heightText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.nextLevelText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.nextXText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);
		connect(ui.nextYText, &QLineEdit::textEdited, this, &EditLinkDialog::textEdited);

		connect(ui.deleteButton, &QPushButton::pressed, this, &EditLinkDialog::deletePressed);
		this->setFixedSize(this->size());

		m_modified = false;

	}

	EditLinkDialog::~EditLinkDialog()
	{}
};

