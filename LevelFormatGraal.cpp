#include "LevelFormatGraal.h"
#include "Level.h"
#include "LevelLink.h"
#include "LevelSign.h"
#include "LevelChest.h"
#include "LevelGraalBaddy.h"


namespace TilesEditor
{
    bool LevelFormatGraal::loadLevel(Level* level, QIODevice* stream)
    {
        int v = -1;
        QString fileVersion = stream->read(8);

        bool isZelda = fileVersion.startsWith("Z3");

        if (fileVersion == "GR-V1.00" || fileVersion == "Z3-V1.03" || fileVersion == "Z3-V1.04") v = 0;
        else if (fileVersion == "GR-V1.01") v = 1;
        else if (fileVersion == "GR-V1.02") v = 2;
        else if (fileVersion == "GR-V1.03") v = 3;
        if (v == -1) return false;

        auto readString = [](QIODevice* base, char end) -> QString {
            QString retval = "";
            unsigned char byte;
            while (base->read((char*)&byte, 1) > 0 && byte != end) {
                retval.append((char)byte);
            }
            return retval;
        };

        // Load tiles.
        {
            level->setSize(64 * 16, 64 * 16);
            level->setUnitWidth(16);
            level->setUnitHeight(16);

            int bits = (v > 1 ? 13 : 12);
            int read = 0;
            unsigned int buffer = 0;
            unsigned short code = 0;
            short tiles[2] = { -1,-1 };
            int boardIndex = 0;
            int count = 1;
            bool doubleMode = false;

            auto tilemap = level->getOrMakeTilemap(0);

            auto setTileIndex = [](Tilemap* tilemap, int boardIndex, int graalTile, Tileset* defaultTileset) {
                auto tileX = boardIndex % 64;
                auto tileY = boardIndex / 64;

                auto tile = Level::convertFromGraalTile(graalTile, defaultTileset);

                tilemap->setTile(tileX, tileY, tile);
            };



            // Read the tiles.
            while (boardIndex < 64 * 64 && !stream->atEnd())
            {
                // Every control code/tile is either 12 or 13 bits.  WTF.
                // Read in the bits.
                while (read < bits)
                {
                    unsigned char byte;
                    stream->read((char*)&byte, 1);

                    buffer += byte << read;
                    read += 8;
                }

                // Pull out a single 12/13 bit code from the buffer.
                code = buffer & (bits == 12 ? 0xFFF : 0x1FFF);
                buffer >>= bits;
                read -= bits;

                // See if we have an RLE control code.
                // Control codes determine how the RLE scheme works.
                if (code & (bits == 12 ? 0x800 : 0x1000))
                {
                    // If the 0x100 bit is set, we are in a double repeat mode.
                    // {double 4}56 = 56565656
                    if (code & 0x100) doubleMode = true;

                    // How many tiles do we count?
                    count = code & 0xFF;
                    continue;
                }

                // If our count is 1, just read in a tile.  This is the default mode.
                if (count == 1)
                {
                    setTileIndex(tilemap, boardIndex++, code, level->getDefaultTileset());
                    continue;
                }

                // If we reach here, we have an RLE scheme.
                // See if we are in double repeat mode or not.
                if (doubleMode)
                {
                    // Read in our first tile.
                    if (tiles[0] == -1)
                    {
                        tiles[0] = (short)code;
                        continue;
                    }

                    // Read in our second tile.
                    tiles[1] = (short)code;

                    // Add the tiles now.
                    for (int i = 0; i < count && boardIndex < 64 * 64 - 1; ++i)
                    {
                        setTileIndex(tilemap, boardIndex++, tiles[0], level->getDefaultTileset());
                        setTileIndex(tilemap, boardIndex++, tiles[1], level->getDefaultTileset());
                    }

                    // Clean up.
                    tiles[0] = tiles[1] = -1;
                    doubleMode = false;
                    count = 1;
                }
                // Regular RLE scheme.
                else
                {
                    for (int i = 0; i < count && boardIndex < 64 * 64; ++i)
                        setTileIndex(tilemap, boardIndex++, code, level->getDefaultTileset());
                    count = 1;
                }
            }
        }

        // Load the links.
        {
            while (!stream->atEnd())
            {
                auto line = readString(stream, '\n');
                if (line.length() == 0 || line == "#") break;

                auto words = line.split(' ');
                if (words.size() >= 7)
                {
                    auto x = words[1].toDouble() * 16;
                    auto y = words[2].toDouble() * 16;
                    auto width = int(words[3].toDouble() * 16);
                    auto height = int(words[4].toDouble() * 16);

                    auto& nextX = words[5];
                    auto& nextY = words[6];

                    bool possibleEdgeLink = false;
                    if ((x == 0 || x == 63 * 16) && width == 16)
                        possibleEdgeLink = true;
                    else if ((y == 0 || y == 63 * 16) && height == 16)
                        possibleEdgeLink = true;

                    auto levelLink = new LevelLink(level->getWorld(), level->getX() + x, level->getY() + y, width, height, possibleEdgeLink);
                    levelLink->setLevel(level);
                    levelLink->setNextLevel(words[0]);
                    levelLink->setNextX(nextX);
                    levelLink->setNextY(nextY);

                    level->addObject(levelLink);
                }
            }
        }

        // Load the baddies.
        {
            while (!stream->atEnd())
            {
                signed char x = stream->read(1).data()[0];
                signed char y = stream->read(1).data()[0];
                signed char type = stream->read(1).data()[0];

                // Ends with an invalid baddy.
                if (x == -1 && y == -1 && type == -1)
                {
                    readString(stream, '\n');
                    break;
                }


                auto baddy = new LevelGraalBaddy(level->getWorld(), (x * 16) + level->getX(), (y * 16) + level->getY(), type);
                baddy->setLevel(level);

                int verseIndex = 0;

                auto verses = readString(stream, '\n').split('\\');
                for (int i = 0; i < verses.size(); ++i)
                {
                    baddy->setBaddyVerse(verseIndex++, verses[i].trimmed());
                }
                level->addObject(baddy);
            }
        }

        if (!isZelda)
        {
            // Load NPCs.
            {
                while (!stream->atEnd())
                {
                    auto line = readString(stream, '\n');
                    if (line.length() == 0 || line == "#") break;

                    auto x = (line[0].unicode() - 32) * 16;
                    auto y = (line[1].unicode() - 32) * 16;

                    int lineIndex = 2;
                    QString image = "";
                    for (; lineIndex < line.length(); ++lineIndex) {
                        if (line[lineIndex] == '#')
                            break;
                        image += line[lineIndex];
                    }

                    auto code = line.mid(lineIndex + 1).replace('\xa7', '\n');

                    auto levelNPC = new LevelNPC(level->getWorld(), x + level->getX(), y + level->getY(), 32, 32);
                    levelNPC->setLevel(level);

                    levelNPC->setImageName(image);
                    levelNPC->setCode(code);
                    level->addObject(levelNPC);
                }
            }

            // Load chests.
            if (v > 0)
            {
                while (!stream->atEnd())
                {
                    auto line = readString(stream, '\n');
                    if (line.length() == 0 || line == "#") break;

                    auto x = (line[0].unicode() - 32) * 16;
                    auto y = (line[1].unicode() - 32) * 16;
                    auto item = line[2].unicode() - 32;
                    auto signindex = line[3].unicode() - 32;

                    static const char* itemNames[] = {
                        "greenrupee",        // 0
                        "bluerupee",        // 1
                        "redrupee",            // 2
                        "bombs",            // 3
                        "darts",            // 4
                        "heart",            // 5
                        "glove1",            // 6
                        "bow",                // 7
                        "bomb",                // 8
                        "shield",            // 9
                        "sword",            // 10
                        "fullheart",        // 11
                        "superbomb",        // 12
                        "battleaxe",        // 13
                        "goldensword",        // 14
                        "mirrorshield",        // 15
                        "glove2",            // 16
                        "lizardshield",        // 17
                        "lizardsword",        // 18
                        "goldrupee",        // 19
                        "fireball",            // 20
                        "fireblast",        // 21
                        "nukeshot",            // 22
                        "joltbomb",            // 23
                        "spinattack"        // 24

                    };

                    if (item >= 0 && item < sizeof(itemNames) / sizeof(itemNames[0]))
                    {
                        auto chest = new LevelChest(level->getWorld(), x + level->getX(), y + level->getY(), itemNames[item], signindex);
                        chest->setLevel(level);

                        level->addObject(chest);
                    }
                }
            }
        }


        auto decodeSign = [](const QString& pText) -> QString
        {
            static const int ctab[] = { 91, 92, 93, 94, 77, 78, 79, 80, 74, 75, 71, 72, 73, 86, 86, 87, 88, 67 };
            static const int ctabindex[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 15, 17 };
            static const QString signSymbols = "ABXYudlrhxyz#4.";
            static const QString signText = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                "0123456789!?-.,#>()#####\"####':/~&### <####;\n";

            QString retVal = "";
            int txtLen = pText.length();
            for (int i = 0; i < txtLen; i++)
            {
                unsigned char letter = pText[i].unicode() - 32;
                bool isCode = false;
                int codeID = -1;
                for (int j = 0; j < 16; ++j)	// ctab length
                {
                    if (letter == ctab[j])
                    {
                        codeID = j;
                        isCode = true;
                        break;
                    }
                }

                if (isCode)
                {
                    int codeIndex = -1;
                    for (int j = 0; j < 14; ++j)	// ctabindex
                    {
                        if (ctabindex[j] == codeID)
                        {
                            codeIndex = j;
                            break;
                        }
                    }
                    if (codeIndex != -1)
                        retVal.append('#').append(signSymbols[codeIndex]);
                }
                else
                    retVal.append(signText[letter]);
            }

            return retVal;
        };

        // Load signs.
        {
            while (!stream->atEnd())
            {
                auto line = readString(stream, '\n');
                if (line.length() == 0) break;

                auto x = (line[0].unicode() - 32) * 16;
                auto y = (line[1].unicode() - 32) * 16;
                auto encodedText = line.mid(2);

                auto sign = new LevelSign(level->getWorld(),
                    level->getX() + x,
                    level->getY() + y,
                    32,
                    16);

                sign->setLevel(level);

                sign->setText(decodeSign(encodedText));

                level->addObject(sign);
            }
        }
        return true;
    }

    bool LevelFormatGraal::saveLevel(Level* level, QIODevice* stream)
    {
        auto tilemap = level->getTilemap(0);

        if (tilemap)
        {
            int buffer = 0;
            int bitCount = 0;
            stream->write("GR-V1.03", 8);

            for (int y = 0; y < 64; ++y)
            {
                for (int x = 0; x < 64; ++x)
                {
                    while (bitCount >= 8)
                    {
                        unsigned char byte = buffer & 0xFF;
                        buffer >>= 8;
                        stream->write((char*)&byte, 1);
                        bitCount -= 8;
                    }

                    auto tile = tilemap->getTile(x, y);
                    auto tileLeft = Tilemap::GetTileX(tile);
                    auto tileTop = Tilemap::GetTileY(tile);

                    //Gonstruct: Converts tile positions (lef/top) to graal tile index
                    auto graalTileIndex = tileLeft / 16 * 512 + tileLeft % 16 + tileTop * 16;

                    buffer = (graalTileIndex << bitCount) | buffer;
                    bitCount += 13;
                }
            }


            //Write remaining bits.
            while (bitCount > 0)
            {
                unsigned char byte = buffer & 0xFF;
                buffer >>= 8;
                stream->write((char*)&byte, 1);
                bitCount -= 8;
            }

            //links
            {
                for (auto link : level->getLinks())
                {
                    auto linkString = QString("%1 %2 %3 %4 %5 %6 %7\n").arg(link->getNextLevel()).arg(int((link->getX() - level->getX()) / 16.0)).arg(int((link->getY() - level->getY()) / 16.0)).arg(link->getWidth() / 16).arg(link->getHeight() / 16).arg(link->getNextX()).arg(link->getNextY());
                    stream->write(linkString.toLocal8Bit().data());
                }
                stream->write("#\n");
            }

            //baddies
            {
                for (auto obj : level->getObjects())
                {
                    if (obj->getEntityType() == LevelEntityType::ENTITY_BADDY)
                    {
                        auto baddy = static_cast<LevelGraalBaddy*>(obj);
                        char x = char((baddy->getX() - level->getX()) / 16.0);
                        char y = char((baddy->getY() - level->getY()) / 16.0);
                        char type = baddy->getBaddyType();

                        stream->write(&x, 1);
                        stream->write(&y, 1);
                        stream->write(&type, 1);

                        auto verses = QString("%1\\%2\\%3\\").arg(baddy->getBaddyVerse(0)).arg(baddy->getBaddyVerse(1)).arg(baddy->getBaddyVerse(2));
                        stream->write(verses.toLocal8Bit());
                        stream->write("\n");
                    }
                }
                char negative = -1;
                stream->write(&negative, 1);
                stream->write(&negative, 1);
                stream->write(&negative, 1);
                stream->write("\n", 1);;
            }

            //npcs
            {
                for (auto obj : level->getObjects())
                {
                    if (obj->getEntityType() == LevelEntityType::ENTITY_NPC)
                    {

                        auto npc = static_cast<LevelNPC*>(obj);
                        char x = 32 + char((npc->getX() - level->getX()) / 16.0);
                        char y = 32 + char((npc->getY() - level->getY()) / 16.0);

                        auto code = npc->getCode();

                        QByteArray npcLine;
                        npcLine.append(x);
                        npcLine.append(y);
                        npcLine.append(npc->getImageName().toLocal8Bit());
                        npcLine.append('#');
                        npcLine.append(code.replace('\n', '\xa7').toLocal8Bit());
                        npcLine.append('\n');

                        stream->write(npcLine.data());

                    }
                }
                stream->write("\n");

            }

            //chests
            {
                static QStringList itemNames = {
                    "greenrupee",        // 0
                    "bluerupee",        // 1
                    "redrupee",            // 2
                    "bombs",            // 3
                    "darts",            // 4
                    "heart",            // 5
                    "glove1",            // 6
                    "bow",                // 7
                    "bomb",                // 8
                    "shield",            // 9
                    "sword",            // 10
                    "fullheart",        // 11
                    "superbomb",        // 12
                    "battleaxe",        // 13
                    "goldensword",        // 14
                    "mirrorshield",        // 15
                    "glove2",            // 16
                    "lizardshield",        // 17
                    "lizardsword",        // 18
                    "goldrupee",        // 19
                    "fireball",            // 20
                    "fireblast",        // 21
                    "nukeshot",            // 22
                    "joltbomb",            // 23
                    "spinattack"        // 24

                };

                for (auto obj : level->getObjects())
                {
                    if (obj->getEntityType() == LevelEntityType::ENTITY_CHEST)
                    {

                        auto chest = static_cast<LevelChest*>(obj);

                        auto itemIndex = itemNames.indexOf(chest->getItemName());
                        if (itemIndex >= 0)
                        {
                            char x = 32 + char((chest->getX() - level->getX()) / 16.0);
                            char y = 32 + char((chest->getY() - level->getY()) / 16.0);
                            char itemIndexGInt = char(32 + itemIndex);


                            QByteArray chestLine;
                            chestLine.append(x);
                            chestLine.append(y);
                            chestLine.append(itemIndexGInt);
                            chestLine.append(char(32 + chest->getSignIndex()));
                            chestLine.append('\n');
                            stream->write(chestLine.data());
                        }

                    }
                }
                stream->write("\n");

            }

            //signs
            {
                auto encodeSign = [](const QString& pText) -> QByteArray
                {

                    static const QString signText = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                        "0123456789!?-.,#>()#####\"####':/~&### <####;\n";
                    static const QString signSymbols = "ABXYudlrhxyz#4.";
                    static const int ctablen[] = { 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 1 };
                    static const int ctabindex[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 15, 17 };
                    static const int ctab[] = { 91, 92, 93, 94, 77, 78, 79, 80, 74, 75, 71, 72, 73, 86, 86, 87, 88, 67 };

                    QByteArray retVal;
                    int txtLen = pText.length();

                    for (int i = 0; i < txtLen; i++)
                    {
                        auto letter = pText[i].unicode();

                        if (letter == '#')
                        {
                            i++;
                            if (i < txtLen)
                            {
                                letter = pText[i].unicode();
                                auto code = signSymbols.indexOf(letter);
                                if (code != -1)
                                {
                                    for (int ii = 0; ii < ctablen[code]; ii++)
                                    {
                                        retVal.push_back(32 + (char)ctab[ctabindex[code] + ii]);
                                    }
                                    continue;
                                }
                                else letter = pText[--i].unicode();
                            }
                        }


                        int code = signText.indexOf(letter);
                        if (letter == '#') code = 86;

                        if (code != -1) {
                            retVal.push_back(32 + code);
                        }
                    }
                    return retVal;
                };

                for (auto sign : level->getSigns())
                {
                    char x = 32 + char((sign->getX() - level->getX()) / 16.0);
                    char y = 32 + char((sign->getY() - level->getY()) / 16.0);

                    QByteArray line;
                    line.append(x);
                    line.append(y);

                    line.append(encodeSign(sign->getText()).data());
                    line.append('\n');
                    stream->write(line.data());
                }


            }

            return true;


        }
        return false;
    }

    void LevelFormatGraal::applyFormat(Level* level)
    {
        level->setUnitWidth(16);
        level->setUnitHeight(16);

        auto layers = level->getTileLayers();

        //Delete all layers other than 0
        for (auto layer : layers)
        {
            if(layer->getLayerIndex() != 0)
                level->deleteTileLayer(layer->getLayerIndex());
        }
    }
};
