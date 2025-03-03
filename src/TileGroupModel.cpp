#include "TileObject.h"
#include "TileGroupModel.h"

namespace TilesEditor
{
    TileGroupModel::~TileGroupModel()
    {
        for (auto tileObject : *this)
        {
            delete tileObject;
        }
    }

    QVariant TileGroupModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        int row = index.row();

        if (row >= 0 && row < this->count())
        {
            auto entity = this->at(row);

            if (role == Qt::DisplayRole)
                return QVariant(entity->getName());

        }

        return QVariant();
    }

    bool TileGroupModel::setData(const QModelIndex& index, const QVariant& value, int role)
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



    int TileGroupModel::rowCount(const QModelIndex& parent) const
    {
        return this->count();
    }
};
