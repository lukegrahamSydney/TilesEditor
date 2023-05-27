#ifndef LEVELFACTORYH
#define LEVELFACTORYH

#include <QStringList>

namespace TilesEditor
{
	class LevelFactory
	{
	private:

	public:

		QStringList getSupportedExtensions();

		static LevelFactory* instance() {
			static auto retval = new LevelFactory;
			return retval;
		}
	};
};
#endif
