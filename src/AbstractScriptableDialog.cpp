#include "AbstractScriptableDialog.h"

namespace TilesEditor
{
	sgs_ObjInterface AbstractScriptableDialog::sgs_interface;
	AbstractScriptableDialog::AbstractScriptableDialog(IEngine* engine):
		m_engine(engine)
	{
		sgs_CreateObject(m_engine->getScriptContext(), &m_thisObject, static_cast<AbstractScriptableDialog*>(this), &sgs_interface);

		engine->addCPPOwnedObject(m_thisObject);
	}

	AbstractScriptableDialog::~AbstractScriptableDialog()
	{
		m_engine->removeCPPOwnedObject(m_thisObject);
		m_thisObject.data.O->data = NULL;
		sgs_Release(m_engine->getScriptContext(), &m_thisObject);
	}

	void AbstractScriptableDialog::registerScriptClass(sgs_Context* ctx)
	{
	}
};