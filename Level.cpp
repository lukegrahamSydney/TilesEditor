#include <QStringList>
#include <QTextStream>
#include <QStringBuilder>
#include <QFile>
#include <QDebug>
#include "Level.h"
#include "ImageDimensions.h"
#include "Tileset.h"
#include "Overworld.h"
#include "LevelNPC.h"
#include "LevelLink.h"
#include "LevelSign.h"
#include "LevelChest.h"
#include "LevelGraalBaddy.h"

#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
    QMap<QString, int> Level::m_imageDimensionsCache;
    bool Level::getImageDimensions(ResourceManager& resourceManager, const QString& imageName, int* w, int* h)
    {
        auto it = m_imageDimensionsCache.find(imageName);

        if (it != m_imageDimensionsCache.end()) {
            int size = it.value();
            *w = size >> 16;
            *h = size & 0xFFFF;
            return true;
        }
        else {
            QString fileName;

            if (resourceManager.locateFile(imageName, &fileName))
            {
                if (ImageDimensions::getImageFileDimensions(fileName, w, h))
                {
                    m_imageDimensionsCache[fileName] = (*w << 16) | *h;
                    return true;
                }
                else m_imageDimensionsCache[fileName] = 0;
            }

        }
        return false;
    }

    Level::Level(double x, double y, int width, int height, Overworld* overworld, const QString& name)
    {
        m_loadFail = false;
        m_modified = false;
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        m_unitWidth = 16;
        m_unitHeight = 16;
        m_overworld = overworld;
        m_name = name;

        m_loaded = false;
        m_entitySpatialMap = nullptr;
        m_mainTileLayer = nullptr;
        if (!m_overworld)
        {
            m_entitySpatialMap = new EntitySpatialGrid<AbstractLevelEntity>(getX(), getY(), getWidth(), getHeight());

        }

    }

    Level::~Level()
    {
        for (auto entity : m_objects)
        {
            delete entity;
        }

        if (m_entitySpatialMap)
            delete m_entitySpatialMap;


        for (auto tilemap : m_tileLayers)
            delete tilemap;
    }

    void Level::release(ResourceManager& resourceManager)
    {
        for (auto entity : m_objects)
        {
            entity->releaseResources(resourceManager);
        }

    }

    bool Level::loadFile(ResourceManager& resourceManager)
    {
        if (m_fileName.endsWith(".nw"))
            m_loadFail = !loadNWFile(resourceManager);

        else if (m_fileName.endsWith(".lvl"))
            m_loadFail = !loadLVLFile(resourceManager);
        return !m_loadFail;
    }

    bool Level::loadNWFile(ResourceManager& resourceManager)
    {
        
        QStringList lines;
        
        if (resourceManager.getFileSystem().readAllLines(m_fileName, lines) && lines.size() > 0)
        {
            auto& version = lines[0];

            static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            static const QString validVersions[] = {
                "GLEVNW01"
            };

            bool valid = false;
            for (auto i = 0U; i < sizeof(validVersions) / sizeof(validVersions[0]); ++i)
            {
                if (validVersions[i] == version) {
                    valid = true;
                    break;
                }
            }

            if (valid)
            {
                m_width = 64 * 16;
                m_height = 64 * 16;



                QVector<Tilemap*> tilemaps;

                auto lineCount = lines.size();
                for (size_t i = 1; i < lineCount; ++i)
                {
                    auto wordCount = 0U;
                    QStringList words = lines[i].split(' ');

                    wordCount = words.size();
                    if (wordCount > 0)
                    {
                        if (words[0] == "BOARD" && wordCount >= 6)
                        {
                            unsigned int x = words[1].toInt();
                            unsigned int y = words[2].toInt();
                            unsigned int width = words[3].toInt();
                            unsigned int layer = words[4].toInt();

                            auto tilemap = getOrMakeTilemap(layer, resourceManager);
                            auto& tileData = words[5];

                            if (x < 64 && y < 64 && x + width <= 64)
                            {
                                if (tileData.length() >= width * 2)
                                {
                                    for (size_t ii = 0u; ii < width; ++ii)
                                    {
                                        char byte1 = tileData[ii * 2].unicode();
                                        char byte2 = tileData[1 + ii * 2].unicode();

                                        auto graalTile = (int)((base64.indexOf(byte1) << 6) + base64.indexOf(byte2));
                                        auto tile = convertFromGraalTile( graalTile);
                                        tilemap->setTile(x + ii, y, tile);

                                    }
                                }
                            }
                        }
                        else if (words[0] == "TILESET" && wordCount >= 2)
                        {
                            m_tilesetName = words[1];
                        }
                        else if (words[0] == "LINK" && wordCount >= 8)
                        {
                            auto x = words[2].toDouble() * 16;
                            auto y = words[3].toDouble() * 16;
                            auto width = words[4].toInt() * 16;
                            auto height = words[5].toInt() * 16;

                            auto& nextX = words[6];
                            auto& nextY = words[7];

                            bool possibleEdgeLink = false;
                            if ((x == 0 || x == 63 * 16) && width == 16)
                                possibleEdgeLink = true;
                            else if ((y == 0 || y == 63 * 16) && height == 16)
                                possibleEdgeLink = true;

                            auto levelLink = new LevelLink(this, getX() + x, getY() + y, width, height, possibleEdgeLink);
                            levelLink->setNextLevel(words[1]);
                            levelLink->setNextX(nextX);
                            levelLink->setNextY(nextY);

                            addObject(levelLink);
                        }
                        else if (words[0] == "CHEST" && wordCount >= 5)
                        {
                            auto& itemName = words[3];

                            auto x = words[1].toDouble() * 16;
                            auto y = words[2].toDouble() * 16;

                            auto signIndex = words[4].toInt();

                            addObject(new LevelChest(this, x + getX(), y + getY(), itemName, signIndex));

                        }
                        else if (words[0] == "BADDY" && wordCount >= 3)
                        {
                            auto x = words[1].toDouble() * 16;
                            auto y = words[2].toDouble() * 16;
                            auto type = words[3].toInt();

                            auto baddy = new LevelGraalBaddy(this, x + getX(), y + getY(), type);
                            int verseIndex = 0;
                            for (++i; i < lineCount && lines[i] != "BADDYEND"; ++i)
                            {
                                baddy->setBaddyVerse(verseIndex++, lines[i].trimmed());
                            }

                            addObject(baddy);

                        }
                        else if (words[0] == "SIGN" && wordCount >= 3)
                        {
                            
                            auto sign = new LevelSign(this,
                                getX() + words[1].toDouble() * 16,
                                getY() + words[2].toDouble() * 16,
                                32,
                                16);

                            QString text = "";
                            for (++i; i < lineCount && lines[i] != "SIGNEND"; ++i)
                                text += lines[i] + '\n';
                            sign->setText(text);


                            addObject(sign);
                            //m_otherEntities->add(sign);

                        }
                        else if (words[0] == "NPC" && wordCount >= 4)
                        {

                            QString image = (words[1] == "-" ? "" : words[1]),
                                code;

                            double x = words[2].toDouble() * 16.0,
                                y = words[3].toDouble() * 16.0;

                            int width = 48, height = 48;

                            for (++i; i < lineCount && lines[i] != "NPCEND"; ++i)
                                code += lines[i] + "\n";
                            
                            QString className = "";

                            auto levelNPC = new LevelNPC(this, x + getX(), y + getY(), width, height);
                            levelNPC->setImageName(image, resourceManager);
                            levelNPC->setCode(code);
                            addObject(levelNPC);
                        }
                    }
                }
                m_loaded = true;
                return true;
            }
            
            return false;
        }
        return false;

    }

    bool Level::loadLVLFile(ResourceManager& resourceManager)
    {
        static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        QString text = resourceManager.getFileSystem().readAllToString(m_fileName);
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

                    m_width = hcount * 16;
                    m_height = vcount * 16;
                    m_tilesetName = jsonGetChildString(jsonRoot, "tileset", "");

                    m_unitWidth = 1;
                    m_unitHeight = 1;
                    if (m_entitySpatialMap) {
                        delete m_entitySpatialMap;
                        m_entitySpatialMap = new EntitySpatialGrid<AbstractLevelEntity>(getX(), getY(), getWidth(), getHeight());
                    }


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
                                    auto tilemap = new Tilemap(this, getX(), getY(), hcount, vcount, index);
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

                                    setTileLayer(index, tilemap);
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

                                auto x = this->getX() + jsonGetChildInt(jsonSign, "x");
                                auto y = this->getY() + jsonGetChildInt(jsonSign, "y");

                                auto width = jsonGetChildInt(jsonSign, "width");
                                auto height = jsonGetChildInt(jsonSign, "height");


                                auto sign = new LevelSign(this, x, y, width, height);
                                sign->setText(jsonGetChildString(jsonSign, "text"));

                                addObject(sign);

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

                                auto x = this->getX() + jsonGetChildInt(jsonLink, "x");
                                auto y = this->getY() + jsonGetChildInt(jsonLink, "y");

                                auto width = jsonGetChildInt(jsonLink, "width");
                                auto height = jsonGetChildInt(jsonLink, "height");


                                auto link = new LevelLink(this, x, y, width, height, false);
                                link->setNextLevel(jsonGetChildString(jsonLink, "destination"));
                                link->setNextX(jsonGetChildString(jsonLink, "destinationX"));
                                link->setNextY(jsonGetChildString(jsonLink, "destinationY"));
                                addObject(link);

                            }
                        }
                    }

                    auto jsonObjects = cJSON_GetObjectItem(jsonRoot, "objects");
                    if (jsonObjects)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonObjects); ++i)
                        {
                            auto jsonObject = cJSON_GetArrayItem(jsonObjects, i);


                            auto x = this->getX() + jsonGetChildInt(jsonObject, "x");
                            auto y = this->getY() + jsonGetChildInt(jsonObject, "y");
                            auto image = jsonGetChildString(jsonObject, "image");

                            int width, height;
                            if (image != "")
                                getImageDimensions(resourceManager, image, &width, &height);
                            else {
                                width = height = 48;
                            }


                            auto npc = new LevelNPC(this, x, y, width, height);
                            npc->setImageName(image, resourceManager);

                            npc->setCode(jsonGetChildString(jsonObject, "code"));
                            addObject(npc);

                        }
                    }
                    cJSON_Delete(jsonRoot);
                    m_loaded = true;
                    return true;
                }
                cJSON_Delete(jsonRoot);
            }


        }
        return false;
    }

    bool Level::saveFile()
    {
        if (m_fileName.endsWith(".nw"))
            return saveNWFile();

        if (m_fileName.endsWith(".lvl"))
            return saveLVLFile();

        return false;
    }

    bool Level::saveNWFile()
    {
        QFile file(m_fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            stream << "GLEVNW01" << Qt::endl;

            auto writeTileLayer = [](QTextStream& stream, Tilemap* tilemap)
            { 
                for (int top = 0; top < 64; ++top)
                {
                    for(int left = 0; left < 64; ++left)
                    {
                        //Start scanning from left until we hit a NON-invisible title
                        if (!Tilemap::IsInvisibleTile(tilemap->getTile(left, top)))
                        {
                            //Continue scanning until we hit the end, or an invisible tile
                            int right = left;
                            for (; right < 64; ++right)
                            {
                                if (Tilemap::IsInvisibleTile(tilemap->getTile(right, top)))
                                    break;
                            }

                            int width = right - left;
                            if (width > 0)
                            {
                                stream << "BOARD " << left << " " << top << " " << width << " " << (int)tilemap->getDepth() << " ";
                                for (;left < right; ++left)
                                {
                                    static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

                                    int tile = tilemap->getTile(left, top);

                                    auto tileLeft = Tilemap::GetTileX(tile);
                                    auto tileTop = Tilemap::GetTileY(tile);

                                    //Convert out normal left-top co-ordinates into graals tile index format
                                    //This line was taken from gonstruct source
                                    auto graalTile = tileLeft / 16 * 512 + tileLeft % 16 + tileTop * 16;

                                    stream << base64[graalTile >> 6];
                                    stream << base64[graalTile & 0x3F];
                                }
                                stream << Qt::endl;
                            }
                            left = right;
                        }
                    }
                }
            };

            for (auto layer : m_tileLayers)
            {
                writeTileLayer(stream, layer);
                
            }
            stream << Qt::endl;

            for (auto link : m_links)
            {
                stream << "LINK " << link->getNextLevel() << " " << int((link->getX() - getX()) / 16.0) << " " << int((link->getY() - getY()) / 16.0) << " " << link->getWidth() / 16 << " " << link->getHeight() / 16 << " " << link->getNextX() << " " << link->getNextY() << Qt::endl;
            }

            for (auto sign : m_signs)
            {
                stream << "SIGN " << int((sign->getX() - getX()) / 16.0) << " " << int((sign->getY() - getY()) / 16.0) << Qt::endl;
                stream << sign->getText();

                if (!sign->getText().endsWith('\n'))
                    stream << Qt::endl;

                stream << "SIGNEND" << Qt::endl << Qt::endl;

            }

            for (auto obj : m_objects)
            {
                switch (obj->getEntityType())
                {
                    case LevelEntityType::ENTITY_NPC:
                    {
                        auto npc = static_cast<LevelNPC*>(obj);

                        auto imageName = npc->getImageName();
                        if (imageName.isEmpty())
                            imageName = "-";

                        stream << "NPC " << imageName << " " << std::floor(((npc->getX() - getX()) / 16.0) * 1000.0) / 1000.0 << " " << std::floor(((npc->getY() - getY()) / 16.0) * 1000.0) / 1000.0 << Qt::endl;

                        stream << npc->getCode();

                        if (!npc->getCode().endsWith('\n'))
                            stream << Qt::endl;
                        stream << "NPCEND" << Qt::endl << Qt::endl;
                    }
                    break;

                    case LevelEntityType::ENTITY_CHEST:
                    {
                        auto chest = static_cast<LevelChest*>(obj);

                        stream << "CHEST " << int((chest->getX() - getX()) / 16.0) << " " << int((chest->getY() - getY()) / 16.0) << " " << chest->getItemName() << " " << chest->getSignIndex() << Qt::endl;
                    }
                    break;

                    case LevelEntityType::ENTITY_BADDY:
                    {
                        auto baddy = static_cast<LevelGraalBaddy*>(obj);

                        stream << "BADDY " << int((baddy->getX() - getX()) / 16.0) << " " << int((baddy->getY() - getY()) / 16.0) << " " << baddy->getBaddyType() << Qt::endl;
                        for (auto i = 0; i < 3; ++i)
                        {
                            stream << baddy->getBaddyVerse(i) << Qt::endl;
                        }
                        stream << "BADDYEND" << Qt::endl << Qt::endl;
                    }
                    break;
                }

            }

            if(!m_tilesetName.isEmpty())
                stream << "TILESET " << m_tilesetName << Qt::endl;
            return true;
        }
        return false;
    }

    bool Level::saveLVLFile()
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
                                int tile = tilemap->getTile(left, top);

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

        QFile file(m_fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            auto jsonRoot = cJSON_CreateObject();

            cJSON_AddStringToObject(jsonRoot, "type", "level");
            cJSON_AddStringToObject(jsonRoot, "version", "1.0");
            cJSON_AddNumberToObject(jsonRoot, "hcount", getWidth() / 16);
            cJSON_AddNumberToObject(jsonRoot, "vcount", getHeight() / 16);

            if (!m_tilesetName.isEmpty())
                cJSON_AddStringToObject(jsonRoot, "tileset", m_tilesetName.toLocal8Bit().data());
            

            if (m_tileLayers.size() > 0)
            {
                auto jsonLayers = cJSON_CreateArray();

                for (auto layer : m_tileLayers)
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

            if (m_signs.size() > 0)
            {
                auto jsonSigns = cJSON_CreateArray();
                for (auto sign : m_signs)
                {
                    auto jsonSign = cJSON_CreateObject();

                    cJSON_AddNumberToObject(jsonSign, "x", int(sign->getX() - this->getX()));
                    cJSON_AddNumberToObject(jsonSign, "y", int(sign->getY() - this->getY()));
                    cJSON_AddNumberToObject(jsonSign, "width", sign->getWidth());
                    cJSON_AddNumberToObject(jsonSign, "height", sign->getHeight());

                    cJSON_AddStringToObject(jsonSign, "text", sign->getText().toLocal8Bit().data());

                    cJSON_AddItemToArray(jsonSigns, jsonSign);
;               }

                cJSON_AddItemToObject(jsonRoot, "signs", jsonSigns);
            }

            if (m_links.size() > 0)
            {
                auto jsonLinks = cJSON_CreateArray();
                for (auto link : m_links)
                {
                    auto jsonLink = cJSON_CreateObject();

                    cJSON_AddNumberToObject(jsonLink, "x", int(link->getX() - this->getX()));
                    cJSON_AddNumberToObject(jsonLink, "y", int(link->getY() - this->getY()));
                    cJSON_AddNumberToObject(jsonLink, "width", link->getWidth());
                    cJSON_AddNumberToObject(jsonLink, "height", link->getHeight());

                    cJSON_AddStringToObject(jsonLink, "destination", link->getNextLevel().toLocal8Bit().data());
                    cJSON_AddStringToObject(jsonLink, "destinationX", link->getNextX().toLocal8Bit().data());
                    cJSON_AddStringToObject(jsonLink, "destinationY", link->getNextY().toLocal8Bit().data());
                    cJSON_AddItemToArray(jsonLinks, jsonLink);
      
                }

                cJSON_AddItemToObject(jsonRoot, "links", jsonLinks);
            }

            auto jsonObjects = cJSON_CreateArray();

            for (auto obj : m_objects)
            {
                if (obj->getEntityType() == LevelEntityType::ENTITY_NPC)
                {
                    auto npc = static_cast<LevelNPC*>(obj);

                    auto jsonNPC = cJSON_CreateObject();
                    if (jsonNPC)
                    {
                        cJSON_AddStringToObject(jsonNPC, "type", "npcV1");
                        cJSON_AddNumberToObject(jsonNPC, "x", int(npc->getX() - this->getX()));
                        cJSON_AddNumberToObject(jsonNPC, "y", int(npc->getY() - this->getY()));

                        cJSON_AddStringToObject(jsonNPC, "image", npc->getImageName().toLocal8Bit().data());
                        cJSON_AddStringToObject(jsonNPC, "code", npc->getCode().toLocal8Bit().data());

                        cJSON_AddItemReferenceToArray(jsonObjects, jsonNPC);
                    }
                }
            }

            cJSON_AddItemToObject(jsonRoot, "objects", jsonObjects);

            auto levelText = cJSON_Print(jsonRoot);
            QTextStream stream(&file);
            stream << levelText;

            free(levelText);
            cJSON_Delete(jsonRoot);
            return true;
        }
        return false;
    }




    AbstractLevelEntity* Level::getObjectAt(double x, double y, LevelEntityType type)
    {
        Rectangle rect(x, y, 1, 1);
        for (auto object : m_objects)
        {
            if (object->getEntityType() == type && object->intersects(rect))
                return object;
        }
        return nullptr;
    }

    Tilemap* Level::getOrMakeTilemap(int index, ResourceManager& resourceManager)
    {
        Tilemap* retval = nullptr;
        if (index == 0)
            retval = m_mainTileLayer;
        else {
            auto it = m_tileLayers.find(index);
            if (it != m_tileLayers.end())
                retval = it.value();
        }

        if (retval == nullptr)
        {
            retval = new Tilemap(this, getX(), getY(), getWidth() / 16, getHeight() / 16, index);
            retval->clear(Tilemap::MakeInvisibleTile(0));
            setTileLayer(index, retval);
            return retval;
        }
        else return retval;
    }

    Tilemap* Level::getTilemap(size_t layer)
    {
        if (layer == 0)
            return m_mainTileLayer;

        auto it = m_tileLayers.find(layer);
        return it == m_tileLayers.end() ? nullptr : it.value();
    }

    void Level::updateSpatialEntity(AbstractLevelEntity* entity)
    {
        if (m_overworld)
            m_overworld->getEntitySpatialMap()->updateEntity(entity);
        else m_entitySpatialMap->updateEntity(entity);
    }

    void Level::addObject(AbstractLevelEntity* object)
    {

        addEntityToSpatialMap(object);
        m_objects.insert(object);

        if (object->getEntityType() == LevelEntityType::ENTITY_LINK)
            m_links.push_back(static_cast<LevelLink*>(object));
        else if (object->getEntityType() == LevelEntityType::ENTITY_SIGN)
            m_signs.push_back(static_cast<LevelSign*>(object));
    }

    void Level::addEntityToSpatialMap(AbstractLevelEntity* object)
    {
        if (m_overworld)
        {
            m_overworld->addEntityToSpatialMap(object);
        }
        else m_entitySpatialMap->add(object);
    }

    void Level::removeObject(AbstractLevelEntity* object)
    {
        m_objects.remove(object);

        removeEntityFromSpatialMap(object);


        if (object->getEntityType() == LevelEntityType::ENTITY_LINK)
            m_links.removeOne(object);

        if (object->getEntityType() == LevelEntityType::ENTITY_SIGN)
            m_signs.removeOne(object);
    }

    void Level::removeEntityFromSpatialMap(AbstractLevelEntity* object)
    {
        if (m_overworld)
            m_overworld->removeEntityFromSpatialMap(object);
        else m_entitySpatialMap->remove(object);
    }

    QString Level::getDisplayTile(int tile) const
    {
        auto tileLeft = Tilemap::GetTileX(tile);
        auto tileTop = Tilemap::GetTileY(tile);

        //Gonstruct: Converts tile positions (lef/top) to graal tile index
        auto tileIndex = tileLeft / 16 * 512 + tileLeft % 16 + tileTop * 16;
        return QString::number(tileIndex);
    }

    Rectangle Level::clampEntity(AbstractLevelEntity* entity)
    {
        auto left = entity->getX();
        auto top = entity->getY();
        auto right = entity->getRight();
        auto bottom = entity->getBottom();

        left = std::max(left, this->getX());
        top = std::max(top, this->getY());
        right = std::min(right, this->getRight());
        bottom = std::min(bottom, this->getBottom());

        return Rectangle(left, top, int(right - left), int(bottom - top));
    }

    void Level::setTileLayer(int index, Tilemap* tilemap)
    {

        auto it = m_tileLayers.find(index);
        if (it != m_tileLayers.end())
        {
            delete it.value();
        }
        m_tileLayers[index] = tilemap;

        if(index == 0)
            m_mainTileLayer = tilemap;
        
    }
};
