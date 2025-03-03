#include "CustomPaintWidget.h"

void TilesEditor::CustomPaintWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	emit mouseDoubleClick(event);
}
