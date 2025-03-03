#ifndef IENGINE
#define IENGINE

#include <QString>
#include "AbstractResourceManager.h"
#include "ObjectManager.h"
#include "sgscript/sgscript.h"
#include "ScriptingLanguage.h"

namespace TilesEditor
{
	class Level;
	class IEngine
	{
	public:
		virtual ObjectManager* getObjectManager() = 0;
		virtual AbstractResourceManager* getResourceManager() = 0;
		virtual QString parseInlineString(const QString& expression) = 0;
		virtual QString parseExpression(const QString& expression) = 0;
		virtual bool testCodeForErrors(const QString& code, QString* errorOutput, ScriptingLanguage language) = 0;
		virtual QString escapeString(const QString& text, ScriptingLanguage language) = 0;
		virtual sgs_Context* getScriptContext() = 0;
		virtual void addCPPOwnedObject(sgs_Variable& var) = 0;
		virtual void removeCPPOwnedObject(sgs_Variable& var) = 0;
		virtual void setErrorText(const QString& text, int seconds = 10) = 0;

		virtual void addTileDef2(const QString& image, const QString& levelStart, int x, int y, bool saved) = 0;
		virtual void applyTileDefs(Level* level) = 0;
	};
};
#endif
