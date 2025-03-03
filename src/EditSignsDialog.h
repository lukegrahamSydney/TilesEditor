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
		void signSelectionChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
		void createSignClicked(bool checked);
		void deleteSignClicked(bool checked);
		void xEditChanged(const QString& text);
		void yEditChanged(const QString& text);
		void signTextEditFinished();
		void accept() override;

	private:
		Level* m_level;
		IWorld* m_world;
		inline static QByteArray savedGeometry;

	public:
		EditSignsDialog(Level* level, IWorld* world, LevelSign* selectedSign = nullptr, QWidget* parent = nullptr);
		~EditSignsDialog();

	private:
		Ui::EditSignsDialogClass ui;

		void updateSign(int index);
		void updateRow(int row);

		void updateVerticalHeader();
	};
}
