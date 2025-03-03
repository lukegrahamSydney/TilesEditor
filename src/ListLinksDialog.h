#pragma once

#include <QDialog>
#include <QItemDelegate>
#include <QLineEdit>
#include "ui_ListLinksDialog.h"
#include "Level.h"
#include "IWorld.h"

namespace TilesEditor
{
	class ListLinksDialog : public QDialog
	{
		Q_OBJECT

	public slots:
		void deleteClicked(bool);
		void editClicked(bool);

	public:
		ListLinksDialog(Level* level, IWorld* world, QWidget* parent = nullptr);
		~ListLinksDialog();

	private:
		Ui::ListLinksDialogClass ui;

		Level* m_level;
		IWorld* m_world;


		void populateTable();
	
	};
};
