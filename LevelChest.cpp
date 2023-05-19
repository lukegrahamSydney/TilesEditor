#include "LevelChest.h"
#include "EditChestDialog.h"

namespace TilesEditor
{
	LevelChest::LevelChest(Level* level, double x, double y, const QString& itemName, int signIndex):
		AbstractLevelEntity(level, x, y)
	{
		m_itemName = itemName;
		m_signIndex = signIndex;
	}

	void LevelChest::draw(QPainter* painter, const IRectangle& viewRect, double x, double y)
	{
		auto chestImage = getChestImage();

		if (chestImage) {
			painter->drawPixmap(int(getX()), int(getY()), chestImage->pixmap());
		}
	}

	void LevelChest::openEditor(IWorld* world)
	{
		EditChestDialog dialog(this, world);
		dialog.exec();
	}

	cJSON* LevelChest::serializeJSON()
	{
		return nullptr;
	}

	void LevelChest::deserializeJSON(cJSON* json, IWorld* world)
	{
	}
};
