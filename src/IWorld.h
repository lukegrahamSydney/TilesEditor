#ifndef IWORLDH
#define IWORLDH

#include <QSet>
#include <QUndoCommand>
#include <QList>
#include <QRect>
#include "AbstractResourceManager.h"
#include "Image.h"


namespace TilesEditor
{
	class Level;
	class Tilemap;
	class AbstractLevelEntity;
	class IEngine;
	
	class IWorld
	{
	public:
		struct TileInfo {
			unsigned short tileX;
			unsigned short tileY;
			int tile;
		};

		virtual QSet<Level*> getLevelsInRect(const QRectF& rect, bool threaded = true) = 0;
		virtual Level* getLevelAt(const QPointF& point) = 0;
		virtual AbstractResourceManager* getResourceManager() = 0;
		virtual AbstractLevelEntity* getEntityAt(const QPointF& point) = 0;
		virtual QList<AbstractLevelEntity*> getEntitiesAt(const QPointF& point) = 0;
		virtual QSet<AbstractLevelEntity*> getEntitiesInRect(const QRectF& rect) = 0;
		virtual bool tryGetTileAt(const QPointF& point, int* outTile) = 0;
		//virtual void setTileAt(double x, double y, int tile) = 0;
		virtual void deleteEntity(AbstractLevelEntity* entity, QUndoCommand* parent = nullptr) = 0;
		virtual void deleteEntities(const QList<AbstractLevelEntity*>& entities, QUndoCommand* parent = nullptr) = 0;
		virtual bool containsLevel(const QString& levelName)const = 0;
		virtual void centerLevel(const QString& levelName) = 0;
		virtual void setModified(Level* level) = 0;

		//Call this when the x/y changes
		virtual void updateMovedEntity(AbstractLevelEntity* entity) = 0;

		//Call this when the width/height changes
		virtual void updateEntityRect(AbstractLevelEntity* entity) = 0;

		virtual QList<Level*> getModifiedLevels() = 0;

		virtual int getUnitWidth() const = 0;
		virtual int getUnitHeight() const = 0;

		virtual int getWidth() const = 0;
		virtual int getHeight() const = 0;



		virtual void setProperty(const QString& name, const QVariant& value) = 0;
		virtual int getTileTranslucency() const = 0;
		virtual int getDefaultTile() const = 0;
		virtual Image* getTilesetImage() = 0;
		virtual void getTiles(const QPointF& point, int layer, Tilemap* output) = 0;
		virtual void putTiles(const QPointF& point, int layer, Tilemap* input, bool ignoreInvisible, bool applyTranslucency) = 0;
		virtual void deleteTiles(const QPointF& point, int layer, int hcount, int vcount, int replacementTile) = 0;
		virtual void floodFillPattern(const QPointF& point, int layer, const Tilemap* pattern, QList<TileInfo>* outputNodes = nullptr) = 0;
		virtual void addUndoCommand(QUndoCommand* command) = 0;
		virtual void removeEntitySelection(AbstractLevelEntity* entity) = 0;
		virtual void setEntityProperty(AbstractLevelEntity* entity, const QString& name, const QVariant& value) = 0;

		virtual void removeTileDefs(const QString& prefix) = 0;
		virtual void redrawScene(const QRectF& rect) = 0;
		virtual void redrawScene() = 0;

		virtual IEngine* getEngine() = 0;
	};
};
#endif
