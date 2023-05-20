#include "LevelGraalBaddy.h"
#include "EditBaddy.h"

namespace TilesEditor
{
	const int LevelGraalBaddy::baddyTypes[][4] = {
		{0, 0, 44, 66},
		{44, 0, 44, 66},
		{88, 0, 44, 64},
		{132, 0, 44, 50},
		{0, 66, 52, 66},
		//frog
		{52, 66, 24, 26},
		{52, 92, 32, 34},
		//golden soldier
		{84, 64, 44, 66},

		//lizardon
		{132, 50, 44, 65},

		//dragon
		{132, 115, 44, 56}

	};

	LevelGraalBaddy::LevelGraalBaddy(Level* level, double x, double y, int type):
		AbstractLevelEntity(level, x, y)
	{
		m_baddyType = type;
	}


	void LevelGraalBaddy::draw(QPainter* painter, const IRectangle& viewRect, double x, double y)
	{
		auto badyImage = getBaddyImage();

		if (badyImage)
		{
			if (m_baddyType >= 0 && m_baddyType < 10)
			{
				painter->drawPixmap(int(getX()), int(getY()), badyImage->pixmap(), baddyTypes[m_baddyType][0], baddyTypes[m_baddyType][1], baddyTypes[m_baddyType][2], baddyTypes[m_baddyType][3]);
			}
		}
	}

	void LevelGraalBaddy::openEditor(IWorld* world)
	{
		EditBaddy dialog(this, world);
		dialog.exec();
	}

	cJSON* LevelGraalBaddy::serializeJSON()
	{
		return nullptr;
	}

	void LevelGraalBaddy::deserializeJSON(cJSON* json, IWorld* world)
	{
	}

};
