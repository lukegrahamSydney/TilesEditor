#include "LevelCommands.h"
#include "AbstractLevelEntity.h"
#include "Level.h"

namespace TilesEditor
{
	//Delete tiles
	CommandDeleteTiles::CommandDeleteTiles(IWorld* world, double x, double y, int layer, const Tilemap* oldTiles, int replaceTile)
	{
		m_world = world;
		m_x = x;
		m_y = y;
		m_layer = layer;
		m_oldTiles = new Tilemap(*oldTiles);
		m_replaceTile = replaceTile;
	}

	CommandDeleteTiles::~CommandDeleteTiles()
	{
		delete m_oldTiles;
	}

	void CommandDeleteTiles::undo()
	{
		m_world->putTiles(m_x, m_y, m_layer, m_oldTiles, true);
	}

	void CommandDeleteTiles::redo()
	{
		m_world->deleteTiles(m_x, m_y, m_layer, m_oldTiles->getHCount(), m_oldTiles->getVCount(), m_replaceTile);
	}


	//Put Tiles
	CommandPutTiles::CommandPutTiles(IWorld* world, double x, double y, int layer, const Tilemap* oldTiles, const Tilemap* newTiles)
	{
		m_world = world;
		m_x = x;
		m_y = y;
		m_layer = layer;
		m_oldTiles = new Tilemap(*oldTiles);
		m_newTiles = new Tilemap(*newTiles);
	}

	CommandPutTiles::~CommandPutTiles()
	{
		delete m_oldTiles;
		delete m_newTiles;
	}

	void CommandPutTiles::undo()
	{
		m_world->putTiles(m_x, m_y, m_layer, m_oldTiles, false);
	}

	void CommandPutTiles::redo()
	{
		m_world->putTiles(m_x, m_y, m_layer, m_newTiles, true);
	}

	

	//Move entity
	CommandMoveEntity::CommandMoveEntity(IWorld* world, const IRectangle& oldRect, const IRectangle& newRect, AbstractLevelEntity* entity, QUndoCommand* parent):
		QUndoCommand(parent)
	{
		m_world = world;
		m_oldRect = oldRect;
		m_newRect = newRect;

		m_entity = entity;
	}

	void CommandMoveEntity::undo()
	{
		m_entity->setX(m_oldRect.getX());
		m_entity->setY(m_oldRect.getY());
		m_entity->setWidth(m_oldRect.getWidth());
		m_entity->setHeight(m_oldRect.getHeight());


		auto level = m_world->getLevelAt(m_entity->getCenterX(), m_entity->getCenterY());

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
			else level->updateSpatialEntity(m_entity);
		}
	}

	void CommandMoveEntity::redo()
	{
		m_entity->setX(m_newRect.getX());
		m_entity->setY(m_newRect.getY());
		m_entity->setWidth(m_newRect.getWidth());
		m_entity->setHeight(m_newRect.getHeight());

		auto level = m_world->getLevelAt(m_entity->getCenterX(), m_entity->getCenterY());

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
			}

		}
	}


	//Delete entity
	CommandDeleteEntity::CommandDeleteEntity(IWorld* world, AbstractLevelEntity* entity, QUndoCommand* parent):
		QUndoCommand(parent)
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
		}
	}

	//Add entity
	CommandAddEntity::CommandAddEntity(IWorld* world, AbstractLevelEntity* entity, QUndoCommand* parent):
		QUndoCommand(parent)
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
	CommandReshapeEntity::CommandReshapeEntity(IWorld* world, const IRectangle& oldRect, const IRectangle& newRect, AbstractLevelEntity* entity, QUndoCommand* parent):
		QUndoCommand(parent)
	{
		m_world = world;
		m_oldRect = oldRect;
		m_newRect = newRect;
		m_entity = entity;

	}

	void CommandReshapeEntity::undo()
	{
		m_entity->setX(m_oldRect.getX());
		m_entity->setY(m_oldRect.getY());
		m_entity->setWidth(m_oldRect.getWidth());
		m_entity->setHeight(m_oldRect.getHeight());

		if(m_entity->getLevel())
			m_entity->getLevel()->updateSpatialEntity(m_entity);
	}

	void CommandReshapeEntity::redo()
	{
		m_entity->setX(m_newRect.getX());
		m_entity->setY(m_newRect.getY());
		m_entity->setWidth(m_newRect.getWidth());
		m_entity->setHeight(m_newRect.getHeight());

		if (m_entity->getLevel())
			m_entity->getLevel()->updateSpatialEntity(m_entity);
	}


	CommandFloodFillPattern::CommandFloodFillPattern(IWorld* world, double x, double y, int layer, const Tilemap* pattern)
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
				level = m_world->getLevelAt(x, y);

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
		m_world->floodFillPattern(m_x, m_y, m_layer, m_pattern, &m_nodes);
	}

}
