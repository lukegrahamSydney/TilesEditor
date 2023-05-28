#ifndef TILESELECTIONH
#define TILESELECTIONH

#include "AbstractSelection.h"
#include "Image.h"

namespace TilesEditor
{
	class TileSelection:
		public AbstractSelection
	{
	private:
		Image* m_tilesetImage;

		bool m_cleared;
		int m_width;
		int m_height;

		bool m_hasInserted;
		double m_lastInsertX;
		double m_lastInsertY;
		Tilemap* m_tilemap;

		//int* m_tiles;
		//int m_hcount;
		//int m_vcount;

		int m_selectionStartHCount;
		int m_selectionStartVCount;


	public:
		TileSelection(double x, double y, int hcount, int vcount);
		~TileSelection();

		Tilemap* getTilemap() { return m_tilemap; }
		int getWidth() const override { return m_width; }
		int getHeight() const override { return m_height; }
		double getRight() const { return getX() + m_width; }
		double getBottom() const { return getY() + m_height; }
		void draw(QPainter* painter, const IRectangle& viewRect) override;
		bool pointInSelection(double x, double y) override;
		void release(ResourceManager& resourceManager) override;
		void reinsertIntoWorld(IWorld* world, int layer) override;
		SelectionType getSelectionType() const override { return SelectionType::SELECTION_TILES; };
		void setTilesetImage(Image* tilesetImage);
		Image* getTilesetImage() { return m_tilesetImage; }
		int getTile(unsigned int x, unsigned int y);
		void setTile(unsigned int x, unsigned int y, int tile);

		int getHCount() const;
		int getVCount() const;
		bool clipboardCopy() override;
		void clearSelection(IWorld* world) override;
		bool canResize() const override { return true; }
		void drag(double x, double y, bool snap, double snapX, double snapY, IWorld* world) override;
		void setDragOffset(double x, double y, bool snap, double snapX, double snapY) override;

		void beginResize(int edges, IWorld* world) override;
		int getResizeEdge(int mouseX, int mouseY) override;
		void updateResize(int mouseX, int mouseY, bool snap, double snapX, double snapY, IWorld* world) override;
	};
};

#endif
