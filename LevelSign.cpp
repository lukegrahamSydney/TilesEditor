#include <QColor>
#include "LevelSign.h"
#include "cJSON/JsonHelper.h"
#include "EditSignsDialog.h"

namespace TilesEditor
{
	LevelSign::LevelSign(Level* level, double x, double y, int width, int height) :
		AbstractLevelEntity(level, x, y)
	{
		m_width = width;
		m_height = height;
	}

	LevelSign::LevelSign(Level* level, cJSON* json, IWorld* world):
		LevelSign(level, 0.0, 0.0, 0, 0)
	{
		deserializeJSON(json, world);
	}

	void LevelSign::setText(const QString& text)
	{
		m_text = text;
	}

	cJSON* LevelSign::serializeJSON()
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelSign");
		cJSON_AddNumberToObject(json, "x", getX());
		cJSON_AddNumberToObject(json, "y", getY());
		cJSON_AddNumberToObject(json, "width", getWidth());
		cJSON_AddNumberToObject(json, "height", getHeight());
		cJSON_AddStringToObject(json, "text", getText().toLocal8Bit().data());
		return json;
	}

	void LevelSign::deserializeJSON(cJSON* json, IWorld* world)
	{
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));
		setWidth(jsonGetChildInt(json, "width"));
		setHeight(jsonGetChildInt(json, "height"));

		setText(jsonGetChildString(json, "text"));
	}

	void LevelSign::openEditor(IWorld* world)
	{
		EditSignsDialog form(getLevel(), world, this);
		form.exec();
	}

	void LevelSign::draw(QPainter* painter, const IRectangle& viewRect, double x, double y)
	{
		QPen pen(QColor(255, 0, 0));

		pen.setWidth(2);
		auto oldPen = painter->pen();
		painter->setPen(pen);
		painter->drawRect(int(getX()), int(getY()), getWidth(), getHeight());

		painter->setPen(oldPen);

	}
}
