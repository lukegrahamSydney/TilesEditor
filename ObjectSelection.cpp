#include <QClipboard>
#include <QApplication>
#include "Level.h"
#include "cJSON/JsonHelper.h"

#include "ObjectSelection.h"
#include "LevelCommands.h"
#include "Selector.h"
#include "LevelChest.h"
#include "LevelSign.h"
#include "LevelGraalBaddy.h"
#include "ObjectFactory.h"

namespace TilesEditor
{
    ObjectSelection::ObjectSelection(double x, double y) :
        AbstractSelection(x, y)
    {
        m_selectMode = SelectMode::MODE_MOVE;
    }

    void ObjectSelection::draw(QPainter* painter, const IRectangle& viewRect)
    {
        for (auto object : m_selectedObjects)
        {
            object->draw(painter, viewRect, object->getX(), object->getY());

            Selector::draw(painter, viewRect, object->getX(), object->getY(), object->getWidth(), object->getHeight(), QColorConstants::White, QColor(255, 255, 255, 60));
        }
    }

    bool ObjectSelection::pointInSelection(double x, double y)
    {
        Rectangle rect(x, y, 1, 1);

        for (auto object : m_selectedObjects)
        {
            if (object->intersects(rect))
                return true;
        }
        return false;
    }

    void ObjectSelection::drag(double x, double y, bool snap, double snapX, double snapY, IWorld* world)
    {
        AbstractSelection::drag(x, y, snap, snapX, snapY, world);

        for (auto object : m_selectedObjects)
        {
            object->drag(x, y, snap, snapX, snapY);

        }
    }

    void ObjectSelection::setDragOffset(double x, double y, bool snap, double snapX, double snapY)
    {
        AbstractSelection::setDragOffset(x, y, snap, snapX, snapY);

        for (auto object : m_selectedObjects)
        {
            object->setDragOffset(x, y, snap, snapX, snapY);
        }
    }

    void ObjectSelection::reinsertIntoWorld(IWorld* world, int layer)
    {
        if (m_selectedObjects.count() > 0)
        {
            if (m_selectMode == SelectMode::MODE_INSERT)
            {
                auto undoCommand = new QUndoCommand();
                for (auto object : m_selectedObjects)
                {
                    if (object->getEntityType() == LevelEntityType::ENTITY_LINK || object->getEntityType() == LevelEntityType::ENTITY_SIGN)
                    {
                        //Links and signs will be split up across overlapping levels
                        auto levels = world->getLevelsInRect(*object);
                        for (auto level : levels)
                        {

                            //Duplicate our object
                            auto newObject = object->duplicate();
                            if (newObject)
                            {
                                newObject->setLevel(level);
                                auto rect = level->clampEntity(newObject);
                                newObject->setX(rect.getX());
                                newObject->setY(rect.getY());
                                newObject->setWidth(rect.getWidth());
                                newObject->setHeight(rect.getHeight());

                                //Special case for graal signs. They're always 2 blocks wide (32 pixels)
                                if (newObject->getEntityType() == LevelEntityType::ENTITY_SIGN)
                                    newObject->setWidth(32);

                                new CommandAddEntity(world, newObject, undoCommand);
                            }
                            
                        }

                        delete object;
                        
                    }
                    else {
                        auto level = world->getLevelAt(object->getCenterX(), object->getCenterY());

                        if (level != nullptr)
                        {
                            object->setLevel(level);
                            new CommandAddEntity(world, object, undoCommand);
                        }
                    }
                    
                }


                world->addUndoCommand(undoCommand);
            }
            else if (m_selectMode == SelectMode::MODE_MOVE)
            {
                auto undoCommand = new QUndoCommand();
                for (auto object : m_selectedObjects)
                {
                    if (object->getEntityType() == LevelEntityType::ENTITY_LINK || object->getEntityType() == LevelEntityType::ENTITY_SIGN)
                    {

                        //Links and signs will be split up across overlapping levels

                        //Get a list of all the levels our object intersects
                        bool deleteOriginal = true;
                        auto levels = world->getLevelsInRect(*object);
                        for (auto level : levels)
                        {
                            if (level == object->getLevel())
                            {
                                Rectangle oldRect = object->getStartRect();
                                auto newRect = level->clampEntity(object);
                                if (object->getEntityType() == LevelEntityType::ENTITY_SIGN)
                                    newRect.setWidth(32);

                                level->addEntityToSpatialMap(object);
                                new CommandReshapeEntity(world, oldRect, newRect, object, undoCommand);


                                deleteOriginal = false;
                            }
                            else {
                                //Duplicate our object
                                auto newObject = object->duplicate();
                                if (newObject)
                                {
                                    newObject->setLevel(level);
                                    auto rect = level->clampEntity(newObject);
                                    newObject->setX(rect.getX());
                                    newObject->setY(rect.getY());
                                    newObject->setWidth(rect.getWidth());
                                    newObject->setHeight(rect.getHeight());

                                    //Special case for graal signs. They're always 2 blocks wide (32 pixels)
                                    if (newObject->getEntityType() == LevelEntityType::ENTITY_SIGN)
                                        newObject->setWidth(32);

                                    new CommandAddEntity(world, newObject, undoCommand);
                                }
                            }
                        }

                        if (deleteOriginal)
                        {
                            //Before we delete it, add the undo command that will revert it back to its start position before we dragged it
                            new CommandReshapeEntity(world, object->getStartRect(), *object, object, undoCommand);
                            new CommandDeleteEntity(world, object, undoCommand);
                        }

                    }else 
                    {

                        if (object->getLevel())
                            object->getLevel()->addEntityToSpatialMap(object);

                        //Npc,chest, etc
                        new CommandMoveEntity(world, object->getStartRect(), *object, object, undoCommand);

                    }
          
                }

                world->addUndoCommand(undoCommand);
            }
        }
        m_selectedObjects.clear();
    }

    void ObjectSelection::clearSelection(IWorld* world)
    {
        if (m_selectMode == SelectMode::MODE_MOVE)
        {
            auto undoCommand = new QUndoCommand();
            for (auto entity : m_selectedObjects)
            {
                new CommandReshapeEntity(world, entity->getStartRect(), *entity, entity, undoCommand);
            }

            world->deleteEntities(m_selectedObjects, undoCommand);
            world->addUndoCommand(undoCommand);
        }
        else if (m_selectMode == SelectMode::MODE_INSERT)
        {
            for (auto obj : m_selectedObjects)
            {
                obj->releaseResources();
                delete obj;
            }
            
        }

        m_selectedObjects.clear();
    }

    int ObjectSelection::getWidth() const
    {
        if (m_selectedObjects.size() == 1)
            return m_selectedObjects.first()->getWidth();
        return 0;
    }

    int ObjectSelection::getHeight() const
    {
        if (m_selectedObjects.size() == 1)
            return m_selectedObjects.first()->getHeight();
        return 0;
    }

    bool ObjectSelection::canResize() const
    {
        if (m_selectedObjects.size() == 1)
        {
            return m_selectedObjects.first()->canResize();
        }
        return false;
    }

    bool ObjectSelection::clipboardCopy()
    {
        auto jsonObject = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonObject, "type", "objectSelection");
        cJSON_AddNumberToObject(jsonObject, "x", getX());
        cJSON_AddNumberToObject(jsonObject, "y", getY());

        auto objectsArray = cJSON_CreateArray();
        for (auto object : m_selectedObjects)
        {
            auto objJSON = object->serializeJSON();

            if (objJSON != nullptr)
            {
                cJSON_AddItemToArray(objectsArray, objJSON);
            }
        }
        
        cJSON_AddItemToObject(jsonObject, "objects", objectsArray);

        QClipboard* clipboard = QApplication::clipboard();
        auto buffer = cJSON_Print(jsonObject);

        clipboard->setText(QString(buffer));
        free(buffer);
        cJSON_Delete(jsonObject);

        return true;
    }

    void ObjectSelection::deserializeJSON(cJSON* json, IWorld* world, int newLayer)
    {
        auto x = jsonGetChildInt(json, "x");
        auto y = jsonGetChildInt(json, "y");

        auto objects = cJSON_GetObjectItem(json, "objects");

        if (objects)
        {
            for (auto i = 0; i < cJSON_GetArraySize(objects); ++i)
            {
                auto jsonObj = cJSON_GetArrayItem(objects, i);
                if (jsonObj)
                {
                    auto objectType = jsonGetChildString(jsonObj, "type");
                    auto entity = ObjectFactory::createObject(world, objectType, jsonObj);

                    if (entity)
                    {
                        auto offsetX = entity->getX() - x;
                        auto offsetY = entity->getY() - y;

                        entity->setX(getX() + offsetX);
                        entity->setY(getY() + offsetY);
                        entity->setLayerIndex(newLayer);
                        addObject(entity);

                        entity->loadResources();
                    }
                }
            }
        }
    }

    AbstractLevelEntity* ObjectSelection::getEntityAtPoint(double x, double y)
    {
        Rectangle rect(x, y, 1, 1);

        for (auto object : m_selectedObjects)
        {
            if (object->intersects(rect))
                return object;
        }
        return nullptr;
    }

    LevelEntityType ObjectSelection::getEntityType() const
    {
        if (m_selectedObjects.size() == 1)
            return m_selectedObjects.first()->getEntityType();
        return LevelEntityType::ENTITY_INVALID;
    }

    void ObjectSelection::addObject(AbstractLevelEntity* entity)
    {
        m_selectedObjects.push_back(entity);
    }


    void ObjectSelection::updateResize(int mouseX, int mouseY, bool snap, double snapX, double snapY, IWorld* world)
    {
        if (canResize())
        {
            m_selectedObjects.first()->updateResize(getResizeEdges(), mouseX, mouseY, snap, snapX, snapY);

            world->setModified(m_selectedObjects.first()->getLevel());
        }
    }

    void ObjectSelection::endResize(IWorld* world)
    {
        AbstractSelection::endResize(world);
        if (canResize())
        {
            m_selectedObjects.first()->endResize();
        }
    }

    int ObjectSelection::getResizeEdge(int mouseX, int mouseY)
    {
        if (canResize())
        {
            auto resizeObject = m_selectedObjects.first();

            if (mouseX > resizeObject->getX() - 4 && mouseX < resizeObject->getRight() + 4 && mouseY > resizeObject->getY() - 4 && mouseY < resizeObject->getBottom() + 4)
            {
                int retval = 0;

                if (mouseX <= resizeObject->getX() + 2)
                {
                    retval |= Edges::EDGE_LEFT;
                }
                else if (mouseX >= resizeObject->getRight() - 2)
                {
                    retval |= Edges::EDGE_RIGHT;
                }

                if (mouseY <= resizeObject->getY() + 2)
                {
                    retval |= Edges::EDGE_TOP;
                }
                else if (mouseY >= resizeObject->getBottom() - 2)
                {
                    retval |= Edges::EDGE_BOTTOM;
                }
                return retval;

            }
        }
       
        return 0;
    }

    Rectangle ObjectSelection::getDrawRect() const
    {
        if (m_selectedObjects.size() > 0)
        {
            double left = m_selectedObjects.first()->getX();
            double top = m_selectedObjects.first()->getY();
            double right = m_selectedObjects.first()->getRight();
            double bottom = m_selectedObjects.first()->getBottom();

            for (int i = 1; i < m_selectedObjects.size(); ++i)
            {
                auto obj = m_selectedObjects[i];

                left = std::min(obj->getX(), left);
                top = std::min(obj->getY(), top);
                right = std::max(obj->getRight(), right);
                bottom = std::max(obj->getBottom(), bottom);
            }
            return Rectangle(left, top, right - left, bottom - top);
        }

        return Rectangle();



    }
}
