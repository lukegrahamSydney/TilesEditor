#include <QFile>
#include "ImageDimensions.h"

namespace TilesEditor
{
    bool ImageDimensions::getImageFileDimensions(const QString& fileName, int* w, int* h)
    {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly))
        {
            unsigned char buf[8];
            

            static const unsigned char pngHeader[8] = { 137,80, 78, 71, 13, 10, 26, 10 };
            static const unsigned char gifHeader[3] = { 'G','I', 'F' };

            unsigned char header[8];
            auto headerLen = file.read((char*)header, 8);

            //this is png
            if (headerLen >= 8 && memcmp(pngHeader, header, 8) == 0)
            {
                file.seek(16);

                
                if (file.read((char*)buf, 8) == 8)
                {
                    *w = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
                    *h = (buf[4] << 24) + (buf[5] << 16) + (buf[6] << 8) + (buf[7] << 0);
                    return true;
                }
            }
            //Its a gif
            else if (headerLen >= 3 && memcmp(gifHeader, header, 3) == 0)
            {
                file.seek(6);

                if (file.read((char*)buf, 4) == 4)
                {
                    *w = (buf[1] << 8) + buf[0];
                    *h = (buf[3] << 8) + buf[2];
                    return true;
                }
            }

        }

        return false;
    }
};
