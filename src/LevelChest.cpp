#include "LevelChest.h"
#include "EditChestDialog.h"
#include "cJSON/JsonHelper.h"
#include "Level.h"

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

	void LevelChest::draw(QPainter* painter, const QRectF& viewRect, double x, double y)
	{
		auto chestImage = getChestImage();

		if (chestImage) {
			painter->drawPixmap(int(getX()), int(getY()), chestImage->pixmap());
		}
	}

	void LevelChest::setProperty(const QString& name, const QVariant& value)
	{
		AbstractLevelEntity::setProperty(name, value);

		if (name == "item")
			setItemName(value.toString());

		else if (name == "signIndex")
			setSignIndex(value.toInt());
	}

	QPixmap LevelChest::getIcon()
	{
		auto chestImage = getChestImage();

		if (chestImage) {
			return chestImage->pixmap();
		}
		return QPixmap();
	}

	void LevelChest::openEditor()
	{
		EditChestDialog dialog(this, getWorld());
		dialog.exec();
	}

	AbstractLevelEntity* LevelChest::duplicate() {
		auto newChest = new LevelChest(getWorld(), getX(), getY(), getItemName(), getSignIndex());
		newChest->setLayerIndex(getLayerIndex());

		return newChest;
	}

	cJSON* LevelChest::serializeJSON(bool useLocalCoordinates)
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelChestv1");
		if (useLocalCoordinates && getLevel()) {
			cJSON_AddNumberToObject(json, "x", getX() - getLevel()->getX());
			cJSON_AddNumberToObject(json, "y", getY() - getLevel()->getY());
		}
		else {
			cJSON_AddNumberToObject(json, "x", getX());
			cJSON_AddNumberToObject(json, "y", getY());
		}
		cJSON_AddNumberToObject(json, "layer", getLayerIndex());
		cJSON_AddStringToObject(json, "item", getItemName().toLocal8Bit().data());
		cJSON_AddNumberToObject(json, "sign", getSignIndex());
		return json;
	}

	void LevelChest::deserializeJSON(cJSON* json)
	{
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));
		setLayerIndex(jsonGetChildInt(json, "layer"));
		setItemName(jsonGetChildString(json, "item"));
		setSignIndex(jsonGetChildInt(json, "sign"));
	}
};
