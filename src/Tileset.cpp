#include <QStringList>
#include <QFile>
#include <QTextStream>
#include "Tileset.h"
#include "StringTools.h"

#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
	Tileset::Tileset()
	{
		m_hcount = m_vcount = 0;
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

	void Tileset::setTileType(int left, int top, int type)
	{
		size_t index = (top * m_hcount) + left;
		if (index < m_tileTypes.size())
			m_tileTypes[index] = type;
	}

	void Tileset::resize(int hcount, int vcount)
	{
		QVector<int> newTileTypes(hcount * vcount);

		for (auto y = 0; y < std::min(m_vcount, vcount); ++y)
		{
			for (auto x = 0; x < std::min(m_hcount, hcount); ++x)
			{
				newTileTypes[y * hcount + x] = getTileType(x, y);

			}
		}

		m_hcount = hcount;
		m_vcount = vcount;
		m_tileTypes = newTileTypes;
	}

	void Tileset::loadFromFile(const QString& fileName)
	{
		QFile f(fileName);
		if (f.open(QFile::ReadOnly | QFile::Text))
		{
			auto text = QTextStream(&f).readAll();

			auto cJSON = cJSON_Parse(text.toLocal8Bit().data());
			if (cJSON != nullptr)
			{
				readFromJSONNode(cJSON);

				cJSON_Delete(cJSON);

			}
		}
	}

	void Tileset::saveToFile()
	{
		saveToFile(getFileName());
	}

	void Tileset::saveToFile(const QString& fileName)
	{
		QFile f(fileName);
		if (f.open(QFile::WriteOnly | QFile::Text))
		{
			auto json = serializeJSON();

			if (json)
			{
				auto text = cJSON_Print(json);
				QTextStream(&f) << text;

				free(text);
				cJSON_Delete(json);
			}

		}
	}

	cJSON* Tileset::serializeJSON()
	{
		auto jsonRoot = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonRoot, "defaultImage", m_imageName.toLocal8Bit().data());
		cJSON_AddNumberToObject(jsonRoot, "hcount", m_hcount);
		cJSON_AddNumberToObject(jsonRoot, "vcount", m_vcount);

		QStringList values;

		auto count = m_hcount * m_vcount;
		for (auto i = 0; i < count; ++i)
		{
			auto value = getTileType(i);
			values.push_back(QString::number(value, 16));
		}

		cJSON_AddStringToObject(jsonRoot, "defaultTileTypes", values.join(" ").toLocal8Bit().data());
		return jsonRoot;
	}


	Tileset* Tileset::loadTileset(const QString& name, AbstractResourceManager* resourceManager)
	{
		if (name.endsWith(".png")) {
			return new Tileset(name);
		}
		else {
			QString fullPath;

			if (resourceManager->locateFile(name + ".json", &fullPath))
			{
				auto text = resourceManager->readAllToString(fullPath);

				auto cJSON = cJSON_Parse(text.toLocal8Bit().data());
				if (cJSON != nullptr)
				{
					auto tileset = new Tileset();
					tileset->setFileName(fullPath);
					tileset->readFromJSONNode(cJSON);

					cJSON_Delete(cJSON);

					tileset->setText(name);
					return tileset;

				}
			}
			
		}

		return nullptr;
	}

};
