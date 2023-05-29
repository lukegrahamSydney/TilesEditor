#ifndef ABSTRACTSELECTIONH
#define ABSTRACTSELECTIONH

#include <QPainter>
#include "Rectangle.h"
#include "ResourceManager.h"
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
		int m_resizeEdges;
		bool m_alternateSelectionMethod;

	public:
		AbstractSelection(double x, double y) {
			m_x = x;
			m_y = y;
			m_dragOffsetX = m_dragOffsetY = 0.0;
			m_resizeEdges = 0;
			m_alternateSelectionMethod = false;
		}

		virtual void draw(QPainter* painter, const IRectangle& viewRect) = 0;
		virtual bool pointInSelection(double x, double y) = 0;
		virtual void release(ResourceManager& resourceManager) = 0;
		virtual void reinsertIntoWorld(IWorld* world, int layer) = 0;
		virtual void clearSelection(IWorld* world) = 0;

		virtual void setDragOffset(double x, double y, bool snap, double snapX, double snapY);

		virtual void drag(double x, double y, bool snap, double snapX, double snapY, IWorld* world);
		virtual bool canResize() const { return false; }
		virtual void deserializeJSON(cJSON* json, IWorld* world) {};
		void setAlternateSelectionMethod(bool val) { m_alternateSelectionMethod = val; }
		bool getAlternateSelectionMethod() const { return m_alternateSelectionMethod; }

		virtual SelectionType getSelectionType() const = 0;
		void setX(double val) { m_x = val; }
		void setY(double val) { m_y = val; }
		double getX() const { return m_x; }
		double getY() const { return m_y; }

		virtual int getWidth() const { return 0; }
		virtual int getHeight() const { return 0; }
		virtual bool clipboardCopy() { return false; }
		int getResizeEdges() const { return m_resizeEdges; }

		virtual Rectangle getDrawRect() const { return Rectangle(); };
		virtual void beginResize(int edges, IWorld* world) {
			m_resizeEdges = edges;
		}

		bool resizing() const {
			return m_resizeEdges != 0;
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
