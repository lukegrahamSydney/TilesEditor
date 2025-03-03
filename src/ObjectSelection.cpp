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
    ObjectSelection::ObjectSelection(double x, double y, int layer) :
        AbstractSelection(x, y, layer)
    {
        m_selectMode = SelectMode::MODE_MOVE;
    }

    void ObjectSelection::draw(QPainter* painter, const QRectF& viewRect)
    {
        for (auto object : m_selectedObjects)
        {
            if(m_selectMode == SelectMode::MODE_INSERT)
                object->draw(painter, viewRect, object->getX(), object->getY());

            auto bbox = object->getBoundingBox();
            Selector::draw(painter, viewRect, bbox.x(), bbox.y(), bbox.width(), bbox.height(), QColorConstants::White, QColor(255, 255, 255, 60));
        }
    }

    bool ObjectSelection::pointInSelection(double x, double y)
    {
        QRectF rect(x, y, 1, 1);

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

        bool doCopy = shouldCopy();
        QMap<AbstractLevelEntity*, QRectF> oldPositions;
        QMap<AbstractLevelEntity*, QRectF> newPositions;

        for (auto object : m_selectedObjects)
        {
            
            auto oldPosition = QRectF(object->getX(), object->getY(), object->getWidth(), object->getHeight());
            auto newPosition = object->getDragRect(x, y, snap, snapX, snapY);

            if (m_selectMode == SelectMode::MODE_MOVE)
            {
                oldPositions[object] = oldPosition;
                newPositions[object] = newPosition;
                //object->drag(x, y, snap, snapX, snapY);

                if (oldPosition.x() != newPosition.x() || oldPosition.y() != newPosition.y())
                {
                    setMoved(true);
                    if (doCopy)
                    {
                        auto level = world->getLevelAt(QPointF(oldPosition.x(), oldPosition.y()));
                        if (level)
                        {
                            auto newObject = object->duplicate();

                            newObject->setLevel(level);
                            newObject->setX(oldPosition.x());
                            newObject->setY(oldPosition.y());
                            newObject->setLayerIndex(getLayer());


                            world->addUndoCommand(new CommandAddEntity(world, newObject));

                        }
                        setCopy(false);
                    }

                }
            }
            else object->drag(x, y, snap, snapX, snapY);


        }

        if(newPositions.size() > 0)
            world->addUndoCommand(new CommandMoveEntities(world, oldPositions, newPositions));
    }

    void ObjectSelection::endDrag(IWorld* world)
    {
        AbstractSelection::endDrag(world);

        if (m_selectMode == SelectMode::MODE_INSERT)
        {
            reinsertIntoWorld(world, false);
            m_selectMode = SelectMode::MODE_MOVE;
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

    void ObjectSelection::reinsertIntoWorld(IWorld* world, bool clearSelection)
    {
        if (m_selectedObjects.count() > 0)
        {
            if (m_selectMode == SelectMode::MODE_INSERT)
            {
                auto undoCommand = new QUndoCommand("Insert Object/s");
                for (auto object : m_selectedObjects)
                {
                    if (object->getEntityType() == LevelEntityType::ENTITY_LINK || object->getEntityType() == LevelEntityType::ENTITY_SIGN)
                    {
                        //Links and signs will be split up across overlapping levels
                        auto levels = world->getLevelsInRect(object->toQRectF());
                        for (auto level : levels)
                        {

                            //Duplicate our object
                            auto newObject = object->duplicate();
                            if (newObject)
                            {
                                newObject->setLevel(level);
                                auto rect = level->clampEntity(newObject);
                                newObject->setX(rect.x());
                                newObject->setY(rect.y());
                                newObject->setWidth(rect.width());
                                newObject->setHeight(rect.height());

                                //Special case for graal signs. They're always 2 blocks wide (32 pixels)
                                if (newObject->getEntityType() == LevelEntityType::ENTITY_SIGN)
                                    newObject->setWidth(32);

                                new CommandAddEntity(world, newObject, undoCommand);
                            }
                            
                        }

                        delete object;
                        
                    }
                    else {
                        auto level = world->getLevelAt(object->toQRectF().center());

                        if (level != nullptr)
                        {
                            object->setLevel(level);
                            new CommandAddEntity(world, object, undoCommand);
                        }
                    }
                    
                }

                if(undoCommand->childCount() > 0)
                    world->addUndoCommand(undoCommand);
            }
            else if (m_selectMode == SelectMode::MODE_MOVE)
            {
                auto undoCommand = new QUndoCommand("Split Links/Signs");
                for (auto object : m_selectedObjects)
                {
                    if (object->getEntityType() == LevelEntityType::ENTITY_LINK || object->getEntityType() == LevelEntityType::ENTITY_SIGN)
                    {

                        //Links and signs will be split up across overlapping levels

                        //Get a list of all the levels our object intersects
                        auto levels = world->getLevelsInRect(object->toQRectF());

                        if (levels.size() >= 2)
                        {

                            for (auto level : levels)
                            {
                                //Duplicate our object
                                auto newObject = object->duplicate();
                                if (newObject)
                                {
                                    newObject->setLevel(level);
                                    auto rect = level->clampEntity(newObject);
                                    newObject->setX(rect.x());
                                    newObject->setY(rect.y());
                                    newObject->setWidth(rect.width());
                                    newObject->setHeight(rect.height());

                                    //Special case for graal signs. They're always 2 blocks wide (32 pixels)
                                    if (newObject->getEntityType() == LevelEntityType::ENTITY_SIGN)
                                        newObject->setWidth(32);

                                    new CommandAddEntity(world, newObject, undoCommand);
                                }
                            }


                            new CommandDeleteEntity(world, object, undoCommand);
                            
                        }

                    }else 
                    {

                       // if (object->getLevel())
                       //     object->getLevel()->addEntityToSpatialMap(object);

                        //Npc,chest, etc
                       // new CommandMoveEntity(world, object->getStartMoveRect(), *object, object, undoCommand);

                    }
          
                }

                if (undoCommand->childCount() > 0)
                    world->addUndoCommand(undoCommand);
                else delete undoCommand;
            }
        }

        if(clearSelection)
            m_selectedObjects.clear();
    }

    void ObjectSelection::clearSelection(IWorld* world)
    {
        if (m_selectMode == SelectMode::MODE_MOVE)
        {
            auto undoCommand = new QUndoCommand("Delete Objects");
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

    AbstractSelection* ObjectSelection::duplicate()
    {
        auto newSelection = new ObjectSelection(getX(), getY(), getLayer());
        for (auto object : m_selectedObjects)
        {
            auto newObject = object->duplicate();
            newSelection->addObject(newObject);
        }

        return newSelection;
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
                    auto entity = ObjectFactory::createObject(world, objectType, jsonObj);

                    if (entity)
                    {
                        auto offsetX = entity->getX() - x;
                        auto offsetY = entity->getY() - y;

                        entity->setX(getX() + offsetX);
                        entity->setY(getY() + offsetY);
                        entity->setLayerIndex(getLayer());
                        addObject(entity);

                        entity->loadResources();
                    }
                }
            }
        }
    }

    void ObjectSelection::removeEntity(AbstractLevelEntity* entity)
    {
        m_selectedObjects.removeAll(entity);
    }

    AbstractLevelEntity* ObjectSelection::getFirstEntity()
    {
        if (m_selectedObjects.size())
            return m_selectedObjects.first();
        return nullptr;
    }

    AbstractLevelEntity* ObjectSelection::getEntityAtPoint(double x, double y)
    {
        QRectF rect(x, y, 1, 1);

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
            auto firstObject = m_selectedObjects.first();
            //m_selectedObjects.first()->updateResize(getResizeEdges(), mouseX, mouseY, snap, snapX, snapY);
            auto newRect = firstObject->updateResizeRect(getResizeEdges(), mouseX, mouseY, snap, snapX, snapY);

            QMap<AbstractLevelEntity*, QRectF> oldPositions;
            QMap<AbstractLevelEntity*, QRectF> newPositions;
            oldPositions[firstObject] = firstObject->toQRectF();
            newPositions[firstObject] = newRect;

            world->addUndoCommand(new CommandMoveEntities(world, oldPositions, newPositions));
            //world->setModified(m_selectedObjects.first()->getLevel());
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

    QRectF ObjectSelection::getBoundingBox() const
    {
        if (m_selectedObjects.size() > 0)
        {
            auto rect = m_selectedObjects.first()->getBoundingBox();

            double left = rect.x();
            double top = rect.y();
            double right = rect.right();
            double bottom = rect.bottom();

            for (int i = 1; i < m_selectedObjects.size(); ++i)
            {
                auto objRect = m_selectedObjects[i]->getBoundingBox();

                left = qMin(objRect.x(), left);
                top = qMin(objRect.y(), top);
                right = qMax(objRect.right(), right);
                bottom = qMax(objRect.bottom(), bottom);
            }
            return QRectF(left, top, right - left, bottom - top);
        }

        return QRectF();
    }
}
