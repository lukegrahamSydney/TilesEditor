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

	void AbstractLevelEntity::setDragOffset(double x, double y, bool snap, double snapX, double snapY) {
		if (snap)
		{
			m_dragOffsetX = int(std::floor((x - this->getX()) / snapX) * snapX);
			m_dragOffsetY = int(std::floor((y - this->getY()) / snapY) * snapY);
		}
		else {
			m_dragOffsetX = x - this->getX();
			m_dragOffsetY = y - this->getY();
		}
	}

	void AbstractLevelEntity::drag(double x, double y, bool snap, double snapX, double snapY) {
		auto oldX = getX();
		auto oldY = getY();

		if (snap)
		{
			m_x = (std::floor(x / snapX) * snapX) - m_dragOffsetX;
			m_y = (std::floor(y / snapY) * snapY) - m_dragOffsetY;
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

	void AbstractLevelEntity::updateResize(int edges, int mouseX, int mouseY, bool snap, double snapX, double snapY)
	{
		if (edges & AbstractSelection::EDGE_LEFT)
		{
			int x = std::min(mouseX, (int)getRight());
			int width = (int)getRight() - x;
			width = std::max(int(snapX), int(std::floor(width / snapX) * snapX));

			setX(getRight() - width);
			setWidth(width);
		}

		if (edges & AbstractSelection::EDGE_RIGHT)
		{
			int pos2 = std::max(mouseX, (int)getX());
			int width = pos2 - (int)getX();
			width = std::max(int(snapX), int(std::ceil(width / snapX) * snapX));

			setWidth(width);
		}

		if (edges & AbstractSelection::EDGE_TOP)
		{
			int y = std::min(mouseY, (int)getBottom());
			int height = (int)getBottom() - y;
			height = std::max(int(snapY), int(std::floor(height / snapY) * snapY));

			setY(getBottom() - height);
			setHeight(height);
		}


		if (edges & AbstractSelection::EDGE_BOTTOM)
		{
			int pos2 = std::max(mouseY, (int)getY());
			int height = pos2 - (int)getY();
			height = std::max(int(snapY), int(std::ceil(height / snapY) * snapY));
			setHeight(height);
		}
	}
}