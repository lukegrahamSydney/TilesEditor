#ifndef EDITCHESTDIALOGH
#define EDITCHESTDIALOGH

#include <QDialog>
#include "ui_EditChestDialog.h"
#include "LevelChest.h"
#include "IWorld.h"

namespace TilesEditor
{
	class EditChestDialog : public QDialog
	{
		Q_OBJECT

	private slots:
		void signsClicked(bool checked);
		void deleteClicked(bool checked);
		void closeClicked(bool checked);

	public:
		EditChestDialog(LevelChest* chest, IWorld* world, QWidget* parent = nullptr);
		~EditChestDialog();

	private:
		Ui::EditChestDialogClass ui;
		LevelChest* m_chest;
		IWorld* m_world;

	};
};

#endif
