#include <QColor>
#include "LevelLink.h"
#include "AbstractSelection.h"
#include "cJSON/JsonHelper.h"
#include "EditLinkDialog.h"

namespace TilesEditor
{
	LevelLink::LevelLink(IWorld* world, double x, double y, int width, int height, bool possibleEdgeLink):
		AbstractLevelEntity(world, x, y)
	{ 
		m_width = width;
		m_height = height;
		m_possibleEdgeLink = possibleEdgeLink;

	}

	LevelLink::LevelLink(IWorld* world, cJSON* json):
		LevelLink(world, 0, 0, 0, 0, false)
	{
		deserializeJSON(json);
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

	void LevelLink::deserializeJSON(cJSON* json)
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

	void LevelLink::setDragOffset(double x, double y, bool snap, double snapX, double snapY) {
		AbstractLevelEntity::setDragOffset(x, y, true, std::ceil(snapX / 16.0) * 16.0, std::ceil(snapY / 16.0) * 16.0);
	}

	void LevelLink::drag(double x, double y, bool snap, double snapX, double snapY) {
		AbstractLevelEntity::drag(x, y, true, std::ceil(snapX / 16.0) * 16.0, std::ceil(snapY / 16.0) * 16.0);
	}

	void LevelLink::openEditor()
	{
		EditLinkDialog form(this, getWorld());
		form.exec();
	}

	AbstractLevelEntity* LevelLink::duplicate() {
		auto link = new LevelLink(this->getWorld(), getX(), getY(), getWidth(), getHeight(), false);
		link->m_nextLevel = this->m_nextLevel;
		link->m_nextX = this->m_nextX;
		link->m_nextY = this->m_nextY;
		return link;
	}

	

}
