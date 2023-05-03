#ifndef TILEGROUPMODELH
#define TILEGROUPMODELH

#include <QList>
#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QString>
#include "TileObject.h"

namespace TilesEditor
{
	class TileGroupModel :
		public QAbstractItemModel, public QList<TileObject*>
	{
	private:
		QString m_name;

	public:
		~TileGroupModel();
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
		int rowCount(const QModelIndex& parent = QModelIndex()) const override;

		int columnCount(const QModelIndex& parent = QModelIndex()) const override {
			return 2;
		}

		int indexOf(TileObject* entity) {
			return QList<TileObject*>::indexOf(entity);
		}

		void swapEntities(int index1, int index2)
		{
			beginMoveRows(QModelIndex(), index1, index1, QModelIndex(), index2);
			this->swapItemsAt(index1, index2);
			endMoveRows();
		}


		QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
		{
			return createIndex(row, column);
		}

		QModelIndex QAbstractItemModel::parent(const QModelIndex& index) const override  {
			return QModelIndex();
		}
		
		void addTileObject(TileObject* item) {
			auto index = this->count();
			this->beginInsertRows(QModelIndex(), index, index);
			this->push_back(item);
			//entity->setIndex(index);
			this->endInsertRows();
		}

		void removeTileObject(TileObject* item) {
			auto index = this->indexOf(item);
			removeTileObject(index);
		}

		void removeTileObject(int index) {
			if (index >= 0)
			{
				auto tileObject = this->at(index);
				if (tileObject)
				{
					this->beginRemoveRows(QModelIndex(), index, index);


					this->removeAt(index);
					//entity->setIndex(-1);
					this->endRemoveRows();

					delete tileObject;
				}
			}
		}


		Qt::ItemFlags flags(const QModelIndex& index) const override {
			if (index.column() == 1)
				return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		}

		void setName(const QString& name) { m_name = name; }
		const QString& getName() const { return m_name; }
	};

};

#endif
