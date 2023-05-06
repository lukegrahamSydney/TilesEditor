#ifndef LEVELCOMMANDSH
#define LEVELCOMMANDSH

#include <QUndoCommand>
#include <IWorld.h>
#include <QList>
#include <QPair>
#include "Tilemap.h"
#include "Rectangle.h"

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

	class CommandFloodFillPattern :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		double m_x;
		double m_y;
		int m_layer;
		int m_oldTile;
		
		Tilemap* m_pattern;
		QList<QPair<unsigned short, unsigned short> > m_nodes;

	public:
		CommandFloodFillPattern(IWorld* world, double x, double y, int layer, const Tilemap* pattern);
		~CommandFloodFillPattern() { delete m_pattern; }
		void undo() override;
		void redo() override;

	};

	class CommandFloodFillPattern2 :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		double m_x;
		double m_y;
		int m_layer;

		Tilemap* m_pattern;
		QList<IWorld::TileInfo> m_nodes;

	public:
		CommandFloodFillPattern2(IWorld* world, double x, double y, int layer, const Tilemap* pattern);
		~CommandFloodFillPattern2() { delete m_pattern; }
		void undo() override;
		void redo() override;

	};

	class CommandAddEntity :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		AbstractLevelEntity* m_entity;
		bool m_doDelete;

	public:
		CommandAddEntity(IWorld* world, AbstractLevelEntity* entity, QUndoCommand* parent = nullptr);
		~CommandAddEntity();
		void undo() override;
		void redo() override;
	};

	class CommandDeleteEntity :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		AbstractLevelEntity* m_entity;
		bool m_doDelete;

	public:
		CommandDeleteEntity(IWorld* world, AbstractLevelEntity* entity, QUndoCommand* parent = nullptr);
		~CommandDeleteEntity();
		void undo() override;
		void redo() override;
	};

	class CommandMoveEntity :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		Rectangle m_oldRect;
		Rectangle m_newRect;

		AbstractLevelEntity* m_entity;

	public:
		CommandMoveEntity(IWorld* world, const IRectangle& oldRect, const IRectangle& newRect, AbstractLevelEntity* entity, QUndoCommand* parent = nullptr);

		int id() const override { return 1; }
		void undo() override;
		void redo() override;
	};

	class CommandReshapeEntity :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		Rectangle m_oldRect;
		Rectangle m_newRect;
		AbstractLevelEntity* m_entity;

	public:
		CommandReshapeEntity(IWorld* world, const IRectangle& oldRect, const IRectangle& newRect, AbstractLevelEntity* entity, QUndoCommand* parent = nullptr);

		void undo() override;
		void redo() override;
	};

};
#endif
