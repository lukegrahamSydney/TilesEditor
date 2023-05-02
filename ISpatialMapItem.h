#ifndef SPATIALMAPITEMH
#define SPATIALMAPITEMH

#include <stdint.h>
#include "Rectangle.h"

namespace TilesEditor
{

	class ISpatialMapItem :
		public IRectangle
	{
		template <typename T>
		friend class EntitySpatialGrid;
	protected:
		int m_spacialGridLeft = 0;
		int m_spacialGridTop = 0;
		int m_spacialGridRight = 0;
		int m_spacialGridBottom = 0;

		uint64_t m_spatialGridSearchIndex = 0;

	public:
		virtual ~ISpatialMapItem() {}
	};
};
#endif
