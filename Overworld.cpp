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
		m_levelMap = nullptr;

		m_unitWidth = m_unitHeight = 16;

		setSize(64, 64);
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
		QFile file(m_fileName);

		if (file.open(QIODeviceBase::ReadOnly))
		{
			return loadStream(&file, resourceManager);
		}
		return false;
	}

	bool Overworld::loadStream(QIODevice* stream, ResourceManager& resourceManager)
	{
		if (m_name.endsWith(".gmap"))
			return loadGMapStream(stream, resourceManager);

		else if (m_name.endsWith(".txt"))
			return loadTXTStream(stream, resourceManager);

		else if (m_name.endsWith(".world"))
			return loadWorldStream(stream, resourceManager);
		return false;
	}

	bool Overworld::loadGMapStream(QIODevice* stream, ResourceManager& resourceManager)
	{
		QStringList lines;
		QTextStream textStream(stream);
		for (QString line = textStream.readLine(); !line.isNull(); line = textStream.readLine())
			lines.push_back(line);

		if (lines.size() > 0)
		{
			m_gmapFileLines.clear();

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

	bool Overworld::loadTXTStream(QIODevice* stream, ResourceManager& resourceManager)
	{
		QStringList lines;

		QTextStream textStream(stream);
		for (QString line = textStream.readLine(); !line.isNull(); line = textStream.readLine())
			lines.push_back(line);
		

		if (lines.size() > 0)
		{
			bool sizeSet = false;
			bool retval = false;
			for (auto y = 0; y < lines.size(); ++y)
			{
				size_t namesCount = 0;
				QStringList namesList;

				m_gmapFileLines.append(lines[y]);

				if ((namesCount = StringTools::ParseCSV(lines[y], namesList)) > 0)
				{
					if (!sizeSet)
					{
						auto width = namesList.size();
						auto height = lines.size();

						sizeSet = true;
						setSize(width * 16 * 64, height * 16 * 64);
						retval = true;
					}

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

			}

			return retval;
		}

		return false;
	}

	bool Overworld::loadWorldStream(QIODevice* stream, ResourceManager& resourceManager)
	{
		if (m_json) {
			cJSON_Delete(m_json);
			m_json = nullptr;
		}

		QTextStream textStream(stream);
		QString text = textStream.readAll();

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

	bool Overworld::saveFile(ResourceManager& resourceManager)
	{
		auto stream = resourceManager.getFileSystem()->openStream(m_fileName, QIODevice::WriteOnly);

		if (stream)
		{
			auto retval = saveStream(stream);
			resourceManager.getFileSystem()->endWrite(m_fileName, stream);

			return retval;
		}

		return false;
	}

	bool Overworld::saveStream(QIODevice* stream)
	{
		if (m_name.endsWith(".gmap"))
			return saveGMapStream(stream);

		else if (m_name.endsWith(".world"))
			return saveWorldStream(stream);

		else if (m_name.endsWith(".txt"))
		{
			return saveTXTStream(stream);
		}
		return false;
	}

	bool Overworld::saveGMapStream(QIODevice* _stream)
	{

		QTextStream stream(_stream);

		stream << "GRMAP001" << Qt::endl;

		for (auto& line : m_gmapFileLines)
		{
			stream << line << Qt::endl;
		}

		if(!m_tilesetName.isEmpty())
			stream << "TILESET " << m_tilesetName << Qt::endl;
		
		return true;
	}

	bool Overworld::saveTXTStream(QIODevice* _stream)
	{
		QTextStream stream(_stream);

		for (auto& line : m_gmapFileLines)
		{
			stream << line << Qt::endl;
		}

		return true;
	}

	bool Overworld::saveWorldStream(QIODevice* _stream)
	{
		if (m_json)
		{
			cJSON_DeleteItemFromObject(m_json, "tileset");
			if (!m_tilesetName.isEmpty())
				cJSON_AddStringToObject(m_json, "tileset", m_tilesetName.toLocal8Bit().data());

			auto levelText = cJSON_Print(m_json);
			QTextStream stream(_stream);
			stream << levelText;
			free(levelText);

			return true;
			
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
					level->loadFile(resourceManager);
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
