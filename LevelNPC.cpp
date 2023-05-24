#include <QDebug>
#include "LevelNPC.h"
#include "cJSON/JsonHelper.h"
#include "EditAnonymousNPC.h"

namespace TilesEditor
{

	LevelNPC::LevelNPC(IWorld* world, double x, double y, int width, int height) :
		AbstractLevelEntity(world, x, y)
	{
		m_width = width;
		m_height = height;
		m_image = nullptr;

		m_loadImageFail = false;
	}

	LevelNPC::LevelNPC(IWorld* world, cJSON* json) :
		LevelNPC(world, 0.0, 0.0, 0, 0)
	{
		deserializeJSON(json);

	}

	LevelNPC::~LevelNPC()
	{
		getWorld()->getResourceManager().getFileSystem()->removeListener(this);

	}

	void LevelNPC::loadResources()
	{
		if (m_image == nullptr)
		{
			if (!m_loadImageFail)
			{
				m_image = static_cast<Image*>(getWorld()->getResourceManager().loadResource(this, m_imageName, ResourceType::RESOURCE_IMAGE));

				m_loadImageFail = m_image == nullptr;
			}
		}
	}

	void LevelNPC::releaseResources()
	{
		if (m_image)
			getWorld()->getResourceManager().freeResource(m_image);
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
			}
			else getBlankNPCImage()->draw(painter, x, y);

		}
	}

	void LevelNPC::setImageName(const QString& name)
	{
		if (name == "") {
			m_imageName = "";
			if (m_image != nullptr)
				getWorld()->getResourceManager().freeResource(m_image);
			m_image = nullptr;
		}

		else 
		{
			m_imageName = name;

			if (m_image != nullptr)
				getWorld()->getResourceManager().freeResource(m_image);

			m_image = static_cast<Image*>(getWorld()->getResourceManager().loadResource(this, m_imageName, ResourceType::RESOURCE_IMAGE));

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

	void LevelNPC::openEditor()
	{
		EditAnonymousNPC frm(this, getWorld());
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

	void LevelNPC::deserializeJSON(cJSON* json)
	{
		setImageName(jsonGetChildString(json, "image"));
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));

		setWidth(jsonGetChildInt(json, "width"));
		setHeight(jsonGetChildInt(json, "height"));

		setCode(jsonGetChildString(json, "code"));
	}

	void LevelNPC::fileReady(const QString& fileName)
	{
		setImageName(getImageName());
	}

	void LevelNPC::fileWritten(const QString& fileName)
	{

	}

	Image* LevelNPC::getBlankNPCImage()
	{
		static Image* blankNPCImage = new Image("", QPixmap(":/MainWindow/icons/npc.png"));
		return blankNPCImage;
	}
};

