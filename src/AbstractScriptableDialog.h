#ifndef ABSTRACTSCRIPTABLEDIALOGH
#define ABSTRACTSCRIPTABLEDIALOGH

#include "sgscript/sgscript.h"
#include "DialogFeatures.h"
#include "IEngine.h"

namespace TilesEditor
{
	class AbstractScriptableDialog
	{
	private:
		IEngine* m_engine;
		sgs_Variable m_thisObject;

	public:
		AbstractScriptableDialog(IEngine* engine);
		~AbstractScriptableDialog();

		virtual void scriptShowFeature(DialogFeatures feature, bool value) {};
		virtual sgs_Variable scriptGetFeatureValue(sgs_Context* ctx, DialogFeatures feature) { return sgs_MakeNull(); }
		virtual void scriptSetFeatureValue(sgs_Context* ctx, DialogFeatures feature, const sgs_Variable& value) {};
		virtual void mark(sgs_Context* ctx) {};

		sgs_Variable& getScriptThisObject() { return m_thisObject; }
		static sgs_ObjInterface sgs_interface;
		static void registerScriptClass(sgs_Context* ctx);
	};
};
#endif
