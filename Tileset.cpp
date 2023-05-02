#include <QStringList>
#include "Tileset.h"
#include "StringTools.h"
#include "ResourceManager.h"
#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
	Tileset::Tileset(const QString& resName) :
		Resource(resName)
	{
		m_image = nullptr;
		m_hcount = m_vcount = 0;
	}

	void Tileset::release(ResourceManager& resourceManager)
	{
		if (m_image != nullptr)
			resourceManager.freeResource(m_image);
	}





	void Tileset::readFromJSONNode(cJSON* node)
	{
		m_imageName = jsonGetChildString(node, "defaultImage", "");
		m_hcount = jsonGetChildInt(node, "hcount", 0);
		m_vcount = jsonGetChildInt(node, "vcount", 0);

		m_tileTypes.clear();
		
		auto tileTypes = jsonGetChildString(node, "defaultTileTypes", "");

		QStringList values = tileTypes.split(' ');
		
		for (auto i = 0U; i < values.size(); ++i)
		{
			int val = values[i].toInt(nullptr, 16);
			m_tileTypes.push_back(val);
		}
	}

	int Tileset::getTileType(size_t index) const
	{
		if (index < m_tileTypes.size())
			return m_tileTypes[index];
		return 0;
	}

	int Tileset::getTileType(int left, int top) const
	{
		size_t index = (top * m_hcount) + left;

		return getTileType(index);
	}


	Tileset* Tileset::loadTileset(const QString& resName, const QString& fileName, ResourceManager& resourceManager)
	{
		
		auto text = resourceManager.getFileSystem().readAllToString(fileName);
		QByteArray ba = text.toLocal8Bit();

		auto cJSON = cJSON_Parse(ba.data());
		if (cJSON != nullptr)
		{
			auto tileset = new Tileset(resName);
			tileset->readFromJSONNode(cJSON);

			cJSON_Delete(cJSON);
			return tileset;

		}
		return nullptr;
	}

};
