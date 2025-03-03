#include "LevelFormatScriptable.h"

namespace TilesEditor
{
	LevelFormatScriptable::LevelFormatScriptable(IEngine* engine, sgs_Variable& table):
		m_engine(engine), m_table(table)
	{
		sgs_Acquire(engine->getScriptContext(), &table);
		engine->addCPPOwnedObject(table);
	}

	LevelFormatScriptable::~LevelFormatScriptable()
	{
		m_engine->removeCPPOwnedObject(m_table);

		sgs_Release(m_engine->getScriptContext(), &m_table);
	}

	bool LevelFormatScriptable::loadLevel(Level* level, QIODevice* stream)
	{
		auto ctx = m_engine->getScriptContext();

		sgs_Variable func;
		if (sgs_GetProperty(ctx, m_table, "load", &func) == SGS_SUCCESS)
		{
			if (func.type == SGS_VT_FUNC)
			{
				sgs_PushVariable(ctx, func);
				sgs_PushVariable(ctx, m_table);



			}
			sgs_Release(ctx, &func);
		}
		return false;
	}

};