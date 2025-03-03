#ifndef FUSIONLIGHTTHEMEH
#define FUSIONLIGHTTHEMEH
#include <QStyleHints>
#include "AbstractTheme.h"

namespace TilesEditor
{
	class FusionLightTheme :
		public AbstractTheme
	{
	public:
		inline FusionLightTheme()
		{
			this->setText("Fusion Light");
		}

		inline void applyTheme(QMainWindow* mainWindow, QApplication& app) override
		{
			app.setStyle("fusion");
			app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
			app.setStyleSheet("");
		}
	};
}
#endif