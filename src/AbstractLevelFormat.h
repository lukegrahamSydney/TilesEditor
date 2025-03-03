#ifndef ABSTRACTLEVELFORMATH
#define ABSTRACTLEVELFORMATH

#include <QIODevice>
#include "IWorld.h"

namespace TilesEditor
{
	class LevelNPC;
	class AbstractLevelFormat
	{
	public:
		virtual bool loadLevel(Level* inputLevel, QIODevice* stream) = 0;
		virtual bool saveLevel(Level* outputLevel, QIODevice* stream) = 0;

		//Apply the format to this level.
		virtual void applyFormat(Level* level) = 0;

		virtual bool canSave() const { return false; }
		virtual bool canLoad() const { return false; }

		virtual void filterLevelSize(int* hcount, int* vcount) {}
		virtual bool customLevelSizes() const { return true; }
		virtual QString getPrimaryExtension() const = 0;
		virtual QStringList getCategories() const { return QStringList(); }

		static LevelNPC* createNPC(Level* level, const QString& image, double x, double y, QString code);
	};
};
#endif
