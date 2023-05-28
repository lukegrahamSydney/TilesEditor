#ifndef ABSTRACTLEVELENTITYH
#define ABSTRACTLEVELENTITYH

#include <QPainter>
#include <cmath>
#include <map>
#include <cstdio>
#include "Rectangle.h"
#include "ISpatialMapItem.h"
#include "RefCounter.h"
#include "LevelEntityType.h"
#include "ResourceManager.h"
#include "IWorld.h"
#include "Image.h"
#include "cJSON/cJSON.h"

namespace TilesEditor
{
	class Level;
	class AbstractLevelEntity :
		public ISpatialMapItem, public RefCounter
	{

	private:
		IWorld* m_world;


		double m_x,
			m_y;

		Rectangle m_startRect;

		Level* m_level;
		double m_dragOffsetX;
		double m_dragOffsetY;
		double m_microDepth;

		static double nextMicroDepth;

	public:
		static bool sortByDepthFunc(AbstractLevelEntity* entity1, AbstractLevelEntity* entity2) {
			return entity1->getRealDepth() < entity2->getRealDepth();
		}

		AbstractLevelEntity(IWorld* world, double x, double y) {
			m_world = world;
			m_x = x;
			m_y = y;
			m_level = nullptr;

			m_dragOffsetX = m_dragOffsetY = 0.0;
			m_microDepth = nextMicroDepth;
			nextMicroDepth += 0.000000001;
		}

		virtual LevelEntityType getEntityType() const = 0;

		IWorld* getWorld() const { return m_world; }
		void setLevel(Level* level) { m_level = level; }
		Level* getLevel() { return m_level; }
		const Level* getLevel() const { return m_level; }

		void setStartRect(const IRectangle& rect) { m_startRect = rect; }
		const IRectangle& getStartRect() const { return m_startRect; }

		void draw(QPainter* painter, const Rectangle& viewRect) {
			draw(painter, viewRect, getX(), getY());
		}

		virtual void loadResources() {}
		virtual void releaseResources() {}
		virtual bool update(double delta) { return false; };
		virtual void draw(QPainter* painter, const IRectangle& viewRect, double x, double y) = 0;

		virtual void openEditor() {}
		virtual double getX() const {
			return m_x;
		}

		virtual double getY() const {
			return m_y;
		}

		virtual void setX(double val) {
			m_x = val;
		}

		virtual void setY(double val) {
			m_y = val;

		}

		virtual QString toString() const { return ""; }
		virtual Image* getIcon() { return nullptr; }
		virtual cJSON* serializeJSON() { return nullptr; };
		virtual void deserializeJSON(cJSON* json) {}

	
		double getUnitWidth() const;
		double getUnitHeight() const;
		double getRealDepth() const {
			return getDepth() + m_microDepth;
		}
		virtual void setWidth(int value) {}
		virtual void setHeight(int value) {};
		virtual double getDepth() const { return 0.0; }
		virtual AbstractLevelEntity* duplicate() = 0;

		virtual void setDragOffset(double x, double y, bool snap, double snapX, double snapY);

		virtual void drag(double x, double y, bool snap, double snapX, double snapY);
		virtual bool canResize() const { return false; }

		virtual void updateResize(int edges, int mouseX, int mouseY, bool snap, double snapX, double snapY);
		virtual void endResize() {};

	};
};
#endif
