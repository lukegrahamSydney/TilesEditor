#ifndef GENERICTHEMEH
#define GENERICTHEMEH

#include <QMainWindow>
#include <QApplication>
#include <QFile>
#include <QStyleHints>
#include "AbstractTheme.h"

namespace TilesEditor
{
	class GenericTheme :
		public AbstractTheme
	{
	private:
		QString m_resourceName;
		bool m_darkMode = false;

	public:
		inline GenericTheme(const QString& name, const QString& resourceName, bool darkMode):
			m_resourceName(resourceName), m_darkMode(darkMode)
		{
			this->setText(name);
		}

		inline const QString& getResourceName() const { return m_resourceName; }
		void setResourceName(const QString& name) { m_resourceName = name; }


		inline bool isDarkTheme() const override { return m_darkMode; }
		void setDarkTheme(bool value) { m_darkMode = value; }
		inline void applyTheme(QMainWindow* mainWindow, QApplication& app) override
		{
			QFile f(m_resourceName);
			if (f.exists()) {

				if(m_darkMode)
					app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
				else app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
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