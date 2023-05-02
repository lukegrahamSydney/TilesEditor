#ifndef TILESETH
#define TILESETH

#include <QVector>
#include <QString>
#include "Image.h"
#include "Resource.h"
#include "cJSON/cJSON.h"
#include "ResourceType.h"

namespace TilesEditor
{
	class ResourceManager;
	class Tileset :
		public Resource
	{
	private:
		QString m_imageName;

		Image* m_image;
		int m_hcount;
		int m_vcount;

		QVector<int> m_tileTypes;

	public:
		Tileset(const QString& resName);

		void release(ResourceManager& resourceManager);

		ResourceType getResourceType() const { return ResourceType::RESOURCE_TILESET; }

		void readFromJSONNode(cJSON* node);

		int getTileType(size_t index) const;
		int getTileType(int left, int top) const;
		const QString& getImageName() const { return m_imageName; }

		static Tileset* loadTileset(const QString& resName, const QString& fileName, ResourceManager& resourceManager);
	};
};

#endif
