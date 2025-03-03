#ifndef LEVELFORMATLVLH
#define LEVELFORMATLVLH

#include "AbstractLevelFormat.h"

namespace TilesEditor
{
	class LevelFormatLVL :
		public AbstractLevelFormat
	{
	public:
		bool loadLevel(Level* level, QIODevice* stream) override;
		bool saveLevel(Level* level, QIODevice* stream) override;

		//Apply the format to this level.
		void applyFormat(Level* level) override;

		bool canSave() const override { return true; }
		bool canLoad() const override { return true; }

		QString getPrimaryExtension() const override { return "lvl"; }
		QStringList getCategories() const override { return QStringList({ "Tiles Editor Level" }); }
	};
};

#endif
