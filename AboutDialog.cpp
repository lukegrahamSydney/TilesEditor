#include "AboutDialog.h"

namespace TilesEditor
{
	AboutDialog::AboutDialog(QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		this->setFixedSize(this->size());
	}

	AboutDialog::~AboutDialog()
	{}
};
