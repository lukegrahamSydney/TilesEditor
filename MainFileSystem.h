#ifndef MAINFILESYSTEMH
#define MAINFILESYSTEMH

#include <QString>
#include <QStringList>
#include <QSet>
#include "AbstractFileSystem.h"

namespace TilesEditor
{
	class MainFileSystem :
		public AbstractFileSystem
	{
	private:


	public:
		MainFileSystem();

		QByteArray readAllToBytes(const QString& name) override {
			auto stream = openStream(name);
			if (stream != nullptr)
			{
				auto barray = stream->readAll();
				delete stream;
				return barray;
			}
			return QByteArray();
		}

		QString readAllToString(const QString& name) override {
			return QString(readAllToBytes(name));
		}

		bool readAllLines(const QString& fileName, QStringList& lines);

		QIODevice* openStream(const QString& fileName) override;

		void close() {}
	};
};

#endif
