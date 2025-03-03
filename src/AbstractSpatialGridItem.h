#ifndef ABSTRACTSPATIALGRIDITEMH
#define ABSTRACTSPATIALGRIDITEMH

#include "ISpatialMapItem.h"

namespace TilesEditor
{
	class AbstractSpatialGridItem :
		public ISpatialMapItem
	{
		template <typename T>
		friend class EntitySpatialGrid;

	protected:
		int m_spacialGridLeft = 0;
		int m_spacialGridTop = 0;
		int m_spacialGridRight = 0;
		int m_spacialGridBottom = 0;

		uint64_t m_spatialGridSearchIndex = 0;
		bool m_spatialGridAdded = false;

	public:
		virtual double getX() const {
			return QRectF::x();
		}

		virtual double getY() const {
			return QRectF::y();
		}

		double getRight() const {
			return getX() + getWidth();
		}

		double getBottom() const {
			return getY() + getHeight();
		}

		double getCenterX() const {
			return getX() + getWidth() / 2;
		}

		double getCenterY() const {
			return getY() + getHeight() / 2;
		}

		virtual int getWidth() const {
			return QRectF::width();
		}


		virtual int getHeight() const {
			return QRectF::height();
		}

		virtual void setX(double val) {
			QRectF::moveLeft(val);
		}

		virtual void setY(double val) {
			QRectF::moveTop(val);
		}

		virtual void setWidth(int width) {
			QRectF::setWidth(width);
		}

		virtual void setHeight(int width) {
			QRectF::setHeight(width);
		}

		QRectF toQRectF() const {
			return QRectF(this->getX(), this->getY(), this->getWidth(), this->getHeight());
		}

		QPointF toQPointF() const {
			return QPointF(getX(), getY());
		}

		bool intersects(const QRectF& other) const {
			auto bbox = getBoundingBox();
			return other.right() > bbox.x() && other.bottom() > bbox.y() &&
				other.x() < bbox.right() && other.y() < bbox.bottom();
		}

		QRectF intersected(const QRectF& other) const {
			return this->toQRectF().intersected(other);
		}

		virtual QRectF getBoundingBox() const { return QRectF(this->getX(), this->getY(), this->getWidth(), this->getHeight());}
	};
};
#endif
