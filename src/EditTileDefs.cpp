#include <QSignalBlocker>
#include <QIntValidator>
#include <QLineEdit>
#include "EditTileDefs.h"

namespace TilesEditor
{
	

	EditTileDefsDelegate::EditTileDefsDelegate(QObject* parent) : QStyledItemDelegate(parent)
	{
	}

	QWidget* EditTileDefsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QLineEdit* editor = new QLineEdit(parent);

		if (index.column() == 2 || index.column() == 3)
		{
			auto val = new QIntValidator(editor);
			val->setBottom(0);
			editor->setValidator(val);
		}
		return editor;
	}
	void EditTileDefsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
	{
		auto value = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit* line = static_cast<QLineEdit*>(editor);
		line->setText(value);
	}
	void EditTileDefsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
	{
		QLineEdit* line = static_cast<QLineEdit*>(editor);
		QString value = line->text();
		model->setData(index, value);
	}

	void EditTileDefsDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		editor->setGeometry(option.rect);
	}

	EditTileDefs::EditTileDefs(const QVector<TileDef>& tileDefs, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		ui.tiledefsWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
		ui.tiledefsWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
		ui.tiledefsWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
		ui.tiledefsWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
		ui.tiledefsWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
		for (auto& tileDef : tileDefs)
		{
			auto rowIndex = ui.tiledefsWidget->rowCount();

			ui.tiledefsWidget->insertRow(rowIndex);

			auto savedItem = new QTableWidgetItem();
			savedItem->setCheckState(tileDef.saves ? Qt::Checked : Qt::Unchecked);

			ui.tiledefsWidget->setItem(rowIndex, 0, new QTableWidgetItem(tileDef.prefix));
			ui.tiledefsWidget->setItem(rowIndex, 1, new QTableWidgetItem(tileDef.imageName));
			ui.tiledefsWidget->setItem(rowIndex, 2, new QTableWidgetItem(QString::number(tileDef.x)));
			ui.tiledefsWidget->setItem(rowIndex, 3, new QTableWidgetItem(QString::number(tileDef.y)));
			ui.tiledefsWidget->setItem(rowIndex, 4, savedItem);
		}

		ui.tiledefsWidget->setItemDelegate(new EditTileDefsDelegate(ui.tiledefsWidget));

		connect(ui.newButton, &QAbstractButton::clicked, this, &EditTileDefs::newButtonClicked);
		connect(ui.deleteButton, &QAbstractButton::clicked, this, &EditTileDefs::deleteButtonClicked);

		if (!savedGeometry.isNull())
			this->restoreGeometry(savedGeometry);
	}

	EditTileDefs::~EditTileDefs()
	{
		savedGeometry = this->saveGeometry();
	}

	const QVector<TileDef> EditTileDefs::getTileDefs() const
	{
		QVector<TileDef> retval;

		for (int i = 0; i < ui.tiledefsWidget->rowCount(); ++i)
		{
			TileDef tileDef;

			tileDef.prefix = ui.tiledefsWidget->item(i, 0)->text();
			tileDef.imageName = ui.tiledefsWidget->item(i, 1)->text();

			tileDef.x = ui.tiledefsWidget->item(i, 2)->text().toInt();
			tileDef.y = ui.tiledefsWidget->item(i, 3)->text().toInt();
			tileDef.saves = ui.tiledefsWidget->item(i, 4)->checkState() == Qt::CheckState::Checked;
			retval.push_back(tileDef);
		}
		return retval;
	}

	void EditTileDefs::newButtonClicked(bool checked)
	{
		auto rowIndex = ui.tiledefsWidget->rowCount();

		auto savedItem = new QTableWidgetItem();
		savedItem->setCheckState(Qt::Checked);
		ui.tiledefsWidget->insertRow(rowIndex);
		ui.tiledefsWidget->setItem(rowIndex, 0, new QTableWidgetItem());
		ui.tiledefsWidget->setItem(rowIndex, 1, new QTableWidgetItem());

		ui.tiledefsWidget->setItem(rowIndex, 2, new QTableWidgetItem(QString::number(0)));
		ui.tiledefsWidget->setItem(rowIndex, 3, new QTableWidgetItem(QString::number(0)));
		ui.tiledefsWidget->setItem(rowIndex, 4, savedItem);

		ui.tiledefsWidget->selectRow(rowIndex);
	}

	void EditTileDefs::deleteButtonClicked(bool checked)
	{
		auto row = ui.tiledefsWidget->currentRow();

		if (row >= 0 && row < ui.tiledefsWidget->rowCount())
		{
			ui.tiledefsWidget->removeRow(row);
		}
	}


};

