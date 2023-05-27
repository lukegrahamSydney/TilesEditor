#ifndef IMAGEH
#define IMAGEH

#include <QPixmap>
#include <QByteArray>
#include "Resource.h"
#include "ResourceType.h"

namespace TilesEditor
{
	class Image :
		public Resource
	{
	private:
		QPixmap m_pixmap;

	

	public:
		Image(const QString& assetName, QPixmap pixmap);

		QPixmap& pixmap() {
			return m_pixmap;
		}

		ResourceType getResourceType() const override {
			return ResourceType::RESOURCE_IMAGE;
		}

		int width() const {
			return m_pixmap.width();
		}

		int height() const {
			return m_pixmap.height();
		}

		void replace(QIODevice* stream) override;
		void draw(QPainter* painter, double x, double y);
		void draw(QPainter* painter, double x, double y, int left, int top, int width, int height);
		static Image* load(const QString& assetName, QIODevice* stream);
	};
};

#endif
