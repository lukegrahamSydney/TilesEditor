#ifndef MAINFILESYSTEMH
#define MAINFILESYSTEMH

#include <QString>
#include <QStringList>
#include <QList>
#include <QSet>
#include "AbstractFileSystem.h"

namespace TilesEditor
{
	class MainFileSystem :
		public AbstractFileSystem
	{
	private:
		static void getFoldersRecursive(const QString& searchPath, QStringList& output, int level, const QString& rootDir);

	public:
		MainFileSystem();

		void requestFile(IFileRequester* requester, const QString& fileName) override {}
		void removeListener(IFileRequester* requester) override {}

		QStringList getFolders(const QString& parent) override;
		bool fileExists(const QString& fileName) override;
		QIODevice* openStream(const QString& fileName, QIODeviceBase::OpenModeFlag mode) override;
		void endWrite(const QString& fileName, QIODevice* stream) override {
			delete stream;

		}
	};
};

#endif
