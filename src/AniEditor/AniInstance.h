#ifndef ANIINSTANCEH
#define ANIINSTANCEH

#include <QString>
#include <QMap>
#include <QPainter>
#include <QColor>
#include "Image.h"
#include "Ani.h"
#include "AbstractResourceManager.h"
#include "IFileRequester.h"
#include "IAniInstance.h"

namespace TilesEditor
{
	class AniInstance:
		public IAniInstance
	{
	private:
		QString m_name;
		Ani* m_ani = nullptr;
		double m_frame = 0.0;
		QMap<QString, QPair<QString, Image*>> m_aniProperties;
		QRgb m_bodyColours[5];

	public:
		AniInstance();
		void freeResources(AbstractResourceManager* resourceManager);

		QString getPropertyValue(const QString& propName) override;
		QRectF getBoundingBox() const;
		QRectF getFrameBoundingBox() const;
		bool aniLoaded() const { return m_ani != nullptr; }
		void setAniName(IFileRequester* requester, const QString& name, int frame, AbstractResourceManager* resourceManager);
		const QString& getAniName() const { return m_name; }
		Image* getPropertyImage(const QString& name);
		void setProperty(const QString& name, const QString& value, AbstractResourceManager* resourceManager);
		QPixmap getIcon();

		double getFrame() const { return m_frame; }

		void setBodyColour(int index, QRgb colour) { m_bodyColours[index] = colour; }

		void applyBodyColours(Image* image);
		void draw(int dir, double x, double y, AbstractResourceManager* resourceManager, QPainter* painter);
		void drawSprite(Ani::AniSprite* sprite, double x, double y, AbstractResourceManager* resourceManager, QPainter* painter, int level = 0);

		
	};
};
#endif
