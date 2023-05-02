#ifndef TILEGROUPLISTMODELH
#define TILEGROUPLISTMODELH

#include <QList>
#include <QAbstractItemModel>
#include <QItemDelegate>
#include "TileGroupModel.h"

namespace TilesEditor
{
	class TileGroupListModel :
		public QAbstractItemModel, public QList<TileGroupModel*>
	{

	public:
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
		int rowCount(const QModelIndex& parent = QModelIndex()) const;

		int columnCount(const QModelIndex& parent = QModelIndex()) const {
			return 2;
		}

		int indexOf(TileGroupModel* entity) {
			return QList<TileGroupModel*>::indexOf(entity);
		}

		void swapEntities(int index1, int index2)
		{
			beginMoveRows(QModelIndex(), index1, index1, QModelIndex(), index2);
			this->swapItemsAt(index1, index2);
			endMoveRows();
		}


		QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex& parent = QModelIndex()) const
		{
			return createIndex(row, column);
		}

		QModelIndex QAbstractItemModel::parent(const QModelIndex& index) const {
			return QModelIndex();
		}

		void addTileGroup(TileGroupModel* item) {
			auto index = this->count();
			this->beginInsertRows(QModelIndex(), index, index);
			this->push_back(item);
			//entity->setIndex(index);
			this->endInsertRows();
		}

		void removeTileGroup(TileGroupModel* item) {
			auto index = this->indexOf(item);

			removeTileGroup(index);

		}

		void removeTileGroup(int index) {

			if (index >= 0)
			{
				auto tileGroup = this->at(index);
				if (tileGroup)
				{
					this->beginRemoveRows(QModelIndex(), index, index);
					this->removeAt(index);
					this->endRemoveRows();

					delete tileGroup;
				}
				
			}

		}


		Qt::ItemFlags flags(const QModelIndex& index) const {
			return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
		}

	};

};

#endif
