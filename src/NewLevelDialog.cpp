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
			ui.formatComboBox->addItem(format->getPrimaryExtension(), QVariant(format->customLevelSizes()));
		}

		connect(ui.formatComboBox, &QComboBox::currentIndexChanged, this, &NewLevelDialog::itemIndexChanged);
	}

	NewLevelDialog::~NewLevelDialog()
	{

	}

	void NewLevelDialog::itemIndexChanged(int index)
	{
		auto enableCustomSizes = ui.formatComboBox->itemData(index).toBool();

		ui.widthLabel->setVisible(enableCustomSizes);
		ui.widthLineEdit->setVisible(enableCustomSizes);
		ui.heightLineEdit->setVisible(enableCustomSizes);
		ui.heightLabel->setVisible(enableCustomSizes);
	}

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

	bool NewLevelDialog::getFormatCustomLevelSizes() const {
		return false;
	}
};
