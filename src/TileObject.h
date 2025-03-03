#ifndef TILEOBJECTH
#define TILEOBJECTH

#include <QString>
#include <QPainter>
#include "Tilemap.h"
#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
	class TileObject:
		public Tilemap
	{
	private:
		QString m_name;

	public:
		TileObject(int hcount, int vcount);

		const QString& getName() const { return m_name; }
		void setName(const QString& name) { m_name = name; }

		cJSON* serializeJSON();
	};
};

#endif

