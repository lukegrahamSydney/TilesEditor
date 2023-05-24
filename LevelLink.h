#ifndef LEVELLINKH
#define LEVELLINKH

#include <QString>
#include "AbstractLevelEntity.h"
#include "LevelEntityType.h"
#include "IWorld.h"

namespace TilesEditor
{
	class LevelLink :
		public AbstractLevelEntity
	{
	private:
		QString m_nextLevel;
		QString m_nextX;
		QString m_nextY;

		int m_width;
		int m_height;

		bool m_possibleEdgeLink;

	public:
		LevelLink(IWorld* world, double x, double y, int width, int height, bool possibleEdgeLink);
		LevelLink(IWorld* world, cJSON* json);

		void setNextLevel(const QString& nextLevel);
		void setNextX(const QString& nextX);
		void setNextY(const QString& nextY);

		const QString& getNextLevel() const { return m_nextLevel; }
		const QString& getNextX() const { return m_nextX; }
		const QString& getNextY() const { return m_nextY; }

		LevelEntityType getEntityType() const override { return LevelEntityType::ENTITY_LINK; }

		cJSON* serializeJSON() override;
		void deserializeJSON(cJSON* json) override;
		void draw(QPainter* painter, const IRectangle& viewRect, double x, double y) override;

		int getWidth() const override { return m_width; }
		int getHeight() const override { return m_height; }
		void setWidth(int value) override { m_width = value; }
		void setHeight(int value) override { m_height = value; };
		double getDepth() const override { return 10000000.0; }

		bool isPossibleEdgeLink() const { return m_possibleEdgeLink; }
		void setDragOffset(double x, double y, bool snap) override;
		bool canResize() const override { return true; }
		void drag(double x, double y, bool snap) override;

		void openEditor() override;
		QString toString() const override { return QString("[Link: %1, %2, %3]").arg(getNextLevel()).arg(getNextX()).arg(getNextY()); }
		AbstractLevelEntity* duplicate() override;


	};

};

#endif

