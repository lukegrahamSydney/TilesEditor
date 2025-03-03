#ifndef LEVELFORMATNWH
#define LEVELFORMATNWH

#include "AbstractLevelFormat.h"

namespace TilesEditor
{
	class LevelFormatNW :
		public AbstractLevelFormat
	{
	public:
		bool loadLevel(Level* level, QIODevice* stream) override;
		bool saveLevel(Level* level, QIODevice* stream) override;

		//Apply the format to this level.
		void applyFormat(Level* level) override;

		bool canSave() const override { return true; }
		bool canLoad() const override { return true; }

		bool customLevelSizes() const override { return false; }
		void filterLevelSize(int* hcount, int* vcount) override { *hcount = 64, *vcount = 64; }
		QString getPrimaryExtension() const override { return "nw"; }
		QStringList getCategories() const override { return QStringList({ "All Graal Formats", "Graal New World Format" }); }
	};
};
#endif
