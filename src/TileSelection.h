#ifndef TILESELECTIONH
#define TILESELECTIONH

#include "AbstractSelection.h"
#include "Image.h"

namespace TilesEditor
{
	class TileSelection:
		public AbstractSelection
	{
	public:
		static const int NO_LAYER = -4624876;
	private:
		bool m_cleared;
		bool m_applyNewTranslucency;
		int m_width;
		int m_height;
		int m_selectionLayer;
		bool m_clearSelection;
		bool m_hasInserted;
		double m_lastInsertX;
		double m_lastInsertY;
		Tilemap* m_tilemap;

		QUndoCommand* m_groupUndoCommand = nullptr;
		//int* m_tiles;
		//int m_hcount;
		//int m_vcount;

		int m_selectionStartHCount;
		int m_selectionStartVCount;


	public:
		TileSelection(double x, double y, int hcount, int vcount, int layer);
		~TileSelection();

		Tilemap* getTilemap() { return m_tilemap; }
		int getWidth() const override { return m_width; }
		int getHeight() const override { return m_height; }
		double getRight() const { return getX() + m_width; }
		double getBottom() const { return getY() + m_height; }
		void draw(QPainter* painter, const QRectF& viewRect) override;
		void draw(Level* level, Image* tilesetImage, QPainter* painter, const QRectF& viewRect);
		bool pointInSelection(double x, double y) override;
		void release(AbstractResourceManager* resourceManager) override;
		void reinsertIntoWorld(IWorld* world, bool clearSelection = true) override;
		SelectionType getSelectionType() const override { return SelectionType::SELECTION_TILES; };

		int getTile(unsigned int x, unsigned int y);
		void setTile(unsigned int x, unsigned int y, int tile);

		int getHCount() const;
		int getVCount() const;

		void setClearSelection(bool value) { m_clearSelection = value; }
		AbstractSelection* duplicate() override;
		bool clipboardCopy() override;
		void clearSelection(IWorld* world) override;
		bool canResize() const override { return true; }
		void drag(double x, double y, bool snap, double snapX, double snapY, IWorld* world) override;
		void setDragOffset(double x, double y, bool snap, double snapX, double snapY) override;
		void setApplyTranslucency(bool value) { m_applyNewTranslucency = value; }
		void beginResize(int edges, IWorld* world) override;
		int getResizeEdge(int mouseX, int mouseY) override;
		void updateResize(int mouseX, int mouseY, bool snap, double snapX, double snapY, IWorld* world) override;

		QRectF getBoundingBox() const override {
			return QRectF(getX(), getY(), getWidth(), getHeight());
		}
	};
};

#endif
