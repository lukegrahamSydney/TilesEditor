#include <memory>
#include <filesystem>
#include <QDir>
#include "ResourceManager.h"
#include "Image.h"
#include "Tileset.h"

namespace TilesEditor
{
	ResourceManager::ResourceManager(AbstractFileSystem* fileSystem)
	{
		m_fileSystem = fileSystem;
	}

	ResourceManager::~ResourceManager()
	{
		for (auto resource : m_resources)
		{
			delete resource;
		}
	}

	void ResourceManager::mergeSearchDirectories(const ResourceManager& source)
	{
		for (auto& searchDir : source.m_searchDirectoriesList)
		{
			if (!m_searchDirectories.contains(searchDir))
			{
				m_searchDirectories.insert(searchDir);
				m_searchDirectoriesList.push_back(searchDir);
			}
		}
	}

	void ResourceManager::addSearchDir(const QString& dir)
	{

		QDir absoluteDir(dir);
		auto searchPath = absoluteDir.absolutePath() + "/";
		if (!m_searchDirectories.contains(searchPath))
		{
			m_searchDirectories.insert(searchPath);
			m_searchDirectoriesList.push_back(searchPath);
		}
	}


	void ResourceManager::addSearchDirRecursive(const QString& dir)
	{
		auto folders = m_fileSystem->getFolders(dir);

		for(auto folder: folders)
		{
			qDebug() << "FOLDER: " << folder;
			if (!m_searchDirectories.contains(folder))
			{
				m_searchDirectories.insert(folder);
				m_searchDirectoriesList.push_back(folder);
			}
		}

	}

	bool ResourceManager::locateFile(const QString& name, QString* outPath)
	{
		if (name != "")
		{
			for (auto& dir : m_searchDirectoriesList)
			{
				QString fullPath = dir + name;

				if(m_fileSystem->fileExists(fullPath))
				{
					*outPath = fullPath;
					return true;
				}
			}
		}

		*outPath = "";
		return false;
	}

	bool ResourceManager::containsDirectory(const QString& dir)
	{
		return m_searchDirectories.find(dir) != m_searchDirectories.end();
	}


	void ResourceManager::addPersistantResource(Resource* resource)
	{
		resource->incrementRef();

		if(m_resources.find(resource->getName()) == m_resources.end())
			m_resources[resource->getName()] = resource;
	}

	bool ResourceManager::writeTextFile(const QString& fileName, const QString& contents)
	{
		/*
		auto fHandle = FileSystem::Instance()->OpenStream(m_rootDir + fileName, "w");
		if (fHandle != nullptr)
		{
			fHandle->Write(contents.c_str(), 1, contents.length());
			fHandle->Close();
			delete fHandle;
		}*/

		return false;
	}
	
	QString ResourceManager::readTextFile(const QString& fileName)
	{
		return "";
	}

	Resource* ResourceManager::loadResource(const QString& resourceName, ResourceType type)
	{
		auto it = m_resources.find(resourceName);
		if (it != m_resources.end())
		{
		
			auto resource = it.value();

			if (resource->getResourceType() == type)
			{
				resource->incrementRef();
				return resource;
			}
			return nullptr;
		}
		else if (m_failedResources.find(resourceName) == m_failedResources.end())
		{
			Resource* res = nullptr;


			//If they wernt loaded by the threaded method use the normal method
			QString fileName;

			if (locateFile(resourceName, &fileName))
			{
				auto stream = m_fileSystem->openStream(fileName, QIODevice::ReadOnly);

				if (stream != nullptr)
				{
					if (type == ResourceType::RESOURCE_IMAGE)
						res = Image::load(resourceName, stream);
				}

				delete stream;

				if (res == nullptr)
				{
					m_failedResources.insert(resourceName);
				}
			}

			if (res != nullptr)
			{
				res->setFileName(fileName);
				res->incrementRef();
				m_resources[resourceName] = res;
			}
			return res;
		}
		return nullptr;
	}



	void ResourceManager::freeResource(Resource* resource)
	{
		if (resource == nullptr)
			return;


		if (resource->decrementRef() == 0)
		{
			auto it = m_resources.find(resource->getName());

			if (it != m_resources.end())
			{
				qDebug() << "Delete resource: " << resource->getName();
				delete resource;
				m_resources.erase(it);
			}
		}
	}

	void ResourceManager::updateResource(const QString& name)
	{
		auto it = m_resources.find(name);
		if (it != m_resources.end())
		{
			auto resource = it.value();

			auto fileName = resource->getFileName();

			auto stream = m_fileSystem->openStream(fileName, QIODevice::ReadOnly);

			if (stream) {
				resource->replace(stream);
				delete stream;
			}
		}
	}



};
