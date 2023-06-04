#ifndef NEWLEVELDIALOGH
#define NEWLEVELDIALOGH

#include <QDialog>
#include "ui_NewLevelDialog.h"

namespace TilesEditor
{
	class NewLevelDialog : public QDialog
	{
		Q_OBJECT

	public:
		NewLevelDialog(QWidget* parent = nullptr);
		~NewLevelDialog();

		int getHCount() const;
		int getVCount() const;
		QString getFormat() const;

	private:
		Ui::NewLevelDialogClass ui;
	};
};

#endif
