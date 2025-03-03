#include "LevelGraalBaddy.h"
#include "EditBaddy.h"
#include "Level.h"

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

	LevelGraalBaddy::LevelGraalBaddy(IWorld* world, double x, double y, int type) :
		AbstractLevelEntity(world, x, y)
	{
		m_baddyType = type;
	}

	LevelGraalBaddy::LevelGraalBaddy(IWorld* world, cJSON* json) :
		LevelGraalBaddy(world, 0.0, 0.0, 0)
	{
		deserializeJSON(json);
	}

	cJSON* LevelGraalBaddy::serializeJSON(bool useLocalCoordinates)
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelBaddy");
		if (useLocalCoordinates && getLevel()) {
			cJSON_AddNumberToObject(json, "x", getX() - getLevel()->getX());
			cJSON_AddNumberToObject(json, "y", getY() - getLevel()->getY());
		}
		else {
			cJSON_AddNumberToObject(json, "x", getX());
			cJSON_AddNumberToObject(json, "y", getY());
		}
		cJSON_AddNumberToObject(json, "layer", getLayerIndex());
		cJSON_AddNumberToObject(json, "baddyType", getBaddyType());
		cJSON_AddStringToObject(json, "verse0", getBaddyVerse(0).toLocal8Bit().data());
		cJSON_AddStringToObject(json, "verse1", getBaddyVerse(1).toLocal8Bit().data());
		cJSON_AddStringToObject(json, "verse2", getBaddyVerse(2).toLocal8Bit().data());
		return json;

	}
	void LevelGraalBaddy::deserializeJSON(cJSON* json)
	{
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));
		setLayerIndex(jsonGetChildInt(json, "layer"));
		setBaddyType(jsonGetChildInt(json, "baddyType"));
		setBaddyVerse(0, jsonGetChildString(json, "verse0"));
		setBaddyVerse(1, jsonGetChildString(json, "verse1"));
		setBaddyVerse(2, jsonGetChildString(json, "verse2"));
	}


	void LevelGraalBaddy::setProperty(const QString& name, const QVariant& value)
	{
		AbstractLevelEntity::setProperty(name, value);
		if (name == "type")
		{
			setBaddyType(value.toInt());
			getWorld()->updateMovedEntity(this);
		}
		else if (name == "verse0")
			setBaddyVerse(0, value.toString());
		else if (name == "verse1")
			setBaddyVerse(1, value.toString());
		else if (name == "verse2")
			setBaddyVerse(2, value.toString());

	}

	void LevelGraalBaddy::draw(QPainter* painter, const QRectF& viewRect, double x, double y)
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

	QPixmap LevelGraalBaddy::getIcon()
	{
		auto baddyImage = getBaddyImage();

		if (baddyImage)
		{
			if (m_baddyType >= 0 && m_baddyType < 10)
			{
				
				return baddyImage->pixmap().copy(baddyTypes[m_baddyType][0], baddyTypes[m_baddyType][1], baddyTypes[m_baddyType][2], baddyTypes[m_baddyType][3]);
			}
		}
		return QPixmap();
	}

	void LevelGraalBaddy::openEditor()
	{
		EditBaddy dialog(this, getWorld());
		dialog.exec();
	}

	AbstractLevelEntity* LevelGraalBaddy::duplicate() {
		auto newBaddy = new LevelGraalBaddy(getWorld(), getX(), getY(), getBaddyType());
		newBaddy->setLayerIndex(getLayerIndex());
		newBaddy->setBaddyVerse(0, getBaddyVerse(0));
		newBaddy->setBaddyVerse(1, getBaddyVerse(1));
		newBaddy->setBaddyVerse(2, getBaddyVerse(2));
		return newBaddy;
	}


};
