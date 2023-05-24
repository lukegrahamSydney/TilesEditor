#include "Tilemap.h"
#include <QDebug>
namespace TilesEditor
{
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

    void Tilemap::draw(QPainter* painter, const IRectangle& viewRect, double x, double y)
	{
	}

    void Tilemap::draw(QPainter* painter, const IRectangle& viewRect, Image* tilesetImage, double x, double y)
    {
        int tileWidth = 16;
        int tileHeight = 16;


        int left = qAbs((int)qFloor((qMax((int)(viewRect.getX() - x), 0) / tileWidth)));
        int top = qAbs((int)qFloor((qMax((int)(viewRect.getY() - y), 0) / tileHeight)));
        int right = qMin(left + (int)qCeil(viewRect.getRight() / tileWidth), left + (int)m_hcount);
        int bottom = qMin(top + (int)qCeil(viewRect.getBottom() / tileWidth), top + (int)m_vcount);
       

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
                            painter->setOpacity(1.0 - (translucency / 15.0));
                        }


                        srcRect.moveTo(Tilemap::GetTileX(tile) * tileWidth, Tilemap::GetTileY(tile) * tileHeight);
             
                        painter->drawPixmap(QPoint(x + (x2 * tileWidth), y + (y2 * tileHeight)), pixmap, srcRect);
                    }
                }
            }
            painter->setOpacity(1.0);
        }
    }
};

