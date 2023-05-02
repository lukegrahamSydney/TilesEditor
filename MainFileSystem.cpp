#include <QDir>
#include "MainFileSystem.h"

namespace TilesEditor
{
	MainFileSystem::MainFileSystem()
	{

	}


	bool MainFileSystem::readAllLines(const QString& fileName, QStringList& lines)
	{
		QFile textFile(fileName);
		if (textFile.open(QIODevice::ReadOnly))
		{
			//... (open the file for reading, etc.)
			QTextStream textStream(&textFile);
			while (true)
			{
				QString line = textStream.readLine();
				if (line.isNull())
					break;
				else
					lines.append(line);
			}
			return true;
		}
		return false;
	}

	QIODevice* MainFileSystem::openStream(const QString& fileName)
	{

		QFile* file = new QFile(fileName);
		if (file->open(QIODevice::ReadOnly))
			return file;

		delete file;
		
		return nullptr;

	}
};