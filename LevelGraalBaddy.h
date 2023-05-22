#ifndef LEVELGRAALBADDYH
#define LEVELGRAALBADDYH

#include <QString>
#include "Rectangle.h"
#include "AbstractLevelEntity.h"
#include "LevelEntityType.h"
#include "cJSON/JsonHelper.h"

namespace TilesEditor
{
	class LevelGraalBaddy :
		public AbstractLevelEntity
	{
	private:
		static const int baddyTypes[][4];

		int m_baddyType;

		QString m_baddyVerses[3];

	public:

		LevelGraalBaddy(Level* level, double x, double y, int type);
		LevelGraalBaddy(Level* level, cJSON* json, IWorld* world);

		cJSON* serializeJSON() override;
		void deserializeJSON(cJSON* json, IWorld* world) override;

		int getBaddyType() const { return m_baddyType; }
		void setBaddyType(int type) { m_baddyType = type; }
		void setBaddyVerse(int index, const QString& verse) {
			if (index >= 0 && index < 3)
				m_baddyVerses[index] = verse;
		}

		QString getBaddyVerse(int index) const {
			if (index >= 0 && index < 3)
				return m_baddyVerses[index];
			return "";
		}

		LevelEntityType getEntityType() const override { return LevelEntityType::ENTITY_BADDY; }

		void draw(QPainter* painter, const IRectangle& viewRect, double x, double y) override;


		int getWidth() const override {
			if (m_baddyType >= 0 && m_baddyType < 10)
			{
				return baddyTypes[m_baddyType][2];
			}
			return 48;
		}

		int getHeight() const override {
			if (m_baddyType >= 0 && m_baddyType < 10)
			{
				return baddyTypes[m_baddyType][3];
			}
			return 48;
		}

		QString toString() const override { return QString("[Baddy: %1, %2, %3]").arg(m_baddyType).arg(getX()).arg(getY()); }

		void openEditor(IWorld* world) override;
		AbstractLevelEntity* duplicate() override {
			return nullptr;
		}


		static Image* getBaddyImage() {
			static auto retval = new Image("", QPixmap(":/MainWindow/icons/opps.png"));
			return retval;
		}
	};
};
#endif

