#ifndef IMAGEH
#define IMAGEH

#include <QImage>
#include <QPixmap>
#include <QByteArray>
#include "Resource.h"
#include "ResourceType.h"

namespace TilesEditor
{
	class Image :
		public Resource
	{
	public:
		static const int BODY_SLEEVE = 0;
		static const int BODY_SHIRT = 1;
		static const int BODY_SKIN = 2;
		static const int BODY_BELT = 3;
		static const int BODY_SHOES = 4;

	private:
		QImage	m_image;
		QPixmap m_pixmap;
	
		char m_bodyColourIndex[5];

		char getColourIndex(QRgb colour) const;
		void calculateBodyColourIndexes();

	public:
		Image(const QString& assetName);
		Image(const QString& assetName, QImage image);

		QPixmap& pixmap() {
			return m_pixmap;
		}

		QImage& image() {
			return m_image;
		}

		QPixmap colorMod(const QColor& modColor, const QRect& srcRect = QRect());
		ResourceType getResourceType() const override {
			return ResourceType::RESOURCE_IMAGE;
		}

		int width() const {
			return m_pixmap.width();
		}

		int height() const {
			return m_pixmap.height();
		}

		char getBodyColourIndex(int bodyColour) const {
			return m_bodyColourIndex[bodyColour];
		}

		void replace(QIODevice* stream, AbstractResourceManager* resourceManager) override;
		void draw(QPainter* painter, double x, double y);
		void draw(QPainter* painter, double x, double y, int left, int top, int width, int height);
		void drawColourMod(QPainter* painter, double x, double y, int left, int top, int width, int height, const QColor& color);
		void drawStretch(QPainter* painter, double x, double y, int left, int top, int width, int height, int stretchw, int stretchh);
		static Image* load(const QString& assetName, QIODevice* stream);
	};
};

#endif
