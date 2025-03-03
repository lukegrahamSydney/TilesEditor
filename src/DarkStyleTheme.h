#ifndef DARKSTYLETHEMEH
#define DARKSTYLETHEMEH

#include <QStyleHints>
#include "AbstractTheme.h"

namespace TilesEditor
{
    class DarkStyleTheme :
		public AbstractTheme
	{
	public:
        inline DarkStyleTheme()
        {
            this->setText("Dark Style");
        }
        inline bool isDarkTheme() const override { return true; }

        inline void applyTheme(QMainWindow* mainWindow, QApplication& app) override
        {
            QFile f(":/qdarkstyle/dark/darkstyle.qss");
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