#include <QColor>
#include "LevelSign.h"
#include "cJSON/JsonHelper.h"
#include "EditSignsDialog.h"

namespace TilesEditor
{
	LevelSign::LevelSign(IWorld* world, double x, double y, int width, int height) :
		AbstractLevelEntity(world, x, y)
	{
		setWidth(width);
		setHeight(height);
	}

	LevelSign::LevelSign(IWorld* world, cJSON* json):
		LevelSign(world, 0.0, 0.0, 0, 0)
	{
		deserializeJSON(json);
	}

	void LevelSign::setText(const QString& text)
	{
		m_text = text;
	}

	cJSON* LevelSign::serializeJSON(bool useLocalCoordinates)
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelSign");
		if (useLocalCoordinates && getLevel()) {
			cJSON_AddNumberToObject(json, "x", getX() - getLevel()->getX());
			cJSON_AddNumberToObject(json, "y", getY() - getLevel()->getY());
		}
		else {
			cJSON_AddNumberToObject(json, "x", getX());
			cJSON_AddNumberToObject(json, "y", getY());
		}
		cJSON_AddNumberToObject(json, "layer", getLayerIndex());
		cJSON_AddNumberToObject(json, "width", getWidth());
		cJSON_AddNumberToObject(json, "height", getHeight());
		cJSON_AddStringToObject(json, "text", getText().toLocal8Bit().data());
		return json;
	}

	void LevelSign::deserializeJSON(cJSON* json)
	{
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));
		setLayerIndex(jsonGetChildInt(json, "layer"));
		setWidth(jsonGetChildInt(json, "width"));
		setHeight(jsonGetChildInt(json, "height"));

		setText(jsonGetChildString(json, "text"));
	}

	void LevelSign::openEditor()
	{
		EditSignsDialog form(getLevel(), getWorld(), this);
		form.exec();
	}

	void LevelSign::setProperty(const QString& name, const QVariant& value)
	{
		AbstractLevelEntity::setProperty(name, value);

		if (name == "text")
			setText(value.toString());
	}

	void LevelSign::draw(QPainter* painter, const QRectF& viewRect, double x, double y)
	{
		QPen pen(QColor(255, 0, 0));

		pen.setWidth(2);
		auto oldPen = painter->pen();
		painter->setPen(pen);
		painter->drawRect(int(getX()), int(getY()), getWidth(), getHeight());

		painter->setPen(oldPen);

	}
}
