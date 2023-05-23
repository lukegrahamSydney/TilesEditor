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
		static Image* load(const QString& assetName, const QString& fileName);
	};
};

#endif
