#ifndef LEVELFORMATNWH
#define LEVELFORMATNWH

#include "ILevelFormat.h"

namespace TilesEditor
{
	class LevelFormatNW :
		public ILevelFormat
	{
	public:
		bool loadLevel(Level* level, QIODevice* stream) override;
		bool saveLevel(Level* level, QIODevice* stream) override;

		//Apply the format to this level.
		void applyFormat(Level* level) override;

		bool canSave() const override { return true; }
		bool canLoad() const override { return true; }
		QString getPrimaryExtension() const override { return "nw"; }
		QStringList getCategories() const override { return QStringList({ "All Graal Formats", "Graal New World Format" }); }
	};
};
#endif
