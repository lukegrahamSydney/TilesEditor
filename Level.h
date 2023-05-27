#ifndef LEVELH
#define LEVELH

#include <QMap>
#include <QString>
#include <QSet>
#include <QVector>
#include <QIODevice>
#include "ISpatialMapItem.h"
#include "ResourceManager.h"
#include "EntitySpatialGrid.h"
#include "Tilemap.h"
#include "AbstractLevelEntity.h"
#include "LevelNPC.h"
#include "LevelLink.h"
#include "LevelSign.h"
#include "IWorld.h"
#include "IFileRequester.h"

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

		IWorld* m_world;

		bool m_modified;

		Overworld* m_overworld;
		
		QString m_name;
		QString m_fileName;
		QString m_tilesetName;

		bool m_loaded;
		bool m_loadFail;
		double m_x;
		double m_y;
		int m_width;
		int m_height;

		int m_unitWidth;
		int m_unitHeight;
		Tilemap* m_mainTileLayer;
		QMap<int, Tilemap*>	m_tileLayers;
		QSet<AbstractLevelEntity*>	m_objects;
		QVector<LevelLink*>	m_links;
		QVector<LevelSign*> m_signs;

		IEntitySpatialMap<AbstractLevelEntity>* m_entitySpatialMap;


	public:

		Level(IWorld* world, double x, double y, int width, int height, Overworld* overworld, const QString& name);
		~Level();

		void release();

		const QString& getTilesetName() const { return m_tilesetName; }
		void setTilesetName(const QString& name) { m_tilesetName = name; }

		void setName(const QString& name) { m_name = name; }
		const QString& getName() const { return m_name; }
		void setFileName(const QString& fileName) { m_fileName = fileName; }
		const QString& getFileName() const { return m_fileName; }

		bool loadFile();

		bool loadStream(QIODevice* stream);
		bool loadNWStream(QIODevice* stream);
		bool loadGraalStream(QIODevice* stream);
		bool loadLVLStream(QIODevice* stream);

		bool saveFile(IFileRequester* requester);
		bool saveStream(QIODevice* stream);
		bool saveNWStream(QIODevice* stream);
		bool saveGraalStream(QIODevice* stream);
		bool saveLVLStream(QIODevice* stream);

		bool getLoaded() const { return m_loaded; }
		void setLoaded(bool val) { m_loaded = val; }

		void setLoadFail(bool value) { m_loadFail = value; }
		bool getLoadFail() const { return m_loadFail; }
		double getX() const { return m_x; }
		double getY() const { return m_y; }
		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }

		int getUnitWidth() const { return m_unitWidth; }
		int getUnitHeight() const { return m_unitHeight; }
		int getTileWidth() const { return 16; }
		int getTileHeight() const { return 16; }

		AbstractLevelEntity* getObjectAt(double x, double y, LevelEntityType type);
		void setModified(bool value) { m_modified = value; }
		bool getModified() const { return m_modified; }
		const QVector<LevelLink*>& getLinks()const { return m_links; }
		const QVector<LevelSign*>& getSigns() const { return m_signs; }
		Tilemap* getOrMakeTilemap(int layer);
		Tilemap* getTilemap(size_t layer);
		IEntitySpatialMap<AbstractLevelEntity>* getEntitySpatialMap() { return m_entitySpatialMap; }
		const QSet<AbstractLevelEntity*>& getObjects() const { return m_objects; }

		const QMap<int, Tilemap*>& getTileLayers() const { return m_tileLayers; }
		void updateSpatialEntity(AbstractLevelEntity* entity);
		void addObject(AbstractLevelEntity* object);
		void addEntityToSpatialMap(AbstractLevelEntity* object);
		void removeObject(AbstractLevelEntity* object);
		void removeEntityFromSpatialMap(AbstractLevelEntity* object);

		QString getDisplayTile(int tile) const;
		Rectangle clampEntity(AbstractLevelEntity* entity);
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
