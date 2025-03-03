#ifndef CUSTOMLISTWIDGETITEMH
#define CUSTOMLISTWIDGETITEMH

#include <QListWidgetItem>
namespace TilesEditor
{
	template <class T>
	class CustomListWidgetItem:
		public QListWidgetItem
	{
	private:
		T* m_userPointer = nullptr;

	public:

		void setUserPointer(T* ptr) { m_userPointer = ptr; }
		T* getUserPointer() { return m_userPointer; }
	};
}
#endif