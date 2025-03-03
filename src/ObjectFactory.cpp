#include "ObjectFactory.h"
#include "LevelNPC.h"
#include "LevelLink.h"
#include "LevelChest.h"
#include "LevelSign.h"
#include "LevelGraalBaddy.h"
#include "LevelObjectInstance.h"

namespace TilesEditor
{
	AbstractLevelEntity* ObjectFactory::createObject(IWorld* world, const QString& type, cJSON* jsonObj)
	{

        //auto objectType = jsonGetChildString(jsonObj, "type");

        if (type == "levelNPCv1")
            return new LevelNPC(world, jsonObj);
        else if (type == "levelNPCv2")
            return new LevelObjectInstance(world, jsonObj);

        else if (type == "levelLink")
            return new LevelLink(world, jsonObj);

        else if (type == "levelChestv1")
            return new LevelChest(world, jsonObj);

        else if (type == "levelSign")
            return new LevelSign(world, jsonObj);

        else if (type == "levelBaddy")
            return new LevelGraalBaddy(world, jsonObj);
        return nullptr;
	}

    AbstractLevelEntity* ObjectFactory::createObject(IWorld* world, sgs_Context* ctx)
    {
        return nullptr;
    }

};
