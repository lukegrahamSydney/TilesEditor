#include <QDebug>
#include "LevelNPC.h"
#include "cJSON/JsonHelper.h"
#include "EditAnonymousNPC.h"

namespace TilesEditor
{

	LevelNPC::LevelNPC(Level* level, double x, double y, int width, int height):
		AbstractLevelEntity(level, x, y)
	{
		m_width = width;
		m_height = height;
		m_image = nullptr;

		m_loadImageFail = false;
	}

	LevelNPC::LevelNPC(Level* level, cJSON* json, IWorld* world):
		LevelNPC(level, 0.0, 0.0, 0, 0)
	{
		deserializeJSON(json, world);

	}

	void LevelNPC::loadResources(ResourceManager& resourceManager)
	{
		if (m_image == nullptr)
		{
			if (!m_loadImageFail)
			{
				m_image = static_cast<Image*>(resourceManager.loadResource(m_imageName, ResourceType::RESOURCE_IMAGE));

				m_loadImageFail = m_image == nullptr;
			}
		}
	}

	void LevelNPC::releaseResources(ResourceManager& resourceManager)
	{
		if (m_image)
			resourceManager.freeResource(m_image);
		m_image = nullptr;
	}

	void LevelNPC::draw(QPainter* painter, const IRectangle& viewRect, double x, double y)
	{
		if (m_image != nullptr)
		{
			m_image->draw(painter, x, y);
		}
		else {
			if (m_imageName == "") {
				getBlankNPCImage()->draw(painter, x, y);
			} else getBlankNPCImage()->draw(painter, x, y);
			
		}
	}

	void LevelNPC::setImageName(const QString& name, ResourceManager& resourceManager)
	{
		if (name == "") {
			m_imageName = "";
			if (m_image != nullptr)
				resourceManager.freeResource(m_image);
			m_image = nullptr;
		}

		else if (name != m_imageName)
		{
			m_imageName = name;

			if (m_image != nullptr)
				resourceManager.freeResource(m_image);
			
			m_image = static_cast<Image*>(resourceManager.loadResource(m_imageName, ResourceType::RESOURCE_IMAGE));

			if (m_image != nullptr) {
				setWidth(m_image->width());
				setHeight(m_image->height());
			}
		}

		if (m_image == nullptr)
		{
			setWidth(48);
			setHeight(48);
		}
	}

	void LevelNPC::openEditor(IWorld* world)
	{
		EditAnonymousNPC frm(this, world);
		frm.exec();
	}

	cJSON* LevelNPC::serializeJSON()
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelNPCv1");
		cJSON_AddStringToObject(json, "image", getImageName().toLocal8Bit().data());
		cJSON_AddNumberToObject(json, "x", getX());
		cJSON_AddNumberToObject(json, "y", getY());
		cJSON_AddNumberToObject(json, "width", getWidth());
		cJSON_AddNumberToObject(json, "height", getHeight());
		cJSON_AddStringToObject(json, "code", getCode().toLocal8Bit().data());

		
		return json;
	}

	void LevelNPC::deserializeJSON(cJSON* json, IWorld* world)
	{
		setImageName(jsonGetChildString(json, "image"), world->getResourceManager());
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));

		setWidth(jsonGetChildInt(json, "width"));
		setHeight(jsonGetChildInt(json, "height"));

		setCode(jsonGetChildString(json, "code"));
	}

	Image* LevelNPC::getBlankNPCImage()
	{
		static Image* blankNPCImage = new Image("", QPixmap(":/MainWindow/icons/npc.png"));
		return blankNPCImage;
	}
};

