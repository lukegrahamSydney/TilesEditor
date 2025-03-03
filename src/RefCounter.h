#ifndef REFCOUNTERH
#define REFCOUNTERH

namespace TilesEditor
{
	class RefCounter
	{

	private:
		int m_refCount;

	protected:
		RefCounter() :
			m_refCount(0) {}

	public:
		virtual ~RefCounter() {}

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

		virtual bool decrementAndDelete() {
			if (decrementRef() == 0) {
				delete this;
				return true;
			}
			return false;
		}

	};
};

#endif
