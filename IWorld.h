#ifndef IWORLDH
#define IWORLDH

#include <QSet>
#include <QUndoCommand>
#include <QList>
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
		struct TileInfo {
			unsigned short tileX;
			unsigned short tileY;
			int tile;
		};

		virtual QSet<Level*> getLevelsInRect(const IRectangle& rect) = 0;
		virtual Level* getLevelAt(double x, double y) = 0;
		virtual ResourceManager& getResourceManager() = 0;
		virtual AbstractLevelEntity* getEntityAt(double x, double y) = 0;
		virtual QList<AbstractLevelEntity*> getEntitiesAt(double x, double y) = 0;
		virtual bool tryGetTileAt(double x, double y, int* outTile) = 0;
		//virtual void setTileAt(double x, double y, int tile) = 0;
		virtual void deleteEntity(AbstractLevelEntity* entity, QUndoCommand* parent = nullptr) = 0;
		virtual void deleteEntities(const QList<AbstractLevelEntity*>& entities, QUndoCommand* parent = nullptr) = 0;
		virtual bool containsLevel(const QString& levelName)const = 0;
		virtual void centerLevel(const QString& levelName) = 0;
		virtual void setModified(Level* level) = 0;
		virtual void updateMovedEntity(AbstractLevelEntity* entity) = 0;
		virtual QList<Level*> getModifiedLevels() = 0;

		virtual void getTiles(double x, double y, int layer, Tilemap* output) = 0;
		virtual void putTiles(double x, double y, int layer, Tilemap* input, bool ignoreInvisible) = 0;
		virtual void deleteTiles(double x, double y, int layer, int hcount, int vcount, int replacementTile) = 0;
		virtual int floodFillPattern(double x, double y, int layer, const Tilemap* pattern, QList<QPair<unsigned short, unsigned short> >* outputNodes = nullptr) = 0;
		virtual void floodFillPattern2(double x, double y, int layer, const Tilemap* pattern, QList<TileInfo>* outputNodes = nullptr) = 0;
		virtual void addUndoCommand(QUndoCommand* command) = 0;
	};
};
#endif
