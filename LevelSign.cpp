#include <QColor>
#include "LevelSign.h"

namespace TilesEditor
{
	LevelSign::LevelSign(Level* level, double x, double y, int width, int height) :
		AbstractLevelEntity(level, x, y)
	{
		m_width = width;
		m_height = height;
	}

	void LevelSign::setText(const QString& text)
	{
		m_text = text;
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
