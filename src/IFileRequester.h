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
		virtual void fileFailed(const QString& name, AbstractResourceManager* resourceManager) {};
		virtual void fileReady(const QString& fileName, AbstractResourceManager* resourceManager) {};
		virtual void fileWritten(const QString& fileName, AbstractResourceManager* resourceManager) {};
	};
};
#endif
