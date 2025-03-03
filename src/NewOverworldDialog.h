#ifndef NEWOVERWORLDDIALOGH
#define NEWOVERWORLDDIALOGH

#include <QDialog>
#include "ui_NewOverworldDialog.h"
#include "Overworld.h"

namespace TilesEditor
{
	class NewOverworldDialog : public QDialog
	{
		Q_OBJECT

	public:
		NewOverworldDialog(QWidget* parent = nullptr);
		~NewOverworldDialog();

	private:
		Ui::NewOverworldDialogClass ui;
		Overworld* m_retval;

	};
};

#endif
