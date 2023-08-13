#include <QIntValidator>
#include "NewLevelDialog.h"
#include "FileFormatManager.h"

namespace TilesEditor
{
	NewLevelDialog::NewLevelDialog(QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

	
		ui.widthLineEdit->setValidator(new QIntValidator(1, 999999999));
		ui.heightLineEdit->setValidator(new QIntValidator(1, 999999999));

		auto formats = FileFormatManager::instance()->getRegisteredFormats();
		for (auto format : formats)
		{
			ui.formatComboBox->addItem(format->getPrimaryExtension());
		}
	}

	NewLevelDialog::~NewLevelDialog()
	{}

	int NewLevelDialog::getHCount() const
	{
		return ui.widthLineEdit->text().toInt();
	}

	int NewLevelDialog::getVCount() const
	{
		return ui.heightLineEdit->text().toInt();
	}

	QString NewLevelDialog::getFormat() const
	{
		return ui.formatComboBox->currentText();
	}
};
