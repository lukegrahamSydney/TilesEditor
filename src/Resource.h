#ifndef RESOURCEH
#define RESOURCEH

#include <cstdio>
#include <QString>
#include <QIODevice>
#include <QFile>
#include "ResourceType.h"
#include "RefCounter.h"

namespace TilesEditor
{
	class AbstractResourceManager;
	class Resource:
		public RefCounter
	{

	private:
		QString m_name;
		QString m_fileName;
		bool m_loaded = false;

	protected:
		Resource(const QString& name, const QString& fileName = "") :
			m_name(name), m_fileName(fileName) {
		}

	public:

		virtual ~Resource() {}


		const QString& getName() {
			return m_name;
		}

		const QString& getFileName() const {
			return m_fileName;
		}

		void setLoaded(bool value) { m_loaded = value; }
		bool isLoaded() const { return m_loaded; }
		void setFileName(const QString& name) { m_fileName = name; }


		virtual void replace(QIODevice* stream, AbstractResourceManager* resourceManager) {}
		
		virtual void release(AbstractResourceManager* resourceManager) {};
		virtual ResourceType getResourceType() const = 0;
	};
}
#endif

