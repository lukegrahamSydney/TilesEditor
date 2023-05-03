#ifndef LEVELCOMMANDSH
#define LEVELCOMMANDSH

#include <QUndoCommand>
#include <IWorld.h>
#include "Tilemap.h"

namespace TilesEditor
{
	class CommandDeleteTiles:
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		double m_x;
		double m_y;
		int m_layer;
		Tilemap* m_oldTiles;
		int m_replaceTile;

	public:
		CommandDeleteTiles(IWorld* world, double x, double y, int layer, const Tilemap* oldTiles, int replaceTile);
		~CommandDeleteTiles();
		void undo() override;
		void redo() override;
	};

	class CommandPutTiles :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		double m_x;
		double m_y;
		int m_layer;
		Tilemap* m_oldTiles;
		Tilemap* m_newTiles;

	public:
		CommandPutTiles(IWorld* world, double x, double y, int layer, const Tilemap* oldTiles, const Tilemap* newTiles);
		~CommandPutTiles();
		void undo() override;
		void redo() override;
	};

	class CommandFloodFill :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		double m_x;
		double m_y;
		int m_layer;
		int m_oldTile;
		int m_newTile;

	public:
		CommandFloodFill(IWorld* world, double x, double y, int layer, int newTile);

		void undo() override;
		void redo() override;
	};
};
#endif
