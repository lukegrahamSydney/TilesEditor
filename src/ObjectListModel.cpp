#include "ObjectListModel.h"
#include "Level.h"

namespace TilesEditor
{

    QVariant ObjectListModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        int row = index.row();

        if (row >= 0 && row < this->count())
        {
            auto item = this->at(row);

            if (role == Qt::DisplayRole)
            {
                if(index.column() == 0)
                    return QVariant(item->getX());
                else if (index.column() == 1)
                    return QVariant(item->getY());
                else if (index.column() == 2)
                {
                    if (item->getLevel())
                        return QVariant(item->getLevel()->getName());
                    else return QVariant();
                }
                else if (index.column() == 3)
                {
                    return QVariant(item->getImageName());
                }
                else if (index.column() == 4)
                {
                    return QVariant(item->getClassName());
                }

            }
                

        }

        return QVariant();
    }

    bool ObjectListModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (!index.isValid())
            return false;

        int row = index.row();
        if (row >= 0 && row < this->count())
        {
            auto entity = this->at(row);

            if (index.column() == 0) {
                return true;
            }

        }
        return false;
    }



    int ObjectListModel::rowCount(const QModelIndex& parent) const
    {
        return this->count();
    }
};