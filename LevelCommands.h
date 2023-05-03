#ifndef LEVELCOMMANDSH
#define LEVELCOMMANDSH

#include <QUndoCommand>
#include <IWorld.h>
#include "Tilemap.h"

namespace TilesEditor
{
	class CommandPutTiles :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		double m_x;
		double m_y;
		Tilemap* m_putTiles;
		int m_layerIndex;
		Tilemap* m_beforeTiles;



	public:
		CommandPutTiles(IWorld* world, double x, double y, const Tilemap& tilemap, int layerIndex);
		~CommandPutTiles();

		void undo() override;
	};
};
#endif
