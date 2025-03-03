#include <QDir>
#include "ObjectManager.h"
#include "AbstractResourceManager.h"

namespace TilesEditor
{

	ObjectManager::ObjectManager(QTreeWidget* objectTree, const QString& folder):
		m_resourceManager("./", this), m_objectTree(objectTree), m_folder(folder)
	{
		if (!m_folder.endsWith('/'))
			m_folder += '/';

	}

	void ObjectManager::mergeResourceManager(AbstractResourceManager* source)
	{
		m_resourceManager.mergeResourceManager(source);
	}

	void ObjectManager::addFolder(QTreeWidgetItem* parent, const QString& name)
	{
		if (parent == nullptr)
			parent = m_objectTree->invisibleRootItem();

		int folderCount = getFolderCount(parent);
		auto item = new QTreeWidgetItem(0);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setIcon(0, QIcon(":/MainWindow/icons/fugue/folder-open.png"));
		item->setText(0, name);
		parent->insertChild(folderCount, item);
	}

	int ObjectManager::getFolderCount(QTreeWidgetItem* parent)
	{
		int folderCount = 0;
		for (int i = 0; i < parent->childCount(); ++i)
		{
			if (parent->child(i)->type() == 0)
				++folderCount;
		}
		return folderCount;
	}

	ObjectClass* ObjectManager::addNewClass(QTreeWidgetItem* parent, const QString& className, const QString& fullPath)
	{
		auto objectClass = new ObjectClass(className, fullPath);
		m_objects[className.toLower()] = objectClass;

		objectClass->load(&m_resourceManager, false);
		//objectClass->loadImage(resourceManager);
		parent->addChild(objectClass);

		objectClass->incrementRef();

		return objectClass;
	}

	void ObjectManager::loadAllExpanded(QTreeWidgetItem* parent)
	{
		if (parent == nullptr)
			parent = m_objectTree->invisibleRootItem();

		//This folder has not yet performed its search
		if (parent->type() == 0 && parent->data(0, Qt::UserRole).toInt() == 0)
		{
			
			populateDirectory(parent, getFolderFullPath(parent));

		}

		for (int i = 0; i < parent->childCount(); ++i)
		{
			auto child = parent->child(i);
			if (child->type() == 0)
			{
				if (child->isExpanded())
				{
					loadAllExpanded(child);
				}
			}
			else if(child->type() == 1)
			{
				auto objectClass = static_cast<ObjectClass*>(child);
				if (objectClass->getLoadState() == LoadState::STATE_NOT_LOADED)
					objectClass->load(&m_resourceManager, true);

				if (objectClass->getLoadState() == LoadState::STATE_LOADED)
				{
					if (!objectClass->imageLoaded())
					{
						objectClass->loadImage(&m_resourceManager);
					}
				}


			}
		
		}

	}

	void ObjectManager::populateDirectory(QTreeWidgetItem* parent, const QString& path)
	{
		QDir dir(path);
		dir.setFilter(QDir::Filter::NoDotAndDotDot | QDir::Filter::AllEntries);
		
		int folderCount = 0;
		auto entries = dir.entryInfoList();

		//2 = directory and all sub directories have been loaded
		int newLoadValue = 2;
		//1 = partial loaded. sub folders have not been loaded
		parent->setData(0, Qt::UserRole, 1);
		for (auto& entry : entries)
		{
			if (entry.isDir())
			{
				auto item = new QTreeWidgetItem(0);
				item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
				item->setIcon(0, QIcon(":/MainWindow/icons/fugue/folder-open.png"));
				item->setText(0, entry.fileName());
				parent->insertChild(folderCount++, item);
				newLoadValue = 1;
			}
			else if (entry.isFile())
			{
				auto fileName = entry.fileName();
				if (fileName.endsWith(".npc"))
				{
				
					auto className = (fileName.left(fileName.length() - 4));
					auto classNameLower = className.toLower();

					if (m_objects.find(classNameLower) == m_objects.end())
					{
						auto objectClass = new ObjectClass(className, entry.absoluteFilePath());
						objectClass->incrementRef();
						m_objects[classNameLower] = objectClass;
						parent->addChild(objectClass);
					}
				}
			}
		}

		parent->setData(0, Qt::UserRole, newLoadValue);
	}

	ObjectClass* ObjectManager::loadNewObject(QTreeWidgetItem* parent, const QString& path, const QString& className)
	{
		//Find if this object exists
		auto it = m_objects.find(className);

		if (it != m_objects.end())
		{
			auto objectClass = it.value();

			if (objectClass->getLoadState() == LoadState::STATE_NOT_LOADED)
				objectClass->load(&m_resourceManager, false);

			if (!objectClass->imageLoaded())
				objectClass->loadImage(&m_resourceManager);

			objectClass->incrementRef();
			return objectClass;

		}

		//There is nothing else to load. exit
		if (parent->data(0, Qt::UserRole).toInt() == 2)
			return nullptr;


		//Loop through all the unopened folders, populate them, then check if loaded again

		for (int i = 0; i < parent->childCount(); ++i)
		{
			auto child = parent->child(i);
			if (child->type() == 0)
			{
				auto loadValue = child->data(0, Qt::UserRole).toInt();
				if (loadValue == 2)
				{
					//This directory is already fully loaded. there is nothing else to search. skip it
					continue;
				}

				//No listings have been loaded into this directory. load them
				if (loadValue == 0)
				{
					populateDirectory(child, path + child->text(0) + "/");
					loadValue = child->data(0, Qt::UserRole).toInt();
				}
				

				auto object = loadNewObject(child, path + child->text(0) + "/", className);

				if (object)
					return object;
			}
			else {
				//We have just looped through all the sub folders and loaded them. set to 2,
				parent->setData(0, Qt::UserRole, 2);
				break;
			}

		}
		
		return nullptr;
	}

	
	ObjectClass* ObjectManager::loadObject(const QString& className, bool threaded)
	{


		return loadNewObject(m_objectTree->invisibleRootItem(), m_folder, className.toLower());

	}

	void ObjectManager::releaseObject(ObjectClass* object)
	{
		if (object->decrementRef() == 0)
		{
			if (!object->removed())
			{
				object->treeWidget()->removeItemWidget(object, 0);

				auto it = m_objects.find(object->getName().toLower());
				if (it != m_objects.end())
					m_objects.erase(it);
			}

			delete object;
		}
	}

	QString ObjectManager::getFolderFullPath(QTreeWidgetItem* folderWidget, const QString& thing)
	{
		if (folderWidget->type() == 0)
		{
			if (folderWidget->parent() != nullptr)
			{
				return getFolderFullPath(folderWidget->parent(), folderWidget->text(0) + "/" + thing);
			}
			else return m_folder + folderWidget->text(0) + "/" + thing;
		}
		return QString();
	}


	void ObjectManager::removeAllChildren(QTreeWidgetItem* parent)
	{

		while(parent->childCount())
		{
			auto child = parent->takeChild(0);

			if (child->type() == 0)
			{
				removeAllChildren(child);
				delete child;
			}
			else if (child->type() == 1) {
				auto object = static_cast<ObjectClass*>(child);

				//Set removed to true to mark it as no longer being in our object library (other objects can still be holding
				//this class even if it's no longer in our library)
				object->setRemoved(true);

				auto it = m_objects.find(object->getName().toLower());
				if (it != m_objects.end())
					m_objects.erase(it);

				//Decrement our reference count and delete the object if count to 0
				object->decrementAndDelete();
			}
		}
	}


	void ObjectManager::changeFolder(const QString& folder)
	{
		removeAllChildren(m_objectTree->invisibleRootItem());
		m_objects.clear();

		m_folder = folder;
		if (!m_folder.endsWith('/'))
			m_folder += '/';

		auto parent = m_objectTree->invisibleRootItem();
		populateDirectory(parent, m_folder);

	}

	void ObjectManager::deleteObjectClass(ObjectClass* object)
	{
		//Set removed to true to mark it as no longer being in our object library (other objects can still be holding
		//this class even if it's no longer in our library)
		object->setRemoved(true);

		QFile::remove(object->getFullPath());


		auto it = m_objects.find(object->getName().toLower());
		if (it != m_objects.end())
			m_objects.erase(it);

		if (static_cast<QTreeWidgetItem*>(object)->parent())
			static_cast<QTreeWidgetItem*>(object)->parent()->removeChild(object);
		else m_objectTree->invisibleRootItem()->removeChild(object);

		//Decrement our reference count and delete the object if count to 0
		object->decrementAndDelete();
	}
};
