#include "ObjectFactory.h"
#include "LevelNPC.h"
#include "LevelLink.h"
#include "LevelChest.h"
#include "LevelSign.h"
#include "LevelGraalBaddy.h"

namespace TilesEditor
{
	AbstractLevelEntity* ObjectFactory::createObject(IWorld* world, cJSON* jsonObj)
	{

        auto objectType = jsonGetChildString(jsonObj, "type");

        if (objectType == "levelNPCv1")
            return new LevelNPC(world, jsonObj);
        else if (objectType == "levelLink")
            return new LevelLink(world, jsonObj);

        else if (objectType == "levelChestv1")
            return new LevelChest(world, jsonObj);

        else if (objectType == "levelSign")
            return new LevelSign(world, jsonObj);

        else if (objectType == "levelBaddy")
            return new LevelGraalBaddy(world, jsonObj);

            

        
        return nullptr;
	}
};
