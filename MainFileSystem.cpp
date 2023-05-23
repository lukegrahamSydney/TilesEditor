#include <QDir>
#include "MainFileSystem.h"

namespace TilesEditor
{

	MainFileSystem::MainFileSystem()
	{

	}

	void MainFileSystem::getFoldersRecursive(const QString& searchPath, QStringList& output, int level, const QString& rootDir)
	{
		if (output.count() < 100 && level <= 2)
		{
			output.push_back(searchPath);

			QDir dir(searchPath);

			auto list = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

			for (auto& folder : list)
			{
				getFoldersRecursive(QString("%1%2/").arg(searchPath, folder), output, level + 1, rootDir + folder + "/");
			}

		}
	}


	QStringList MainFileSystem::getFolders(const QString& parent)
	{
		QDir absoluteDir(parent);

		QStringList retval;
		getFoldersRecursive(absoluteDir.absolutePath() + "/", retval, 0, "");
		return retval;
	}

	bool MainFileSystem::fileExists(const QString& fileName)
	{
		return QFile::exists(fileName);
	}

	QIODevice* MainFileSystem::openStream(const QString& fileName, QIODeviceBase::OpenModeFlag mode)
	{
		QFile* file = new QFile(fileName);
		if (file->open(mode))
			return file;

		delete file;
		
		return nullptr;

	}
};