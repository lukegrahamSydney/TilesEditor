#pragma once

#include <QDialog>
#include <QVector>
#include <QMenu>
#include <QActionGroup>
#include "ui_NewCustomTheme.h"
#include "GenericTheme.h"

namespace TilesEditor
{
	class NewCustomTheme : public QDialog
	{
		Q_OBJECT
	private slots:
		void themesItemSelectionChanged();
		void themeNewPressed();
		void themeDeletePressed();
		void themeNameChanged(const QString& text);
		void themeStyleSheetChanged(const QString& text);
		void themeDarkModeChanged(Qt::CheckState state);

	public:
		NewCustomTheme(QMenu& menu, QActionGroup* actionGroup, QWidget* parent = nullptr);
		~NewCustomTheme();

		void addTheme(GenericTheme* theme);

		int getThemeCount() const;
	private:
		Ui::NewCustomThemeClass ui;
		QMenu& m_menu;
		QActionGroup* m_menuGroup;

	};
};
