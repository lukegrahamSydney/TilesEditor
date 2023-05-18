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
		QString m_fileName;
		QString m_imageName;

		int m_hcount;
		int m_vcount;

		QVector<int> m_tileTypes;
		bool m_editable;

	public:
		Tileset();
		Tileset(const QString& imageName) { m_imageName = imageName; this->setText(m_imageName); }

		void setFileName(const QString& name) { m_fileName = name; }
		const QString& getFileName() const { return m_fileName; }
		void readFromJSONNode(cJSON* node);

		bool hasTileTypes() const { return m_tileTypes.size() > 0; }
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

		static Tileset* loadTileset(const QString& name, ResourceManager& resourceManager);
	};
};

#endif
