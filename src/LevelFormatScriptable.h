#ifndef LEVELFORMATSCRIPTABLEH
#define LEVELFORMATSCRIPTABLEH

#include "AbstractLevelFormat.h"
#include "IEngine.h"

namespace TilesEditor
{
	class LevelFormatScriptable :
		public AbstractLevelFormat
	{
	private:
		IEngine* m_engine;
		sgs_Variable m_table;

	public:
		LevelFormatScriptable(IEngine* engine, sgs_Variable& table);
		~LevelFormatScriptable();

		bool loadLevel(Level* level, QIODevice* stream) override;

	};
};
#endif