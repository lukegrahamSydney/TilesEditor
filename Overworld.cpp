#include <QDebug>
#include <iterator>
#include "Overworld.h"
#include "MainFileSystem.h"
#include "Level.h"
#include "EntitySpatialGrid.h"
#include "StringTools.h"

namespace TilesEditor
{
	Overworld::Overworld(const QString& name)
	{
		m_name = name;
		m_entitySpatialMap = nullptr;
		m_levelMap = nullptr;

		m_tilesetName = "pics1.png";
	}

	Overworld::~Overworld()
	{
		for (auto level : m_levelNames)
			delete level;

		if (m_entitySpatialMap)
			delete m_entitySpatialMap;

		if (m_levelMap)
			delete m_levelMap;
	}

	void Overworld::release(ResourceManager& resourceManager)
	{

		for (auto level : m_levelNames)
			level->release(resourceManager);
	}


	void Overworld::loadGMap(const QString& fileName, ResourceManager& resourceManager)
	{
		QStringList lines;

		if (resourceManager.getFileSystem().readAllLines(fileName, lines))
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
		}
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
