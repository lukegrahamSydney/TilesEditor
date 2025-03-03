#include <QStringList>
#include <QTextStream>
#include <QStringBuilder>
#include <QFile>
#include <QThread>
#include <QBuffer>
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
#include "IEngine.h"
#include "cJSON/JsonHelper.h"
#include "FileFormatManager.h"
#include "StringHash.h"

namespace TilesEditor
{
 
    QMap<QString, int> Level::m_imageDimensionsCache;

    sgs_ObjInterface Level::sgs_interface;
    sgs_Variable Level::sgs_classMembers;

    bool Level::getImageDimensions(AbstractResourceManager* resourceManager, const QString& imageName, int* w, int* h)
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

            if (resourceManager->locateFile(imageName, &fileName))
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
        auto engine = world->getEngine();
        sgs_CreateObject(engine->getScriptContext(), &m_thisObject, this, &sgs_interface);
        sgs_CreateDict(engine->getScriptContext(), &m_sgsUserTable, 0);
        engine->addCPPOwnedObject(m_thisObject);

        m_levelFlags.all = 1;
        m_levelFlags.autoEmbedCodeForNewObjectClass = false;

        m_defaultTileset = nullptr;
        m_world = world;
        m_modified = false;
        this->setX(x);
        this->setY(y);
        this->setWidth(width);
        this->setHeight(height);
        m_unitWidth = 16;
        m_unitHeight = 16;
        m_overworld = overworld;
        m_name = name;
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
            tilemap->decrementAndDelete();
    }

    void Level::release()
    {
        for (auto entity : m_objects)
        {
            entity->releaseResources();
        }

        removeTileDefs();
    }


    void Level::loadFileData(QByteArray fileData)
    {
        QBuffer dataStream(&fileData);
        dataStream.open(QIODeviceBase::ReadOnly);
        loadStream(&dataStream);
        
        m_world->redrawScene(QRectF(this->getX(), this->getY(), this->getWidth(), this->getHeight()));
    }

    bool Level::loadFile(bool threaded)
    {

        setLoadState(LoadState::STATE_LOADING);
        if (threaded)
        {

            QThread* thread = new QThread();
            auto fileLoader = new FileDataLoader(m_world->getResourceManager(), m_fileName);

            fileLoader->moveToThread(thread);
            QObject::connect(thread, &QThread::started, fileLoader, &FileDataLoader::start);
            QObject::connect(fileLoader, &FileDataLoader::finished, this, &Level::loadFileData);
            QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
            QObject::connect(thread, &QThread::finished, fileLoader, &FileDataLoader::deleteLater);
            thread->start();
            return true;
        }
        else {
            auto stream = m_world->getResourceManager()->openStreamFullPath(m_fileName, QIODeviceBase::ReadOnly);
            if (stream)
            {
                loadStream(stream);
                delete stream;
            }
            else setLoadState(LoadState::STATE_FAILED);
            return m_loadState == LoadState::STATE_LOADED;
        }

    }

    bool Level::loadStream(QIODevice* stream)
    {
        auto loaded = FileFormatManager::instance()->loadLevel(this, stream);

        setLoadState(loaded ? LoadState::STATE_LOADED : LoadState::STATE_FAILED);

        getWorld()->getEngine()->applyTileDefs(this);
        return loaded;
    }

    bool Level::saveFile(IFileRequester* requester)
    {
        auto stream = m_world->getResourceManager()->openStreamFullPath(m_fileName, QIODevice::WriteOnly);

        if (stream)
        {
            auto retval = saveStream(stream);
            m_world->getResourceManager()->endWrite(requester, m_fileName, stream);

            return retval;
        }

        return false;
    }

    bool Level::saveStream(QIODevice* stream)
    {
        return FileFormatManager::instance()->saveLevel(this, stream);
    }

    void Level::setSize(int width, int height) {
        this->setWidth(width);
        this->setHeight(height);

        if (m_entitySpatialMap) {
            delete m_entitySpatialMap;
            m_entitySpatialMap = new EntitySpatialGrid<AbstractLevelEntity>(getX(), getY(), getWidth(), getHeight());
        }
    }

    AbstractLevelEntity* Level::getObjectAt(double x, double y, LevelEntityType type)
    {
        QRectF rect(x, y, 1, 1);
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
            retval->incrementRef();

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
        object->setLevel(this);

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

            it.value()->decrementAndDelete();
            m_tileLayers.erase(it);
        }
    }

    QRectF Level::clampEntity(AbstractLevelEntity* entity)
    {
        return entity->toQRectF().intersected(*this);
    }

    void Level::setTileLayer(int index, Tilemap* tilemap)
    {
        tilemap->setLevel(this);

        auto it = m_tileLayers.find(index);
        if (it != m_tileLayers.end())
        {
            it.value()->decrementAndDelete();
        }
        m_tileLayers[index] = tilemap;

        if(index == 0)
            m_mainTileLayer = tilemap;
        
    }

    void Level::removeTileDefs()
    {
        for(auto& pair: m_tileDefs)
        {
            getWorld()->getResourceManager()->freeResource(pair.second);
        }
        m_tileDefs.clear();
    }


    void Level::addTileDef(const TileDef& tileDef)
    {
        //Free previous one in same spot
        for(auto it = m_tileDefs.begin(); it != m_tileDefs.end();)
        {
            auto& pair = *it;
            if (pair.first.x() == tileDef.x && pair.first.y() == tileDef.y)
            {
                getWorld()->getResourceManager()->freeResource(pair.second);
                it = m_tileDefs.erase(it);
            }
            else ++it;
        }

        auto image = static_cast<Image*>(getWorld()->getResourceManager()->loadResource(nullptr, tileDef.imageName, ResourceType::RESOURCE_IMAGE));

        if (image)
        {
            m_tileDefs.push_back(QPair<QRect, Image*>(QRect(tileDef.x, tileDef.y, image->width(), image->height()), image));
        }
    }

    void Level::drawTile(double x, double y, Image* tilesetImage, int tileLeft, int tileTop, QPainter* painter)
    {
        QRect srcRect(tileLeft * 16, tileTop * 16, 16, 16);
        if (m_tileDefs.size() > 0)
        {
            for (auto it = m_tileDefs.rbegin(); it != m_tileDefs.rend(); ++it)
            {
                auto& pair = *it;
                if (pair.second && pair.first.contains(srcRect))
                {
                    srcRect.moveTo(srcRect.x() - pair.first.x(), srcRect.y() - pair.first.y());
                    painter->drawPixmap(QPoint(x, y), pair.second->pixmap(), srcRect);
                    return;
                }
            }
        }

        painter->drawPixmap(QPoint(x, y), tilesetImage->pixmap(), srcRect);
    }

    void Level::drawTilemap(Tilemap* tilemap, Image* tilesetImage, double x, double y, QPainter* painter, const QRectF& viewRect)
    {
        qreal startOpacity = painter->opacity();

        int tileWidth = 16;
        int tileHeight = 16;


        int left = qAbs((int)qFloor((qMax((int)(viewRect.x() - x), 0) / tileWidth)));
        int top = qAbs((int)qFloor((qMax((int)(viewRect.y() - y), 0) / tileHeight)));
        int right = qMin((int)qCeil((viewRect.right() - x) / tileWidth), (int)tilemap->getHCount());
        int bottom = qMin((int)qCeil((viewRect.bottom() - y) / tileWidth), (int)tilemap->getVCount());


        //keep within map dimensions
        left = left > tilemap->getHCount() - 1 ? tilemap->getHCount() - 1 : left;
        top = top > tilemap->getVCount() - 1 ? tilemap->getVCount() - 1 : top;
        right = right > tilemap->getHCount() - 1 ? tilemap->getHCount() - 1 : right;
        bottom = bottom > tilemap->getVCount() - 1 ? tilemap->getVCount() - 1 : bottom;


        if (tilesetImage != nullptr)
        {
            int currentTranslucency = 0;
            auto& pixmap = tilesetImage->pixmap();

            QRect srcRect(0, 0, tileWidth, tileHeight);

            for (int y2 = top; y2 <= bottom; ++y2)
            {
                for (int x2 = left; x2 <= right; ++x2)
                {
                    int tile = 0;


                    if (tilemap->tryGetTile(x2, y2, &tile))
                    {
                        auto translucency = Tilemap::GetTileTranslucency(tile);
                        if (translucency != currentTranslucency) {
                            currentTranslucency = translucency;

                            //If completely invisible
                            if (currentTranslucency != 15)
                                painter->setOpacity(startOpacity * (1.0 - (translucency / 15.0)));
                            else painter->setOpacity(0.05);
                        }

                        auto tileLeft = Tilemap::GetTileX(tile) * tileWidth;
                        auto tileTop = Tilemap::GetTileY(tile) * tileHeight;
                        srcRect.moveTo(tileLeft, tileTop);

                        bool foundDef = false;

                        if (m_tileDefs.size() > 0)
                        {
                            for(auto it = m_tileDefs.rbegin(); it != m_tileDefs.rend(); ++it)
                            {
                                auto& pair = *it;
                                if (pair.second && pair.first.contains(srcRect))
                                {

                                    foundDef = true;
                                    srcRect.moveTo(tileLeft - pair.first.x(), tileTop - pair.first.y());
                                    painter->drawPixmap(QPoint(x + (x2 * tileWidth), y + (y2 * tileHeight)), pair.second->pixmap(), srcRect);
                                    break;
                                }
                            }
                        }

                        if (!foundDef)
                            painter->drawPixmap(QPoint(x + (x2 * tileWidth), y + (y2 * tileHeight)), pixmap, srcRect);


                        if (currentTranslucency == 15) {
                            auto oldOpacity = painter->opacity();
                            auto oldPen = painter->pen();
                            painter->setOpacity(1.0);
                            painter->setPen(QColor(255, 0, 0));
                            painter->drawLine(QPoint(x + (x2 * tileWidth), y + (y2 * tileHeight)), QPoint(x + (x2 * tileWidth) + 16, y + (y2 * tileHeight) + 16));
                            painter->setOpacity(oldOpacity);
                            painter->setPen(oldPen);
                        }
                    }
                }
            }
            painter->setOpacity(startOpacity);
        }
    }

    void Level::drawTileset(Image* image, const QColor& backColour, QPainter* painter, const QRectF& rect)
    {
        painter->drawPixmap(0, 0, image->pixmap());

        for (auto& pair : m_tileDefs)
        {
            painter->fillRect(pair.first, backColour);

            if (pair.second)
                painter->drawPixmap(pair.first.x(), pair.first.y(), pair.second->pixmap());
        }
    }



    //This is the scripting interface
    void Level::mark(sgs_Context* ctx)
    {
        sgs_GCMark(ctx, &m_sgsUserTable);
    }

    void Level::registerScriptClass(IEngine* engine)
    {
        static StringHash S_STATE("state");
        static StringHash S_UNITWIDTH("unitWidth");
        static StringHash S_UNITHEIGHT("unitHeight");
        static StringHash S_X("x");
        static StringHash S_Y("y");
        static StringHash S_WIDTH("width");
        static StringHash S_HEIGHT("height");
        static StringHash S_HCOUNT("hcount");
        static StringHash S_VCOUNT("vcount");

        auto ctx = engine->getScriptContext();

        
        auto startStackSize = sgs_StackSize(ctx);

        sgs_PushString(ctx, "getOrMakeTilemap");
        sgs_PushCFunc(ctx, [](sgs_Context* ctx) -> int
        {
            sgs_Method(ctx);
            sgs_Int index;

            Level* self = nullptr;
            if (sgs_LoadArgs(ctx, "@oi", &Level::sgs_interface, &self, &index))
            {

                if (self == nullptr)
                    return 0;

                    
                auto tilemap = self->getOrMakeTilemap(int(index));
                if (tilemap)
                {
                    tilemap->incrementRef();

                    sgs_Variable tilemapObj;
                    sgs_CreateObject(ctx, &tilemapObj, tilemap, &Tilemap::sgs_interface);
                    sgs_PushVariable(ctx, tilemapObj);
                    sgs_Release(ctx, &tilemapObj);
                    return 1;
                }

                
            }

            return 0;
        });

        sgs_PushString(ctx, "setSize");
        sgs_PushCFunc(ctx, [](sgs_Context* ctx) -> int
            {
                sgs_Method(ctx);
                sgs_Int hcount, vcount;
                Level* self = nullptr;
                if (sgs_LoadArgs(ctx, "@oii", &Level::sgs_interface, &self, &hcount, &vcount))
                {
                    if (self == nullptr)
                        return 0;

                    self->setSize(hcount * self->getUnitWidth(), vcount * self->getUnitHeight());
                }

                return 0;
            });

        auto memberCount = sgs_StackSize(ctx) - startStackSize;
        sgs_CreateDict(ctx, &sgs_classMembers, memberCount);
        engine->addCPPOwnedObject(sgs_classMembers);
        
        auto objDestruct = [](sgs_Context* ctx, sgs_VarObj* obj) -> int
        {
            auto self = static_cast<Level*>(obj->data);

            if (self == nullptr)
                return 0;

            return 1;
        };


        auto objMark = [](sgs_Context* C, sgs_VarObj* obj) -> int
        {
            auto self = static_cast<Level*>(obj->data);
            if (self == nullptr)
                return 0;

            self->mark(C);
            return 1;
        };

        auto objGetIndex = [](sgs_Context* C, sgs_VarObj* obj) -> int
        {
            auto self = static_cast<Level*>(obj->data);

            if (self == nullptr)
                return 0;

            auto propName = sgs_StackItem(C, 0);

            if (S_STATE.equals(propName))
            {
                sgs_PushInt(C, self->getLoadState());
                return 1;
            }
            else if (S_UNITWIDTH.equals(propName))
            {
                sgs_PushInt(C, self->getUnitWidth());
                return 1;
            }
            else if (S_UNITHEIGHT.equals(propName))
            {
                sgs_PushInt(C, self->getUnitHeight());
                return 1;
            }
            else if (S_X.equals(propName))
            {
                sgs_PushReal(C, self->getX());
                return 1;
            }
            else if (S_Y.equals(propName))
            {
                sgs_PushReal(C, self->getY());
                return 1;
            }
            else if (S_WIDTH.equals(propName))
            {
                sgs_PushInt(C, self->getWidth());
                return 1;
            }
            else if (S_HEIGHT.equals(propName))
            {
                sgs_PushInt(C, self->getHeight());
                return 1;
            }

            if (sgs_PushIndex(C, Level::sgs_classMembers, propName, 1))
                return 1;


            if (sgs_PushIndex(C, self->m_sgsUserTable, propName, 1))
                return 1;

            return 0;
        };

        auto objSetIndex = [](sgs_Context* C, sgs_VarObj* obj) -> int
        {
            auto self = static_cast<Level*>(obj->data);

            if (self == nullptr)
                return 0;

            auto propName = sgs_StackItem(C, 0);

            if (S_STATE.equals(propName))
            {
                self->setLoadState((LoadState)sgs_GetInt(C, 1));
                return 0;
            }
            else if (S_UNITWIDTH.equals(propName))
            {
                self->setUnitWidth(sgs_GetInt(C, 1));
                return 0;
            }
            else if (S_UNITHEIGHT.equals(propName))
            {
                self->setUnitHeight(sgs_GetInt(C, 1));
                return 0;
            }

            sgs_SetIndex(C, self->m_sgsUserTable, propName, sgs_OptStackItem(C, 1), 1);
            return 0;
        };


        sgs_interface = {
            "Level",
            objDestruct, objMark,  /* destruct, gcmark */
            objGetIndex, objSetIndex,  /* getindex, setindex */
            NULL, NULL, NULL, NULL, /* convert, serialize, dump, getnext */
            NULL, NULL,              /* call, expr */
            NULL, &Level::sgs_interface	/*proplist, parent*/
        };
    }
};
