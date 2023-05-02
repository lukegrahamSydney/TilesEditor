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
		int m_id = -1;


		double m_x,
			m_y;

		
		Level* m_level;
		double m_dragOffsetX;
		double m_dragOffsetY;
		double m_microDepth;

		static double nextMicroDepth;

	public:
		static bool sortByDepthFunc(AbstractLevelEntity* entity1, AbstractLevelEntity* entity2) {
			return entity1->getRealDepth() < entity2->getRealDepth();
		}

		AbstractLevelEntity(Level* level, double x, double y) {

			m_x = x;
			m_y = y;
			m_id = -1;
			m_level = level;

			m_dragOffsetX = m_dragOffsetY = 0.0;
			m_microDepth = nextMicroDepth;
			nextMicroDepth += 0.000000001;
		}

		virtual LevelEntityType getEntityType() const = 0;

		void setLevel(Level* level) { m_level = level; }
		Level* getLevel() { return m_level; }
		const Level* getLevel() const { return m_level; }

		void setID(int id) {
			m_id = id;
		}

		int getID() const {
			return m_id;
		}


		void draw(QPainter* painter, const Rectangle& viewRect) {
			draw(painter, viewRect, getX(), getY());
		}

		virtual void loadResources(ResourceManager& resourceManager) {}
		virtual void releaseResources(ResourceManager& resourceManager) {}
		virtual bool update(double delta) { return false; };
		virtual void draw(QPainter* painter, const IRectangle& viewRect, double x, double y) = 0;

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
		virtual void deserializeJSON(cJSON* json, IWorld* world) {}

	
		double getUnitWidth() const;
		double getUnitHeight() const;
		double getRealDepth() const {
			return getDepth() + m_microDepth;
		}
		virtual void setWidth(int value) {}
		virtual void setHeight(int value) {};
		virtual double getDepth() const { return 0.0; }
		virtual AbstractLevelEntity* duplicate() = 0;

		virtual void setDragOffset(double x, double y, bool snap);

		virtual void drag(double x, double y, bool snap, IWorld* world);
		virtual bool canResize() const { return false; }

		virtual void updateResize(int edges, int mouseX, int mouseY, bool snap, IWorld* world);
		virtual void endResize(IWorld* world) {};

	};
};
#endif
