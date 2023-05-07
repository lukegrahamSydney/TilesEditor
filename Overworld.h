#ifndef OVERWORLDH
#define OVERWORLDH

#include <QString>
#include <QMap>
#include <QList>
#include <QSet>
#include "IEntitySpatialMap.h"
#include "Rectangle.h"
#include "Level.h"
#include "AbstractLevelEntity.h"
#include "ResourceManager.h"

namespace TilesEditor
{
	class Overworld
	{
	private:
		QString m_name;
		QString m_fileName;
		QString m_tilesetName;
		QStringList m_gmapFileLines;

		int m_width;
		int m_height;

		IEntitySpatialMap<Level>* m_levelMap;
		IEntitySpatialMap<AbstractLevelEntity>* m_entitySpatialMap;
		QMap<QString, Level*> m_levelNames;

	public:
		Overworld(const QString& name);
		~Overworld();

		void setFileName(const QString& name) { m_fileName = name; }
		const QString& getFileName() const { return m_fileName; }
		void release(ResourceManager& resourceManager);

		const QString& getName() const { return m_name; }

		const QString& getTilesetName() const { return m_tilesetName; }
		void setTilesetName(const QString& name) { m_tilesetName = name; }

		void loadGMap(ResourceManager& resourceManager);
		bool saveGMap();

		void setSize(int width, int height);
		void searchLevels(const IRectangle& rect, QSet<Level*>& output);

		void updateObjectMoved(AbstractLevelEntity* entity);

		void addEntityToSpatialMap(AbstractLevelEntity* entity);
		void removeEntityFromSpatialMap(AbstractLevelEntity* entity);
		int getTileAt(int tileDepth, double x, double y);
		void preloadLevels(ResourceManager& resourceManager);

		bool containsLevel(const QString& name) const;
		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; }
		IEntitySpatialMap<AbstractLevelEntity>* getEntitySpatialMap() { return m_entitySpatialMap; }
		Level* getLevel(const QString& levelName);
		Level* getLevelAt(double x, double y);

		QList<Level*> getModifiedLevels();
		QMap<QString, Level*>& getLevelList() { return m_levelNames; }
	};
};

#endif
