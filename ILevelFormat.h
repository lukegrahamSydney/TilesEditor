#ifndef ILEVELFORMATH
#define ILEVELFORMATH

#include <QIODevice>
#include "IWorld.h"

namespace TilesEditor
{
	class ILevelFormat
	{
	public:
		virtual bool loadLevel(Level* inputLevel, QIODevice* stream) = 0;
		virtual bool saveLevel(Level* outputLevel, QIODevice* stream) = 0;

		//Apply the format to this level.
		virtual void applyFormat(Level* level) = 0;

		virtual bool canSave() const { return false; }
		virtual bool canLoad() const { return false; }

		virtual QString getCategory() const { return ""; }
	};
};
#endif
