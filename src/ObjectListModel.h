#ifndef OBJECTLISTMODELH
#define OBJECTLISTMODELH

#include <QList>
#include <QAbstractItemModel>

#include "LevelNPC.h"

namespace TilesEditor
{
	class ObjectListModel :
		public QAbstractItemModel, public QList<LevelNPC*>
	{
	public:
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
		int rowCount(const QModelIndex& parent = QModelIndex()) const;

		int columnCount(const QModelIndex& parent = QModelIndex()) const {
			return 5;
		}

		int indexOf(LevelNPC* entity) {
			return QList<LevelNPC*>::indexOf(entity);
		}

		void swapEntities(int index1, int index2)
		{
			beginMoveRows(QModelIndex(), index1, index1, QModelIndex(), index2);
			this->swapItemsAt(index1, index2);
			endMoveRows();
		}


		[[nodiscard]] QModelIndex index(int row, int column, const QModelIndex& parent) const override
		{
			return createIndex(row, column);
		}

		[[nodiscard]] QModelIndex parent(const QModelIndex& index) const override {
			return {};
		}
		/*
		void addEntity(LevelNPC* item) {
			auto index = this->count();
			this->beginInsertRows(QModelIndex(), index, index);
			this->push_back(item);
			this->endInsertRows();
		}*/

		void reset() {
			this->endResetModel();
		}

		void removeEntity(LevelNPC* item) {
			auto index = this->indexOf(item);

			removeEntity(index);

		}

		void removeEntity(int index) {

			if (index >= 0)
			{
				auto tileGroup = this->at(index);
				if (tileGroup)
				{
					this->beginRemoveRows(QModelIndex(), index, index);
					this->removeAt(index);
					this->endRemoveRows();
				}

			}

		}

		QVariant headerData(int section, Qt::Orientation orientation, int role) const
		{
			if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			{
				if (section == 0)
				{
					return QVariant("X");
				}
				else if (section == 1)
				{
					return QVariant("Y");
				}
				else if (section == 2)
				{
					return QVariant("Level");
				}
				else if (section == 3)
				{
					return QVariant("Image");
				}
				else if (section == 4)
				{
					return QVariant("Class");
				}
			}
			return QVariant();
		}

		Qt::ItemFlags flags(const QModelIndex& index) const override {
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		}


	};

};
#endif
