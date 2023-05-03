#include "LevelCommands.h"

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
		m_world->putTiles(m_x, m_y, m_layer, m_oldTiles);
	}

	void CommandDeleteTiles::redo()
	{
		Tilemap tiles(nullptr, 0.0, 0.0, m_oldTiles->getHCount(), m_oldTiles->getVCount(), 0);
		tiles.clear(m_layer == 0 ? m_replaceTile : Tilemap::MakeInvisibleTile(0));

		m_world->putTiles(m_x, m_y, m_layer, &tiles);
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
		m_world->putTiles(m_x, m_y, m_layer, m_oldTiles);
	}

	void CommandPutTiles::redo()
	{
		m_world->putTiles(m_x, m_y, m_layer, m_newTiles);
	}

	//Flood fill
	CommandFloodFill::CommandFloodFill(IWorld* world, double x, double y, int layer, int newTile)
	{
		m_world = world;
		m_x = x;
		m_y = y;
		m_layer = layer;
		m_newTile = newTile;
	}

	void CommandFloodFill::undo()
	{
		m_world->floodFill(m_x, m_y, m_oldTile);
	}

	void CommandFloodFill::redo()
	{
		m_oldTile = m_world->floodFill(m_x, m_y, m_newTile);
	}

}
