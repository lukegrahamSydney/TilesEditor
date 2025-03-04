#include "LevelFormatNW.h"
#include "Level.h"
#include "LevelLink.h"
#include "LevelSign.h"
#include "LevelChest.h"
#include "LevelGraalBaddy.h"
#include "LevelObjectInstance.h"

namespace TilesEditor
{
	bool LevelFormatNW::loadLevel(Level* level, QIODevice* stream)
	{
        QStringList lines;

        QTextStream textStream(stream);
        for (QString line = textStream.readLine(); !line.isNull(); line = textStream.readLine())
            lines.push_back(line);

        if (lines.size() > 0)
        {
            auto& version = lines[0];

            static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            static const QString validVersions[] = {
                "GLEVNW01"
            };

            bool valid = false;
            for (auto i = 0U; i < sizeof(validVersions) / sizeof(validVersions[0]); ++i)
            {
                if (validVersions[i] == version) {
                    valid = true;
                    break;
                }
            }



            if (valid)
            {
                if (level->getDefaultTileset()) {
                    level->setTilesetName(level->getDefaultTileset()->text());
                    level->setTilesetImageName(level->getDefaultTileset()->getImageName());
                }

                applyFormat(level);
                level->setSize(64 * 16, 64 * 16);

                QVector<Tilemap*> tilemaps;

                auto lineCount = lines.size();
                for (size_t i = 1; i < lineCount; ++i)
                {
                    auto wordCount = 0U;
                    QStringList words = lines[i].split(' ');

                    wordCount = words.size();
                    if (wordCount > 0)
                    {
                        if (words[0] == "BOARD" && wordCount >= 6)
                        {
                            unsigned int x = words[1].toInt();
                            unsigned int y = words[2].toInt();
                            unsigned int width = words[3].toInt();
                            unsigned int layer = words[4].toInt();

                            auto tilemap = level->getOrMakeTilemap(layer);
                            auto& tileData = words[5];


                            if (tileData.length() >= width * 2)
                            {
                                for (size_t ii = 0u; ii < width; ++ii)
                                {
                                    char byte1 = tileData[ii * 2].unicode();
                                    char byte2 = tileData[1 + ii * 2].unicode();

                                    auto graalTile = (int)((base64.indexOf(byte1) << 6) + base64.indexOf(byte2));
                                    auto tile = Level::convertFromGraalTile(graalTile, level->getDefaultTileset());
                                    tilemap->setTile(x + ii, y, tile);

                                }
                            }
                            
                        }
                        else if (words[0] == "TILESET" && wordCount >= 2)
                        {
                            level->setTilesetName(words[1]);
                        }
                        else if (words[0] == "TILESETIMAGE" && wordCount >= 2)
                        {
                            level->setTilesetImageName(words[1]);
                        }
                        else if (words[0] == "LINK" && wordCount >= 8)
                        {
                            auto x = words[2].toDouble() * 16;
                            auto y = words[3].toDouble() * 16;
                            auto width = words[4].toInt() * 16;
                            auto height = words[5].toInt() * 16;

                            auto& nextX = words[6];
                            auto& nextY = words[7];

                            bool possibleEdgeLink = false;
                            if ((x == 0 || x == 63 * 16) && width == 16)
                                possibleEdgeLink = true;
                            else if ((y == 0 || y == 63 * 16) && height == 16)
                                possibleEdgeLink = true;

                            auto nextLayer = wordCount >= 9 ? words[8].toInt() : 0;
                            auto layerIndex = wordCount >= 10 ? words[9].toInt() : 0;
                            
                            auto levelLink = new LevelLink(level->getWorld(), level->getX() + x, level->getY() + y, width, height, possibleEdgeLink);
                            
                            levelLink->setLevel(level);
                            levelLink->setNextLevel(words[1]);
                            levelLink->setNextX(nextX);
                            levelLink->setNextY(nextY);
                            levelLink->setNextLayer(nextLayer);
                            levelLink->setLayerIndex(layerIndex);

                            level->addObject(levelLink);
                        }
                        else if (words[0] == "CHEST" && wordCount >= 5)
                        {
                            auto& itemName = words[3];

                            auto x = words[1].toDouble() * 16;
                            auto y = words[2].toDouble() * 16;


                            auto signIndex = words[4].toInt();
                            auto layerIndex = wordCount >= 6 ? words[5].toInt() : 0;
                            auto chest = new LevelChest(level->getWorld(), x + level->getX(), y + level->getY(), itemName, signIndex);
                            chest->setLayerIndex(layerIndex);
                            chest->setLevel(level);

                            level->addObject(chest);

                        }
                        else if (words[0] == "BADDY" && wordCount >= 4)
                        {
                            auto x = words[1].toDouble() * 16;
                            auto y = words[2].toDouble() * 16;
                            auto type = words[3].toInt();
                            auto layerIndex = wordCount >= 5 ? words[4].toInt() : 0;

                            auto baddy = new LevelGraalBaddy(level->getWorld(), x + level->getX(), y + level->getY(), type);
                            baddy->setLayerIndex(layerIndex);
                            baddy->setLevel(level);

                            int verseIndex = 0;
                            for (++i; i < lineCount && lines[i] != "BADDYEND"; ++i)
                            {
                                baddy->setBaddyVerse(verseIndex++, lines[i].trimmed());
                            }

                            level->addObject(baddy);

                        }
                        else if (words[0] == "SIGN" && wordCount >= 3)
                        {

                            auto sign = new LevelSign(level->getWorld(),
                                level->getX() + words[1].toDouble() * 16,
                                level->getY() + words[2].toDouble() * 16,
                                32,
                                16);

                            auto layerIndex = wordCount >= 4 ? words[3].toInt() : 0;
                            sign->setLayerIndex(layerIndex);

                            sign->setLevel(level);

                            QString text = "";
                            for (++i; i < lineCount && lines[i] != "SIGNEND"; ++i)
                                text += lines[i] + '\n';
                            sign->setText(text);


                            level->addObject(sign);

                        }
                        else if (words[0] == "NPC" && wordCount >= 4)
                        {
                            QString image = (words[1] == "-" ? "" : words[1]),
                                code;

                            double x = words[2].toDouble() * 16.0,
                                y = words[3].toDouble() * 16.0;

                            auto layerIndex = wordCount >= 5 ? words[4].toInt() : 0;

                            int width = 48, height = 48;

                           

                            for (++i; i < lineCount && lines[i] != "NPCEND"; ++i)
                            {
                                auto& line = lines[i];
                                code += line + "\n";
                            }

                            createNPC(level, image, x, y, code);

                            
                        }
                    }
                }
                return true;
            }
        }
        return false;
	}

	bool LevelFormatNW::saveLevel(Level* level, QIODevice* _stream)
	{
        QTextStream stream(_stream);

        stream << "GLEVNW01" << Qt::endl;

        auto writeTileLayer = [](QTextStream& stream, Tilemap* tilemap)
        {
            auto hcount = std::max(tilemap->getHCount(), 64);
            auto vcount = std::max(tilemap->getVCount(), 64);

            for (int top = 0; top < vcount; ++top)
            {
                for (int left = 0; left < hcount; ++left)
                {
                    //Start scanning from left until we hit a NON-invisible title
                    if (!Tilemap::IsInvisibleTile(tilemap->getTile(left, top)))
                    {
                        //Continue scanning until we hit the end, or an invisible tile
                        int right = left;
                        for (; right < hcount; ++right)
                        {
                            if (Tilemap::IsInvisibleTile(tilemap->getTile(right, top)))
                                break;
                        }

                        int width = right - left;
                        if (width > 0)
                        {
                            stream << "BOARD " << left << " " << top << " " << width << " " << (int)tilemap->getDepth() << " ";
                            for (; left < right; ++left)
                            {
                                static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

                                int tile = tilemap->getTile(left, top);

                                auto tileLeft = Tilemap::GetTileX(tile);
                                auto tileTop = Tilemap::GetTileY(tile);

                                //Convert out normal left-top co-ordinates into graals tile index format
                                //This line was taken from gonstruct source
                                auto graalTile = tileLeft / 16 * 512 + tileLeft % 16 + tileTop * 16;

                                stream << base64[graalTile >> 6];
                                stream << base64[graalTile & 0x3F];
                            }
                            stream << Qt::endl;
                        }
                        left = right;
                    }
                }
            }
        };

        for (auto layer : level->getTileLayers())
        {
            writeTileLayer(stream, layer);

        }
        stream << Qt::endl;

        for (auto link : level->getLinks())
        {
            stream << "LINK " << link->getNextLevel() << " " << int((link->getX() - level->getX()) / 16.0) << " " << int((link->getY() - level->getY()) / 16.0) << " " << link->getWidth() / 16 << " " << link->getHeight() / 16 << " " << link->getNextX() << " " << link->getNextY();
            
            if (link->getNextLayer() != 0 || link->getLayerIndex() != 0)
                stream << " " << link->getNextLayer() << " " << link->getLayerIndex();

            stream << Qt::endl;
        }

        for (auto sign : level->getSigns())
        {
            stream << "SIGN " << int((sign->getX() - level->getX()) / 16.0) << " " << int((sign->getY() - level->getY()) / 16.0);
            if (sign->getLayerIndex() != 0)
                stream << " " << sign->getLayerIndex();

            stream << Qt::endl;

            stream << sign->getText();

            if (!sign->getText().endsWith('\n'))
                stream << Qt::endl;

            stream << "SIGNEND" << Qt::endl << Qt::endl;

        }

        QString baddiesStr = "";
        QString chestsStr = "";
        QString npcsStr = "";

        QTextStream baddiesStream(&baddiesStr, QIODeviceBase::WriteOnly);
        QTextStream chestStream(&chestsStr, QIODeviceBase::WriteOnly);
        QTextStream npcsStream(&npcsStr, QIODeviceBase::WriteOnly);

        
        QList<AbstractLevelEntity*> sortedObjects(level->getObjects().begin(), level->getObjects().end());
        std::sort(sortedObjects.begin(), sortedObjects.end(), AbstractLevelEntity::sortByDepthFunc);

        for (auto obj : sortedObjects)
        {
            switch (obj->getEntityType())
            {
            case LevelEntityType::ENTITY_NPC:
            {
                auto npc = static_cast<LevelNPC*>(obj);

                auto imageName = npc->getImageName();

                if (!npc->isObjectInstance())
                {
                    if (imageName.isEmpty())
                        imageName = "-";
                    npcsStream << "NPC " << imageName << " " << std::floor(((npc->getX() - level->getX()) / 16.0) * 1000.0) / 1000.0 << " " << std::floor(((npc->getY() - level->getY()) / 16.0) * 1000.0) / 1000.0;
                    if (npc->getLayerIndex() != 0)
                        npcsStream << " " << npc->getLayerIndex();

                    npcsStream << Qt::endl;

                    npcsStream << npc->getCode();

                    if (!npc->getCode().endsWith('\n'))
                        npcsStream << Qt::endl;
                    npcsStream << "NPCEND" << Qt::endl << Qt::endl;
                }
                else {
                    auto objectInstance = static_cast<LevelObjectInstance*>(npc);
                    auto objectClass = objectInstance->getObjectClass();


                    if (imageName.isEmpty())
                        imageName = "-";

                    npcsStream << "NPC " << imageName << " " << std::floor(((npc->getX() - level->getX()) / 16.0) * 1000.0) / 1000.0 << " " << std::floor(((npc->getY() - level->getY()) / 16.0) * 1000.0) / 1000.0;
                    if (npc->getLayerIndex() != 0)
                        npcsStream << " " << npc->getLayerIndex();

                    npcsStream << Qt::endl;

                    npcsStream << objectInstance->formatGraalCode(false, true, objectInstance->getParams());
                    if (!npc->getCode().endsWith('\n'))
                        npcsStream << Qt::endl;
                    npcsStream << "NPCEND" << Qt::endl << Qt::endl;
                }
            }
            break;

            case LevelEntityType::ENTITY_CHEST:
            {
                auto chest = static_cast<LevelChest*>(obj);

                chestStream << "CHEST " << int((chest->getX() - level->getX()) / 16.0) << " " << int((chest->getY() - level->getY()) / 16.0) << " " << chest->getItemName() << " " << chest->getSignIndex();
                if (chest->getLayerIndex() != 0)
                    chestStream << " " << chest->getLayerIndex();

                chestStream << Qt::endl;
            }
            break;

            case LevelEntityType::ENTITY_BADDY:
            {
                auto baddy = static_cast<LevelGraalBaddy*>(obj);

                baddiesStream << "BADDY " << int((baddy->getX() - level->getX()) / 16.0) << " " << int((baddy->getY() - level->getY()) / 16.0) << " " << baddy->getBaddyType();
                if (baddy->getLayerIndex() != 0)
                    baddiesStream << " " << baddy->getLayerIndex();

                baddiesStream << Qt::endl;
                for (auto i = 0; i < 3; ++i)
                {
                    baddiesStream << baddy->getBaddyVerse(i) << Qt::endl;
                }
                baddiesStream << "BADDYEND" << Qt::endl << Qt::endl;
            }
            break;
            }

        }

        chestStream.flush();
        baddiesStream.flush();
        npcsStream.flush();

        stream << chestsStr;
        stream << baddiesStr;
        stream << npcsStr;

        if (!level->getTilesetName().isEmpty())
            stream << "TILESET " << level->getTilesetName() << Qt::endl;

        if (!level->getTilesetImageName().isEmpty())
            stream << "TILESETIMAGE " << level->getTilesetImageName() << Qt::endl;
        return true;
	}

    void LevelFormatNW::applyFormat(Level* level)
    {
        level->setUnitWidth(16);
        level->setUnitHeight(16);
        level->getLevelFlags().canLayObjectInstance = false;
        level->getLevelFlags().canLayAnonymousNPC = true;
        level->getLevelFlags().autoEmbedCodeForNewObjectClass = true;
        level->setDefaultObjectLanguage(ScriptingLanguage::SCRIPT_GS1);
    }
};
