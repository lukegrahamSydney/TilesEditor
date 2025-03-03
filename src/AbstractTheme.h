#ifndef ITEMEH
#define ITEMEH

#include <QApplication>
#include <QString>
#include <QMainWindow>
#include <QAction>
#include <QColor>
namespace TilesEditor
{
	class AbstractTheme:
		public QAction
	{
	public:
		virtual void applyTheme(QMainWindow* mainWindow, QApplication& application) = 0;
		virtual bool isDarkTheme() const {
			return false;
		};
		virtual QColor titlebarColour() { return QColor("#237fca"); }
	};
};
#endif
