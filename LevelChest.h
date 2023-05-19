#ifndef LEVELCHESTH
#define LEVELCHESTH

#include <QString>
#include "AbstractLevelEntity.h"
#include "LevelEntityType.h"


namespace TilesEditor
{
	class LevelChest :
		public AbstractLevelEntity
	{
	private:
		QString m_itemName;
		int m_signIndex;

	public:

		LevelChest(Level* level, double x, double y, const QString& itemName, int signIndex);

		void setItemName(const QString& name) {
			m_itemName = name;
		}

		const QString& getItemName() const {
			return m_itemName;
		}

		void setSignIndex(int index) {
			m_signIndex = index;
		}
		int getSignIndex() const {
			return m_signIndex;
		}

		LevelEntityType getEntityType() const override { return LevelEntityType::ENTITY_CHEST; }

		void draw(QPainter* painter, const IRectangle& viewRect, double x, double y) override;


		int getWidth() const override { return 32; }
		int getHeight() const override { return 32; }

		QString toString() const override { return QString("[Chest: %1, %2]").arg(getX()).arg(getY()); }

		void openEditor(IWorld* world) override;
		AbstractLevelEntity* duplicate() override {
			return nullptr;
		}

		cJSON* serializeJSON() override;
		void deserializeJSON(cJSON* json, IWorld* world) override;

		static Image* getChestImage() {
			static auto retval = new Image("", QPixmap(":/MainWindow/icons/chest.png"));
			return retval;
		}
	};
};
#endif

