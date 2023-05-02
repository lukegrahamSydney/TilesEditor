#ifndef LEVELH
#define LEVELH

#include <QMap>
#include <QString>
#include <QSet>
#include <QVector>
#include "ISpatialMapItem.h"
#include "ResourceManager.h"
#include "EntitySpatialGrid.h"
#include "Tilemap.h"
#include "AbstractLevelEntity.h"
#include "LevelNPC.h"
#include "LevelLink.h"
#include "LevelSign.h"

namespace TilesEditor
{
	class Overworld;
	class Level :
		public ISpatialMapItem
	{
	public:

	private:
		template <typename T>
		friend class EntitySpatialGrid;

		static QMap<QString, int> m_imageDimensionsCache;

		static bool getImageDimensions(ResourceManager& resourceManager, const QString& imageName, int* w, int* h);

		bool m_modified;

		Overworld* m_overworld;
		
		QString m_name;
		QString m_fileName;

		bool m_loaded;

		double m_x;
		double m_y;
		int m_width;
		int m_height;

		Tilemap* m_mainTileLayer;
		QMap<int, Tilemap*>	m_tileLayers;
		QSet<AbstractLevelEntity*>	m_objects;
		QVector<LevelLink*>	m_links;
		QVector<LevelSign*> m_signs;

		IEntitySpatialMap<AbstractLevelEntity>* m_entitySpatialMap;

		void drawTilesLayer(QPainter* painter, const IRectangle& viewRect, Image* tilesetImage, int index, bool fade);

	public:

		Level(double x, double y, int width, int height, Overworld* overworld, const QString& name);
		~Level();

		void release(ResourceManager& resourceManager);

		void setName(const QString& name) { m_name = name; }
		const QString& getName() const { return m_name; }
		void setFileName(const QString& fileName) { m_fileName = fileName; }
		const QString& getFileName() const { return m_fileName; }
		bool loadNWFile(ResourceManager& resourceManager);
		bool saveNWFile();

		bool getLoaded() const { return m_loaded; }
		void setLoaded(bool val) { m_loaded = val; }
		double getX() const { return m_x; }
		double getY() const { return m_y; }
		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }

		double getUnitWidth() const { return 16.0; }
		double getUnitHeight() const { return 16.0; }
		void drawAllTileLayers(QPainter* painter, const IRectangle& viewRect, Image* tilesetImage, int selectedLayer, QMap<int, bool>& visibleLayers);

		AbstractLevelEntity* getObjectAt(double x, double y, LevelEntityType type);
		void setModified(bool value) { m_modified = value; }
		bool getModified() const { return m_modified; }
		QVector<LevelLink*>& getLinks() { return m_links; }
		QVector<LevelSign*>& getSigns() { return m_signs; }
		Tilemap* getOrMakeTilemap(int layer, ResourceManager& resourceManager);
		Tilemap* getTilemap(size_t layer);
		IEntitySpatialMap<AbstractLevelEntity>* getEntitySpatialMap() { return m_entitySpatialMap; }
		QSet<AbstractLevelEntity*>& getObjects() { return m_objects; }
		void updateSpatialEntity(AbstractLevelEntity* entity);
		void addObject(AbstractLevelEntity* object);
		void addEntityToSpatialMap(AbstractLevelEntity* object);
		void removeObject(AbstractLevelEntity* object);
		void removeEntityFromSpatialMap(AbstractLevelEntity* object);

		void clampEntity(AbstractLevelEntity* entity);
		void setTileLayer(int index, Tilemap* tilemap);

		static int convertFromGraalTile(int graalTileIndex) {
			int left = graalTileIndex & 0xF;
			int top = graalTileIndex >> 4;
			left = left + (16 * (top / 32));
			top = top % 32;

			return Tilemap::MakeTile(left, top, 0);
		}
	};
};
#endif
