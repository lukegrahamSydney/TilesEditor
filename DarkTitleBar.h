#ifndef DARKTITLEBARH
#define DARKTITLEBARH

#ifdef WIN32
#include <dwmapi.h>

#endif

#include <QWidget>
namespace TilesEditor
{
	namespace DarkTitleBar
	{
		inline void ApplyStyle(WId handle) {
#ifdef WIN32
			BOOL USE_DARK_MODE = true;
			DwmSetWindowAttribute(
				(HWND)handle, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
				&USE_DARK_MODE, sizeof(USE_DARK_MODE));

#endif
		}
	};
}

#endif
