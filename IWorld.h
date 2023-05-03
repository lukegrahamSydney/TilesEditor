#ifndef IWORLDH
#define IWORLDH

#include <QSet>
#include <QUndoCommand>
#include "Rectangle.h"
#include "ResourceManager.h"

namespace TilesEditor
{
	class Level;
	class Tilemap;
	class AbstractLevelEntity;
	class IWorld
	{
	public:

		virtual QSet<Level*> getLevelsInRect(const IRectangle& rect) = 0;
		virtual Level* getLevelAt(double x, double y) = 0;
		virtual ResourceManager& getResourceManager() = 0;
		virtual AbstractLevelEntity* getEntityAt(double x, double y) = 0;
		virtual QList<AbstractLevelEntity*> getEntitiesAt(double x, double y) = 0;
		virtual bool tryGetTileAt(double x, double y, int* outTile) = 0;
		//virtual void setTileAt(double x, double y, int tile) = 0;
		virtual void deleteEntity(AbstractLevelEntity* entity) = 0;
		virtual bool containsLevel(const QString& levelName)const = 0;
		virtual void centerLevel(const QString& levelName) = 0;
		virtual void setModified(Level* level) = 0;
		virtual void updateMovedEntity(AbstractLevelEntity* entity) = 0;
		virtual QList<Level*> getModifiedLevels() = 0;

		virtual void getTiles(double x, double y, int layer, Tilemap* output, bool deleteTiles = false) = 0;
		virtual void putTiles(double x, double y, int layer, Tilemap* input) = 0;
		virtual int floodFill(double x, double y, int newTile) = 0;
		virtual void addUndoCommand(QUndoCommand* command) = 0;
	};
};
#endif
