#include <QPainter>
#include "Image.h"

namespace TilesEditor
{
    char Image::getColourIndex(QRgb colour) const
    {
        for (int i = 0; i < m_image.colorCount(); ++i)
        {
            if (m_image.color(i) == colour)
                return i;
        }
        return -1;
    }

    void Image::calculateBodyColourIndexes()
    {
        m_bodyColourIndex[BODY_SLEEVE] = getColourIndex(qRgb(255, 0, 0));
        m_bodyColourIndex[BODY_SHIRT] = getColourIndex(qRgb(255, 255, 255));
        m_bodyColourIndex[BODY_SKIN] = getColourIndex(qRgb(255, 173, 107));
        m_bodyColourIndex[BODY_BELT] = getColourIndex(qRgb(0, 0, 255));
        m_bodyColourIndex[BODY_SHOES] = getColourIndex(qRgb(206, 24, 41));
    }

    Image::Image(const QString& assetName):
        Resource(assetName)
    {
    }

    Image::Image(const QString& assetName, QImage image):
        Resource(assetName)
    {
        m_image = image;
        m_pixmap = QPixmap::fromImage(image);
        calculateBodyColourIndexes();
    }

    QPixmap Image::colorMod(const QColor& modColor, const QRect& srcRect)
    {
        // Check if the source rectangle (srcRect) is null (empty or invalid).
        // If it is null, use the entire image (m_pixmap) and convert it to a QImage.
        // Otherwise, copy the specified region (srcRect) from m_pixmap and convert it to a QImage.
        QImage image = srcRect.isNull() ? m_pixmap.toImage() : m_pixmap.copy(m_image.rect().intersected(srcRect)).toImage();

        // Loop through each pixel in the image row by row (y-axis).
        for (int y = 0; y < image.height(); ++y)
        {
            // Loop through each pixel in the current row column by column (x-axis).
            for (int x = 0; x < image.width(); ++x)
            {
                // Get the color of the current pixel at (x, y).
                QColor color(image.pixelColor(x, y));

                // Modify the pixel's red, green, and blue components by multiplying them
                // with the corresponding normalized (0.0 to 1.0) components of modColor.
                // This effectively applies a color filter to the pixel.
                color.setRedF(color.redF() * modColor.redF());    // Adjust red component
                color.setGreenF(color.greenF() * modColor.greenF()); // Adjust green component
                color.setBlueF(color.blueF() * modColor.blueF());   // Adjust blue component

                // Set the modified color back to the pixel at (x, y).
                image.setPixelColor(x, y, color);
            }
        }

        // Convert the modified QImage back to a QPixmap and return it.
        return QPixmap::fromImage(image);
    }

    void Image::replace(QIODevice* stream, AbstractResourceManager* resourceManager)
    {
        QImage image;

        if (image.loadFromData(stream->readAll()))
        {
            m_image = image;
            m_pixmap = QPixmap::fromImage(m_image);
            calculateBodyColourIndexes();
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

    void Image::drawColourMod(QPainter* painter, double x, double y, int left, int top, int width, int height, const QColor& color)
    {
        painter->drawPixmap((int)x, (int)y, this->colorMod(color, QRect(left, top, width, height)));
    }

    void Image::drawStretch(QPainter* painter, double x, double y, int left, int top, int width, int height, int stretchw, int stretchh)
    {
        painter->drawPixmap((int)x, (int)y, stretchw, stretchh, this->pixmap(), left, top, width, height);
    }

    Image* Image::load(const QString& assetName, QIODevice* stream)
    {
        QImage img;

        if (img.loadFromData(stream->readAll()))
        {
            auto image = new Image(assetName, img);
            image->setLoaded(true);
            return image;
        }
        return nullptr;
    }
};

