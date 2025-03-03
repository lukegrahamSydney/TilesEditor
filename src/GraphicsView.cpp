#include "GraphicsView.h"

namespace TilesEditor
{
	void GraphicsView::drawBackground(QPainter* painter, const QRectF& rect)
	{
		//if(m_antialias)
		painter->setRenderHint(QPainter::SmoothPixmapTransform, m_antialias);
		painter->setRenderHint(QPainter::VerticalSubpixelPositioning, 1);
		emit renderView(painter, rect);
	}



	void GraphicsView::keyPressEvent(QKeyEvent* event)
	{

		emit keyPress(event);
	}

	void GraphicsView::wheelEvent(QWheelEvent* event)
	{
		emit mouseWheelEvent(event);

		if (!event->isAccepted())
			QGraphicsView::wheelEvent(event);
	}

	void GraphicsView::mouseMoveEvent(QMouseEvent* event)
	{
		emit mouseMove(event);
	}

	void GraphicsView::mousePressEvent(QMouseEvent* event)
	{
		emit mousePress(event);
	}

	void GraphicsView::mouseReleaseEvent(QMouseEvent* event)
	{
		emit mouseRelease(event);
	}
	void GraphicsView::mouseDoubleClickEvent(QMouseEvent* event)
	{
		emit mouseDoubleClick(event);


	}

	void GraphicsView::scrollContentsBy(int dx, int dy)
	{
		QGraphicsView::scrollContentsBy(dx, dy);
		emit contentsScrolled();
	}

}

