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
		void IncrementRef() {
			++m_refCount;
		}

		int DecrementRef() {
			if (m_refCount > 0)
				return --m_refCount;
			return 0;
		}

		int GetRefCount() const {
			return m_refCount;
		}

		void SetRefCount(int value) {
			m_refCount = value;
		}

		bool DecrementAndDelete() {
			if (DecrementRef() == 0) {
				delete this;
				return true;
			}
			return false;
		}

	};
};

#endif
