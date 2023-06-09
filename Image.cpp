#include <QPainter>
#include "Image.h"

namespace TilesEditor
{
    Image::Image(const QString& assetName, QPixmap pixmap):
        Resource(assetName)
    {
        m_pixmap = pixmap;
    }

    void Image::replace(QIODevice* stream)
    {
        QPixmap pixmap;

        if (pixmap.loadFromData(stream->readAll()))
        {
            m_pixmap = pixmap;
        }
    }

    void Image::draw(QPainter* painter, double x, double y)
    {
        painter->drawPixmap((int)x, (int)y, this->pixmap());
    }

    void Image::draw(QPainter* painter, double x, double y, int left, int top, int width, int height)
    {
        painter->drawPixmap((int)x, (int)y, this->pixmap(), left, top, width, height);
    }

    Image* Image::load(const QString& assetName, QIODevice* stream)
    {
        QPixmap pixmap;

        if (pixmap.loadFromData(stream->readAll()))
        {
            return new Image(assetName, pixmap);
        }
        return nullptr;
    }
};

