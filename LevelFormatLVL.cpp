#include "LevelFormatLVL.h"
#include "Level.h"
#include "LevelLink.h"
#include "LevelSign.h"
#include "ObjectFactory.h"
#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
    bool LevelFormatLVL::loadLevel(Level* level, QIODevice* stream)
    {
        static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        QTextStream textStream(stream);

        QString text = textStream.readAll();
        if (!text.isEmpty())
        {
            auto jsonRoot = cJSON_Parse(text.toLocal8Bit().data());

            if (jsonRoot)
            {
                auto type = jsonGetChildString(jsonRoot, "type");
                auto version = jsonGetChildString(jsonRoot, "version");


                if (type == "level" && version == "1.0")
                {

                    auto hcount = jsonGetChildInt(jsonRoot, "hcount", 1);
                    auto vcount = jsonGetChildInt(jsonRoot, "vcount", 1);



                    level->setSize(hcount * 16, vcount * 16);
                    level->setTilesetName(jsonGetChildString(jsonRoot, "tileset", ""));
                    level->setTilesetImageName(jsonGetChildString(jsonRoot, "tilesetImage", ""));

                    level->setUnitWidth(16);
                    level->setUnitHeight(16);


                    auto jsonLayers = cJSON_GetObjectItem(jsonRoot, "tileLayers");
                    if (jsonLayers)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonLayers); ++i)
                        {
                            auto jsonLayer = cJSON_GetArrayItem(jsonLayers, i);

                            if (jsonLayer)
                            {
                                auto index = jsonGetChildInt(jsonLayer, "index");


                                auto jsonChunks = cJSON_GetObjectItem(jsonLayer, "chunks");
                                if (jsonChunks)
                                {
                                    auto tilemap = new Tilemap(level->getWorld(), level->getX(), level->getY(), hcount, vcount, index);
                                    tilemap->setLevel(level);
                                    tilemap->clear(Tilemap::MakeInvisibleTile(0));
                                    for (int ii = 0; ii < cJSON_GetArraySize(jsonChunks); ++ii)
                                    {
                                        auto jsonChunk = cJSON_GetArrayItem(jsonChunks, ii);

                                        if (jsonChunk)
                                        {
                                            auto left = jsonGetArrayInt(jsonChunk, 0);
                                            auto top = jsonGetArrayInt(jsonChunk, 1);

                                            auto line = jsonGetArrayString(jsonChunk, 2, "");

                                            auto parts = line.split(' ', Qt::SkipEmptyParts);
                                            for (auto x = 0U; x < parts.size(); ++x)
                                            {
                                                int tile = 0;
                                                auto& part = parts[x];
                                                int bitcount = 0;


                                                for (auto i = part.length() - 1; i >= 0; --i) {
                                                    auto value = base64.indexOf(part[i]);

                                                    tile |= value << bitcount;
                                                    bitcount += 6;
                                                }
                                                tilemap->setTile(left + x, top, tile);
                                            }

                                        }
                                    }

                                    level->setTileLayer(index, tilemap);
                                }
                            }
                        }
                    }

                    auto jsonSigns = cJSON_GetObjectItem(jsonRoot, "signs");
                    if (jsonSigns)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonSigns); ++i)
                        {
                            auto jsonSign = cJSON_GetArrayItem(jsonSigns, i);

                            if (jsonSign)
                            {
                                auto sign = ObjectFactory::createObject(level->getWorld(), "levelSign", jsonSign);
                                sign->setX(level->getX() + sign->getX());
                                sign->setY(level->getY() + sign->getY());
                                sign->setLevel(level);

                                /*
                                auto x = level->getX() + jsonGetChildInt(jsonSign, "x");
                                auto y = level->getY() + jsonGetChildInt(jsonSign, "y");

                                auto width = jsonGetChildInt(jsonSign, "width");
                                auto height = jsonGetChildInt(jsonSign, "height");


                                auto sign = new LevelSign(level->getWorld(), x, y, width, height);
                                sign->setLevel(level);

                                sign->setText(jsonGetChildString(jsonSign, "text"));
                                */

                                level->addObject(sign);

                            }
                        }
                    }

                    auto jsonLinks = cJSON_GetObjectItem(jsonRoot, "links");
                    if (jsonLinks)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonLinks); ++i)
                        {
                            auto jsonLink = cJSON_GetArrayItem(jsonLinks, i);

                            if (jsonLink)
                            {
                                auto link = ObjectFactory::createObject(level->getWorld(), "levelLink", jsonLink);
                                link->setX(level->getX() + link->getX());
                                link->setY(level->getY() + link->getY());
                                link->setLevel(level);

                                /*
                                auto x = level->getX() + jsonGetChildInt(jsonLink, "x");
                                auto y = level->getY() + jsonGetChildInt(jsonLink, "y");

                                auto width = jsonGetChildInt(jsonLink, "width");
                                auto height = jsonGetChildInt(jsonLink, "height");


                                auto link = new LevelLink(level->getWorld(), x, y, width, height, false);
                                link->setLevel(level);

                                link->setNextLevel(jsonGetChildString(jsonLink, "destination"));
                                link->setNextX(jsonGetChildString(jsonLink, "destinationX"));
                                link->setNextY(jsonGetChildString(jsonLink, "destinationY"));*/
                                level->addObject(link);

                            }
                        }
                    }

                    auto jsonObjects = cJSON_GetObjectItem(jsonRoot, "objects");
                    if (jsonObjects)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonObjects); ++i)
                        {
                            auto jsonObject = cJSON_GetArrayItem(jsonObjects, i);

                            auto objectType = jsonGetChildString(jsonObject, "type");
                            auto entity = ObjectFactory::createObject(level->getWorld(), objectType, jsonObject);

                            if (entity)
                            {
                                entity->setX(level->getX() + entity->getX());
                                entity->setY(level->getY() + entity->getY());
                                entity->setLevel(level);
                                level->addObject(entity);
                            }

                        }
                    }
                    cJSON_Delete(jsonRoot);
                    return true;
                }
                cJSON_Delete(jsonRoot);
            }
        }
        return false;
    }

    bool LevelFormatLVL::saveLevel(Level* level, QIODevice* stream)
    {
        auto getJSONChunks = [](Tilemap* tilemap) -> cJSON*
        {
            cJSON* retval = nullptr;
            for (int top = 0; top < tilemap->getVCount(); ++top)
            {
                //Start scanning from left until we hit a NON-invisible title
                for (int left = 0; left < tilemap->getHCount(); ++left)
                {
                    //A visible tile has been found, now lets find the right end of the chunk
                    if (!Tilemap::IsInvisibleTile(tilemap->getTile(left, top)))
                    {
                        //Continue scanning until we hit the end, or an invisible tile
                        int right = left;
                        for (; right < tilemap->getHCount(); ++right)
                        {
                            if (Tilemap::IsInvisibleTile(tilemap->getTile(right, top)))
                                break;
                        }

                        int width = right - left;
                        if (width > 0)
                        {


                            auto jsonChunk = cJSON_CreateArray();

                            cJSON_AddItemToArray(jsonChunk, cJSON_CreateNumber(left));
                            cJSON_AddItemToArray(jsonChunk, cJSON_CreateNumber(top));

                            QString tileData;
                            for (; left < right; ++left)
                            {
                                static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

                                QString tileString;
                                unsigned int tile = (unsigned)tilemap->getTile(left, top);

                                do {

                                    tileString = base64[tile & 0x3F] + tileString;
                                    tile = tile >> 6;
                                } while (tile != 0);

                                tileData += tileString + " ";

                            }

                            cJSON_AddItemToArray(jsonChunk, cJSON_CreateString(tileData.toLocal8Bit().data()));

                            if (retval == nullptr)
                                retval = cJSON_CreateArray();

                            cJSON_AddItemToArray(retval, jsonChunk);
                        }
                        left = right;
                    }
                }
            }

            return retval;

        };


        auto jsonRoot = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "type", "level");
        cJSON_AddStringToObject(jsonRoot, "version", "1.0");
        cJSON_AddNumberToObject(jsonRoot, "hcount", level->getWidth() / 16);
        cJSON_AddNumberToObject(jsonRoot, "vcount", level->getHeight() / 16);

        
        if (!level->getTilesetName().isEmpty()) {
            cJSON_AddStringToObject(jsonRoot, "tileset", level->getTilesetName().toLocal8Bit().data());
        }

        if (!level->getTilesetImageName().isEmpty()) {
            cJSON_AddStringToObject(jsonRoot, "tilesetImage", level->getTilesetImageName().toLocal8Bit().data());
        }


        if (level->getTileLayers().size() > 0)
        {
            auto jsonLayers = cJSON_CreateArray();

            for (auto layer : level->getTileLayers())
            {
                auto jsonChunks = getJSONChunks(layer);
                if (jsonChunks)
                {
                    auto jsonLayer = cJSON_CreateObject();
                    cJSON_AddNumberToObject(jsonLayer, "index", layer->getLayerIndex());
                    cJSON_AddItemToObject(jsonLayer, "chunks", jsonChunks);

                    cJSON_AddItemToArray(jsonLayers, jsonLayer);
                }
            }

            cJSON_AddItemToObject(jsonRoot, "tileLayers", jsonLayers);
        }

        if (level->getSigns().size() > 0)
        {
            auto jsonSigns = cJSON_CreateArray();
            for (auto sign : level->getSigns())
            {
                auto jsonSign = sign->serializeJSON(true);
                cJSON_DeleteItemFromObject(jsonSign, "type");
                cJSON_AddItemToArray(jsonSigns, jsonSign);
            }

            cJSON_AddItemToObject(jsonRoot, "signs", jsonSigns);
        }

        if (level->getLinks().size() > 0)
        {
            auto jsonLinks = cJSON_CreateArray();
            for (auto link : level->getLinks())
            {
                auto jsonLink = link->serializeJSON(true);
                cJSON_DeleteItemFromObject(jsonLink, "type");
                cJSON_AddItemToArray(jsonLinks, jsonLink);
            }

            cJSON_AddItemToObject(jsonRoot, "links", jsonLinks);
        }

        auto jsonObjects = cJSON_CreateArray();

        for (auto obj : level->getObjects())
        {
            switch (obj->getEntityType())
            {
            case LevelEntityType::ENTITY_LINK:
            case LevelEntityType::ENTITY_SIGN:
                break;

            default: {
                auto jsonObj = obj->serializeJSON(true);
                if (jsonObj)
                {
                    cJSON_AddItemReferenceToArray(jsonObjects, jsonObj);
                }
            }

            }

        }

        cJSON_AddItemToObject(jsonRoot, "objects", jsonObjects);

        auto levelText = cJSON_Print(jsonRoot);

        QTextStream textStream(stream);
        textStream << levelText;

        free(levelText);
        cJSON_Delete(jsonRoot);
        return true;
    }

    void LevelFormatLVL::applyFormat(Level* level)
    {
        level->setUnitWidth(16);
        level->setUnitHeight(16);
    }
};
