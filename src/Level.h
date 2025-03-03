#ifndef LEVELH
#define LEVELH

#include <QMap>
#include <QString>
#include <QSet>
#include <QVector>
#include <QIODevice>
#include <QByteArray>
#include "AbstractSpatialGridItem.h"
#include "AbstractResourceManager.h"
#include "EntitySpatialGrid.h"
#include "Tilemap.h"
#include "AbstractLevelEntity.h"
#include "LevelNPC.h"
#include "LevelLink.h"
#include "LevelSign.h"
#include "IWorld.h"
#include "IFileRequester.h"
#include "Tileset.h"
#include "QObject"
#include "FileDataLoader.h"
#include "LoadState.h"
#include "sgscript/sgscript.h"
#include "TileDefs.h"

namespace TilesEditor
{
	class Overworld;
	

	class Level :
		public QObject,
		public AbstractSpatialGridItem
	{
		Q_OBJECT
	public slots:
		void loadFileData(QByteArray fileData);

	public:
		union LevelFlags {
			struct {
				bool canLayObjectInstance : 1;
				bool canLayAnonymousNPC : 1;
				bool autoEmbedCodeForNewObjectClass : 1;
			};
			unsigned char all = 0;
		};

	private:
		template <typename T>
		friend class EntitySpatialGrid;

		static sgs_Variable sgs_classMembers;
		static QMap<QString, int> m_imageDimensionsCache;

		static bool getImageDimensions(AbstractResourceManager* resourceManager, const QString& imageName, int* w, int* h);

		IWorld* m_world;
		sgs_Variable m_thisObject;
		sgs_Variable m_sgsUserTable;

		Tileset* m_defaultTileset;

		ScriptingLanguage m_defaultObjectLanguage = ScriptingLanguage::SCRIPT_UNDEFINED;
		bool m_modified;
		LevelFlags m_levelFlags;
		Overworld* m_overworld;
		
		QString m_name;
		QString m_fileName;
		QString m_tilesetName;
		QString m_tilesetImageName;

		LoadState m_loadState = LoadState::STATE_NOT_LOADED;


		int m_unitWidth;
		int m_unitHeight;
		Tilemap* m_mainTileLayer;
		QMap<int, Tilemap*>	m_tileLayers;
		QSet<AbstractLevelEntity*>	m_objects;
		QVector<LevelLink*>	m_links;
		QVector<LevelSign*> m_signs;

		IEntitySpatialMap<AbstractLevelEntity>* m_entitySpatialMap;
		QList<QPair<QRect, Image*>> m_tileDefs;

	public:

		Level(IWorld* world, double x, double y, int width, int height, Overworld* overworld, const QString& name);
		~Level();

		Tileset* getDefaultTileset() { return m_defaultTileset; }
		void setDefaultTileset(Tileset* tileset) { m_defaultTileset = tileset; }
		void release();
		IWorld* getWorld() { return m_world; }
		Overworld* getOverworld() { return m_overworld; }
		const QString& getTilesetName() const { return m_tilesetName; }
		void setTilesetName(const QString& name) { m_tilesetName = name; }
		void setTilesetImageName(const QString& name) { m_tilesetImageName = name; }
		const QString& getTilesetImageName() const { return m_tilesetImageName; }

		void setDefaultObjectLanguage(ScriptingLanguage scriptingLanguage) {
			m_defaultObjectLanguage = scriptingLanguage;
		}

		ScriptingLanguage getDefaultScriptingLanguage() const { return m_defaultObjectLanguage; }
		LevelFlags& getLevelFlags() { return m_levelFlags; }
		void setName(const QString& name) { m_name = name; }
		const QString& getName() const { return m_name; }
		void setFileName(const QString& fileName) { m_fileName = fileName; }
		const QString& getFileName() const { return m_fileName; }

		LoadState getLoadState() const { return m_loadState; }
		void setLoadState(LoadState state) { m_loadState = state; }
		bool loadFile(bool threaded);

		bool loadStream(QIODevice* stream);

		bool saveFile(IFileRequester* requester);
		bool saveStream(QIODevice* stream);


		double getX() const { return this->x(); }
		double getY() const { return this->y(); }

		void setSize(int width, int height);
		int getWidth() const { return this->width(); }
		int getHeight() const { return this->height(); }

		void setUnitWidth(int value) { m_unitWidth = value; }
		void setUnitHeight(int value) { m_unitHeight = value; }
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

		QRectF getRect() const {
			return QRectF(getX(), getY(), getWidth(), getHeight());
		}
		void deleteTileLayer(int index);
		QString getDisplayTile(int tile) const;
		QRectF clampEntity(AbstractLevelEntity* entity);
		void setTileLayer(int index, Tilemap* tilemap);

		void removeTileDefs();
		void addTileDef(const TileDef& tileDef);
		void drawTile(double x, double y, Image* tilesetImage, int tileLeft, int tileTop, QPainter* painter);
		void drawTilemap(Tilemap* tilemap, Image* tilesetImage, double x, double y, QPainter* painter, const QRectF& viewRect);
		void drawTileset(Image* image, const QColor& backColour, QPainter* painter, const QRectF& rect);

		static int convertFromGraalTile(int graalTileIndex, Tileset* defaultTileset) {
			int left = graalTileIndex & 0xF;
			int top = graalTileIndex >> 4;
			left = left + (16 * (top / 32));
			top = top % 32;

			return Tilemap::MakeTile(left, top, defaultTileset == nullptr ? 0 : defaultTileset->getTileType(left, top));
		}

		void mark(sgs_Context* ctx);

		sgs_Variable& getScriptThisObject() { return m_thisObject; }


		static sgs_ObjInterface sgs_interface;
		static void registerScriptClass(IEngine* engine);

	};
};
#endif
