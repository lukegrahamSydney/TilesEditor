#ifndef SAVEOVERWORLDDIALOGH
#define SAVEOVERWORLDDIALOGH

#include <QDialog>
#include <QListWidgetItem>
#include "IWorld.h"
#include "ui_SaveOverworldDialog.h"
#include "Level.h"

namespace TilesEditor
{
	class SaveOverworldDialog : public QDialog
	{
	
		Q_OBJECT

		class LevelListItem :
			public QListWidgetItem
		{
		private:
			Level* m_level;

		public:
			LevelListItem(Level* level, QListWidget* parent) :
				QListWidgetItem(level->getName(), parent) {
				m_level = level;

			}

			Level* getLevel() { return m_level; }
		};

	public:
		SaveOverworldDialog(IWorld* world, QWidget* parent = nullptr);
		~SaveOverworldDialog();

		QList<Level*> getCheckedLevels();

	private:
		Ui::SaveOverworldDialogClass ui;
	};
}

#endif
