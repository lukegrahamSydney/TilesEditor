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
	public:
		enum SelectMode {
			MODE_MOVE,
			MODE_INSERT
		};
	private:
		QList<AbstractLevelEntity*>	m_selectedObjects;
		SelectMode m_selectMode;

	public:

		ObjectSelection(double x, double y);
		void draw(QPainter* painter, const IRectangle& viewRect) override;
		bool pointInSelection(double x, double y) override;
		void drag(double x, double y, bool snap, double snapX, double snapY, IWorld* world) override;
		void setDragOffset(double x, double y, bool snap, double snapX, double snapY) override;
		void release(ResourceManager& resourceManager) override {}
		void reinsertIntoWorld(IWorld* world, int layer) override;
		void clearSelection(IWorld* world) override;

		int getWidth() const override;
		int getHeight() const override;
		bool canResize() const override;
		bool clipboardCopy() override;
		void deserializeJSON(cJSON* json, IWorld* world, int newLayer) override;

		AbstractLevelEntity* getEntityAtPoint(double x, double y);
		SelectionType getSelectionType() const override { return SelectionType::SELECTION_OBJECTS; };

		LevelEntityType getEntityType() const;

		void setSelectMode(SelectMode val) { m_selectMode = val; }
		SelectMode getMoveMode() const { return m_selectMode; }
		void addObject(AbstractLevelEntity* entity);

		size_t objectCount() { return m_selectedObjects.size(); }
		void updateResize(int mouseX, int mouseY, bool snap, double snapX, double snapY, IWorld* world) override;
		void endResize(IWorld* world) override;
		int getResizeEdge(int mouseX, int mouseY) override;

		Rectangle getDrawRect() const override;

	};
};
#endif
