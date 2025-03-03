#include <Level.h>
#include "Tilemap.h"
#include "IEngine.h"

namespace TilesEditor
{

    sgs_Variable Tilemap::sgs_classMembers;
    sgs_ObjInterface Tilemap::sgs_interface;

    Tilemap::Tilemap(const Tilemap& source):
        AbstractLevelEntity(source.getWorld(), source.getX(), source.getY())
    {
        m_hcount = source.m_hcount;
        m_vcount = source.m_vcount;
        m_layerIndex = source.m_layerIndex;
        m_tiles = new int[m_hcount * m_vcount];

        memcpy(m_tiles, source.m_tiles, m_hcount * m_vcount * sizeof(int));
    }

    Tilemap::Tilemap(IWorld* world, double x, double y, int hcount, int vcount, int layerIndex) :
		AbstractLevelEntity(world, x, y), m_hcount(hcount), m_vcount(vcount)
	{
		m_tiles = new int[hcount * vcount];
		clear(Tilemap::MakeTile(0, 0, 0));

		m_layerIndex = layerIndex;
	}



	void Tilemap::clear(int tile)
	{
		for (unsigned int i = 0; i < m_hcount * m_vcount; ++i)
			m_tiles[i] = tile;
	}


    Tilemap& Tilemap::operator=(const Tilemap& other)
    {
        // Guard self assignment
        if (this == &other)
            return *this; // delete[]/size=0 would also be ok

        m_hcount = other.m_hcount;
        m_vcount = other.m_vcount;
        m_layerIndex = other.m_layerIndex;
        m_tiles = new int[m_hcount * m_vcount];

        memcpy(m_tiles, other.m_tiles, m_hcount * m_vcount * sizeof(int));
        return *this;
    }

    void Tilemap::draw(QPainter* painter, const QRectF& viewRect, double x, double y)
	{
	}

    void Tilemap::draw(QPainter* painter, const QRectF& viewRect, Image* tilesetImage, double x, double y)
    {
        qreal startOpacity = painter->opacity();

        int tileWidth = 16;
        int tileHeight = 16;


        int left = qAbs((int)qFloor((qMax((int)(viewRect.x() - x), 0) / tileWidth)));
        int top = qAbs((int)qFloor((qMax((int)(viewRect.y() - y), 0) / tileHeight)));
        int right = qMin((int)qCeil((viewRect.right() - x) / tileWidth), (int)m_hcount);
        int bottom = qMin((int)qCeil((viewRect.bottom() - y) / tileWidth), (int)m_vcount);


        //keep within map dimensions
        left = left > m_hcount - 1 ? m_hcount - 1 : left;
        top = top > m_vcount - 1 ? m_vcount - 1 : top;
        right = right > m_hcount - 1 ? m_hcount - 1 : right;
        bottom = bottom > m_vcount - 1 ? m_vcount - 1 : bottom;


        if (tilesetImage != nullptr)
        {
            int currentTranslucency = 0;
            auto& pixmap = tilesetImage->pixmap();

            QRect srcRect(0, 0, tileWidth, tileHeight);

            for (int y2 = top; y2 <= bottom; ++y2)
            {
                for (int x2 = left; x2 <= right; ++x2)
                {
                    int tile = 0;


                    if (tryGetTile(x2, y2, &tile))
                    {
                        auto translucency = Tilemap::GetTileTranslucency(tile);
                        if (translucency != currentTranslucency) {
                            currentTranslucency = translucency;

                            //If completely invisible
                            if (currentTranslucency != 15)
                                painter->setOpacity(startOpacity * (1.0 - (translucency / 15.0)));
                            else painter->setOpacity(0.05);
                        }

                        auto tileLeft = Tilemap::GetTileX(tile) * tileWidth;
                        auto tileTop = Tilemap::GetTileY(tile) * tileHeight;
                        srcRect.moveTo(tileLeft, tileTop);

                       
                        painter->drawPixmap(QPoint(x + (x2 * tileWidth), y + (y2 * tileHeight)), pixmap, srcRect);


                        if (currentTranslucency == 15) {
                            auto oldOpacity = painter->opacity();
                            auto oldPen = painter->pen();
                            painter->setOpacity(1.0);
                            painter->setPen(QColor(255, 0, 0));
                            painter->drawLine(QPoint(x + (x2 * tileWidth), y + (y2 * tileHeight)), QPoint(x + (x2 * tileWidth) + 16, y + (y2 * tileHeight) + 16));
                            painter->setOpacity(oldOpacity);
                            painter->setPen(oldPen);
                        }
                    }
                }
            }
            painter->setOpacity(startOpacity);
        }
    }


    void Tilemap::registerScriptClass(IEngine* engine)
    {
        auto ctx = engine->getScriptContext();
        auto startStackSize = sgs_StackSize(ctx);


        sgs_PushString(ctx, "setTile");
        sgs_PushCFunc(ctx, [](sgs_Context* ctx) -> int
            {
                sgs_Variable thisObject;

                sgs_Method(ctx);
                sgs_Int left, top, tile;

                if (sgs_LoadArgs(ctx, "@viii", &thisObject, &left, &top, &tile))
                {
                    if (sgs_IsObjectP(&thisObject, &Tilemap::sgs_interface))
                    {
                        auto self = static_cast<Tilemap*>(thisObject.data.O->data);
                        if (self == nullptr)
                            return 0;

                        self->setTile(left, top, tile);

                    }
                }

                return 0;
            });

        sgs_PushString(ctx, "getTile");
        sgs_PushCFunc(ctx, [](sgs_Context* ctx) -> int
            {
                sgs_Variable thisObject;

                sgs_Method(ctx);
                sgs_Int left, top;

                if (sgs_LoadArgs(ctx, "@vii", &thisObject, &left, &top))
                {
                    if (sgs_IsObjectP(&thisObject, &Tilemap::sgs_interface))
                    {
                        auto self = static_cast<Tilemap*>(thisObject.data.O->data);
                        if (self == nullptr)
                            return 0;

                        int tile = 0;

                        self->tryGetTile(left, top, &tile);
                        sgs_PushInt(ctx, tile);
                        return 1;

                    }
                }

                return 0;
            });


        sgs_PushString(ctx, "clear");
        sgs_PushCFunc(ctx, [](sgs_Context* ctx) -> int
            {
                sgs_Variable thisObject;

                sgs_Method(ctx);
                sgs_Int tile;

                if (sgs_LoadArgs(ctx, "@vi", &thisObject, &tile))
                {
                    if (sgs_IsObjectP(&thisObject, &Tilemap::sgs_interface))
                    {
                        auto self = static_cast<Tilemap*>(thisObject.data.O->data);
                        if (self == nullptr)
                            return 0;

                        int tile = 0;

                        self->clear(tile);

                    }
                }

                return 0;
            });
        auto memberCount = sgs_StackSize(ctx) - startStackSize;
        sgs_CreateDict(ctx, &sgs_classMembers, memberCount);
        engine->addCPPOwnedObject(sgs_classMembers);

        auto objDestruct = [](sgs_Context* ctx, sgs_VarObj* obj) -> int
            {
                auto self = static_cast<Tilemap*>(obj->data);

                if (self == nullptr)
                    return 0;

                self->decrementAndDelete();
                return 1;
            };


        auto objMark = [](sgs_Context* C, sgs_VarObj* obj) -> int
            {
                auto self = static_cast<Level*>(obj->data);
                if (self == nullptr)
                    return 0;
                return 1;
            };

        auto objGetIndex = [](sgs_Context* C, sgs_VarObj* obj) -> int
            {
                auto self = static_cast<Level*>(obj->data);

                if (self == nullptr)
                    return 0;

                sgs_Variable propName = sgs_StackItem(C, 0);


                if (sgs_PushIndex(C, Tilemap::sgs_classMembers, propName, 1))
                    return 1;

                return 0;
            };

        auto objSetIndex = [](sgs_Context* C, sgs_VarObj* obj) -> int
            {
                auto self = static_cast<Level*>(obj->data);

                if (self == nullptr)
                    return 0;

                sgs_Variable propName = sgs_StackItem(C, 0);



                return 0;
            };


        sgs_interface = {
            "Tilemap",
            objDestruct, objMark,  /* destruct, gcmark */
            objGetIndex, objSetIndex,  /* getindex, setindex */
            NULL, NULL, NULL, NULL, /* convert, serialize, dump, getnext */
            NULL, NULL,              /* call, expr */
            NULL, &Tilemap::sgs_interface	/*proplist, parent*/
        };

    }
};

