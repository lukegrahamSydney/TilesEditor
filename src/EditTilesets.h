#ifndef EDITTILESETSH
#define EDITTILESETSH

#include <QDialog>
#include <QAbstractItemModel>
#include "ui_EditTilesets.h"

namespace TilesEditor
{
	class EditTilesets : public QDialog
	{
		Q_OBJECT
	public slots:
		void moveUpClicked(bool checked);
		void moveDownClicked(bool checked);

	private:
		Ui::EditTilesetsClass ui;

	public:
		EditTilesets(QAbstractItemModel* tilesetList, QWidget* parent = nullptr);
		~EditTilesets();


	};
};

#endif