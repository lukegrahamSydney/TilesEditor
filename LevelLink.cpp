#include <QColor>
#include "LevelLink.h"
#include "AbstractSelection.h"
#include "cJSON/JsonHelper.h"
#include "EditLinkDialog.h"

namespace TilesEditor
{
	LevelLink::LevelLink(Level* level, double x, double y, int width, int height, bool possibleEdgeLink):
		AbstractLevelEntity(level, x, y)
	{ 
		m_width = width;
		m_height = height;
		m_possibleEdgeLink = possibleEdgeLink;

	}

	LevelLink::LevelLink(Level* level, cJSON* json, IWorld* world):
		LevelLink(level, 0, 0, 0, 0, false)
	{
		deserializeJSON(json, world);
	}

	void LevelLink::setNextLevel(const QString& nextLevel)
	{
		m_nextLevel = nextLevel;
	}

	void LevelLink::setNextX(const QString& nextX)
	{
		m_nextX = nextX;
	}

	void LevelLink::setNextY(const QString& nextY)
	{
		m_nextY = nextY;
	}

	cJSON* LevelLink::serializeJSON()
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelLink");
		cJSON_AddNumberToObject(json, "x", getX());
		cJSON_AddNumberToObject(json, "y", getY());
		cJSON_AddNumberToObject(json, "width", getWidth());
		cJSON_AddNumberToObject(json, "height", getHeight());
		cJSON_AddStringToObject(json, "nextLevel", getNextLevel().toLocal8Bit().data());
		cJSON_AddStringToObject(json, "nextX", getNextX().toLocal8Bit().data());
		cJSON_AddStringToObject(json, "nextY", getNextY().toLocal8Bit().data());
		return json;
	}

	void LevelLink::deserializeJSON(cJSON* json, IWorld* world)
	{
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));

		setWidth(jsonGetChildInt(json, "width"));
		setHeight(jsonGetChildInt(json, "height"));

		setNextLevel(jsonGetChildString(json, "nextLevel"));
		setNextX(jsonGetChildString(json, "nextX"));
		setNextY(jsonGetChildString(json, "nextY"));
	}


	void LevelLink::draw(QPainter* painter, const IRectangle& viewRect, double x, double y)
	{
		QPen pen(QColor(255, 215, 0));

		pen.setWidth(2);
		auto oldPen = painter->pen();
		painter->setPen(pen);
		painter->drawRect(int(getX()), int(getY()), getWidth(), getHeight());

		painter->setPen(oldPen);

	}

	void LevelLink::setDragOffset(double x, double y, bool snap) {
		AbstractLevelEntity::setDragOffset(x, y, true);
	}

	void LevelLink::drag(double x, double y, bool snap, IWorld* world) {
		AbstractLevelEntity::drag(x, y, true, world);
	}

	void LevelLink::openEditor(IWorld* world)
	{
		EditLinkDialog form(this, world);
		form.exec();
	}

	AbstractLevelEntity* LevelLink::duplicate() {
		auto link = new LevelLink(this->getLevel(), getX(), getY(), getWidth(), getHeight(), false);
		link->m_nextLevel = this->m_nextLevel;
		link->m_nextX = this->m_nextX;
		link->m_nextY = this->m_nextY;
		return link;
	}

	

}
