#include "LevelCommands.h"

namespace TilesEditor
{
	CommandPutTiles::CommandPutTiles(IWorld* world, double x, double y, const Tilemap& tilemap, int layerIndex)
	{
		m_world = world;

	}

	CommandPutTiles::~CommandPutTiles()
	{
		delete m_beforeTiles;
	}

	void CommandPutTiles::undo()
	{

	}
}
