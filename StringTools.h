#ifndef STRINGTOOLSH
#define STRINGTOOLSH

#include <QStringList>
#include <QString>

namespace StringTools
{
	static size_t ParseCSV(const QString& line, QStringList& output)
	{
		int valueCount = 0;
		QString value = "";
		bool quotes = false;
		for (size_t i = 0; i < line.length(); ++i)
		{
				
			switch (line[i].unicode()) {
			case '"':
				value = "";
				quotes = true;
				for (++i; i < line.length(); ++i) {
					if (line[i] == '"')
					{
						if (i + 1 < line.length() && line[i + 1] == '"') {
							++i;
						}
						else break;
					}
					value += line[i];
				}
				break;

			case ',':
				output.push_back(!quotes ? value.trimmed() : value);
				value = "";
				quotes = false;
				++valueCount;
				break;

			case ' ':
				if (quotes)
					break;

			default:
				value += line[i];
				break;
			}
		}
		output.push_back(!quotes ? value.trimmed() : value);
		return valueCount + 1;
	}
};

#endif