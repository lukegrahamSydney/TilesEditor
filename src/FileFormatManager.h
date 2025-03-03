#ifndef FILEFORMATMANAGERH
#define FILEFORMATMANAGERH

#include <QMap>
#include <QString>
#include <QStringList>
#include <QList>
#include "AbstractLevelFormat.h"

namespace TilesEditor
{
	class Level;
	class FileFormatManager
	{
	private:
		QList<QString>	m_levelFormatsOrdered;
		QMap<QString, AbstractLevelFormat*> m_levelFormats;

	public:
		bool saveLevel(Level* level, QIODevice* stream);
		bool loadLevel(Level* level, QIODevice* stream);

		void applyFormat(Level* level);
		void applyFormat(const QString& format, Level* level);
		void registerLevelExtension(const QString& ext, AbstractLevelFormat* levelFormat);
		void registerLevelExtension(const QStringList& extensions, AbstractLevelFormat* levelFormat);
		AbstractLevelFormat* getFormatObject(const QString& format);

		QString getLevelSaveFilters() const;
		QString getLevelLoadFilters() const;
		QStringList getAllLevelLoadExtensions() const;
		QStringList getAllLevelSaveExtensions() const;

		QList<AbstractLevelFormat*> getRegisteredFormats() const;
		static FileFormatManager* instance() {
			static auto retval = new FileFormatManager();
			return retval;
		}
	};
};
#endif
