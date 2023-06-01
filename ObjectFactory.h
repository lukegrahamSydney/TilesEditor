#ifndef OBJECTFACTORYH
#define OBJECTFACTORYH

#include "IWorld.h"
#include "AbstractLevelEntity.h"
#include "cJSON/JsonHelper.h"
namespace TilesEditor
{

	class ObjectFactory
	{
	public:
		static AbstractLevelEntity* createObject(IWorld* world, const QString& type, cJSON* json);

	};
};
#endif
