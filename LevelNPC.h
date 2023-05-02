#ifndef LEVELNPCH
#define LEVELNPCH

#include <QString>
#include "AbstractLevelEntity.h"
#include "LevelEntityType.h"
#include "Image.h"
#include "ResourceManager.h"

namespace TilesEditor
{
	class LevelNPC:
		public AbstractLevelEntity
	{
		
		

	private:
		bool m_loadImageFail;

		QString m_imageName;
		QString m_code;
		Image* m_image;

		int m_width;
		int m_height;


	public:

		LevelNPC(Level* level, double x, double y, int width, int height);
		LevelNPC(Level* level, cJSON* json, IWorld* world);

		
		LevelEntityType getEntityType() const override { return LevelEntityType::ENTITY_NPC; }

		void loadResources(ResourceManager& resourceManager) override;
		void releaseResources(ResourceManager& resourceManager) override;
		void draw(QPainter* painter, const IRectangle& viewRect, double x, double y) override;

		void setImageName(const QString& name, ResourceManager& resourceManager);
		const QString& getImageName() const { return m_imageName; }
		void setCode(const QString& code) { m_code = code; }
		const QString& getCode() const { return m_code; }
		int getWidth() const override { return m_width; }
		int getHeight() const override { return m_height; }
		void setWidth(int value) override {
			m_width = value;
		}
		void setHeight(int value) override {
			m_height = value;
		};

		QString toString() const override { return QString("[Npc: %1, %2, %3]").arg(getImageName()).arg(getX()).arg(getY()); }
		Image* getIcon() { return m_image; }
		AbstractLevelEntity* duplicate() override {
			auto newNPC = new LevelNPC(this->getLevel(), getX(), getY(), getWidth(), getHeight());
			newNPC->m_imageName = this->m_imageName;
			newNPC->m_code = this->m_code;
			return newNPC;
		}

		cJSON* serializeJSON() override;
		void deserializeJSON(cJSON* json, IWorld* world) override;

		static Image* getBlankNPCImage();

	};
};

#endif

