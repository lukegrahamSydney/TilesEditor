#ifndef OBJECTSELECTIONH
#define OBJECTSELECTIONH

#include <QList>
#include "AbstractSelection.h"
#include "LevelNPC.h"

namespace TilesEditor
{
	class ObjectSelection:
		public AbstractSelection
	{
	private:
		QList<AbstractLevelEntity*>	m_selectedObjects;

	public:

		ObjectSelection(double x, double y);
		void draw(QPainter* painter, const IRectangle& viewRect) override;
		bool pointInSelection(double x, double y) override;
		void drag(double x, double y, bool snap, IWorld* world) override;
		void setDragOffset(double x, double y, bool snap) override;
		void release(ResourceManager& resourceManager) override {}
		void reinsertIntoWorld(IWorld* world, int layer) override;
		void clearSelection(IWorld* world) override;

		bool canResize() const override;
		bool clipboardCopy() override;
		void deserializeJSON(cJSON* json, IWorld* world) override;

		AbstractLevelEntity* getEntityAtPoint(double x, double y);
		SelectionType getSelectionType() const override { return SelectionType::SELECTION_OBJECTS; };

		LevelEntityType getEntityType() const;
		void addObject(AbstractLevelEntity* entity);

		void deleteObject(AbstractLevelEntity* entity);
		size_t objectCount() { return m_selectedObjects.size(); }
		void updateResize(int mouseX, int mouseY, bool snap, IWorld* world) override;
		void endResize(IWorld* world) override;
		int getResizeEdge(int mouseX, int mouseY) override;


	};
};
#endif
