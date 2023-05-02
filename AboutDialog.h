#pragma once

#include <QDialog>
#include "ui_AboutDialog.h"

namespace TilesEditor
{
	class AboutDialog : public QDialog
	{
		Q_OBJECT

	public:
		AboutDialog(QWidget* parent = nullptr);
		~AboutDialog();

	private:
		Ui::AboutDialogClass ui;
	};
}
