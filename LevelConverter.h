#ifndef LEVELCONVERTERH
#define LEVELCONVERTERH

#include <QDialog>
#include "ui_LevelConverter.h"

namespace TilesEditor
{
	class LevelConverter : public QDialog
	{
		Q_OBJECT

	private slots:
		void inputBrowseClicked(bool checked);
		void outputBrowseClicked(bool checked);

	public:
		LevelConverter(QWidget* parent = nullptr);
		~LevelConverter();

	protected:

		void accept() override;

	private:
		Ui::LevelConverterClass ui;
	};
};

#endif
