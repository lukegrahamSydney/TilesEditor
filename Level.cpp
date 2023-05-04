#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include "Level.h"
#include "ImageDimensions.h"
#include "Tileset.h"
#include "Overworld.h"
#include "LevelNPC.h"
#include "LevelLink.h"
#include "LevelSign.h"

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
        m_modified = false;
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;

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

        if (m_mainTileLayer)
            delete m_mainTileLayer;

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

    bool Level::loadNWFile(ResourceManager& resourceManager)
    {
        
        QStringList lines;
        
        if (resourceManager.getFileSystem().readAllLines(m_fileName, lines))
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
                auto getOrMakeTilemap = [&](int index) {
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
                        retval = new Tilemap(this, getX(), getY(), 64, 64, index);
                        retval->clear(Tilemap::MakeInvisibleTile(0));
                        setTileLayer(index, retval);
                        return retval;
                    }
                    else return retval;

                };



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

                            auto tilemap = getOrMakeTilemap(layer);
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
                        else if (words[0] == "LINK" && wordCount >= 8)
                        {
                            double x = words[2].toInt() * 16;
                            double y = words[3].toInt() * 16;
                            int width = words[4].toInt() * 16;
                            int height = words[5].toInt() * 16;

                            auto& nextX = words[6];
                            auto& nextY = words[7];

                            bool possibleEdgeLink = false;
                            if ((x == 0 || x == 63 * 16) && width == 16)
                                possibleEdgeLink = true;
                            else if ((y == 0 || y == 63 * 16) && height == 16)
                                possibleEdgeLink = true;

                            auto levelLink = new LevelLink(this, getX() + x, getY() + y, width, height);
                            levelLink->setNextLevel(words[1]);
                            levelLink->setNextX(nextX);
                            levelLink->setNextY(nextY);

                            addObject(levelLink);
                        }
                        else if (words[0] == "CHEST" && wordCount >= 5)
                        {
                            auto itemName = words[3].toLower();
                            /*
                             for (unsigned int ii = 0; ii < sizeof(ITEM_NAMES) / sizeof(*ITEM_NAMES); ++ii)
                             {
                                 if (itemName == ITEM_NAMES[ii])
                                 {
                                     Chest* chest = new Chest();
                                     chest->m_x = atoi(words[1].c_str());
                                     chest->m_y = atoi(words[2].c_str());
                                     chest->m_signIndex = atoi(words[4].c_str());
                                     chest->m_itemId = ii;
                                     this->AddChest(chest);
                                     break;
                                 }
                             }*/


                        }
                        else if (words[0] == "SIGN" && wordCount >= 3)
                        {
                            
                            auto sign = new LevelSign(this,
                                getX() + words[1].toInt() * 16,
                                getY() + words[2].toInt() * 16,
                                32,
                                16);

                            QString text = "";
                            for (++i; i < lineCount && lines[i] != "SIGNEND"; ++i)
                                text += lines[i] + '\n';
                            sign->setText(text);


                            addObject(sign);
                            //m_otherEntities->add(sign);

                        }
                        else if (words[0] == "BADDY" && wordCount >= 3)
                        {
                            /*
                            float x = atof(words[1].c_str());
                            float y = atof(words[2].c_str());
                            int type = atoi(words[3].c_str());

                            if (Server::Instance()->IsEventRegistered(GMEVENT_LEVEL_BADDY_ADDED))
                            {
                                gmVariable params[] = {
                                    gmVariable(this->GetUserObject()),
                                    gmVariable(type),
                                    gmVariable(x),
                                    gmVariable(y)
                                };
                                Server::Instance()->CallGMEvent(GMEVENT_LEVEL_BADDY_ADDED, params, 4);
                            }
                            for (++i; i < lineCount && lines[i] != "BADDYEND"; ++i) {}
                            */
                        }
                        else if (words[0] == "NPC" && wordCount >= 4)
                        {

                            QString image = (words[1] == "-" ? "" : words[1]),
                                code;

                            double x = words[2].toDouble() * 16.0,
                                y = words[3].toDouble() * 16.0;

                            int width = 0, height = 0;

                            for (++i; i < lineCount && lines[i] != "NPCEND"; ++i)
                                code += lines[i] + "\n";

                            if(image != "")
                                getImageDimensions(resourceManager, image, &width, &height);
                            else {
                                width = height = 48;
                            }

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

            if(m_mainTileLayer)
                writeTileLayer(stream, m_mainTileLayer);
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
                if (obj->getEntityType() == LevelEntityType::ENTITY_NPC)
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

            }
            return true;
        }
        return false;
    }

    void Level::drawTilesLayer(QPainter* painter, const IRectangle& viewRect, Image* tilesetImage, int index, bool fade)
    {
        Tilemap* tilemap = nullptr;
        if (index == 0)
            tilemap = m_mainTileLayer;

        auto it = m_tileLayers.find(index);
        if (it != m_tileLayers.end())
            tilemap = it.value();

        if (tilemap != nullptr)
        {
            if (fade)
            {
                painter->setOpacity(0.33);
                tilemap->draw(painter, viewRect, tilesetImage, tilemap->getX(), tilemap->getY());
                painter->setOpacity(1.0);
            } else tilemap->draw(painter, viewRect, tilesetImage,  tilemap->getX(), tilemap->getY());
        }
    }

    void Level::drawAllTileLayers(QPainter* painter, const IRectangle& viewRect, Image* tilesetImage, int selectedLayer, QMap<int, bool>& visibleLayers)
    {
        if(visibleLayers[0])
            drawTilesLayer(painter, viewRect, tilesetImage, 0, selectedLayer != 0);

        for (auto layer : m_tileLayers)
        {
            if(visibleLayers.find(layer->getLayerIndex()) == visibleLayers.end() || visibleLayers[layer->getLayerIndex()])
                drawTilesLayer(painter, viewRect, tilesetImage, layer->getLayerIndex(), selectedLayer != layer->getLayerIndex());
        }

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
            retval = new Tilemap(this, getX(), getY(), 64, 64, index);
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

    Rectangle Level::clampEntity(AbstractLevelEntity* entity)
    {
        auto left = entity->getX();
        auto top = entity->getY();
        auto right = entity->getRight();
        auto bottom = entity->getBottom();

        //Keep our new duplicated object within the level bounds
        left = std::max(left, this->getX());
        top = std::max(top, this->getY());
        right = std::min(right, this->getRight());
        bottom = std::min(bottom, this->getBottom());

        return Rectangle(left, top, int(right - left), int(bottom - top));
    }

    void Level::setTileLayer(int index, Tilemap* tilemap)
    {
        if (index == 0)
        {
            if (m_mainTileLayer != nullptr)
                delete m_mainTileLayer;
            m_mainTileLayer = tilemap;
        }

        else if (index > 0)
        {
            auto it = m_tileLayers.find(index);
            if (it != m_tileLayers.end())
            {
                delete it.value();
            }
            m_tileLayers[index] = tilemap;
        }
    }
};
