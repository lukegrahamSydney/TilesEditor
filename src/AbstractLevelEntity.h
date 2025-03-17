#ifndef ABSTRACTLEVELENTITYH
#define ABSTRACTLEVELENTITYH

#include <QPainter>
#include <cmath>
#include <map>
#include <cstdio>
#include <QRect>
#include "AbstractSpatialGridItem.h"
#include "RefCounter.h"
#include "LevelEntityType.h"
#include "IWorld.h"
#include "Image.h"
#include "cJSON/cJSON.h"
#include "sgscript/sgscript.h"
#include "IScriptableLevelObject.h"
namespace TilesEditor
{
	class Level;
	class AbstractLevelEntity :
		public AbstractSpatialGridItem, 
		public RefCounter,
		public IScriptableLevelObject
	{

	private:
		static sgs_Variable sgs_classMembers;

		sgs_Variable m_thisObject;
		sgs_Variable m_sgsUserTable;
		IWorld* m_world;

		Level* m_level;
		int m_tileLayer;
		double m_dragOffsetX;
		double m_dragOffsetY;
		double m_entityOrder;

		inline static double nextEntityOrder = 0.000001;

	public:
		static bool sortByDepthFunc(AbstractLevelEntity* entity1, AbstractLevelEntity* entity2) {
			return entity1->getRealDepth() < entity2->getRealDepth();
		}

		AbstractLevelEntity(IWorld* world, double x, double y);

		virtual ~AbstractLevelEntity();

		virtual LevelEntityType getEntityType() const = 0;

		
		IWorld* getWorld() const override { return m_world; }
		void setLevel(Level* level) { m_level = level; }
		Level* getLevel() { return m_level; }
		const Level* getLevel() const { return m_level; }

		void draw(QPainter* painter, const QRectF& viewRect) {
			draw(painter, viewRect, getX(), getY());
		}

		virtual void loadResources() {}
		virtual void releaseResources() {}
		virtual bool update(double delta) { return false; };
		virtual void draw(QPainter* painter, const QRectF& viewRect, double x, double y) = 0;

		virtual void openEditor() {}
		virtual void setProperty(const QString& name, const QVariant& value);

		

		void setLayerIndex(int layer) {
			m_tileLayer = layer;
		}

		int getLayerIndex() const {
			return m_tileLayer;
		}


		virtual QString name() const { return ""; }
		virtual QString toString() const { return ""; }
		virtual QPixmap getIcon() { return QPixmap(); }
		virtual cJSON* serializeJSON(bool useLocalCoordinates = false) { return nullptr; };
		virtual void deserializeJSON(cJSON* json) {}
		virtual bool canAddToLevel(Level* level) { return true; }
	
		void setEntityOrder(double value) {
			m_entityOrder = value;
		}

		sgs_Variable& getScriptObject();
		bool hasScriptObject() const { return m_thisObject.type != SGS_VT_NULL; }
		double getEntityOrder() const { return m_entityOrder; }
		double getUnitWidth() const;
		double getUnitHeight() const;
		virtual double getRealDepth() const {
			return getDepth() + m_entityOrder;
		}

		virtual double getDepth() const { return getLayerIndex(); }
		virtual AbstractLevelEntity* duplicate() = 0;

		virtual void setDragOffset(double x, double y, bool snap, double snapX, double snapY);

		virtual void drag(double x, double y, bool snap, double snapX, double snapY);
		virtual QRectF getDragRect(double x, double y, bool snap, double snapX, double snapY) const;
		virtual bool canResize() const { return false; }

		virtual void updateResize(int edges, int mouseX, int mouseY, bool snap, double snapX, double snapY);
		virtual QRectF updateResizeRect(int edges, int mouseX, int mouseY, bool snap, double snapX, double snapY);
		virtual void endResize() {};


		virtual void mark(sgs_Context* ctx);
		virtual int scriptSetMember(sgs_Context* ctx) { return 0; };
		virtual int scriptGetMember(sgs_Context* ctx) { return 0; };

		static sgs_ObjInterface sgs_interface;
		static void registerScriptClass(IEngine* engine);

	};
};
#endif
