#include "AbstractSelection.h"

namespace TilesEditor
{
	void AbstractSelection::setDragOffset(double x, double y, bool snap)
	{
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

	void AbstractSelection::drag(double x, double y, bool snap, IWorld* world)
	{
		if (snap)
		{
			m_x = (std::floor(x / 16.0) * 16) - m_dragOffsetX;
			m_y = (std::floor(y / 16.0) * 16) - m_dragOffsetY;
		}
		else {
			m_x = x - m_dragOffsetX;
			m_y = y - m_dragOffsetY;
		}
	}


}