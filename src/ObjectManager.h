#ifndef OBJECTMANAGERH
#define OBJECTMANAGERH

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMap>
#include "ObjectClass.h"
#include "ResourceManagerFileSystem.h"

namespace TilesEditor
{
	class ResourceManagerFileSystem;
	class ObjectManager
	{
	private:
		ResourceManagerFileSystem m_resourceManager;
		QTreeWidget* m_objectTree;
		QString m_folder;
		QMap<QString, ObjectClass*> m_objects;


		ObjectClass* loadNewObject(QTreeWidgetItem* parent, const QString& path, const QString& className);
		void removeAllChildren(QTreeWidgetItem* parent);

	public:
		ObjectManager(QTreeWidget* objectTree, const QString& folder);
		void mergeResourceManager(AbstractResourceManager* source);

		void addFolder(QTreeWidgetItem* parent, const QString& name);
		int getFolderCount(QTreeWidgetItem* parent);
		void populateDirectory(QTreeWidgetItem* parent, const QString& path);

		ObjectClass* addNewClass(QTreeWidgetItem* parent, const QString& className, const QString& fullPath);
		void loadAllExpanded(QTreeWidgetItem* parent = nullptr);
		ObjectClass* loadObject(const QString& className, bool threaded);
		void releaseObject(ObjectClass* object);
		QString getFolderFullPath(QTreeWidgetItem* folderItem, const QString& thing = "");
		const QString& getRootDirectory() const { return m_folder; }
		void changeFolder(const QString& folder);
		void deleteObjectClass(ObjectClass* object);

		AbstractResourceManager* getResourceManager() { return &m_resourceManager; }
	};
};
#endif