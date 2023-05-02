#include "SaveOverworldDialog.h"
#include "Level.h"

namespace TilesEditor
{
	SaveOverworldDialog::SaveOverworldDialog(IWorld* world, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		auto levels = world->getModifiedLevels();

		for (auto level : levels)
		{
			auto item = new LevelListItem(level, ui.levelsList);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
			item->setCheckState(Qt::Checked);
		}

		this->setFixedSize(this->size());

	}

	SaveOverworldDialog::~SaveOverworldDialog()
	{}

	QList<Level*> SaveOverworldDialog::getCheckedLevels()
	{
		QList<Level*> retval;
		
		for (auto i = 0; i < ui.levelsList->count(); ++i)
		{
			auto item = ui.levelsList->item(i);
			if(item->checkState() == Qt::Checked)
				retval.push_back(static_cast<LevelListItem*>(item)->getLevel());
		}

		return retval;
	}
};
