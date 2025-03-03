#ifndef FUSIONDARKTHEMEH
#define FUSIONDARKTHEMEH

#include <QStyleHints>
#include "AbstractTheme.h"
#include "DarkStyle.h"

namespace TilesEditor
{
    class FusionDarkTheme :
		public AbstractTheme
	{
    private:
        DarkStyle m_darkStyle;
	public:
        FusionDarkTheme()
        {
            this->setText("Fusion Dark");
        }

        inline bool isDarkTheme() const override { return true; }
        inline void applyTheme(QMainWindow* mainWindow, QApplication& app) override
        {
            app.setStyle("fusion");
            app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
            app.setStyleSheet("");

           // app.setPalette(QPalette());
            return;

            QFile f(":/darkstyle/darkstyle.qss");
            if (f.exists()) {
                f.open(QFile::ReadOnly | QFile::Text);
                QTextStream ts(&f);
                app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
                app.setStyle("fusion");
                app.setStyleSheet(ts.readAll());
             
                auto palette = app.palette();
          
                // modify palette to dark
                palette.setColor(QPalette::Window, QColor(53, 53, 53));
                palette.setColor(QPalette::WindowText, Qt::white);
                palette.setColor(QPalette::Disabled, QPalette::WindowText,
                    QColor(127, 127, 127));
                palette.setColor(QPalette::Base, QColor(42, 42, 42));
                palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
                palette.setColor(QPalette::ToolTipBase, Qt::white);
                palette.setColor(QPalette::ToolTipText, QColor(53, 53, 53));
                palette.setColor(QPalette::Text, Qt::white);
                palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
                palette.setColor(QPalette::Dark, QColor(35, 35, 35));
                palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
                palette.setColor(QPalette::Button, QColor(53, 53, 53));
                palette.setColor(QPalette::ButtonText, Qt::white);
                palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                    QColor(127, 127, 127));
                palette.setColor(QPalette::BrightText, Qt::red);
                palette.setColor(QPalette::Link, QColor(42, 130, 218));
                palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
                palette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
                palette.setColor(QPalette::HighlightedText, Qt::white);
                palette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                    QColor(127, 127, 127));

         



                app.setPalette(palette);

            }

            return;
            
            //DarkTitleBar::ApplyStyle(mainWindow->winId(), UsingDarkMode);

            app.setStyle("fusion");
            app.setStyleSheet("");
            mainWindow->setStyleSheet("");
            return;
            //
            //app.setPalette(QPalette());
            QPalette darkPalette = app.palette();

            darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::WindowText, Qt::white);
            darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
            darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
            darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
            darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
            darkPalette.setColor(QPalette::ToolTipText, Qt::white);
            darkPalette.setColor(QPalette::Text, Qt::white);
            darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
            darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
            darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
            darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
            darkPalette.setColor(QPalette::ButtonText, Qt::white);
            darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
            darkPalette.setColor(QPalette::BrightText, Qt::red);
            darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
            darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
            darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
            darkPalette.setColor(QPalette::HighlightedText, Qt::white);
            darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

            app.setPalette(darkPalette);
        }
	};

}
#endif