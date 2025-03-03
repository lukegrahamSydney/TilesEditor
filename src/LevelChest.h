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

		LevelChest(IWorld* world, double x, double y, const QString& itemName, int signIndex);
		LevelChest(IWorld* world, cJSON* json);

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

		void draw(QPainter* painter, const QRectF& viewRect, double x, double y) override;


		int getWidth() const override { return 32; }
		int getHeight() const override { return 32; }

		void setProperty(const QString& name, const QVariant& value) override;
		QString toString() const override { return QString("[Chest: %1, %2, %3]").arg(m_itemName).arg(getX()).arg(getY()); }

		QPixmap getIcon() override;
		void openEditor() override;
		AbstractLevelEntity* duplicate() override;

		cJSON* serializeJSON(bool useLocalCoordinates = false) override;
		void deserializeJSON(cJSON* json) override;

		void setDragOffset(double x, double y, bool snap, double snapX, double snapY) override {
			AbstractLevelEntity::setDragOffset(x, y, true, std::ceil(snapX / 16.0) * 16.0, std::ceil(snapY / 16.0) * 16.0);
		}

		void drag(double x, double y, bool snap, double snapX, double snapY) override {
			AbstractLevelEntity::drag(x, y, true, std::ceil(snapX / 16.0) * 16.0, std::ceil(snapY / 16.0) * 16.0);
		}


		static Image* getChestImage() {
			static auto retval = new Image("", QImage(":/MainWindow/icons/chest.png"));
			return retval;
		}
	};
};
#endif

