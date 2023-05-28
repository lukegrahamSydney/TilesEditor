#include <QStringList>
#include <QTextStream>
#include <QStringBuilder>
#include <QFile>
#include <QDebug>

#include "Level.h"
#include "ImageDimensions.h"
#include "Tileset.h"
#include "Overworld.h"
#include "LevelNPC.h"
#include "LevelLink.h"
#include "LevelSign.h"
#include "LevelChest.h"
#include "LevelGraalBaddy.h"
#include "ObjectFactory.h"

#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
 
    QMap<QString, int> Level::m_imageDimensionsCache;


    bool Level::getImageDimensions(ResourceManager& resourceManager, const QString& imageName, int* w, int* h)
    {
        auto it = m_imageDimensionsCache.find(imageName);

        if (it != m_imageDimensionsCache.end()) {
            int size = it.value();
            *w = size >> 16;
            *h = size & 0xFFFF;
            return true;
        }
        else {
            QString fileName;

            if (resourceManager.locateFile(imageName, &fileName))
            {
                if (ImageDimensions::getImageFileDimensions(fileName, w, h))
                {
                    m_imageDimensionsCache[fileName] = (*w << 16) | *h;
                    return true;
                }
                else m_imageDimensionsCache[fileName] = 0;
            }

        }
        return false;
    }

    Level::Level(IWorld* world, double x, double y, int width, int height, Overworld* overworld, const QString& name)
    {
        m_world = world;
        m_loadFail = false;
        m_modified = false;
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        m_unitWidth = 16;
        m_unitHeight = 16;
        m_overworld = overworld;
        m_name = name;

        m_loaded = false;
        m_entitySpatialMap = nullptr;
        m_mainTileLayer = nullptr;
        if (!m_overworld)
        {
            m_entitySpatialMap = new EntitySpatialGrid<AbstractLevelEntity>(getX(), getY(), getWidth(), getHeight());

        }

    }

    Level::~Level()
    {
        for (auto entity : m_objects)
        {
            delete entity;
        }

        if (m_entitySpatialMap)
            delete m_entitySpatialMap;


        for (auto tilemap : m_tileLayers)
            delete tilemap;
    }

    void Level::release()
    {
        for (auto entity : m_objects)
        {
            entity->releaseResources();
        }

    }

    bool Level::loadFile()
    {
        auto stream = m_world->getResourceManager().getFileSystem()->openStream(m_fileName, QIODeviceBase::ReadOnly);
        if (stream)
        {
            loadStream(stream);
            delete stream;
        }
        return !m_loadFail;
    }

    bool Level::loadStream(QIODevice* stream)
    {
        if (m_name.endsWith(".nw"))
            m_loadFail = !loadNWStream(stream);

        else if (m_name.endsWith(".graal") || m_name.endsWith(".zelda") || m_name.endsWith(".editor"))
            m_loadFail = !loadGraalStream(stream);

        else if (m_name.endsWith(".lvl"))
            m_loadFail = !loadLVLStream(stream);

        return !m_loadFail;
    }

    bool Level::loadNWStream(QIODevice* stream)
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
                m_width = 64 * 16;
                m_height = 64 * 16;



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

                            auto tilemap = getOrMakeTilemap(layer);
                            auto& tileData = words[5];

                            if (x < 64 && y < 64 && x + width <= 64)
                            {
                                if (tileData.length() >= width * 2)
                                {
                                    for (size_t ii = 0u; ii < width; ++ii)
                                    {
                                        char byte1 = tileData[ii * 2].unicode();
                                        char byte2 = tileData[1 + ii * 2].unicode();

                                        auto graalTile = (int)((base64.indexOf(byte1) << 6) + base64.indexOf(byte2));
                                        auto tile = convertFromGraalTile( graalTile);
                                        tilemap->setTile(x + ii, y, tile);

                                    }
                                }
                            }
                        }
                        else if (words[0] == "TILESET" && wordCount >= 2)
                        {
                            m_tilesetName = words[1];
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

                            auto levelLink = new LevelLink(m_world, getX() + x, getY() + y, width, height, possibleEdgeLink);
                            levelLink->setLevel(this);
                            levelLink->setNextLevel(words[1]);
                            levelLink->setNextX(nextX);
                            levelLink->setNextY(nextY);

                            addObject(levelLink);
                        }
                        else if (words[0] == "CHEST" && wordCount >= 5)
                        {
                            auto& itemName = words[3];

                            auto x = words[1].toDouble() * 16;
                            auto y = words[2].toDouble() * 16;

                            auto signIndex = words[4].toInt();
                            
                            auto chest = new LevelChest(m_world, x + getX(), y + getY(), itemName, signIndex);
                            chest->setLevel(this);

                            addObject(chest);

                        }
                        else if (words[0] == "BADDY" && wordCount >= 3)
                        {
                            auto x = words[1].toDouble() * 16;
                            auto y = words[2].toDouble() * 16;
                            auto type = words[3].toInt();

                            auto baddy = new LevelGraalBaddy(m_world, x + getX(), y + getY(), type);
                            baddy->setLevel(this);

                            int verseIndex = 0;
                            for (++i; i < lineCount && lines[i] != "BADDYEND"; ++i)
                            {
                                baddy->setBaddyVerse(verseIndex++, lines[i].trimmed());
                            }

                            addObject(baddy);

                        }
                        else if (words[0] == "SIGN" && wordCount >= 3)
                        {
                            
                            auto sign = new LevelSign(m_world,
                                getX() + words[1].toDouble() * 16,
                                getY() + words[2].toDouble() * 16,
                                32,
                                16);

                            sign->setLevel(this);

                            QString text = "";
                            for (++i; i < lineCount && lines[i] != "SIGNEND"; ++i)
                                text += lines[i] + '\n';
                            sign->setText(text);


                            addObject(sign);

                        }
                        else if (words[0] == "NPC" && wordCount >= 4)
                        {

                            QString image = (words[1] == "-" ? "" : words[1]),
                                code;

                            double x = words[2].toDouble() * 16.0,
                                y = words[3].toDouble() * 16.0;

                            int width = 48, height = 48;

                            for (++i; i < lineCount && lines[i] != "NPCEND"; ++i)
                                code += lines[i] + "\n";
                            

                            QString className = "";

                            auto levelNPC = new LevelNPC(m_world, x + getX(), y + getY(), width, height);
                            levelNPC->setLevel(this);

                            levelNPC->setImageName(image);
                            levelNPC->setCode(code);


                            addObject(levelNPC);
                        }
                    }
                }
                m_loaded = true;
                return true;
            }
            
            return false;
        }
        return false;

    }

    //Ripped from graal reborn gserver2 code (TLevel.cpp)
    bool Level::loadGraalStream(QIODevice* stream)
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
            int i = 0;
            while (base->read((char*)&byte, 1) > 0) {
                if (byte == end)
                    break;

                retval.append((char)byte);
                ++i;
            }
            return retval;
        };

        // Load tiles.
        {
            m_width = 64 * 16;
            m_height = 64 * 16;

            int bits = (v > 1 ? 13 : 12);
            int read = 0;
            unsigned int buffer = 0;
            unsigned short code = 0;
            short tiles[2] = { -1,-1 };
            int boardIndex = 0;
            int count = 1;
            bool doubleMode = false;

            auto tilemap = getOrMakeTilemap(0);

            auto setTileIndex = [](Tilemap* tilemap, int boardIndex, int graalTile) {
                auto tileX = boardIndex % 64;
                auto tileY = boardIndex / 64;

                auto tile = convertFromGraalTile(graalTile);

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
                    stream->read((char*) & byte, 1);

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
                    setTileIndex(tilemap, boardIndex++, code);
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
                        setTileIndex(tilemap, boardIndex++, tiles[0]);
                        setTileIndex(tilemap, boardIndex++, tiles[1]);
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
                        setTileIndex(tilemap, boardIndex++, code);
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

                    auto levelLink = new LevelLink(m_world, getX() + x, getY() + y, width, height, possibleEdgeLink);
                    levelLink->setLevel(this);
                    levelLink->setNextLevel(words[0]);
                    levelLink->setNextX(nextX);
                    levelLink->setNextY(nextY);

                    addObject(levelLink);
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


                auto baddy = new LevelGraalBaddy(m_world, (x * 16) + getX(), (y * 16) + getY(), type);
                baddy->setLevel(this);

                int verseIndex = 0;

                auto verses = readString(stream, '\n').split('\\');
                for (int i = 0; i < verses.size(); ++i)
                {
                    baddy->setBaddyVerse(verseIndex++, verses[i].trimmed());
                }
                addObject(baddy);
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

                    auto levelNPC = new LevelNPC(m_world, x + getX(), y + getY(), 32, 32);
                    levelNPC->setLevel(this);

                    levelNPC->setImageName(image);
                    levelNPC->setCode(code);
                    addObject(levelNPC);
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
                        auto chest = new LevelChest(m_world, x + getX(), y + getY(), itemNames[item], signindex);
                        chest->setLevel(this);

                        addObject(chest);
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

                auto sign = new LevelSign(m_world,
                    getX() + x,
                    getY() + y,
                    32,
                    16);

                sign->setLevel(this);

                sign->setText(decodeSign(encodedText));

                addObject(sign);
            }
        }
        m_loaded = true;
        return true;
    }

    bool Level::loadLVLStream(QIODevice* stream)
    {
        static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        QTextStream textStream(stream);

        QString text = textStream.readAll();
        if (!text.isEmpty())
        {
            auto jsonRoot = cJSON_Parse(text.toLocal8Bit().data());

            if (jsonRoot)
            {
                auto type = jsonGetChildString(jsonRoot, "type");
                auto version = jsonGetChildString(jsonRoot, "version");


                if (type == "level" && version == "1.0")
                {

                    auto hcount = jsonGetChildInt(jsonRoot, "hcount", 1);
                    auto vcount = jsonGetChildInt(jsonRoot, "vcount", 1);

                    m_width = hcount * 16;
                    m_height = vcount * 16;
                    m_tilesetName = jsonGetChildString(jsonRoot, "tileset", "");

                    m_unitWidth = 1;
                    m_unitHeight = 1;
                    if (m_entitySpatialMap) {
                        delete m_entitySpatialMap;
                        m_entitySpatialMap = new EntitySpatialGrid<AbstractLevelEntity>(getX(), getY(), getWidth(), getHeight());
                    }


                    auto jsonLayers = cJSON_GetObjectItem(jsonRoot, "tileLayers");
                    if (jsonLayers)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonLayers); ++i)
                        {
                            auto jsonLayer = cJSON_GetArrayItem(jsonLayers, i);

                            if (jsonLayer)
                            {
                                auto index = jsonGetChildInt(jsonLayer, "index");


                                auto jsonChunks = cJSON_GetObjectItem(jsonLayer, "chunks");
                                if (jsonChunks)
                                {
                                    auto tilemap = new Tilemap(m_world, getX(), getY(), hcount, vcount, index);
                                    tilemap->setLevel(this);
                                    tilemap->clear(Tilemap::MakeInvisibleTile(0));
                                    for (int ii = 0; ii < cJSON_GetArraySize(jsonChunks); ++ii)
                                    {
                                        auto jsonChunk = cJSON_GetArrayItem(jsonChunks, ii);

                                        if (jsonChunk)
                                        {
                                            auto left = jsonGetArrayInt(jsonChunk, 0);
                                            auto top = jsonGetArrayInt(jsonChunk, 1);

                                            auto line = jsonGetArrayString(jsonChunk, 2, "");

                                            auto parts = line.split(' ', Qt::SkipEmptyParts);
                                            for (auto x = 0U; x < parts.size(); ++x)
                                            {
                                                int tile = 0;
                                                auto& part = parts[x];
                                                int bitcount = 0;


                                                for (auto i = part.length() - 1; i >= 0; --i) {
                                                    auto value = base64.indexOf(part[i]);

                                                    tile |= value << bitcount;
                                                    bitcount += 6;
                                                }
                                                tilemap->setTile(left + x, top, tile);
                                            }

                                        }
                                    }

                                    setTileLayer(index, tilemap);
                                }
                            }
                        }
                    }

                    auto jsonSigns = cJSON_GetObjectItem(jsonRoot, "signs");
                    if (jsonSigns)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonSigns); ++i)
                        {
                            auto jsonSign = cJSON_GetArrayItem(jsonSigns, i);

                            if (jsonSign)
                            {

                                auto x = this->getX() + jsonGetChildInt(jsonSign, "x");
                                auto y = this->getY() + jsonGetChildInt(jsonSign, "y");

                                auto width = jsonGetChildInt(jsonSign, "width");
                                auto height = jsonGetChildInt(jsonSign, "height");


                                auto sign = new LevelSign(m_world, x, y, width, height);
                                sign->setLevel(this);

                                sign->setText(jsonGetChildString(jsonSign, "text"));

                                addObject(sign);

                            }
                        }
                    }

                    auto jsonLinks = cJSON_GetObjectItem(jsonRoot, "links");
                    if (jsonLinks)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonLinks); ++i)
                        {
                            auto jsonLink = cJSON_GetArrayItem(jsonLinks, i);

                            if (jsonLink)
                            {

                                auto x = this->getX() + jsonGetChildInt(jsonLink, "x");
                                auto y = this->getY() + jsonGetChildInt(jsonLink, "y");

                                auto width = jsonGetChildInt(jsonLink, "width");
                                auto height = jsonGetChildInt(jsonLink, "height");


                                auto link = new LevelLink(m_world, x, y, width, height, false);
                                link->setLevel(this);

                                link->setNextLevel(jsonGetChildString(jsonLink, "destination"));
                                link->setNextX(jsonGetChildString(jsonLink, "destinationX"));
                                link->setNextY(jsonGetChildString(jsonLink, "destinationY"));
                                addObject(link);

                            }
                        }
                    }

                    auto jsonObjects = cJSON_GetObjectItem(jsonRoot, "objects");
                    if (jsonObjects)
                    {
                        for (int i = 0; i < cJSON_GetArraySize(jsonObjects); ++i)
                        {
                            auto jsonObject = cJSON_GetArrayItem(jsonObjects, i);

                            auto entity = ObjectFactory::createObject(m_world, jsonObject);

                            if (entity)
                            {
                                entity->setLevel(this);
                                addObject(entity);
                            }

                        }
                    }
                    cJSON_Delete(jsonRoot);
                    m_loaded = true;
                    return true;
                }
                cJSON_Delete(jsonRoot);
            }
        }
        return false;
    }

    bool Level::saveFile(IFileRequester* requester)
    {
        auto stream = m_world->getResourceManager().getFileSystem()->openStream(m_fileName, QIODevice::WriteOnly);

        if (stream)
        {
            auto retval = saveStream(stream);
            m_world->getResourceManager().getFileSystem()->endWrite(requester, m_fileName, stream);

            return retval;
        }

        return false;
    }

    bool Level::saveStream(QIODevice* stream)
    {
        if (m_name.endsWith(".nw"))
            return saveNWStream(stream);

        if (m_name.endsWith(".graal") || m_name.endsWith(".zelda") || m_name.endsWith(".editor"))
            return saveGraalStream(stream);

        if (m_name.endsWith(".lvl"))
            return saveLVLStream(stream);

        return false;
    }

    bool Level::saveNWStream(QIODevice* _stream)
    {
        QTextStream stream(_stream);

        stream << "GLEVNW01" << Qt::endl;

        auto writeTileLayer = [](QTextStream& stream, Tilemap* tilemap)
        { 
            for (int top = 0; top < 64; ++top)
            {
                for(int left = 0; left < 64; ++left)
                {
                    //Start scanning from left until we hit a NON-invisible title
                    if (!Tilemap::IsInvisibleTile(tilemap->getTile(left, top)))
                    {
                        //Continue scanning until we hit the end, or an invisible tile
                        int right = left;
                        for (; right < 64; ++right)
                        {
                            if (Tilemap::IsInvisibleTile(tilemap->getTile(right, top)))
                                break;
                        }

                        int width = right - left;
                        if (width > 0)
                        {
                            stream << "BOARD " << left << " " << top << " " << width << " " << (int)tilemap->getDepth() << " ";
                            for (;left < right; ++left)
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

        for (auto layer : m_tileLayers)
        {
            writeTileLayer(stream, layer);
                
        }
        stream << Qt::endl;

        for (auto link : m_links)
        {
            stream << "LINK " << link->getNextLevel() << " " << int((link->getX() - getX()) / 16.0) << " " << int((link->getY() - getY()) / 16.0) << " " << link->getWidth() / 16 << " " << link->getHeight() / 16 << " " << link->getNextX() << " " << link->getNextY() << Qt::endl;
        }

        for (auto sign : m_signs)
        {
            stream << "SIGN " << int((sign->getX() - getX()) / 16.0) << " " << int((sign->getY() - getY()) / 16.0) << Qt::endl;
            stream << sign->getText();

            if (!sign->getText().endsWith('\n'))
                stream << Qt::endl;

            stream << "SIGNEND" << Qt::endl << Qt::endl;

        }

        for (auto obj : m_objects)
        {
            switch (obj->getEntityType())
            {
                case LevelEntityType::ENTITY_NPC:
                {
                    auto npc = static_cast<LevelNPC*>(obj);

                    auto imageName = npc->getImageName();
                    if (imageName.isEmpty())
                        imageName = "-";

                    stream << "NPC " << imageName << " " << std::floor(((npc->getX() - getX()) / 16.0) * 1000.0) / 1000.0 << " " << std::floor(((npc->getY() - getY()) / 16.0) * 1000.0) / 1000.0 << Qt::endl;

                    stream << npc->getCode();

                    if (!npc->getCode().endsWith('\n'))
                        stream << Qt::endl;
                    stream << "NPCEND" << Qt::endl << Qt::endl;
                }
                break;

                case LevelEntityType::ENTITY_CHEST:
                {
                    auto chest = static_cast<LevelChest*>(obj);

                    stream << "CHEST " << int((chest->getX() - getX()) / 16.0) << " " << int((chest->getY() - getY()) / 16.0) << " " << chest->getItemName() << " " << chest->getSignIndex() << Qt::endl;
                }
                break;

                case LevelEntityType::ENTITY_BADDY:
                {
                    auto baddy = static_cast<LevelGraalBaddy*>(obj);

                    stream << "BADDY " << int((baddy->getX() - getX()) / 16.0) << " " << int((baddy->getY() - getY()) / 16.0) << " " << baddy->getBaddyType() << Qt::endl;
                    for (auto i = 0; i < 3; ++i)
                    {
                        stream << baddy->getBaddyVerse(i) << Qt::endl;
                    }
                    stream << "BADDYEND" << Qt::endl << Qt::endl;
                }
                break;
            }

        }

        if(!m_tilesetName.isEmpty())
            stream << "TILESET " << m_tilesetName << Qt::endl;
        return true;
    }

    bool Level::saveGraalStream(QIODevice* stream)
    {
        auto tilemap = getTilemap(0);

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
                for (auto link : m_links)
                {
                    auto linkString = QString("%1 %2 %3 %4 %5 %6 %7\n").arg(link->getNextLevel()).arg(int((link->getX() - getX()) / 16.0)).arg(int((link->getY() - getY()) / 16.0)).arg(link->getWidth() / 16).arg(link->getHeight() / 16).arg(link->getNextX()).arg(link->getNextY());
                    stream->write(linkString.toLocal8Bit().data());
                }
                stream->write("#\n");
            }

            //baddies
            {
                for (auto obj : m_objects)
                {
                    if (obj->getEntityType() == LevelEntityType::ENTITY_BADDY)
                    {
                        auto baddy = static_cast<LevelGraalBaddy*>(obj);
                        char x = char((baddy->getX() - this->getX()) / 16.0);
                        char y = char((baddy->getY() - this->getY()) / 16.0);
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
                for (auto obj : m_objects)
                {
                    if (obj->getEntityType() == LevelEntityType::ENTITY_NPC)
                    {
                            
                        auto npc = static_cast<LevelNPC*>(obj);
                        char x = 32 + char((npc->getX() - this->getX()) / 16.0);
                        char y = 32 + char((npc->getY() - this->getY()) / 16.0);

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

                for (auto obj : m_objects)
                {
                    if (obj->getEntityType() == LevelEntityType::ENTITY_CHEST)
                    {

                        auto chest = static_cast<LevelChest*>(obj);

                        auto itemIndex = itemNames.indexOf(chest->getItemName());
                        if (itemIndex >= 0)
                        {
                            char x = 32 + char((chest->getX() - this->getX()) / 16.0);
                            char y = 32 + char((chest->getY() - this->getY()) / 16.0);
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

                auto signs = getSigns();
                for (auto sign : signs)
                {
                    char x = 32 + char((sign->getX() - this->getX()) / 16.0);
                    char y = 32 + char((sign->getY() - this->getY()) / 16.0);

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
        else return false;
    }

    bool Level::saveLVLStream(QIODevice* stream)
    {
        auto getJSONChunks = [](Tilemap* tilemap) -> cJSON*
        {
            cJSON* retval = nullptr;
            for (int top = 0; top < tilemap->getVCount(); ++top)
            {
                //Start scanning from left until we hit a NON-invisible title
                for (int left = 0; left < tilemap->getHCount(); ++left)
                {
                    //A visible tile has been found, now lets find the right end of the chunk
                    if (!Tilemap::IsInvisibleTile(tilemap->getTile(left, top)))
                    {
                        //Continue scanning until we hit the end, or an invisible tile
                        int right = left;
                        for (; right < tilemap->getHCount(); ++right)
                        {
                            if (Tilemap::IsInvisibleTile(tilemap->getTile(right, top)))
                                break;
                        }

                        int width = right - left;
                        if (width > 0)
                        {


                            auto jsonChunk = cJSON_CreateArray();
                            
                            cJSON_AddItemToArray(jsonChunk, cJSON_CreateNumber(left));
                            cJSON_AddItemToArray(jsonChunk, cJSON_CreateNumber(top));

                            QString tileData;
                            for (; left < right; ++left)
                            {
                                static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

                                QString tileString;
                                int tile = tilemap->getTile(left, top);

                                do {
                                 
                                    tileString = base64[tile & 0x3F] + tileString;
                                    tile = tile >> 6;
                                } while (tile != 0);

                                tileData += tileString + " ";

                            }

                            cJSON_AddItemToArray(jsonChunk, cJSON_CreateString(tileData.toLocal8Bit().data()));

                            if (retval == nullptr)
                                retval = cJSON_CreateArray();

                            cJSON_AddItemToArray(retval, jsonChunk);
                        }
                        left = right;
                    }
                }
            }

            return retval;

        };

        
        auto jsonRoot = cJSON_CreateObject();

        cJSON_AddStringToObject(jsonRoot, "type", "level");
        cJSON_AddStringToObject(jsonRoot, "version", "1.0");
        cJSON_AddNumberToObject(jsonRoot, "hcount", getWidth() / 16);
        cJSON_AddNumberToObject(jsonRoot, "vcount", getHeight() / 16);

        if (!m_tilesetName.isEmpty())
            cJSON_AddStringToObject(jsonRoot, "tileset", m_tilesetName.toLocal8Bit().data());
            

        if (m_tileLayers.size() > 0)
        {
            auto jsonLayers = cJSON_CreateArray();

            for (auto layer : m_tileLayers)
            {
                auto jsonChunks = getJSONChunks(layer);
                if (jsonChunks)
                {
                    auto jsonLayer = cJSON_CreateObject();
                    cJSON_AddNumberToObject(jsonLayer, "index", layer->getLayerIndex());
                    cJSON_AddItemToObject(jsonLayer, "chunks", jsonChunks);

                    cJSON_AddItemToArray(jsonLayers, jsonLayer);
                }
            }

            cJSON_AddItemToObject(jsonRoot, "tileLayers", jsonLayers);
        }

        if (m_signs.size() > 0)
        {
            auto jsonSigns = cJSON_CreateArray();
            for (auto sign : m_signs)
            {
                auto jsonSign = cJSON_CreateObject();

                cJSON_AddNumberToObject(jsonSign, "x", int(sign->getX() - this->getX()));
                cJSON_AddNumberToObject(jsonSign, "y", int(sign->getY() - this->getY()));
                cJSON_AddNumberToObject(jsonSign, "width", sign->getWidth());
                cJSON_AddNumberToObject(jsonSign, "height", sign->getHeight());

                cJSON_AddStringToObject(jsonSign, "text", sign->getText().toLocal8Bit().data());

                cJSON_AddItemToArray(jsonSigns, jsonSign);
;               }

            cJSON_AddItemToObject(jsonRoot, "signs", jsonSigns);
        }

        if (m_links.size() > 0)
        {
            auto jsonLinks = cJSON_CreateArray();
            for (auto link : m_links)
            {
                auto jsonLink = cJSON_CreateObject();

                cJSON_AddNumberToObject(jsonLink, "x", int(link->getX() - this->getX()));
                cJSON_AddNumberToObject(jsonLink, "y", int(link->getY() - this->getY()));
                cJSON_AddNumberToObject(jsonLink, "width", link->getWidth());
                cJSON_AddNumberToObject(jsonLink, "height", link->getHeight());

                cJSON_AddStringToObject(jsonLink, "destination", link->getNextLevel().toLocal8Bit().data());
                cJSON_AddStringToObject(jsonLink, "destinationX", link->getNextX().toLocal8Bit().data());
                cJSON_AddStringToObject(jsonLink, "destinationY", link->getNextY().toLocal8Bit().data());
                cJSON_AddItemToArray(jsonLinks, jsonLink);
      
            }

            cJSON_AddItemToObject(jsonRoot, "links", jsonLinks);
        }

        auto jsonObjects = cJSON_CreateArray();

        for (auto obj : m_objects)
        {
            switch (obj->getEntityType()) 
            {
                case LevelEntityType::ENTITY_LINK:
                case LevelEntityType::ENTITY_SIGN:
                    break;

                default: {
                    auto jsonObj = obj->serializeJSON();
                    if (jsonObj)
                    {
                        cJSON_AddItemReferenceToArray(jsonObjects, jsonObj);

                    
                    }
                }
                   
            }

        }

        cJSON_AddItemToObject(jsonRoot, "objects", jsonObjects);

        auto levelText = cJSON_Print(jsonRoot);

        QTextStream textStream(stream);
        textStream << levelText;

        free(levelText);
        cJSON_Delete(jsonRoot);
        return true;
    }


    AbstractLevelEntity* Level::getObjectAt(double x, double y, LevelEntityType type)
    {
        Rectangle rect(x, y, 1, 1);
        for (auto object : m_objects)
        {
            if (object->getEntityType() == type && object->intersects(rect))
                return object;
        }
        return nullptr;
    }

    Tilemap* Level::getOrMakeTilemap(int index)
    {
        Tilemap* retval = nullptr;
        if (index == 0)
            retval = m_mainTileLayer;
        else {
            auto it = m_tileLayers.find(index);
            if (it != m_tileLayers.end())
                retval = it.value();
        }

        if (retval == nullptr)
        {
            retval = new Tilemap(m_world, getX(), getY(), getWidth() / 16, getHeight() / 16, index);
            retval->setLevel(this);

            retval->clear(Tilemap::MakeInvisibleTile(0));
            setTileLayer(index, retval);
            return retval;
        }
        else return retval;
    }

    Tilemap* Level::getTilemap(size_t layer)
    {
        if (layer == 0)
            return m_mainTileLayer;

        auto it = m_tileLayers.find(layer);
        return it == m_tileLayers.end() ? nullptr : it.value();
    }

    void Level::updateSpatialEntity(AbstractLevelEntity* entity)
    {
        if (m_overworld)
            m_overworld->getEntitySpatialMap()->updateEntity(entity);
        else m_entitySpatialMap->updateEntity(entity);
    }

    void Level::addObject(AbstractLevelEntity* object)
    {

        addEntityToSpatialMap(object);
        m_objects.insert(object);

        if (object->getEntityType() == LevelEntityType::ENTITY_LINK)
            m_links.push_back(static_cast<LevelLink*>(object));
        else if (object->getEntityType() == LevelEntityType::ENTITY_SIGN)
            m_signs.push_back(static_cast<LevelSign*>(object));
    }

    void Level::addEntityToSpatialMap(AbstractLevelEntity* object)
    {
        if (m_overworld)
        {
            m_overworld->addEntityToSpatialMap(object);
        }
        else m_entitySpatialMap->add(object);
    }

    void Level::removeObject(AbstractLevelEntity* object)
    {
        m_objects.remove(object);

        removeEntityFromSpatialMap(object);


        if (object->getEntityType() == LevelEntityType::ENTITY_LINK)
            m_links.removeOne(object);

        else if (object->getEntityType() == LevelEntityType::ENTITY_SIGN)
            m_signs.removeOne(object);
    }

    void Level::removeEntityFromSpatialMap(AbstractLevelEntity* object)
    {
        if (m_overworld)
            m_overworld->removeEntityFromSpatialMap(object);
        else m_entitySpatialMap->remove(object);
    }

    QString Level::getDisplayTile(int tile) const
    {
        auto tileLeft = Tilemap::GetTileX(tile);
        auto tileTop = Tilemap::GetTileY(tile);

        //Gonstruct: Converts tile positions (lef/top) to graal tile index
        auto tileIndex = tileLeft / 16 * 512 + tileLeft % 16 + tileTop * 16;
        return QString::number(tileIndex);
    }

    Rectangle Level::clampEntity(AbstractLevelEntity* entity)
    {
        auto left = entity->getX();
        auto top = entity->getY();
        auto right = entity->getRight();
        auto bottom = entity->getBottom();

        left = std::max(left, this->getX());
        top = std::max(top, this->getY());
        right = std::min(right, this->getRight());
        bottom = std::min(bottom, this->getBottom());

        return Rectangle(left, top, int(right - left), int(bottom - top));
    }

    void Level::setTileLayer(int index, Tilemap* tilemap)
    {

        auto it = m_tileLayers.find(index);
        if (it != m_tileLayers.end())
        {
            delete it.value();
        }
        m_tileLayers[index] = tilemap;

        if(index == 0)
            m_mainTileLayer = tilemap;
        
    }
};
