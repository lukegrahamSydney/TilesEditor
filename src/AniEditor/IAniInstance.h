#ifndef IANIINSTANCEH
#define IANIINSTANCEH
#include <QString>

namespace TilesEditor
{
	class IAniInstance
	{
	public:
		virtual QString getPropertyValue(const QString& propName) = 0;
	};
}
#endif

