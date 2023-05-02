#pragma once

#include <QDialog>
#include "ui_EditExternalNPC.h"

namespace TilesEditor
{
	class EditExternalNPC : public QDialog
	{
		Q_OBJECT

	public:
		EditExternalNPC(QWidget* parent = nullptr);
		~EditExternalNPC();

	private:
		Ui::EditExternalNPCClass ui;
	};
};
