#ifndef OVERWORLDH
#define OVERWORLDH

#include <QString>
#include <QMap>
#include <QList>
#include <QSet>
#include <QIODevice>
#include <QRect>
#include "IEntitySpatialMap.h"
#include "Level.h"
#include "AbstractLevelEntity.h"
#include "IWorld.h"
#include "IFileRequester.h"

namespace TilesEditor
{
	class Overworld
	{
	private:
		IWorld* m_world;

		QString m_name;
		QString m_fileName;
		QString m_tilesetName;
		QString m_tilesetImageName;
		QStringList m_gmapFileLines;
		 
		cJSON* m_json;

		int m_width;
		int m_height;

		int m_unitWidth;
		int m_unitHeight;

		IEntitySpatialMap<Level>* m_levelMap;
		IEntitySpatialMap<AbstractLevelEntity>* m_entitySpatialMap;
		QMap<QString, Level*> m_levelNames;

	public:
		Overworld(IWorld* world, const QString& name);
		~Overworld();

		void setFileName(const QString& name) { m_fileName = name; }
		const QString& getFileName() const { return m_fileName; }
		void release();

		const QString& getName() const { return m_name; }

		const QString& getTilesetName() const { return m_tilesetName; }
		void setTilesetName(const QString& name) { m_tilesetName = name; }

		void setTilesetImageName(const QString& name) { m_tilesetImageName = name; }
		const QString& getTilesetImageName() const { return m_tilesetImageName; }

		bool loadFile();
		bool loadStream(QIODevice* stream);
		bool loadGMapStream(QIODevice* stream);
		bool loadTXTStream(QIODevice* stream);
		bool loadWorldStream(QIODevice* stream);

		bool saveFile(IFileRequester* requester);
		bool saveStream(QIODevice* stream);
		bool saveGMapStream(QIODevice* stream);
		bool saveTXTStream(QIODevice* stream);
		bool saveWorldStream(QIODevice* stream);

		void setSize(int width, int height);
		void searchLevels(const QRectF& rect, QSet<Level*>& output);

		void updateObjectMoved(AbstractLevelEntity* entity);

		void addEntityToSpatialMap(AbstractLevelEntity* entity);
		void removeEntityFromSpatialMap(AbstractLevelEntity* entity);
		int getTileAt(int tileDepth, const QPointF& point);
		void preloadLevels();

		bool containsLevel(const QString& name) const;
		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }

		int getUnitWidth() const { return m_unitWidth; }
		int getUnitHeight() const { return m_unitHeight; }

		int getTileWidth() const { return 16; }
		int getTileHeight() const { return 16; }
		IEntitySpatialMap<AbstractLevelEntity>* getEntitySpatialMap() { return m_entitySpatialMap; }
		Level* getLevel(const QString& levelName);
		Level* getLevelAt(const QPointF& point);

		QList<Level*> getModifiedLevels();
		QMap<QString, Level*>& getLevelList() { return m_levelNames; }
	};
};

#endif
