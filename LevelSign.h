#ifndef LEVELSIGNH
#define LEVELSIGNH

#include <QString>
#include "AbstractLevelEntity.h"
#include "LevelEntityType.h"
#include "IWorld.h"

namespace TilesEditor
{
	class LevelSign :
		public AbstractLevelEntity
	{
	private:
		QString m_text;

		int m_width;
		int m_height;

	public:
		LevelSign(IWorld* world, double x, double y, int width, int height);
		LevelSign(IWorld* world, cJSON* json);

		void setText(const QString& text);
		const QString& getText() const { return m_text; }

		LevelEntityType getEntityType() const override { return LevelEntityType::ENTITY_SIGN; }

		cJSON* serializeJSON(bool useLocalCoordinates = false) override;
		void deserializeJSON(cJSON* json) override;

		void draw(QPainter* painter, const IRectangle& viewRect, double x, double y) override;

		int getWidth() const override { return m_width; }
		int getHeight() const override { return m_height; }
		void setWidth(int value) override { m_width = value; }
		void setHeight(int value) override { m_height = value; };

		double getDepth() const override { return 9999999.0; }

		void openEditor() override;

		QString toString() const override { return QString("[Sign: %1, %2]").arg(getX()).arg(getY()); }
		void setDragOffset(double x, double y, bool snap, double snapX, double snapY) override {
			AbstractLevelEntity::setDragOffset(x, y, true, std::ceil(snapX / 16.0) * 16.0, std::ceil(snapY / 16.0) * 16.0);
		}

		void drag(double x, double y, bool snap, double snapX, double snapY) override {
			AbstractLevelEntity::drag(x, y, true, std::ceil(snapX / 16.0) * 16.0, std::ceil(snapY / 16.0) * 16.0);
		}

		AbstractLevelEntity* duplicate() override {
			auto sign = new LevelSign(this->getWorld(), getX(), getY(), getWidth(), getHeight());
			sign->m_text = this->m_text;
			return sign;
		}


	};

};

#endif

