#include "TileGroupListModel.h"

namespace TilesEditor
{

    QVariant TileGroupListModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        int row = index.row();

        if (row >= 0 && row < this->count())
        {
            auto item = this->at(row);

            if (role == Qt::DisplayRole)
                return QVariant(item->getName());

        }

        return QVariant();
    }

    bool TileGroupListModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (!index.isValid())
            return false;

        int row = index.row();
        if (row >= 0 && row < this->count())
        {
            auto entity = this->at(row);

            if (index.column() == 0) {
                entity->setName(value.toString());
                return true;
            }

        }
        return false;
    }



    int TileGroupListModel::rowCount(const QModelIndex& parent) const
    {
        return this->count();
    }
};
