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

		int* m_tiles;
		int m_hcount;
		int m_vcount;

		int m_selectionStartHCount;
		int m_selectionStartVCount;

		void resize(int hcount, int vcount);

	public:
		TileSelection(double x, double y, int hcount, int vcount);
		~TileSelection();

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

		int getHCount() const { return m_hcount; }
		int getVCount() const { return m_vcount; }
		bool clipboardCopy() override;
		void clearSelection(IWorld* world) override;
		bool canResize() const override { return true; }
		void drag(double x, double y, bool snap, IWorld* world) override;
		void setDragOffset(double x, double y, bool snap) override;

		void beginResize(int edges, IWorld* world) override;
		int getResizeEdge(int mouseX, int mouseY) override;
		void updateResize(int mouseX, int mouseY, bool snap, IWorld* world) override;
	};
};

#endif
