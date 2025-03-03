#ifndef EDITTILEDEFSH
#define EDITTILEDEFSH
#include <QDialog>
#include <QVector>
#include <QStyledItemDelegate>
#include "ui_EditTileDefs.h"
#include "TileDefs.h"

namespace TilesEditor
{

	class EditTileDefsDelegate :
		public QStyledItemDelegate
	{
		Q_OBJECT

	public:
		EditTileDefsDelegate(QObject* parent);
		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
		void setEditorData(QWidget* editor, const QModelIndex& index) const;
		void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
		void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	};

	class EditTileDefs : public QDialog
	{
		Q_OBJECT
	private:
		Ui::EditTileDefsClass ui;

		inline static QByteArray savedGeometry;

	public slots:
		void newButtonClicked(bool checked);
		void deleteButtonClicked(bool checked);



	public:
		EditTileDefs(const QVector<TileDef>& tileDefs, QWidget* parent = nullptr);
		~EditTileDefs();

		const QVector<TileDef> getTileDefs() const;
	};
}

#endif

