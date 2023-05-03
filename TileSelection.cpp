#include <QClipboard>
#include <QApplication>
#include "TileSelection.h"
#include "Tilemap.h"
#include "Selector.h"
#include "Level.h"
#include "LevelCommands.h"

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

		m_tilemap = new Tilemap(nullptr, 0.0, 0.0, hcount, vcount, 0);
		m_tilemap->clear(invisibleTile);

		m_selectionStartHCount = m_selectionStartVCount = -1;

		m_hasInserted = false;
		m_lastInsertX = 0.0;
		m_lastInsertY = 0.0;

	}

	TileSelection::~TileSelection()
	{
		delete m_tilemap;
	}

	void TileSelection::draw(QPainter* painter, const IRectangle& viewRect)
	{
		if (m_tilesetImage)
		{
			m_tilemap->draw(painter, viewRect, m_tilesetImage, getX(), getY());
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

		//If hasnt moved since last insert then ignore
		if (m_hasInserted && getX() == m_lastInsertX && getY() == m_lastInsertY)
			return;

		Tilemap oldTiles(nullptr, 0.0, 0.0, getHCount(), getVCount(), 0);
		oldTiles.clear(Tilemap::MakeInvisibleTile(0));

		auto levels = world->getLevelsInRect(Rectangle(getX(), getY(), m_width, m_height));

		for (auto level : levels)
		{
			auto tilemap = level->getOrMakeTilemap(layer, world->getResourceManager());
			if (tilemap != nullptr)
			{
				int destTileX = int((this->getX() - tilemap->getX()) / 16.0);
				int destTileY = int((this->getY() - tilemap->getY()) / 16.0);

				bool modified = false;
				for (int y = 0; y < m_tilemap->getVCount(); ++y)
				{
					for (int x = 0; x < m_tilemap->getHCount(); ++x)
					{
						int oldTile = 0;
						if (tilemap->tryGetTile(destTileX + x, destTileY + y, &oldTile))
						{

							oldTiles.setTile(x, y, oldTile);
						}
						
					}
				}


			}
		}

		world->addUndoCommand(new CommandPutTiles(world, this->getX(), this->getY(), layer, &oldTiles, m_tilemap));
		m_hasInserted = true;
		m_lastInsertX = getX();
		m_lastInsertY = getY();

	}

	void TileSelection::setTilesetImage(Image* tilesetImage)
	{
		m_tilesetImage = tilesetImage;
		m_tilesetImage->incrementRef();
	}

	int TileSelection::getTile(unsigned int x, unsigned int y)
	{
		return m_tilemap->getTile(x, y);
	}

	void TileSelection::setTile(unsigned int x, unsigned int y, int tile)
	{
		m_tilemap->setTile(x, y, tile);
	}

	int TileSelection::getHCount() const { return m_tilemap->getHCount(); }

	int TileSelection::getVCount() const { return m_tilemap->getVCount(); }

	bool TileSelection::clipboardCopy()
	{

		auto jsonObject = cJSON_CreateObject();

		cJSON_AddStringToObject(jsonObject, "type", "tileSelection");
		cJSON_AddNumberToObject(jsonObject, "hcount", m_tilemap->getHCount());
		cJSON_AddNumberToObject(jsonObject, "vcount", m_tilemap->getVCount());

		auto tilesArray = cJSON_CreateArray();
		for (int y = 0; y < m_tilemap->getVCount(); ++y)
		{
			QString line = "";
			for (int x = 0; x < m_tilemap->getHCount(); ++x)
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
			m_selectionStartHCount = m_tilemap->getHCount();
		if (m_selectionStartVCount == -1)
			m_selectionStartVCount = m_tilemap->getVCount();
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
		auto newHCount = m_tilemap->getHCount();
		auto newVCount = m_tilemap->getVCount();

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

		if (newHCount != m_tilemap->getHCount() || newVCount != m_tilemap->getVCount())
		{
			auto tilemap = new Tilemap(nullptr, 0.0, 0.0, newHCount, newVCount, 0);

			int* tiles = new int[newHCount * newVCount];

			for (int y = 0; y < newVCount; ++y)
			{
				for (int x = 0; x < newHCount; ++x)
				{
					int tile = getTile(x % m_tilemap->getHCount(), y % m_tilemap->getVCount());
					tilemap->setTile(x, y, tile);

				}

			}
			delete m_tilemap;
			m_tilemap = tilemap;
			m_width = newHCount * 16;
			m_height = newVCount * 16;

		}



	}
};

