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
#include "ObjectFactory.h"

#include "cJSON/JsonHelper.h"
#include "FileFormatManager.h"

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

    Level::Level(IWorld* world, double x, double y, int width, int height, Overworld* overworld, const QString& name)
    {
        m_world = world;
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

    void Level::release()
    {
        for (auto entity : m_objects)
        {
            entity->releaseResources();
        }

    }

    bool Level::loadFile()
    {
        auto stream = m_world->getResourceManager().getFileSystem()->openStream(m_fileName, QIODeviceBase::ReadOnly);
        if (stream)
        {
            loadStream(stream);
            delete stream;
        }
        return !m_loadFail;
    }

    bool Level::loadStream(QIODevice* stream)
    {
        m_loaded = FileFormatManager::instance()->loadLevel(this, stream);
        m_loadFail = !m_loaded;
        return m_loaded;
    }

    bool Level::saveFile(IFileRequester* requester)
    {
        auto stream = m_world->getResourceManager().getFileSystem()->openStream(m_fileName, QIODevice::WriteOnly);

        if (stream)
        {
            auto retval = saveStream(stream);
            m_world->getResourceManager().getFileSystem()->endWrite(requester, m_fileName, stream);

            return retval;
        }

        return false;
    }

    bool Level::saveStream(QIODevice* stream)
    {
        return FileFormatManager::instance()->saveLevel(this, stream);
    }

    void Level::setSize(int width, int height) {
        m_width = width;
        m_height = height;

        if (m_entitySpatialMap) {
            delete m_entitySpatialMap;
            m_entitySpatialMap = new EntitySpatialGrid<AbstractLevelEntity>(getX(), getY(), getWidth(), getHeight());
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

    Tilemap* Level::getOrMakeTilemap(int index)
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
            retval = new Tilemap(m_world, getX(), getY(), getWidth() / 16, getHeight() / 16, index);
            retval->setLevel(this);

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

        else if (object->getEntityType() == LevelEntityType::ENTITY_SIGN)
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

    void Level::deleteTileLayer(int index)
    {
        auto it = m_tileLayers.find(index);
        if(it != m_tileLayers.end())
        {
            if (it.key() == 0)
                m_mainTileLayer = nullptr;

            delete it.value();
            m_tileLayers.erase(it);
        }
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
