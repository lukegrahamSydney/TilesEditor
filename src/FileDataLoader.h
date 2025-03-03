#ifndef FILEDATALOADERH
#define FILEDATALOADERH

#include <QString>
#include <QByteArray>
#include "AbstractResourceManager.h"

namespace TilesEditor
{
	class FileDataLoader :
		public QObject
	{
		Q_OBJECT

	private:
		AbstractResourceManager* m_resourceManager;
		QString m_fileName;
		QByteArray m_fileData;
		bool m_searchFile = false;


	signals:
		void finished(QByteArray fileData);
		void failed();

	public slots:
		void start()
		{
			if (m_searchFile)
			{
				if (!m_resourceManager->locateFile(m_fileName, &m_fileName))
				{
					emit failed();
					return;
				}
			}

			auto stream = m_resourceManager->openStreamFullPath(m_fileName, QIODeviceBase::ReadOnly);
			if (stream)
			{
				m_fileData = stream->readAll();
				delete stream;

				emit finished(m_fileData);
				return;
			}
			emit failed();
		}



	public:
		FileDataLoader(AbstractResourceManager* resourceManager, const QString& fileName) :
			m_resourceManager(resourceManager), m_fileName(fileName) {
			m_resourceManager->incrementRef();
		}

		~FileDataLoader() {
			m_resourceManager->decrementAndDelete();
		}

		void enableFileSearch(bool search) { m_searchFile = search; }

		AbstractResourceManager* getResourceManager() { return m_resourceManager; }
	};
};

#endif