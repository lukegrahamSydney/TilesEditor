#pragma once

#include <QWidget>
#include "ui_TileObjectsWidget.h"

namespace TilesEditor
{
	class TileObjectsWidget : public QWidget
	{
		Q_OBJECT

	public:
		TileObjectsWidget(QWidget* parent = nullptr);
		~TileObjectsWidget();

	private:
		Ui::TileObjectsWidgetClass ui;
	};
};