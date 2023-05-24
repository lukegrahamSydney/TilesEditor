#ifndef IFILEREQUESTERH
#define IFILEREQUESTERH

#include <QString>

namespace TilesEditor
{
	class IFileRequester
	{
	public:
		//Filename should be name part only. not FULL PATH
		virtual void fileReady(const QString& fileName) = 0;
		virtual void fileWritten(const QString& fileName) = 0;
	};
};
#endif
