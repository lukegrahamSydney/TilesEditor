#ifndef EDITBADDYH
#define EDITBADDYH

#include <QDialog>
#include "ui_EditBaddy.h"
#include "LevelGraalBaddy.h"
#include "IWorld.h"

namespace TilesEditor
{

	class EditBaddy : public QDialog
	{
		Q_OBJECT

	private slots:
		void deleteClicked(bool checked);
		void closeClicked(bool checked);

	public:
		EditBaddy(LevelGraalBaddy* baddy, IWorld* world, QWidget* parent = nullptr);
		~EditBaddy();

	private:
		Ui::EditBaddyClass ui;

		LevelGraalBaddy* m_baddy;
		IWorld* m_world;

	};
}
#endif
