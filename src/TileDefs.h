#ifndef TILEDEFSH
#define TILEDEFSH

#include <QString>

namespace TilesEditor
{
	struct TileDef
	{
		QString imageName;
		QString prefix;
		int x;
		int y;
		bool saves = false;
	};
}
#endif
