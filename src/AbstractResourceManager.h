#ifndef ABSTRACTRESOURCEMANAGERH
#define ABSTRACTRESOURCEMANAGERH

#include <QMap>
#include <QString>
#include <QSet>
#include <QIODevice>
#include <QPixmap>
#include "Resource.h"
#include "IFileRequester.h"
#include "RefCounter.h"

namespace TilesEditor
{
	class ObjectManager;
	class AbstractResourceManager :
		public RefCounter
	{
	private:
		ObjectManager*				m_objectManager;
		QMap<QString, Resource*>	m_resources;
		QSet<QString>				m_failedResources;

		QMap<IFileRequester*, QSet<QString>> m_listeners;
		QMap<QString, QSet<IFileRequester*>> m_fileRequests;

	private:
		void clearFileRequest(const QString& fileName);
		void clearFileListener(IFileRequester* listener);

	protected:
		//Filename should be name part only. not FULL PATH
		virtual void requestFile(const QString& fileName) {};


		//Call this to mark the requested file as failed
		void requestFileFailed(const QString& fileName);
		
	public:
		AbstractResourceManager(ObjectManager* objectManager) :
			m_objectManager(objectManager) {}

		virtual ~AbstractResourceManager();

		QString readAllToString(const QString& fileName);

		ObjectManager* getObjectManager() { return m_objectManager; }
		void setObjectManager(ObjectManager* objectManager) { m_objectManager = objectManager; }
		Resource* loadResource(IFileRequester* requester, const QString& name, ResourceType type);
		Resource* acquireExistingResource(const QString& name, ResourceType type);

		void freeResource(Resource* resource);
		QPixmap loadPixmap(const QString& name);

		void updateFile(const QString& name);
		void updateFile(const QString& resourceName, QIODevice* stream);

		void addFailedResource(const QString& name) { m_failedResources.insert(name.toLower()); }

		virtual void requestFile(IFileRequester* listener, const QString& fileName);
		void removeListener(IFileRequester* listener);


		//Override these
	

		virtual const QString& getType() const = 0;

		//These methods are used to check if a resource manager can be shared with another level/gani
		virtual QString getConnectionString() const = 0;
		virtual bool isCompatible(const QString& connectionString) = 0;

		//mergeResourceManager can be used to merge some properties from another resource manager
		//for example merging folders and file name cache
		virtual void mergeResourceManager(AbstractResourceManager* other) = 0;

		virtual QString getOpenFileName(const QString& caption, const QString& dir, const QString& filter) = 0;
		virtual QStringList getOpenFileNames(const QString& caption, const QString& dir, const QString& filter) = 0;
		virtual QString getSaveFileName(const QString& caption, const QString& dir, const QString& filter) = 0;

		//Open a stream with only a file name
		virtual QIODevice* openStream(const QString& fileName, QIODeviceBase::OpenModeFlag mode, QString* fullpath = nullptr) = 0;

		virtual bool locateFile(const QString& name, QString* outPath = nullptr) = 0;

		//Open a stream providing a full path to the file
		virtual QIODevice* openStreamFullPath(const QString& fileName, QIODeviceBase::OpenModeFlag mode) = 0;



		//this is called when a file has finished being written to. also delete the stream object in this function
		virtual void endWrite(IFileRequester* requester, const QString& fileName, QIODevice* stream) = 0;

	};
};
#endif