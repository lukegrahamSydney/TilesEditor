#include "AbstractLevelEntity.h"
#include "AbstractSelection.h"
#include "Level.h"

namespace TilesEditor
{
	double AbstractLevelEntity::nextMicroDepth = 0.000000001;
	double AbstractLevelEntity::getUnitWidth() const
	{
		return getLevel()->getUnitWidth();
	}

	double AbstractLevelEntity::getUnitHeight() const {
		return getLevel()->getUnitHeight();
	}

	void AbstractLevelEntity::setDragOffset(double x, double y, bool snap) {
		if (snap)
		{
			m_dragOffsetX = int(std::floor((x - this->getX()) / 16.0) * 16.0);
			m_dragOffsetY = int(std::floor((y - this->getY()) / 16.0) * 16.0);
		}
		else {
			m_dragOffsetX = x - this->getX();
			m_dragOffsetY = y - this->getY();
		}
	}

	void AbstractLevelEntity::drag(double x, double y, bool snap) {
		auto oldX = getX();
		auto oldY = getY();

		if (snap)
		{
			m_x = (std::floor(x / 16.0) * 16) - m_dragOffsetX;
			m_y = (std::floor(y / 16.0) * 16) - m_dragOffsetY;
		}
		else {
			m_x = x - m_dragOffsetX;
			m_y = y - m_dragOffsetY;
		}

		if (this->getLevel())
		{
			if (oldX != getX() || oldY != getY())
				getWorld()->setModified(this->getLevel());
		}
	}

	void AbstractLevelEntity::updateResize(int edges, int mouseX, int mouseY, bool snap)
	{
		if (edges & AbstractSelection::EDGE_LEFT)
		{
			int x = std::min(mouseX, (int)getRight());
			int width = (int)getRight() - x;
			width = std::max(16, int(std::floor(width / 16.0) * 16.0));

			setX(getRight() - width);
			setWidth(width);
		}

		if (edges & AbstractSelection::EDGE_RIGHT)
		{
			int pos2 = std::max(mouseX, (int)getX());
			int width = pos2 - (int)getX();
			width = std::max(16, int(std::ceil(width / 16.0) * 16.0));

			setWidth(width);
		}

		if (edges & AbstractSelection::EDGE_TOP)
		{
			int y = std::min(mouseY, (int)getBottom());
			int height = (int)getBottom() - y;
			height = std::max(16, int(std::floor(height / 16.0) * 16.0));

			setY(getBottom() - height);
			setHeight(height);
		}


		if (edges & AbstractSelection::EDGE_BOTTOM)
		{
			int pos2 = std::max(mouseY, (int)getY());
			int height = pos2 - (int)getY();
			height = std::max(16, int(std::ceil(height / 16.0) * 16.0));
			setHeight(height);
		}
	}
}