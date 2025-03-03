#ifndef FIXMAPNAMESDIALOGH
#define FIXMAPNAMESDIALOGH

#include <QDialog>
#include "ui_FixMapNamesDialog.h"

namespace TilesEditor
{
	class FixMapNamesDialog : public QDialog
	{
		Q_OBJECT

	public:
		FixMapNamesDialog(QWidget* parent = nullptr);
		~FixMapNamesDialog();

	private:
		Ui::FixMapNamesDialogClass ui;
	};
}

#endif
