#include <QStandardItemModel>
#include "EditTilesets.h"

namespace TilesEditor
{
	EditTilesets::EditTilesets(QAbstractItemModel* tilesetList, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		ui.listViewTilesets->setModel(tilesetList);

		connect(ui.upButton, &QAbstractButton::clicked, this, &EditTilesets::moveUpClicked);
		connect(ui.downButton, &QAbstractButton::clicked, this, &EditTilesets::moveDownClicked);
	}

	EditTilesets::~EditTilesets()
	{}

	void EditTilesets::moveUpClicked(bool checked)
	{
		auto model = static_cast<QStandardItemModel*>(ui.listViewTilesets->model());
		int selectedRow = ui.listViewTilesets->currentIndex().row();
		if (selectedRow > 0)
		{
			auto takenRow = model->takeRow(selectedRow);

			model->insertRow(selectedRow - 1, takenRow);
			ui.listViewTilesets->setCurrentIndex(model->index(selectedRow - 1, 0));
		}
	}

	void EditTilesets::moveDownClicked(bool checked)
	{
		auto model = static_cast<QStandardItemModel*>(ui.listViewTilesets->model());
		int selectedRow = ui.listViewTilesets->currentIndex().row();

		if (selectedRow < model->rowCount() - 1)
		{
			auto takenRow = model->takeRow(selectedRow);

			model->insertRow(selectedRow + 1, takenRow);
			ui.listViewTilesets->setCurrentIndex(model->index(selectedRow + 1, 0));
		}

	}
}
