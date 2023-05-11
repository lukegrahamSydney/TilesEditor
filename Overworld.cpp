#include <QDebug>
#include <QFile>
#include <iterator>
#include "Overworld.h"
#include "MainFileSystem.h"
#include "Level.h"
#include "EntitySpatialGrid.h"
#include "StringTools.h"
#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
	Overworld::Overworld(const QString& name)
	{
		m_json = nullptr;
		m_name = name;
		m_entitySpatialMap = nullptr;
		m_levelMap = nullptr;

		m_unitWidth = m_unitHeight = 16;
	}

	Overworld::~Overworld()
	{
		for (auto level : m_levelNames)
			delete level;

		if (m_entitySpatialMap)
			delete m_entitySpatialMap;

		if (m_levelMap)
			delete m_levelMap;

		if (m_json)
			cJSON_Delete(m_json);

	}

	void Overworld::release(ResourceManager& resourceManager)
	{

		for (auto level : m_levelNames)
			level->release(resourceManager);
	}


	bool Overworld::loadFile(ResourceManager& resourceManager)
	{
		if (m_fileName.endsWith(".gmap"))
			return loadGMapFile(resourceManager);

		else if (m_fileName.endsWith(".world"))
			return loadWorldFile(resourceManager);
		return false;
	}

	bool Overworld::loadGMapFile(ResourceManager& resourceManager)
	{
		QStringList lines;

		if (resourceManager.getFileSystem().readAllLines(m_fileName, lines) && lines.size() > 0)
		{
		
			int width = 0,
				height = 0;

			for (auto i = 1U; i < lines.size(); ++i)
			{
				auto& line = lines[i];
				size_t wordCount;

				auto words = line.split(' ');
			
				if (!words.isEmpty())
				{
					wordCount = words.size();

					if (words[0] == "TILESET" && wordCount >= 2) {
						m_tilesetName = words[1];
					}
					else m_gmapFileLines.append(line);

					if (words[0] == "WIDTH" && wordCount >= 2) {
						width = words[1].toInt();
					}
					else if (words[0] == "HEIGHT" && wordCount >= 2) {
						height = words[1].toInt();
					}

					else if (words[0] == "LEVELNAMES")
					{
						setSize(width * 16 * 64, height * 16 * 64);

						int y = 0;
						for (++i; i < lines.size(); ++i)
						{
							auto& levelNameLine = lines[i];
							m_gmapFileLines.append(levelNameLine);

							if (levelNameLine == "LEVELNAMESEND")
								break;


							size_t namesCount = 0;
							QStringList namesList;

							if ((namesCount = StringTools::ParseCSV(levelNameLine, namesList)) > 0)
							{
								for (size_t x = 0; x < namesList.count(); ++x)
								{
									auto& levelName = namesList[x];

									if (levelName != "")
									{
										auto levelX = x * (64.0 * 16);
										auto levelY = y * (64.0 * 16);

										auto level = new Level(levelX, levelY, 64 * 16, 64 * 16, this, levelName);

										m_levelMap->add(level);

										m_levelNames[levelName] = level;
									}
								}
							}
							++y;
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	bool Overworld::loadWorldFile(ResourceManager& resourceManager)
	{
		if (m_json) {
			cJSON_Delete(m_json);
			m_json = nullptr;
		}

		QString text = resourceManager.getFileSystem().readAllToString(m_fileName);
		if (!text.isEmpty())
		{
			m_json = cJSON_Parse(text.toLocal8Bit().data());

			if (m_json)
			{
				auto type = jsonGetChildString(m_json, "type");
				auto version = jsonGetChildString(m_json, "version");


				if (type == "overworld" && version == "1.0")
				{
					m_unitWidth = m_unitHeight = 1;
					auto width = jsonGetChildInt(m_json, "width", 1) * 16;
					auto height = jsonGetChildInt(m_json, "height", 1) * 16;

					auto defaultLevelWidth = jsonGetChildInt(m_json, "defaultLevelWidth", 1) * 16;
					auto defaultLevelHeight = jsonGetChildInt(m_json, "defaultLevelHeight", 1) * 16;

					setSize(width, height);

					auto jsonLevels = cJSON_GetObjectItem(m_json, "levels");
					if (jsonLevels)
					{
						auto nextLevelX = 0.0;
						auto nextLevelY = 0.0;

						for (auto row = 0; row < cJSON_GetArraySize(jsonLevels); ++row)
						{
							auto jsonRow = cJSON_GetArrayItem(jsonLevels, row);

							if (jsonRow)
							{
								for (auto column = 0; column < cJSON_GetArraySize(jsonRow); ++column)
								{
									auto jsonItem = cJSON_GetArrayItem(jsonRow, column);

									if (jsonItem->type == cJSON_String)
									{
										auto levelName = QString(jsonItem->valuestring);

										auto levelWidth = defaultLevelWidth;
										auto levelHeight = defaultLevelHeight;


										auto level = new Level(nextLevelX, nextLevelY, levelWidth, levelHeight, this, levelName);
										m_levelMap->add(level);
										m_levelNames[levelName] = level;
										nextLevelX += levelWidth;
									}
									else if (jsonItem->type == cJSON_Object)
									{
										auto levelName = jsonGetChildString(jsonItem, "name");
										auto levelWidth = jsonGetChildInt(jsonItem, "width") * 16;
										auto levelHeight = jsonGetChildInt(jsonItem, "height") * 16;
										auto level = new Level(nextLevelX, nextLevelY, levelWidth, levelHeight, this, levelName);
										m_levelMap->add(level);
										m_levelNames[levelName] = level;
										nextLevelX += levelWidth;
									}
								}
								nextLevelX = 0.0;
								nextLevelY += defaultLevelHeight;
							}
						}
					}

					return true;
				}
				cJSON_Delete(m_json);
				m_json = nullptr;
			}


		}
		return false;
	}

	bool Overworld::saveFile()
	{
		if (m_fileName.endsWith(".gmap"))
			return saveGMapFile();

		else if (m_fileName.endsWith(".world"))
			return saveWorldFile();
		return false;
	}

	bool Overworld::saveGMapFile()
	{
		QFile file(m_fileName);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream stream(&file);

			stream << "GRMAP001" << Qt::endl;

			for (auto& line : m_gmapFileLines)
			{
				stream << line << Qt::endl;
			}

			if(!m_tilesetName.isEmpty())
				stream << "TILESET " << m_tilesetName << Qt::endl;
		}

		return false;
	}

	bool Overworld::saveWorldFile()
	{
		if (m_json)
		{
			QFile file(m_fileName);
			if (file.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				cJSON_DeleteItemFromObject(m_json, "tileset");
				if (!m_tilesetName.isEmpty())
					cJSON_AddStringToObject(m_json, "tileset", m_tilesetName.toLocal8Bit().data());

				auto levelText = cJSON_Print(m_json);
				QTextStream stream(&file);
				stream << levelText;
				free(levelText);

				return true;
			}
		}
		return false;
	}

	void Overworld::setSize(int width, int height)
	{
		m_width = width;
		m_height = height;
		m_levelMap = new EntitySpatialGrid<Level>(0.0, 0.0, width, height, 64 * 16, 64 * 16);
		m_entitySpatialMap = new EntitySpatialGrid<AbstractLevelEntity>(0.0, 0.0, width, height);
	}

	void Overworld::searchLevels(const IRectangle& rect, QSet<Level*>& output)
	{
		m_levelMap->search(rect, false, output);
	}



	void Overworld::updateObjectMoved(AbstractLevelEntity* entity)
	{
		m_entitySpatialMap->updateEntity(entity);
	}

	void Overworld::addEntityToSpatialMap(AbstractLevelEntity* entity)
	{
		m_entitySpatialMap->add(entity);
	}

	void Overworld::removeEntityFromSpatialMap(AbstractLevelEntity* entity)
	{
		m_entitySpatialMap->remove(entity);
	}

	int Overworld::getTileAt(int tileDepth, double x, double y)
	{
		auto level = getLevelAt(x, y);

		if (level != nullptr)
		{
			//return level->getTileAtLocal(tileDepth, x, y);
		}
		return 0;
	}

	void Overworld::preloadLevels(ResourceManager& resourceManager)
	{
		for (auto level : m_levelNames)
		{
			if (!level->getLoaded())
			{
				QString fullPath;
				if (resourceManager.locateFile(level->getName(), &fullPath))
				{
					level->setFileName(fullPath);
					level->loadNWFile(resourceManager);
				}
			}
		}
	}


	bool Overworld::containsLevel(const QString& name) const
	{
		return m_levelNames.find(name) != m_levelNames.end();
	}


	Level* Overworld::getLevel(const QString& levelName)
	{
		auto it = m_levelNames.find(levelName);
		if (it != m_levelNames.end())
			return it.value();
		return nullptr;
	}

	Level* Overworld::getLevelAt(double x, double y)
	{
		auto level = m_levelMap->entityAt(x, y);
		if (level != nullptr)
			return level;

		return nullptr;

	}

	QList<Level*> Overworld::getModifiedLevels()
	{
		QList<Level*> retval;
		for (auto a : m_levelNames)
		{
			if (a->getModified())
			{
				retval.push_back(a);
			}
		}
		return retval;
	}
};
