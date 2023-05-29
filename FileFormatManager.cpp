#include "FileFormatManager.h"
#include "Level.h"

namespace TilesEditor
{

	bool FileFormatManager::saveLevel(Level* level, QIODevice* stream)
	{
		auto& levelName = level->getName();

		auto pos = levelName.lastIndexOf('.');
		if (pos >= 0)
		{
			auto ext = levelName.mid(pos + 1);

			auto it = m_levelFormats.find(ext);
			if (it != m_levelFormats.end())
			{
				auto& filter = it.value();

				if(filter->canSave())
					return filter->saveLevel(level, stream);
			}
		}
		return false;
	}

	bool FileFormatManager::loadLevel(Level* level, QIODevice* stream)
	{
		auto& levelName = level->getName();

		auto pos = levelName.lastIndexOf('.');
		if (pos >= 0)
		{
			auto ext = levelName.mid(pos + 1);

			auto it = m_levelFormats.find(ext);
			if (it != m_levelFormats.end())
			{
				auto& filter = it.value();

				if (filter->canLoad())
					return filter->loadLevel(level, stream);
			}
		}
		return false;
	}

	void FileFormatManager::registerLevelExtension(const QString& ext,  ILevelFormat* levelFormat)
	{

		m_levelFormats[ext] = levelFormat;
	}

	void FileFormatManager::registerLevelExtension(const QStringList& extensions, ILevelFormat* levelFormat)
	{
		for (auto ext : extensions)
			registerLevelExtension(ext, levelFormat);
	}

	QString FileFormatManager::getLevelSaveFilters() const
	{
		QMap<QString, QStringList> filters;
		QStringList allFilters;
		for (auto it = m_levelFormats.constKeyValueBegin(); it != m_levelFormats.constKeyValueEnd(); ++it)
		{
			auto& ext = it->first;
			auto& format = it->second;

			if (format->canSave())
			{
				auto& filter = filters[format->getCategory()];
				filter.append(QString("*.%1").arg(ext));
				allFilters.append(QString("*.%1").arg(ext));
			}
		}

		QStringList retval;

		retval.append(QString("All Level Files (%1)").arg(allFilters.join(" ")));

		for (auto it = filters.constKeyValueBegin(); it != filters.constKeyValueEnd(); ++it)
		{
			auto& category = it->first;
			auto& filter = it->second;

			retval.append(QString("%1 (%2)").arg(category).arg(filter.join(" ")));
		}
		return retval.join(";;");
	}

	QString FileFormatManager::getLevelLoadFilters() const
	{
		QMap<QString, QStringList> filters;
		QStringList allFilters;
		for (auto it = m_levelFormats.constKeyValueBegin(); it != m_levelFormats.constKeyValueEnd(); ++it)
		{
			auto& ext = it->first;
			auto& format = it->second;

			if (format->canLoad())
			{
				auto& filter = filters[format->getCategory()];
				filter.append(QString("*.%1").arg(ext));
				allFilters.append(QString("*.%1").arg(ext));
			}
		}

		QStringList retval;

		retval.append(QString("All Level Files (%1)").arg(allFilters.join(" ")));

		for (auto it = filters.constKeyValueBegin(); it != filters.constKeyValueEnd(); ++it)
		{
			auto& category = it->first;
			auto& filter = it->second;

			retval.append(QString("%1 (%2)").arg(category).arg(filter.join(" ")));
		}
		return retval.join(";;");
	}
}