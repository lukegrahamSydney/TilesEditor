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

	void FileFormatManager::applyFormat(Level* level)
	{
		auto& levelName = level->getName();

		auto pos = levelName.lastIndexOf('.');
		if (pos >= 0)
		{
			auto format = levelName.mid(pos + 1);
			applyFormat(format, level);
		}
	}

	void FileFormatManager::applyFormat(const QString& format, Level* level)
	{

		auto it = m_levelFormats.find(format.toLower());
		if (it != m_levelFormats.end())
		{
			auto& filter = it.value();

			filter->applyFormat(level);
		}
		
	}

	void FileFormatManager::registerLevelExtension(const QString& ext, AbstractLevelFormat* levelFormat)
	{
		auto extLower = ext.toLower();
		m_levelFormats[extLower] = levelFormat;
		m_levelFormatsOrdered.push_back(extLower);
	}

	void FileFormatManager::registerLevelExtension(const QStringList& extensions, AbstractLevelFormat* levelFormat)
	{
		for (auto ext : extensions)
			registerLevelExtension(ext, levelFormat);
	}

	AbstractLevelFormat* FileFormatManager::getFormatObject(const QString& format)
	{
		auto it = m_levelFormats.find(format.toLower());
		if (it != m_levelFormats.end())
			return it.value();
		return nullptr;
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
				auto categories = format->getCategories();
				for (auto category : categories)
				{
					auto& filter = filters[category];
					filter.append(QString("*.%1").arg(ext));

				}
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
				auto categories = format->getCategories();
				for (auto category : categories)
				{
					auto& filter = filters[category];
					filter.append(QString("*.%1").arg(ext));
				}

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

	QStringList FileFormatManager::getAllLevelLoadExtensions() const
	{
		QStringList allFilters;
		for (auto it = m_levelFormats.constKeyValueBegin(); it != m_levelFormats.constKeyValueEnd(); ++it)
		{
			auto& ext = it->first;
			auto& format = it->second;

			if (format->canLoad())
			{
				allFilters.append(ext);
			}
		}

		return allFilters;
	}

	QStringList FileFormatManager::getAllLevelSaveExtensions() const
	{
		QStringList allFilters;
		for (auto it = m_levelFormats.constKeyValueBegin(); it != m_levelFormats.constKeyValueEnd(); ++it)
		{
			auto& ext = it->first;
			auto& format = it->second;

			if (format->canSave())
			{
				allFilters.append(ext);
			}
		}

		return allFilters;
	}

	QList<AbstractLevelFormat*> FileFormatManager::getRegisteredFormats() const
	{
		QList<AbstractLevelFormat*> retval;

		for (auto ext : m_levelFormatsOrdered)
		{
			auto format = m_levelFormats[ext];
			if (format != nullptr)
			{
				if (retval.indexOf(format) == -1)
					retval.push_back(format);
			}
		}
		return retval;
	}
}