#include <QClipboard>
#include <QApplication>
#include "TileSelection.h"
#include "Tilemap.h"
#include "Selector.h"
#include "Level.h"

#include "cJSON/JsonHelper.h"

namespace TilesEditor
{


	TileSelection::TileSelection(double x, double y, int hcount, int vcount):
		AbstractSelection(x, y)
	{
		static int invisibleTile = Tilemap::MakeInvisibleTile(0);
		m_tilesetImage = nullptr;
		m_width = hcount * 16;
		m_height = vcount * 16;
		m_cleared = false;

		m_hcount = hcount;
		m_vcount = vcount;
		m_tiles = new int[m_hcount * m_vcount];
		m_selectionStartHCount = m_selectionStartVCount = -1;

		for (int y = 0; y < m_vcount; ++y)
		{
			for (int x = 0; x < m_hcount; ++x)
			{
				m_tiles[y * m_hcount + x] = invisibleTile;
			}
		}
	}

	TileSelection::~TileSelection()
	{
		delete m_tiles;
	}

	void TileSelection::draw(QPainter* painter, const IRectangle& viewRect)
	{
		if (m_tilesetImage)
		{
			static const int tileWidth = 16;
			static const int tileHeight = 16;

			auto& pixmap = m_tilesetImage->pixmap();

			if (!pixmap.isNull())
			{
				QRectF dstRect(0, 0, -1, -1),
					srcRect(0, 0, tileWidth, tileHeight);

				for (auto y = 0; y < m_vcount; ++y)
				{
					for (auto x = 0; x < m_hcount; ++x)
					{
						auto& tile = m_tiles[y * m_hcount + x];

						dstRect.moveTo(getX() + (x * tileWidth), getY() + (y * tileHeight));
						srcRect.moveTo(Tilemap::GetTileX(tile) * tileWidth, Tilemap::GetTileY(tile) * tileHeight);

						painter->drawPixmap(dstRect, pixmap, srcRect);
					}
				}
			}
		}

		Selector::draw(painter, viewRect, getX(), getY(), m_width, m_height, QColorConstants::White, QColor(255, 255, 255, 60));
	}

	bool TileSelection::pointInSelection(double x, double y)
	{
		return x >= getX() && y >= getY() && x < getX() + m_width && y < getY() + m_height;
	}

	void TileSelection::release(ResourceManager& resourceManager)
	{
		if (m_tilesetImage)
		{
			resourceManager.freeResource(m_tilesetImage);

		}
	}

	void TileSelection::reinsertIntoWorld(IWorld* world, int layer)
	{
		if (m_cleared)
			return;

		auto levels = world->getLevelsInRect(Rectangle(getX(), getY(), m_width, m_height));

		for (auto level : levels)
		{
			auto tilemap = level->getOrMakeTilemap(layer, world->getResourceManager());
			if (tilemap != nullptr)
			{
				int destTileX = int((this->getX() - tilemap->getX()) / 16.0);
				int destTileY = int((this->getY() - tilemap->getY()) / 16.0);

				bool modified = false;
				for (int y = 0; y < m_vcount; ++y)
				{
					for (int x = 0; x < m_hcount; ++x)
					{
						int tile = m_tiles[y * m_hcount + x];
						if (!Tilemap::IsInvisibleTile(tile))
						{
							modified = true;
							tilemap->setTile(destTileX + x, destTileY + y, tile);
						}
					}
				}

				if(modified)
					world->setModified(level);

			}
		}
	}

	void TileSelection::setTilesetImage(Image* tilesetImage)
	{
		m_tilesetImage = tilesetImage;
		m_tilesetImage->incrementRef();
	}

	int TileSelection::getTile(unsigned int x, unsigned int y)
	{
		if (x < m_hcount && y < m_vcount)
		{
			return m_tiles[y * m_hcount + x];
		}
		return Tilemap::MakeInvisibleTile(0);
	}

	void TileSelection::setTile(unsigned int x, unsigned int y, int tile)
	{
		if (x < m_hcount && y < m_vcount)
		{
			m_tiles[y * m_hcount + x] = tile;
		}
	}

	bool TileSelection::clipboardCopy()
	{

		auto jsonObject = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonObject, "type", "tileSelection");
		cJSON_AddNumberToObject(jsonObject, "hcount", m_hcount);
		cJSON_AddNumberToObject(jsonObject, "vcount", m_vcount);

		auto tilesArray = cJSON_CreateArray();
		for (int y = 0; y < m_vcount; ++y)
		{
			QString line = "";
			for (int x = 0; x < m_hcount; ++x)
			{
				line += QString::number(getTile(x, y), 16) + " ";
			}
			QByteArray ba = line.toLocal8Bit();
			cJSON_AddItemToArray(tilesArray, cJSON_CreateString(ba.data()));

		}
		cJSON_AddItemToObject(jsonObject, "tiles", tilesArray);

		QClipboard* clipboard = QApplication::clipboard();
		auto buffer = cJSON_Print(jsonObject);

		clipboard->setText(QString(buffer));
		delete buffer;
		cJSON_Delete(jsonObject);

		return true;

	}

	void TileSelection::clearSelection(IWorld* world)
	{
		m_cleared = true;
	}

	void TileSelection::drag(double x, double y, bool snap, IWorld* world)
	{
		AbstractSelection::drag(x, y, true, world);
	}

	void TileSelection::setDragOffset(double x, double y, bool snap)
	{
		AbstractSelection::setDragOffset(x, y, true);
	}
	
	void TileSelection::beginResize(int edges, IWorld* world)
	{
		AbstractSelection::beginResize(edges, world);

		if(m_selectionStartHCount == -1)
			m_selectionStartHCount = m_hcount;
		if (m_selectionStartVCount == -1)
			m_selectionStartVCount = m_vcount;
	}

	int TileSelection::getResizeEdge(int mouseX, int mouseY)
	{
		if (canResize())
		{
			if (mouseX > getX() - 4 && mouseX < getRight() + 4 && mouseY > getY() - 4 && mouseY < getBottom() + 4)
			{
				int retval = 0;

				if (mouseX <= getX() + 2)
				{
					retval |= Edges::EDGE_LEFT;
				}
				else if (mouseX >= getRight() - 2)
				{
					retval |= Edges::EDGE_RIGHT;
				}

				if (mouseY <= getY() + 2)
				{
					retval |= Edges::EDGE_TOP;
				}
				else if (mouseY >= getBottom() - 2)
				{
					retval |= Edges::EDGE_BOTTOM;
				}
				return retval;

			}
		}

		return 0;
	}

	void TileSelection::updateResize(int mouseX, int mouseY, bool snap, IWorld* world)
	{
		auto edges = getResizeEdges();
		auto newHCount = m_hcount;
		auto newVCount = m_vcount;

		if (edges & AbstractSelection::EDGE_LEFT)
		{
			int x = std::min(mouseX, (int)getRight());
			int width = ((int)getRight() - x);

			newHCount = std::max(m_selectionStartHCount, int(std::round((width / 16.0) / m_selectionStartHCount) * m_selectionStartHCount));
			setX(getRight() - newHCount * 16);
		}

		if (edges & AbstractSelection::EDGE_RIGHT)
		{
			int pos2 = std::max(mouseX, (int)getX());
			int width = pos2 - (int)getX();


			newHCount = std::max(m_selectionStartHCount, int(std::round((width / 16.0) / m_selectionStartHCount) * m_selectionStartHCount));
			
		}

		if (edges & AbstractSelection::EDGE_TOP)
		{
			int y = std::min(mouseY, (int)getBottom());
			int height = (int)getBottom() - y;
			newVCount = std::max(m_selectionStartVCount, int(std::round((height / 16.0) / m_selectionStartVCount) * m_selectionStartVCount));

			setY(getBottom() - newVCount * 16);
		}

		if (edges & AbstractSelection::EDGE_BOTTOM)
		{
			int pos2 = std::max(mouseY, (int)getY());
			int height = pos2 - (int)getY();
			newVCount = std::max(m_selectionStartVCount, int(std::round((height / 16.0) / m_selectionStartVCount) * m_selectionStartVCount));
		}

		if (newHCount != m_hcount || newVCount != m_vcount)
		{
			int* tiles = new int[newHCount * newVCount];

			for (int y = 0; y < newVCount; ++y)
			{
				for (int x = 0; x < newHCount; ++x)
				{
					int tile = getTile(x % m_hcount, y % m_vcount);
					tiles[y * newHCount + x] = tile;

				}

			}
			delete m_tiles;
			m_tiles = tiles;
			m_hcount = newHCount;
			m_vcount = newVCount;
			m_width = newHCount * 16;
			m_height = newVCount * 16;

		}



	}
};

