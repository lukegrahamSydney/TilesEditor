#include "LevelChest.h"
#include "EditChestDialog.h"
#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
	LevelChest::LevelChest(IWorld* world, double x, double y, const QString& itemName, int signIndex):
		AbstractLevelEntity(world, x, y)
	{
		m_itemName = itemName;
		m_signIndex = signIndex;
	}

	LevelChest::LevelChest(IWorld* world, cJSON* json):
		LevelChest(world, 0.0, 0.0, "", 0)
	{
		deserializeJSON(json);
	}

	void LevelChest::draw(QPainter* painter, const IRectangle& viewRect, double x, double y)
	{
		auto chestImage = getChestImage();

		if (chestImage) {
			painter->drawPixmap(int(getX()), int(getY()), chestImage->pixmap());
		}
	}

	void LevelChest::openEditor()
	{
		EditChestDialog dialog(this, getWorld());
		dialog.exec();
	}

	cJSON* LevelChest::serializeJSON()
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelChest");
		cJSON_AddNumberToObject(json, "x", getX());
		cJSON_AddNumberToObject(json, "y", getY());
		cJSON_AddStringToObject(json, "item", getItemName().toLocal8Bit().data());
		cJSON_AddNumberToObject(json, "sign", getSignIndex());
		return json;
	}

	void LevelChest::deserializeJSON(cJSON* json)
	{
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));
		setItemName(jsonGetChildString(json, "item"));
		setSignIndex(jsonGetChildInt(json, "sign"));
	}
};
