#include "LevelCommands.h"
#include "AbstractLevelEntity.h"
#include "Level.h"

namespace TilesEditor
{
	//Delete tiles
	CommandDeleteTiles::CommandDeleteTiles(IWorld* world, double x, double y, int layer, const Tilemap* oldTiles, int replaceTile):
		QUndoCommand("Delete Tiles"), m_oldTiles(*oldTiles)
	{
		m_world = world;
		m_x = x;
		m_y = y;
		m_layer = layer;
		m_replaceTile = replaceTile;
	}

	CommandDeleteTiles::~CommandDeleteTiles()
	{

	}

	void CommandDeleteTiles::undo()
	{
		m_world->putTiles(QPointF(m_x, m_y), m_layer, &m_oldTiles, true, false);
	}

	void CommandDeleteTiles::redo()
	{
		m_world->deleteTiles(QPointF(m_x, m_y), m_layer, m_oldTiles.getHCount(), m_oldTiles.getVCount(), m_replaceTile);
	}


	//Put Tiles
	CommandPutTiles::CommandPutTiles(IWorld* world, double x, double y, int layer, const Tilemap* oldTiles, const Tilemap* newTiles, bool applyNewTranslucency):
		QUndoCommand("Put Tiles"), m_oldTiles(*oldTiles), m_newTiles(*newTiles)
	{
		m_world = world;
		m_applyNewTranslucency = applyNewTranslucency;
		m_x = x;
		m_y = y;
		m_layer = layer;
	}

	CommandPutTiles::~CommandPutTiles()
	{

	}

	void CommandPutTiles::undo()
	{
		m_world->putTiles(QPointF(m_x, m_y), m_layer, &m_oldTiles, false, false);
	}

	void CommandPutTiles::redo()
	{
		m_world->putTiles(QPointF(m_x, m_y), m_layer, &m_newTiles, true, m_applyNewTranslucency);
	}

	

	//Move entity
	CommandMoveEntity::CommandMoveEntity(IWorld* world, const QRectF& oldRect, const QRectF& newRect, AbstractLevelEntity* entity, QUndoCommand* parent):
		QUndoCommand("Move Object", parent)
	{
		m_world = world;
		m_oldRect = oldRect;
		m_newRect = newRect;

		m_entity = entity;
	}

	void CommandMoveEntity::undo()
	{
		m_entity->setX(m_oldRect.x());
		m_entity->setY(m_oldRect.y());
		m_entity->setWidth(m_oldRect.width());
		m_entity->setHeight(m_oldRect.height());


		auto level = m_world->getLevelAt(QPointF(m_entity->getCenterX(), m_entity->getCenterY()));

		if (level != nullptr)
		{
			//A change of level is required
			if (level != m_entity->getLevel()) {

				//If they've changed level, remove it from the old level
				if (m_entity->getLevel()) {
					m_entity->getLevel()->removeObject(m_entity);
					m_world->setModified(m_entity->getLevel());
				}

				level->addObject(m_entity);
				m_entity->setLevel(level);
				m_world->setModified(level);
			}
			//ONLY add it to the spatial map
			else {
				level->updateSpatialEntity(m_entity);
				m_world->setModified(level);
			}
		}
	}

	void CommandMoveEntity::redo()
	{
		m_entity->setX(m_newRect.x());
		m_entity->setY(m_newRect.y());
		m_entity->setWidth(m_newRect.width());
		m_entity->setHeight(m_newRect.height());

		auto level = m_world->getLevelAt(QPointF(m_entity->getCenterX(), m_entity->getCenterY()));

		if (level != nullptr)
		{
			//A change of level is required
			if (level != m_entity->getLevel()) {
				//If they've changed level, remove it from the old level
				if (m_entity->getLevel()) {
					m_entity->getLevel()->removeObject(m_entity);
					m_world->setModified(m_entity->getLevel());
				}

				//Add to the new level
				level->addObject(m_entity);
				m_entity->setLevel(level);
				m_world->setModified(level);
			}
			//ONLY add it to the spatial map
			else {
				level->updateSpatialEntity(m_entity);
				m_world->setModified(level);
			}

		}
	}


	//Delete entity
	CommandDeleteEntity::CommandDeleteEntity(IWorld* world, AbstractLevelEntity* entity, QUndoCommand* parent):
		QUndoCommand(QString("Delete Object %1").arg(entity->name()), parent)
	{
		m_doDelete = true;
		m_world = world;
		m_entity = entity;
	}

	CommandDeleteEntity::~CommandDeleteEntity()
	{
		if (m_doDelete)
		{
			delete m_entity;
		}
	}

	void CommandDeleteEntity::undo()
	{
		m_doDelete = false;
		if (m_entity->getLevel())
		{
			m_entity->getLevel()->addObject(m_entity);
			m_world->setModified(m_entity->getLevel());
		}
	}

	void CommandDeleteEntity::redo()
	{
		m_doDelete = true;
		
		if (m_entity->getLevel())
		{
			m_entity->getLevel()->removeObject(m_entity);
			m_world->setModified(m_entity->getLevel());
			m_world->removeEntitySelection(m_entity);
		}
	}

	//Add entity
	CommandAddEntity::CommandAddEntity(IWorld* world, AbstractLevelEntity* entity, QUndoCommand* parent):
		QUndoCommand(QString("Add Object %1").arg(entity->name()), parent)
	{
		m_world = world;
		m_doDelete = false;
		m_entity = entity;
	}

	CommandAddEntity::~CommandAddEntity()
	{
		if (m_doDelete)
			delete m_entity;
	}

	void CommandAddEntity::undo()
	{
		m_doDelete = true;

		if (m_entity->getLevel())
		{
			m_entity->getLevel()->removeObject(m_entity);
			m_world->setModified(m_entity->getLevel());
			m_world->removeEntitySelection(m_entity);
		}
	}

	void CommandAddEntity::redo()
	{
		m_doDelete = false;
		if (m_entity->getLevel())
		{
			m_entity->getLevel()->addObject(m_entity);
			m_world->setModified(m_entity->getLevel());
		}
	}


	//Reshape entity
	CommandReshapeEntity::CommandReshapeEntity(IWorld* world, const QRectF& oldRect, const QRectF& newRect, AbstractLevelEntity* entity, QUndoCommand* parent):
		QUndoCommand("Change Object Size", parent)
	{
		m_world = world;
		m_oldRect = oldRect;
		m_newRect = newRect;
		m_entity = entity;

	}

	void CommandReshapeEntity::undo()
	{
		m_entity->setX(m_oldRect.x());
		m_entity->setY(m_oldRect.y());
		m_entity->setWidth(m_oldRect.width());
		m_entity->setHeight(m_oldRect.height());

		if(m_entity->getLevel())
			m_entity->getLevel()->updateSpatialEntity(m_entity);
	}

	void CommandReshapeEntity::redo()
	{
		m_entity->setX(m_newRect.x());
		m_entity->setY(m_newRect.y());
		m_entity->setWidth(m_newRect.width());
		m_entity->setHeight(m_newRect.height());

		if (m_entity->getLevel())
			m_entity->getLevel()->updateSpatialEntity(m_entity);
	}


	CommandFloodFillPattern::CommandFloodFillPattern(IWorld* world, double x, double y, int layer, const Tilemap* pattern):
		QUndoCommand("Flood Fill")
	{
		m_world = world;
		m_x = x;
		m_y = y;
		m_layer = layer;
		m_pattern = new Tilemap(*pattern);
	}

	void CommandFloodFillPattern::undo()
	{
		Level* level = nullptr;
		//Each node in m_nodes contains a tile position in the world that needs to revert back to m_oldTile
		while (m_nodes.count() > 0)
		{
			auto node = m_nodes.front();
			m_nodes.pop_front();

			auto x = node.tileX * 16.0;
			auto y = node.tileY * 16.0;


			if (level == nullptr || x < level->getX() || x >= level->getRight() || y < level->getY() || y >= level->getBottom())
				level = m_world->getLevelAt(QPointF(x, y));

			if (level != nullptr)
			{
				auto tilemap = level->getTilemap(m_layer);
				if (tilemap != nullptr)
				{
					auto tileX = int((x - tilemap->getX()) / 16.0);
					auto tileY = int((y - tilemap->getY()) / 16.0);

					tilemap->setTile(tileX, tileY, node.tile);
					m_world->setModified(level);
				}
			}
		}
	}

	void CommandFloodFillPattern::redo()
	{
		m_nodes.clear();
		m_world->floodFillPattern(QPointF(m_x, m_y), m_layer, m_pattern, &m_nodes);
	}

	void CommandMoveEntities::undo()
	{
		for (auto pair : m_oldPositions.asKeyValueRange())
		{
			auto entity = pair.first;
			auto& rect = pair.second;
			entity->setX(rect.x());
			entity->setY(rect.y());
			entity->setWidth(rect.width());
			entity->setHeight(rect.height());

			auto level = m_world->getLevelAt(QPointF(entity->getCenterX(), entity->getCenterY()));

			if (level != nullptr)
			{
				//A change of level is required
				if (level != entity->getLevel()) {
					//If they've changed level, remove it from the old level
					if (entity->getLevel()) {
						entity->getLevel()->removeObject(entity);
						m_world->setModified(entity->getLevel());
					}

					//Add to the new level
					level->addObject(entity);
					entity->setLevel(level);
					m_world->setModified(level);
				}
				//ONLY add it to the spatial map
				else {
					level->updateSpatialEntity(entity);
					m_world->setModified(level);
				}

			}
		}
	}

	void CommandMoveEntities::redo()
	{
		for (auto pair : m_newPositions.asKeyValueRange())
		{
			auto entity = pair.first;
			auto& rect = pair.second;
			entity->setX(rect.x());
			entity->setY(rect.y());
			entity->setWidth(rect.width());
			entity->setHeight(rect.height());

			auto level = m_world->getLevelAt(QPointF(entity->getCenterX(), entity->getCenterY()));

			if (level != nullptr)
			{
				//A change of level is required
				if (level != entity->getLevel()) {
					//If they've changed level, remove it from the old level
					if (entity->getLevel()) {
						entity->getLevel()->removeObject(entity);
						m_world->setModified(entity->getLevel());
					}

					//Add to the new level
					level->addObject(entity);
					entity->setLevel(level);
					m_world->setModified(level);
				}
				//ONLY add it to the spatial map
				else {
					level->updateSpatialEntity(entity);
					m_world->setModified(level);
				}

			}
		}

	}

	bool CommandMoveEntities::mergeWith(const QUndoCommand* command)
	{
		auto other = static_cast<const CommandMoveEntities*>(command);

		if (this->m_newPositions.size() == other->m_newPositions.size() && this->m_newPositions.keys() == other->m_newPositions.keys())
		{
			this->m_newPositions = other->m_newPositions;
			return true;
		}

		return false;
	}

}
