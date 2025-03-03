#ifndef NEWLEVELDIALOGH
#define NEWLEVELDIALOGH

#include <QDialog>
#include "ui_NewLevelDialog.h"

namespace TilesEditor
{
	class NewLevelDialog : public QDialog
	{
		Q_OBJECT
	private slots:
		void itemIndexChanged(int index);

	public:
		NewLevelDialog(QWidget* parent = nullptr);
		~NewLevelDialog();

		int getHCount() const;
		int getVCount() const;
		QString getFormat() const;
		bool getFormatCustomLevelSizes() const;

	private:
		Ui::NewLevelDialogClass ui;
	};
};

#endif
