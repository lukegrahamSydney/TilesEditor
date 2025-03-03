#include <QDirIterator>
#include <QFile>
#include "ResourceManagerFileSystem.h"

namespace TilesEditor
{
	void ResourceManagerFileSystem::searchDirectory(const QString& searchPath, int level, int maxLevel)
	{
		auto count = m_searchDirectories.count();
		m_searchDirectories.insert(searchPath);

		//Checks if it was added to m_searchDirectories
		if (count != m_searchDirectories.count())
			m_searchDirectoriesList.push_back(searchPath);

		if (level <= maxLevel)
		{
			QDirIterator it(searchPath, QStringList(), QDir::Filter::Files | QDir::Filter::NoDotAndDotDot | QDir::Filter::Dirs);
			while (it.hasNext())
			{
				auto fInfo = it.nextFileInfo();

				if (fInfo.isDir())
				{
					auto newPath = fInfo.absoluteFilePath();
					if (!newPath.endsWith('/'))
						newPath += '/';
					searchDirectory(newPath, level + 1, maxLevel);
				}
				else if (fInfo.isFile())
				{
					m_fileNameCache[fInfo.fileName().toLower()] = fInfo.absoluteFilePath();
				}
			}
			
		}
	}

	QIODevice* ResourceManagerFileSystem::openStreamFullPath(const QString& fullPath, QIODeviceBase::OpenModeFlag mode)
	{
		QFile* file = new QFile(fullPath);
		if (file->open(mode))
			return file;

		delete file;

		return nullptr;
	}


	ResourceManagerFileSystem::ResourceManagerFileSystem(const QString& rootDir, ObjectManager* objectManager) :
		AbstractResourceManager(objectManager)
	{
		setRootDir(rootDir);
		addSearchDir(m_rootDir);
			
	}

	void ResourceManagerFileSystem::setRootDir(const QString& dir)
	{
		QFileInfo fi(dir);
		m_rootDir = fi.absoluteFilePath();

		if (!m_rootDir.endsWith('/'))
			m_rootDir += '/';
	}

	void ResourceManagerFileSystem::addSearchDir(const QString& dir)
	{
		QFileInfo fi(dir);
		auto searchPath = fi.absoluteFilePath();

		if (!searchPath.endsWith('/'))
			searchPath += '/';

		auto count = m_searchDirectories.count();
		m_searchDirectories.insert(searchPath);

		if (count != m_searchDirectories.count())
			m_searchDirectoriesList.push_back(searchPath);
	}

	void ResourceManagerFileSystem::addSearchDirRecursive(const QString& dir, int max)
	{
		QFileInfo fi(dir);
		auto searchPath = fi.absoluteFilePath();
		if (!searchPath.endsWith('/'))
			searchPath += '/';

		searchDirectory(searchPath, 0, max);
	}

	bool ResourceManagerFileSystem::isCompatible(const QString& connectionString)
	{
		return connectionString.startsWith(getConnectionString());
	}

	void ResourceManagerFileSystem::mergeResourceManager(AbstractResourceManager* other)
	{
		//Merges the folders and file name cache from other
		if (other->getType() == "FileSystem")
		{
			auto otherFileSystem = static_cast<ResourceManagerFileSystem*>(other);
			for (auto pair : otherFileSystem->m_fileNameCache.asKeyValueRange())
				this->m_fileNameCache[pair.first] = pair.second;

			for (auto& dir : otherFileSystem->m_searchDirectoriesList)
			{
				auto count = this->m_searchDirectories.count();
				m_searchDirectories.insert(dir);
				if (count != this->m_searchDirectories.count())
					m_searchDirectoriesList.push_back(dir);
			}
		}
	}

	bool ResourceManagerFileSystem::locateFile(const QString& name, QString* outPath)
	{
		auto fileNameLower = name.toLower();

		auto it = m_fileNameCache.find(fileNameLower);
		if (it != m_fileNameCache.end())
		{
			if (outPath)
				*outPath = it.value();
			return true;
		}

		for (auto& dir : m_searchDirectoriesList)
		{
			QString fullPath = dir + fileNameLower;

			if (QFile::exists(fullPath))
			{
				m_fileNameCache[fileNameLower] = fullPath;

				if (outPath)
					*outPath = fullPath;
				return true;
			}
		}

		*outPath = "";
		return false;
	}

	QIODevice* ResourceManagerFileSystem::openStream(const QString& fileName, QIODeviceBase::OpenModeFlag mode, QString* outPath)
	{
		QString outPath2;
		if (locateFile(fileName, &outPath2))
		{
			if (outPath)
				*outPath = outPath2;
			return openStreamFullPath(outPath2, mode);
		}
		
		if (outPath)
			*outPath = "";
		return nullptr;
	}
}
