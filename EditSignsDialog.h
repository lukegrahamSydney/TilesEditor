#pragma once

#include <QDialog>
#include "ui_EditSignsDialog.h"
#include "Level.h"
#include "LevelSign.h"
#include "IWorld.h"

namespace TilesEditor
{
	class EditSignsDialog : public QDialog
	{
		Q_OBJECT
	private slots:
		void signSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
		void createSignClicked(bool checked);
		void deleteSignClicked(bool checked);
		void xEditChanged(const QString& text);
		void yEditChanged(const QString& text);
		void signTextEditFinished();

	private:
		Level* m_level;
		IWorld* m_world;

	public:
		EditSignsDialog(Level* level, IWorld* world, LevelSign* selectedSign = nullptr, QWidget* parent = nullptr);
		~EditSignsDialog();

	private:
		Ui::EditSignsDialogClass ui;

		void populateTable();
		void updateSelectedSign();
		void updateRow(int row);
	};
}
