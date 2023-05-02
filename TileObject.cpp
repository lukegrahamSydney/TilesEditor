#include "TileObject.h"

namespace TilesEditor
{
	TileObject::TileObject(int hcount, int vcount):
		Tilemap(nullptr, 0.0, 0.0, hcount, vcount, 0)
	{
	}

	cJSON* TileObject::serializeJSON()
	{

		auto jsonObject = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonObject, "type", "tileObject");
		cJSON_AddStringToObject(jsonObject, "name", getName().toLocal8Bit().data());
		cJSON_AddNumberToObject(jsonObject, "hcount", this->getHCount());
		cJSON_AddNumberToObject(jsonObject, "vcount", this->getVCount());

		auto tilesArray = cJSON_CreateArray();
		for (int y = 0; y < this->getVCount(); ++y)
		{
			QString line = "";
			for (int x = 0; x < this->getHCount(); ++x)
			{
				line += QString::number(this->getTile(x, y), 16) + " ";
			}
			QByteArray ba = line.toLocal8Bit();
			cJSON_AddItemToArray(tilesArray, cJSON_CreateString(ba.data()));

		}
		cJSON_AddItemToObject(jsonObject, "tiles", tilesArray);

		return jsonObject;
	}
};
