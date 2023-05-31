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

	};
};
#endif
