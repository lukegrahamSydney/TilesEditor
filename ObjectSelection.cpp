#include <QClipboard>
#include <QApplication>
#include "Level.h"
#include "cJSON/JsonHelper.h"

#include "ObjectSelection.h"
#include "LevelCommands.h"
#include "Selector.h"

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

    void ObjectSelection::drag(double x, double y, bool snap, IWorld* world)
    {
        AbstractSelection::drag(x, y, snap, world);

        for (auto object : m_selectedObjects)
        {
            object->drag(x, y, snap, world);

        }
    }

    void ObjectSelection::setDragOffset(double x, double y, bool snap)
    {
        AbstractSelection::setDragOffset(x, y, snap);

        for (auto object : m_selectedObjects)
        {
            object->setDragOffset(x, y, snap);
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
                    if (object->getEntityType() == LevelEntityType::ENTITY_NPC)
                    {
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
                    if (object->getEntityType() == LevelEntityType::ENTITY_NPC)
                    {
                        new CommandMoveEntity(world, object->getStartRect(), *object, object, undoCommand);

                    }
                    else if (object->getEntityType() == LevelEntityType::ENTITY_LINK || object->getEntityType() == LevelEntityType::ENTITY_SIGN) {

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
        }else world->deleteEntities(m_selectedObjects);

        m_selectedObjects.clear();
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

    void ObjectSelection::deserializeJSON(cJSON* json, IWorld* world)
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
                    if (objectType == "levelNPCv1")
                    {
                        auto npc = new LevelNPC(nullptr, jsonObj, world);
                        

                        auto offsetX = npc->getX() - x;
                        auto offsetY = npc->getY() - y;

                        npc->setX(getX() + offsetX);
                        npc->setY(getY() + offsetY);
                        addObject(npc);

                        npc->loadResources(world->getResourceManager());
                    }
                    else if (objectType == "levelLink")
                    {
                        
                        auto link = new LevelLink(nullptr, jsonObj, world);


                        auto offsetX = link->getX() - x;
                        auto offsetY = link->getY() - y;

                        link->setX(getX() + offsetX);
                        link->setY(getY() + offsetY);
                        addObject(link);

                        link->loadResources(world->getResourceManager());

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


    void ObjectSelection::updateResize(int mouseX, int mouseY, bool snap, IWorld* world)
    {
        if (canResize())
        {
            m_selectedObjects.first()->updateResize(getResizeEdges(), mouseX, mouseY, snap, world);

            world->setModified(m_selectedObjects.first()->getLevel());
        }
    }

    void ObjectSelection::endResize(IWorld* world)
    {
        AbstractSelection::endResize(world);
        if (canResize())
        {
            m_selectedObjects.first()->endResize(world);
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
}
