#ifndef LEVELOBJECTINSTANCEH
#define LEVELOBJECTINSTANCEH

#include <QStringList>
#include "LevelNPC.h"
#include "ObjectClass.h"
#include "IObjectClassInstance.h"

namespace TilesEditor
{
	class LevelObjectInstance :
		public LevelNPC,
		public IObjectClassInstance
	{
	private:
		QString m_className;
		ObjectClass* m_objectClass;
		QStringList m_params;
		int m_startWidth = 0, m_startHeight = 0;

		void parseClassCode();

	public:
		LevelObjectInstance(IWorld* world, double x, double y, const QString& className, ObjectClass* objectClass, const QStringList& params);
		LevelObjectInstance(IWorld* world, cJSON* json);
		~LevelObjectInstance();

		void setProperty(const QString& name, const QVariant& value) override;
		QString getClassName() const override { return m_className; }
		void openEditor() override;
		AbstractLevelEntity* duplicate() override;
		cJSON* serializeJSON(bool useLocalCoordinates) override;
		void deserializeJSON(cJSON* json) override;
		QString toString() const override;
		void draw(QPainter* painter, const QRectF& viewRect, double x, double y) override;
		bool canResize() const override;
		void updateResize(int edges, int mouseX, int mouseY, bool snap, double snapX, double snapY) override;
		void resetSize() override;
		bool canAddToLevel(Level* level) override;
		void unsetObjectClass() override { m_objectClass = nullptr; }
		void markObjectChanged() override;
		QString formatGraalCode(bool forceEmbed, bool insertHeader, const QStringList& params) const;
		QStringList& getParams() { return m_params; }
		ObjectClass* getObjectClass() { return m_objectClass; }
	};
};
#endif
