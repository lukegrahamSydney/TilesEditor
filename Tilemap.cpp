#include "Tilemap.h"

namespace TilesEditor
{
	Tilemap::Tilemap( Level* level, double x, double y, int hcount, int vcount, int layerIndex) :
		AbstractLevelEntity(level, x, y), m_hcount(hcount), m_vcount(vcount)
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

        auto image = tilesetImage;

        if (image != nullptr)
        {
            auto& pixmap = image->pixmap();

            QRectF dstRect(0, 0, -1, -1),
                srcRect(0, 0, tileWidth, tileHeight);

            for (int y2 = top; y2 <= bottom; ++y2)
            {
                for (int x2 = left; x2 <= right; ++x2)
                {
                    int tile = 0;


                    if (tryGetTile(x2, y2, &tile))
                    {
                        dstRect.moveTo(x + (x2 * tileWidth), y + (y2 * tileHeight));
                        srcRect.moveTo(Tilemap::GetTileX(tile) * tileWidth, Tilemap::GetTileY(tile) * tileHeight);

                        painter->drawPixmap(dstRect, pixmap, srcRect);
                    }
                }
            }
        }
    }
};

