#include "NewOverworldDialog.h"

namespace TilesEditor
{
	NewOverworldDialog::NewOverworldDialog(QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		m_retval = nullptr;
	}

	NewOverworldDialog::~NewOverworldDialog()
	{}
};
