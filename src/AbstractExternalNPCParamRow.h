#ifndef ABSTRACTEXTERNALNPCPARAMROWH
#define ABSTRACTEXTERNALNPCPARAMROWH

#include <QString>
#include <QWidget>

namespace TilesEditor
{
	class AbstractExternalNPCParamRow:
		public QWidget
	{
	public:
		virtual QString getValue() const = 0;
		virtual void setValue(const QString& value) = 0;

	};
};
#endif

