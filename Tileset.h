#ifndef TILESETH
#define TILESETH

#include <QVector>
#include <QString>
#include <QStandardItem>
#include "Image.h"
#include "cJSON/cJSON.h"
#include "ResourceType.h"

namespace TilesEditor
{
	class ResourceManager;
	class Tileset:
		public QStandardItem
	{
	private:
		QString m_imageName;

		int m_hcount;
		int m_vcount;

		QVector<int> m_tileTypes;

	public:
		Tileset();
		void readFromJSONNode(cJSON* node);

		int getHCount() const { return m_hcount; }
		int getVCount() const { return m_vcount; }
		int getTileType(size_t index) const;
		int getTileType(int left, int top) const;

		void setTileType(int left, int top, int type);

		const QString& getImageName() const { return m_imageName; }
		void setImageName(const QString& name) { m_imageName = name; }
		void loadFromFile(const QString& fileName);
		void saveToFile(const QString& fileName);
		cJSON* serializeJSON();

		static Tileset* loadTileset(const QString& resName, const QString& fileName, ResourceManager& resourceManager);
	};
};

#endif
