#ifndef SCRIPTABLELEVELOBJECTH
#define SCRIPTABLELEVELOBJECTH

#include "AniInstance.h"
#include "IWorld.h"
#include "IFileRequester.h"

namespace TilesEditor
{
	class IScriptableLevelObject:
		public IFileRequester
	{
	public:

		virtual IWorld* getWorld() const = 0;
		virtual AniInstance* getAniInstance() {
			return nullptr;
		};
		virtual void setColourEffect(double r, double g, double b, double a) {};
		virtual void setAniName(const QString& fileName, int frame) {};
		virtual void showCharacter() {};
		virtual void setDir(int dir) {};
		virtual int getDir() const { return 0; }
		virtual void setImageName(const QString& name) {}
		virtual void setImageShape(int left, int top, int width, int height) {};

		
	};
};
#endif

