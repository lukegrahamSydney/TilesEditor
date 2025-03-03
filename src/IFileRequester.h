#ifndef IFILEREQUESTERH
#define IFILEREQUESTERH

#include <QString>

namespace TilesEditor
{
	class AbstractResourceManager;
	class IFileRequester
	{
	public:
		//Filename should be name part only. not FULL PATH
		virtual void fileFailed(const QString& name, AbstractResourceManager* resourceManager) = 0;
		virtual void fileReady(const QString& fileName, AbstractResourceManager* resourceManager) = 0;
		virtual void fileWritten(const QString& fileName, AbstractResourceManager* resourceManager) = 0;
	};
};
#endif
