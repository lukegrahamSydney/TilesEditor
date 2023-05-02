#ifndef IFILESYSTEMH
#define IFILESYSTEMH

#include <QByteArray>
#include <QString>
#include <QIODevice>

namespace TilesEditor
{
	class AbstractFileSystem
	{

	public:
		virtual QByteArray readAllToBytes(const QString& name) = 0;
		virtual QString readAllToString(const QString& name) = 0;
		virtual QIODevice* openStream(const QString& name) = 0;
		virtual void close() = 0;
	};
}
#endif
