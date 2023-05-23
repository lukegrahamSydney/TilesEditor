#include <QFileDialog>
#include "LevelConverter.h"

namespace TilesEditor
{


	LevelConverter::LevelConverter(QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		connect(ui.inputBrowseButton, &QAbstractButton::clicked, this, &LevelConverter::inputBrowseClicked);
		connect(ui.outputBrowseButton, &QAbstractButton::clicked, this, &LevelConverter::outputBrowseClicked);

		this->setFixedSize(this->size());
	}

	LevelConverter::~LevelConverter()
	{}

	void LevelConverter::accept()
	{
		//do it here
	}

	void LevelConverter::inputBrowseClicked(bool checked)
	{
		auto folder = QFileDialog::getExistingDirectory(nullptr, "Select folder containing levels");
		if (!folder.isEmpty())
			ui.inputEdit->setText(folder);

	}

	void LevelConverter::outputBrowseClicked(bool checked)
	{
		auto folder = QFileDialog::getExistingDirectory(nullptr, "Select folder to save levels");
		if (!folder.isEmpty())
			ui.outFolderEdit->setText(folder);
	}
};

