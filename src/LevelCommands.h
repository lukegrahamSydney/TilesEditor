#ifndef LEVELCOMMANDSH
#define LEVELCOMMANDSH

#include <QUndoCommand>
#include <IWorld.h>
#include <QList>
#include <QPair>
#include <QMap>
#include <QVariant>
#include "Tilemap.h"

namespace TilesEditor
{
	enum LevelCommandType
	{
		LEVEL_COMMAND_MOVE_ENTITIES,
		LEVEL_COMMAND_SET_ENTITY_PROPERTY
	};

	class CommandDeleteTiles:
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		double m_x;
		double m_y;
		int m_layer;
		Tilemap m_oldTiles;
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
		bool m_applyNewTranslucency;
		Tilemap m_oldTiles;
		Tilemap m_newTiles;

	public:
		CommandPutTiles(IWorld* world, double x, double y, int layer, const Tilemap* oldTiles, const Tilemap* newTiles, bool applyNewTranslucency);
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

		Tilemap* m_pattern;
		QList<IWorld::TileInfo> m_nodes;

	public:
		CommandFloodFillPattern(IWorld* world, double x, double y, int layer, const Tilemap* pattern);
		~CommandFloodFillPattern() { delete m_pattern; }
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
		QRectF m_oldRect;
		QRectF m_newRect;

		AbstractLevelEntity* m_entity;

	public:
		CommandMoveEntity(IWorld* world, const QRectF& oldRect, const QRectF& newRect, AbstractLevelEntity* entity, QUndoCommand* parent = nullptr);

		void undo() override;
		void redo() override;
	};

	class CommandMoveEntities :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		QMap<AbstractLevelEntity*, QRectF> m_newPositions;
		QMap<AbstractLevelEntity*, QRectF> m_oldPositions;

	public:
		CommandMoveEntities(IWorld* world, const QMap<AbstractLevelEntity*, QRectF>& oldPositions, const QMap<AbstractLevelEntity*, QRectF>& newPositions, QUndoCommand* parent = nullptr) :
			QUndoCommand("Move Object/s", parent), m_world(world), m_oldPositions(oldPositions), m_newPositions(newPositions) {}

		void undo() override;

		void redo() override;

		int id() const override { return LEVEL_COMMAND_MOVE_ENTITIES; }
		bool mergeWith(const QUndoCommand* command) override;
	};

	class CommandReshapeEntity :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		QRectF m_oldRect;
		QRectF m_newRect;
		AbstractLevelEntity* m_entity;

	public:
		CommandReshapeEntity(IWorld* world, const QRectF& oldRect, const QRectF& newRect, AbstractLevelEntity* entity, QUndoCommand* parent = nullptr);

		void undo() override;
		void redo() override;
	};

	class CommandSetWorldProperty :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		QString m_propName;
		QVariant  m_newValue;
		QVariant  m_oldValue;

	public:
		CommandSetWorldProperty(IWorld* world, const QString& propName, const QVariant& newValue, const QVariant& oldValue, QUndoCommand* parent) :
			QUndoCommand(parent), m_world(world), m_propName(propName), m_newValue(newValue), m_oldValue(oldValue) {}

		void undo() override {
			m_world->setProperty(m_propName, m_oldValue);
			m_world->setModified(nullptr);
		}
		void redo() override {
			m_world->setProperty(m_propName, m_newValue);
			m_world->setModified(nullptr);
		}
	};

	class CommandSetEntityProperty :
		public QUndoCommand
	{
	private:
		IWorld* m_world;
		AbstractLevelEntity* m_entity;
		QString m_propName;
		QVariant  m_newValue;
		QVariant  m_oldValue;

	public:
		CommandSetEntityProperty(IWorld* world, AbstractLevelEntity* entity, const QString& propName, const QVariant& newValue, const QVariant& oldValue, QUndoCommand* parent) :
			QUndoCommand(parent), m_world(world), m_entity(entity), m_propName(propName), m_newValue(newValue), m_oldValue(oldValue) {}

		void undo() override {
			m_world->setEntityProperty(m_entity, m_propName, m_oldValue);
			m_world->setModified(m_entity->getLevel());
		}
		void redo() override {
			m_world->setEntityProperty(m_entity, m_propName, m_newValue);
			m_world->setModified(m_entity->getLevel());
		}

		int id() const override { return LEVEL_COMMAND_SET_ENTITY_PROPERTY; }
		bool mergeWith(const QUndoCommand* command) override
		{
			auto other = static_cast<const CommandSetEntityProperty*>(command);

			if (this->m_entity == other->m_entity && other->m_propName == this->m_propName)
			{
				if (other->m_newValue == this->m_oldValue)
					this->setObsolete(true);

				this->m_newValue = other->m_newValue;

				return true;
			}
			return false;
		}
	};

};
#endif
