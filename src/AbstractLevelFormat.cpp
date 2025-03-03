#include "AbstractLevelFormat.h"
#include "LevelNPC.h"
#include "LevelObjectInstance.h"
#include "Level.h"

namespace TilesEditor
{
    LevelNPC* AbstractLevelFormat::createNPC(Level* level, const QString& image, double x, double y, QString code)
    {
        QTextStream stream(&code);
       
        QRect imageRect;
        QSize size;

        auto line = stream.readLine();
        if (line.startsWith("//#OBJECT "))
        {
            QString className = line.mid(strlen("//#OBJECT ")).trimmed();
            QStringList params;

            while (!stream.atEnd())
            {
                line = stream.readLine();

                if (!line.startsWith("//#"))
                    break;

                if (line.startsWith("//#PARAM "))
                    params.push_back(line.mid(strlen("//#PARAM ")).trimmed());
                
                else if (line.startsWith("//#IMAGERECT "))
                {
                    QString rectString = line.mid(strlen("//#IMAGERECT "));
                    auto parts = rectString.split(' ', Qt::SplitBehaviorFlags::SkipEmptyParts);
                    if (parts.size() >= 4)
                    {
                        imageRect = QRect(parts[0].toInt(), parts[1].toInt(), parts[2].toInt(), parts[3].toInt());
                    }
                }
                else if (line.startsWith("//#SIZE "))
                {
                    QString sizeString = line.mid(strlen("//#SIZE "));
                    auto parts = sizeString.split(' ', Qt::SplitBehaviorFlags::SkipEmptyParts);
                    if (parts.size() >= 2)
                    {
                        size = QSize(parts[0].toInt(), parts[1].toInt());
                    }
                }
            }

            auto objectClass = level->getWorld()->getResourceManager()->getObjectManager()->loadObject(className, false);
            if (objectClass)
            {
                auto levelNPC = new LevelObjectInstance(level->getWorld(), x + level->getX(), y + level->getY(), className, objectClass, params);
                levelNPC->setLevel(level);
                levelNPC->setImageName(image);
                
                if (!size.isEmpty())
                {
                    levelNPC->setWidth(size.width());
                    levelNPC->setHeight(size.height());
                    levelNPC->setHasResized(true);
                }

                level->addObject(levelNPC);
                return levelNPC;
            }
        }

        auto levelNPC = new LevelNPC(level->getWorld(), x + level->getX(), y + level->getY(), 32, 32);
        levelNPC->setLevel(level);

        levelNPC->setImageName(image);
        levelNPC->setCode(code);
        if (!imageRect.isNull())
        {
            levelNPC->setImageShape(imageRect.x(), imageRect.y(), imageRect.width(), imageRect.height());
        }

        if (!size.isEmpty())
        {
            levelNPC->setWidth(size.width());
            levelNPC->setHeight(size.height());
            levelNPC->setHasResized(true);
        }

        level->addObject(levelNPC);
        return levelNPC;
    }
};