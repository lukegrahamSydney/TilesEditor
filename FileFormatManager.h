#ifndef FILEFORMATMANAGERH
#define FILEFORMATMANAGERH

#include <QMap>
#include <QString>
#include <QStringList>
#include "ILevelFormat.h"

namespace TilesEditor
{
	class Level;
	class FileFormatManager
	{
	private:
		QMap<QString, ILevelFormat*> m_levelFormats;

	public:
		bool saveLevel(Level* level, QIODevice* stream);
		bool loadLevel(Level* level, QIODevice* stream);
		void registerLevelExtension(const QString& ext, ILevelFormat* levelFormat);
		void registerLevelExtension(const QStringList& extensions, ILevelFormat* levelFormat);

		QString getLevelSaveFilters() const;
		QString getLevelLoadFilters() const;

		static FileFormatManager* instance() {
			static auto retval = new FileFormatManager();
			return retval;
		}
	};
};
#endif
