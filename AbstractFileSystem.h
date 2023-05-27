#ifndef IFILESYSTEMH
#define IFILESYSTEMH

#include <QByteArray>
#include <QString>
#include <QIODevice>
#include <QTextStream>
#include <QStringList>
#include "IFileRequester.h"

namespace TilesEditor
{
	class AbstractFileSystem
	{

	public:

		QString readAllToString(const QString& name) {
			auto stream = openStream(name, QIODevice::ReadOnly);
			if (stream != nullptr)
			{
				QTextStream textStream(stream);
				QString retval = textStream.readAll();
				delete stream;

				return retval;
			}
			return "";
		}

		//Filename should be name part only. not FULL PATH
		virtual void requestFile(IFileRequester* requester, const QString& fileName) = 0;
		virtual void removeListener(IFileRequester* requester) = 0;

		virtual QStringList getFolders(const QString& parent) = 0;
		virtual bool fileExists(const QString& fileName) = 0;
		virtual QIODevice* openStream(const QString& fileName, QIODeviceBase::OpenModeFlag mode) = 0;

		virtual QString getOpenFileName(const QString& caption, const QString& dir, const QString& filter) = 0;
		virtual QString getSaveFileName(const QString& caption, const QString& dir, const QString& filter) = 0;
		//this is called when a file has finished being written to. also delete the stream object in this function
		virtual void endWrite(IFileRequester* requester, const QString& fileName, QIODevice* stream) = 0;
	};
}
#endif
