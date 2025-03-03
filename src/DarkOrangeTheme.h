#ifndef DARKORANGETHEMEH
#define DARKORANGETHEMEH

#include <QMainWindow>
#include <QApplication>
#include <QFile>
#include <QStyleHints>
#include "AbstractTheme.h"

namespace TilesEditor
{
	class DarkOrangeTheme :
		public AbstractTheme
	{
	public: 
		inline DarkOrangeTheme()
		{
			this->setText("Dark Orange");
		}


		inline bool isDarkTheme() const override { return true; }
		inline void applyTheme(QMainWindow* mainWindow, QApplication& app) override
		{
			QFile f(":/darkorange.qss");
			if (f.exists()) {

				app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
				app.setStyle("fusion");
				app.setPalette(QPalette());

				f.open(QFile::ReadOnly | QFile::Text);
				QTextStream ts(&f);
				app.setStyleSheet(ts.readAll());

			}

		}
	};

}
#endif