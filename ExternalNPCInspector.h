#ifndef EXTERNALNPCINSPECTORH
#define EXTERNALNPCINSPECTORH

#include <QWidget>
#include "ui_ExternalNPCInspector.h"

namespace TilesEditor
{
	class ExternalNPCInspector : public QWidget
	{
		Q_OBJECT

	public:
		ExternalNPCInspector(QWidget* parent = nullptr);
		~ExternalNPCInspector();

	private:
		Ui::ExternalNPCInspectorClass ui;
	};
};

#endif
