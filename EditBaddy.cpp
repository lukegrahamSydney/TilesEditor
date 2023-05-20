#include "EditBaddy.h"

namespace TilesEditor
{

	EditBaddy::EditBaddy(LevelGraalBaddy* baddy, IWorld* world, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		m_baddy = baddy;
		m_world = world;

		connect(ui.closeButton, &QAbstractButton::clicked, this, &EditBaddy::closeClicked);
		connect(ui.deleteButton, &QAbstractButton::clicked, this, &EditBaddy::deleteClicked);

		ui.typeCombo->setCurrentIndex(m_baddy->getBaddyType());
		ui.attackVerseEdit->setText(m_baddy->getBaddyVerse(0));
		ui.hurtVerseEdit->setText(m_baddy->getBaddyVerse(1));
		ui.playerHurtEdit->setText(m_baddy->getBaddyVerse(2));


		this->setFixedSize(this->size());
	}

	EditBaddy::~EditBaddy()
	{}

	void EditBaddy::deleteClicked(bool checked)
	{
		m_world->deleteEntity(m_baddy);
		this->reject();
	}

	void EditBaddy::closeClicked(bool checked)
	{
		if (ui.typeCombo->currentIndex() != m_baddy->getBaddyType() 
			|| ui.attackVerseEdit->text() != m_baddy->getBaddyVerse(0) 
			|| ui.hurtVerseEdit->text() != m_baddy->getBaddyVerse(1)
			|| ui.playerHurtEdit->text() != m_baddy->getBaddyVerse(2))
		{
			m_baddy->setBaddyType(ui.typeCombo->currentIndex());

			m_baddy->setBaddyVerse(0, ui.attackVerseEdit->text());
			m_baddy->setBaddyVerse(1, ui.hurtVerseEdit->text());
			m_baddy->setBaddyVerse(2, ui.playerHurtEdit->text());

			m_world->updateMovedEntity(m_baddy);
			m_world->setModified(m_baddy->getLevel());

		}
		this->accept();
	}
};
