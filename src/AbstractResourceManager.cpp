#include <QTextStream>
#include "AbstractResourceManager.h"
#include "Image.h"
#include "AniEditor/Ani.h"

namespace TilesEditor
{

	AbstractResourceManager::~AbstractResourceManager()
	{
		for (auto resource : m_resources)
		{
			resource->decrementAndDelete();
		}
	}

	QString AbstractResourceManager::readAllToString(const QString& fileName)
	{
		auto stream = openStreamFullPath(fileName, QIODevice::ReadOnly);
		if (stream != nullptr)
		{
			QTextStream textStream(stream);
			QString retval = textStream.readAll();
			delete stream;

			return retval;
		}
		return QString();
	
	}

    Resource* AbstractResourceManager::loadResource(IFileRequester* requester, const QString& resourceName, ResourceType type)
    {
		auto resourceNameLower = resourceName.toLower();
		auto it = m_resources.find(resourceNameLower);
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
		else if (m_failedResources.find(resourceNameLower) == m_failedResources.end())
		{
			Resource* res = nullptr;

			QString fullPath;
			auto stream = openStream(resourceNameLower, QIODevice::ReadOnly, &fullPath);

			if (stream)
			{
				if (type == ResourceType::RESOURCE_IMAGE)
					res = Image::load(resourceNameLower, stream);

				else if (type == ResourceType::RESOURCE_ANI)
					res = Ani::loadGraalAni(resourceNameLower, stream, this);

				delete stream;

				if (res == nullptr)
				{
					m_failedResources.insert(resourceNameLower);
				}
				else
				{
					res->setFileName(fullPath);
					res->incrementRef();
					m_resources[resourceNameLower] = res;
				}
			}
			else if (requester)
			{
				requestFile(requester, resourceNameLower);

				//Return a blank
				if (type == ResourceType::RESOURCE_IMAGE)
					res = new Image(resourceName);
				else if (type == ResourceType::RESOURCE_ANI)
					res = new Ani(resourceName, this);

				if (res)
				{
					res->setFileName(resourceName);
					res->incrementRef();
					m_resources[resourceNameLower] = res;
				}
				return res;
			}


			return res;
		}
		return nullptr;
    }

	Resource* AbstractResourceManager::acquireExistingResource(const QString& name, ResourceType type)
	{
		auto resourceNameLower = name.toLower();
		auto it = m_resources.find(resourceNameLower);
		if (it != m_resources.end())
		{

			auto resource = it.value();

			if (resource->getResourceType() == type)
			{
				resource->incrementRef();
				return resource;
			}
		}
		return nullptr;
	}

	void AbstractResourceManager::freeResource(Resource* resource)
	{
		if (resource == nullptr)
			return;

		if (resource->decrementRef() == 0)
		{
			auto it = m_resources.find(resource->getName().toLower());
			if (it != m_resources.end())
				m_resources.erase(it);

			delete resource;
		}
	}

	QPixmap AbstractResourceManager::loadPixmap(const QString& name)
	{
		auto resourceNameLower = name.toLower();
		auto it = m_resources.find(resourceNameLower);
		if (it != m_resources.end())
		{
			auto resource = it.value();

			if (resource->getResourceType() == ResourceType::RESOURCE_IMAGE)
			{
				return static_cast<Image*>(resource)->pixmap();
			}
		}

		auto stream = openStream(resourceNameLower, QIODevice::ReadOnly, nullptr);
	
		if (stream)
		{
			QPixmap pixmap;

			if (pixmap.loadFromData(stream->readAll()))
			{
				delete stream;
				return pixmap;
			}
			delete stream;
		}

		return QPixmap();
	}

	void AbstractResourceManager::updateFile(const QString& name)
	{
		auto resourceNameLower = name.toLower();
		auto it = m_resources.find(resourceNameLower);
		if (it != m_resources.end())
		{
			auto resource = it.value();

			auto fileName = resource->getFileName();

			auto stream = openStreamFullPath(fileName, QIODevice::ReadOnly);

			if (stream) {
				resource->replace(stream, this);
				resource->setLoaded(true);
				delete stream;
			}
		}

		auto it2 = m_fileRequests.find(resourceNameLower);
		if (it2 != m_fileRequests.end())
		{
			auto listeners = it2.value();
			for (auto listener : listeners)
				listener->fileReady(resourceNameLower, this);
			m_fileRequests.erase(it2);
		}
	}

	void AbstractResourceManager::updateFile(const QString& resourceName, QIODevice* stream)
	{
		//Check if this file is a loaded resource. If it is, replace it
		auto resourceNameLower = resourceName.toLower();
		auto it = m_resources.find(resourceNameLower);
		if (it != m_resources.end())
		{
			auto resource = it.value();
			resource->replace(stream, this);
			resource->setLoaded(true);
		}

		//Alert all our listeners that this file has been received
		auto it2 = m_fileRequests.find(resourceNameLower);
		if (it2 != m_fileRequests.end())
		{
			auto listeners = it2.value();
			for (auto listener : listeners)
				listener->fileReady(resourceNameLower, this);
			m_fileRequests.erase(it2);
		}
	}

	void AbstractResourceManager::requestFile(IFileRequester* listener, const QString& fileName)
	{
		auto& thing = m_listeners[listener];
		thing.insert(fileName);

		auto& thing2 = m_fileRequests[fileName];
		thing2.insert(listener);


		requestFile(fileName);
	}

	void AbstractResourceManager::removeListener(IFileRequester* listener)
	{
		clearFileListener(listener);
	}

	void AbstractResourceManager::clearFileRequest(const QString& fileName)
	{
		auto it = m_fileRequests.find(fileName);
		if (it != m_fileRequests.end())
		{
			for (auto& listener : it.value())
			{
				auto it2 = m_listeners.find(listener);
				if (it2 != m_listeners.end())
				{
					it2.value().remove(fileName);
				}
			}

			m_fileRequests.erase(it);
		}
	}

	void AbstractResourceManager::clearFileListener(IFileRequester* listener)
	{
		auto it = m_listeners.find(listener);

		if (it != m_listeners.end())
		{
			auto& files = it.value();
			for (auto& fileName : files)
			{
				auto it2 = m_fileRequests.find(fileName);
				if (it2 != m_fileRequests.end())
					it2.value().remove(listener);
			}

			m_listeners.erase(it);
		}
	}

	void AbstractResourceManager::requestFileFailed(const QString& fileName)
	{
		m_failedResources.insert(fileName);
		auto it = m_fileRequests.find(fileName);
		if (it != m_fileRequests.end())
		{
			auto listeners = it.value();
			m_fileRequests.erase(it);

			for (auto& listener : listeners)
			{
				auto it2 = m_listeners.find(listener);
				if (it2 != m_listeners.end())
				{
					it2.value().remove(fileName);
				}
				listener->fileFailed(fileName, this);
			}
		}
	}
}

