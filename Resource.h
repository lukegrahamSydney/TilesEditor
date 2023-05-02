#ifndef RESOURCEH
#define RESOURCEH

#include <cstdio>
#include <QString>
#include "ResourceType.h"

namespace TilesEditor
{
	class ResourceManager;
	class Resource
	{

	private:
		int m_refCount;

		QString m_name;
		QString m_fileName;

	protected:
		Resource(const QString& name, const QString& fileName = "") :
			m_refCount(0), m_name(name), m_fileName(fileName) {
		}

	public:

		virtual ~Resource() {}

		void incrementRef() {
			++m_refCount;
		}

		int decrementRef() {
			if (m_refCount > 0)
				return --m_refCount;
			return 0;
		}

		int getRefCount() const {
			return m_refCount;
		}

		void setRefCount(int value) {
			m_refCount = value;
		}

		bool decrementAndDelete() {
			if (decrementRef() == 0) {
				delete this;
				return true;
			}
			return false;
		}

		const QString& getName() {
			return m_name;
		}

		const QString& getFileName() const {
			return m_fileName;
		}

		virtual void replace(const QString& fileName) {}
		virtual void release(ResourceManager& resourceManager) {};
		virtual ResourceType getResourceType() const = 0;
	};
}
#endif

