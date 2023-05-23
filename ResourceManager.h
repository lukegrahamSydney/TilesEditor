#ifndef RESOURCEMANAGERH
#define RESOURCEMANAGERH


#include <QString>
#include <QMap>
#include <QList>
#include <QSet>
#include <algorithm>

#include "Resource.h"
#include "ResourceType.h"

#include "AbstractFileSystem.h"

namespace TilesEditor
{
	class ResourceManager
	{

	private:
		QString			m_rootDir;

		QMap<QString, Resource*>    m_resources;
		QSet<QString> m_searchDirectories;
		QList<QString> m_searchDirectoriesList;
		QSet<QString> m_failedResources;

		AbstractFileSystem* m_fileSystem;

		static QString getResourceExt(const QString& resName) {
			auto pos = resName.lastIndexOf('.');
			if (pos != -1)
			{
				return resName.right(pos + 1);
			}
			return "";
		}



	public:

		ResourceManager(AbstractFileSystem* fileSystem);
		~ResourceManager();

		void setRootDir(const QString& dir) { m_rootDir = dir + "/"; }
		const QString& getRootDir() const { return m_rootDir; }
		void mergeSearchDirectories(const ResourceManager& source);
		void addSearchDir(const QString& dir);
		void addSearchDirRecursive(const QString& dir);
		bool locateFile(const QString& name, QString* outPath);
		bool containsDirectory(const QString& dir);

		void addPersistantResource(Resource* resource);
		bool writeTextFile(const QString& fileName, const QString& contents);
		QString readTextFile(const QString& fileName);

		AbstractFileSystem* getFileSystem() { return m_fileSystem; }
		Resource* loadResource(const QString& name, ResourceType type);
		void freeResource(Resource* resource);
		void updateResource(const QString& name);

		static QString formatResName(const QString& resName)
		{
			return resName.toLower();
		}
	};
};

#endif
