#ifndef LEVELFORMATGRAALH
#define LEVELFORMATGRAALH

#include "ILevelFormat.h"

namespace TilesEditor
{
	class LevelFormatGraal :
		public ILevelFormat
	{
	public:
		bool loadLevel(Level* level, QIODevice* stream) override;
		bool saveLevel(Level* level, QIODevice* stream) override;

		//Apply the format to this level.
		void applyFormat(Level* level) override;

		bool canSave() const override { return true; }
		bool canLoad() const override { return true; }
		QString getPrimaryExtension() const override { return "graal"; }
		QStringList getCategories() const override { return QStringList({ "All Graal Formats", "Graal Level Format" }); }
	};
};

#endif
