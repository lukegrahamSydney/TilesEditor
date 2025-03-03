#pragma once

#include <QWidget>
#include "ui_ObjectsWidget.h"

namespace TilesEditor
{
	class ObjectsWidget : public QWidget
	{
		Q_OBJECT

	public:
		ObjectsWidget(QWidget* parent = nullptr);
		~ObjectsWidget();

	private:
		Ui::ObjectsWidgetClass ui;
	};
}
