#ifndef ABSTRACTSELECTIONH
#define ABSTRACTSELECTIONH

#include <QPainter>
#include <QRect>
#include "AbstractResourceManager.h"
#include "IWorld.h"
#include "SelectionType.h"
#include "cJSON/cJSON.h"

namespace TilesEditor
{
	class AbstractSelection
	{
	public:
		enum Edges {
			EDGE_LEFT = 1,
			EDGE_TOP = 2,
			EDGE_RIGHT = 4,
			EDGE_BOTTOM = 8
		};


	private:
		double m_x;
		double m_y;

		double m_dragOffsetX;
		double m_dragOffsetY;
		int m_layer = 0;
		bool m_visible = true;
		bool m_moved = false;
		int m_resizeEdges;
		bool m_alternateSelectionMethod;
		bool m_dragging = false;
		bool m_copy = false;
		Qt::MouseButton m_dragMouseButton = Qt::MouseButton::LeftButton;

	public:
		AbstractSelection(double x, double y, int layer) {
			m_x = x;
			m_y = y;
			m_layer = layer;
			m_dragOffsetX = m_dragOffsetY = 0.0;
			m_resizeEdges = 0;
			m_alternateSelectionMethod = false;
		}
		void setCopy(bool enabled) { m_copy = enabled; }
		bool shouldCopy() const { return m_copy; }

		void setVisible(bool visible) { m_visible = visible; }
		bool isVisible() const { return m_visible; }

		void setMoved(bool value) { m_moved = value; }
		bool hasMoved() const { return m_moved; }

		virtual void draw(QPainter* painter, const QRectF& viewRect) = 0;
		virtual bool pointInSelection(double x, double y) = 0;
		virtual void release(AbstractResourceManager* resourceManager) = 0;
		virtual void reinsertIntoWorld(IWorld* world, bool clearSelection = true) = 0;
		virtual void clearSelection(IWorld* world) = 0;


		virtual void setDragOffset(double x, double y, bool snap, double snapX, double snapY);
		virtual void beginDrag() { m_moved = false; m_dragging = true; }
		virtual void endDrag(IWorld* world) { m_dragging = false; }
		virtual void drag(double x, double y, bool snap, double snapX, double snapY, IWorld* world);
		virtual bool canResize() const { return false; }
		virtual void deserializeJSON(cJSON* json, IWorld* world) {};
		virtual AbstractSelection* duplicate() = 0;

		void setMouseDragButton(Qt::MouseButton button) { m_dragMouseButton = button; }
		Qt::MouseButton getMouseDragButton() const { return m_dragMouseButton; }
		void setAlternateSelectionMethod(bool val) { m_alternateSelectionMethod = val; }
		bool getAlternateSelectionMethod() const { return m_alternateSelectionMethod; }

		virtual SelectionType getSelectionType() const = 0;

		bool dragging() const { return m_dragging; }
		void setX(double val) { m_x = val; }
		void setY(double val) { m_y = val; }
		double getX() const { return m_x; }
		double getY() const { return m_y; }

		virtual int getWidth() const { return 0; }
		virtual int getHeight() const { return 0; }
		virtual bool clipboardCopy() { return false; }
		int getResizeEdges() const { return m_resizeEdges; }

		virtual QRectF getBoundingBox() const { return QRectF(); };
		virtual void beginResize(int edges, IWorld* world) {
			m_resizeEdges = edges;
		}

		bool resizing() const {
			return m_resizeEdges != 0;
		}

		void setLayer(int layer) {
			m_layer = layer;
		}

		int getLayer() const {
			return m_layer;
		}

		virtual void updateResize(int mouseX, int mouseY, bool snap, double snapX, double snapY, IWorld* world) {};
		virtual void endResize(IWorld* world) {
			m_resizeEdges = 0;
		};

		virtual int getResizeEdge(int mouseX, int mouseY) {
			return 0;
		};

	};
};
#endif
