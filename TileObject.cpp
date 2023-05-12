#include "TileObject.h"

namespace TilesEditor
{
	TileObject::TileObject(int hcount, int vcount):
		Tilemap(nullptr, 0.0, 0.0, hcount, vcount, 0)
	{
	}

	cJSON* TileObject::serializeJSON()
	{
		static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		auto jsonObject = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonObject, "name", getName().toLocal8Bit().data());
		cJSON_AddNumberToObject(jsonObject, "hcount", this->getHCount());
		cJSON_AddNumberToObject(jsonObject, "vcount", this->getVCount());

		auto tilesArray = cJSON_CreateArray();
		for (int y = 0; y < this->getVCount(); ++y)
		{

			QString line = "";
			for (int x = 0; x < this->getHCount(); ++x)
			{
				auto tile = getTile(x, y);
				QString tileString = "";
				do {
					tileString = base64[tile & 0x3F] + tileString;
					tile = tile >> 6;
				} while (tile != 0);

				line += tileString + " ";
			}

			cJSON_AddItemToArray(tilesArray, cJSON_CreateString(line.toLocal8Bit().data()));

		}
		cJSON_AddItemToObject(jsonObject, "tiles", tilesArray);

		return jsonObject;
	}
};
