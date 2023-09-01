#include <QIntValidator>
#include "EditChestDialog.h"
#include "EditSignsDialog.h"
#include "LevelCommands.h"
#include "DarkTitleBar.h"

namespace TilesEditor
{

	EditChestDialog::EditChestDialog(LevelChest* chest, IWorld* world, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		DarkTitleBar::ApplyStyle(this->winId());
		m_chest = chest;
		m_world = world;

		connect(ui.signsButton, &QAbstractButton::clicked, this, &EditChestDialog::signsClicked);
		connect(ui.closeButton, &QAbstractButton::clicked, this, &EditChestDialog::closeClicked);
		connect(ui.deleteButton, &QAbstractButton::clicked, this, &EditChestDialog::deleteClicked);
		ui.itemComboBox->setEditText(chest->getItemName());
		ui.signIndexEdit->setValidator(new QIntValidator(-1, 1000000));

		ui.signIndexEdit->setText(QString::number(chest->getSignIndex()));

		this->setFixedSize(this->size());
		
	}

	EditChestDialog::~EditChestDialog()
	{}

	void EditChestDialog::signsClicked(bool checked)
	{
		auto level = m_chest->getLevel();

		if (level) {
			EditSignsDialog dialog(level, m_world);
			dialog.exec();
		}
	}

	void EditChestDialog::deleteClicked(bool checked)
	{
		m_world->deleteEntity(m_chest);
		this->reject();
	}

	void EditChestDialog::closeClicked(bool checked)
	{
		if (ui.signIndexEdit->text() != QString::number(m_chest->getSignIndex()) || ui.itemComboBox->currentText() != m_chest->getItemName())
		{
			m_chest->setSignIndex(ui.signIndexEdit->text().toInt());
			m_chest->setItemName(ui.itemComboBox->currentText());
			m_world->setModified(m_chest->getLevel());

		}
		this->accept();
	}

};

