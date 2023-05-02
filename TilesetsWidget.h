#pragma once

#include <QWidget>
#include "ui_TilesetsWidget.h"

class TilesetsWidget : public QWidget
{
	Q_OBJECT

public:
	TilesetsWidget(QWidget *parent = nullptr);
	~TilesetsWidget();

private:
	Ui::TilesetsWidgetClass ui;
};
