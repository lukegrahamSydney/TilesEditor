#include "AboutDialog.h"
#include "DarkTitleBar.h"

namespace TilesEditor
{
	AboutDialog::AboutDialog(QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		DarkTitleBar::ApplyStyle(this->winId());
		this->setFixedSize(this->size());
	}

	AboutDialog::~AboutDialog()
	{}
};
