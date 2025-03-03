#ifndef SPATIALMAPITEMH
#define SPATIALMAPITEMH

#include <stdint.h>
#include <QRect>

namespace TilesEditor
{

	class ISpatialMapItem :
		protected QRectF
	{
	public:
		
		/*
		bool intersects(const QRectF& other) const {
			return QRectF::intersects(other);
		}
		*/

	};
};
#endif
