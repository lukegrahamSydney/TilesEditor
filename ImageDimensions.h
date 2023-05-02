#ifndef IMAGEDIMENSIONSH
#define IMAGEDIMENSIONSH

#include <QString>
namespace TilesEditor
{
	class ImageDimensions
	{
	public:
		static bool getImageFileDimensions(const QString& fileName, int* w, int* h);
	};
};
#endif
