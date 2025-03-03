#ifndef IEDITMODEH
#define IEDITMODEH

#include <QMouseEvent>

namespace TilesEditor
{
	class IEditMode
	{
	public:
		enum EditMode {
			MODE_INSERT,
			MODE_MOVE
		};

	public:
		virtual void mousePress(QMouseEvent* event) = 0;
		virtual void mouseRelease(QMouseEvent* event) = 0;
		virtual void mouseMove(QMouseEvent* event) = 0;
		virtual void mouseDoubleClick(QMouseEvent* event) = 0;
		virtual void mouseWheel(QWheelEvent* event) = 0;
	};
};
#endif