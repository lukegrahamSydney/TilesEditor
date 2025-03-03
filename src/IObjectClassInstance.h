#ifndef IOBJECTCLASSINSTANCEH
#define IOBJECTCLASSINSTANCEH

namespace TilesEditor
{
	class ObjectClass;
	class IObjectClassInstance
	{
	public:
		virtual void unsetObjectClass() = 0;

		virtual void markObjectChanged() = 0;
	};
};
#endif
