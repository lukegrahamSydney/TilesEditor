#include "AbstractSelection.h"

namespace TilesEditor
{
	void AbstractSelection::setDragOffset(double x, double y, bool snap, double snapX, double snapY)
	{
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

	void AbstractSelection::drag(double x, double y, bool snap, double snapX, double snapY, IWorld* world)
	{
		if (snap)
		{
			m_x = (std::round(x / snapX) * snapX) - m_dragOffsetX;
			m_y = (std::round(y / snapY) * snapY) - m_dragOffsetY;
		}
		else {
			m_x = x - m_dragOffsetX;
			m_y = y - m_dragOffsetY;
		}
	}


}