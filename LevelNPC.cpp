#include <QDebug>

#include "LevelNPC.h"
#include "cJSON/JsonHelper.h"
#include "EditAnonymousNPC.h"

namespace TilesEditor
{

	QRegularExpression LevelNPC::m_imgPartExpression("(?:^|[^/])setimgpart[\\( ]+(.*?),*([\\s\\d]+)*,*([\\s\\d]+)*,*([\\s\\d]+)*,([\\s\\d]+)\\)?");

	LevelNPC::LevelNPC(IWorld* world, double x, double y, int width, int height) :
		AbstractLevelEntity(world, x, y)
	{
		m_width = width;
		m_height = height;
		m_image = nullptr;

		m_loadImageFail = false;
		m_useImageShape = false;
	}

	LevelNPC::LevelNPC(IWorld* world, cJSON* json) :
		LevelNPC(world, 0.0, 0.0, 0, 0)
	{
		deserializeJSON(json);

	}

	LevelNPC::~LevelNPC()
	{
		getWorld()->getResourceManager().getFileSystem()->removeListener(this);

		if (m_image)
			getWorld()->getResourceManager().freeResource(m_image);

	}

	void LevelNPC::setImageShape(int left, int top, int width, int height)
	{
		m_useImageShape = true;
		m_imageShape[0] = left;
		m_imageShape[1] = top;
		m_imageShape[2] = width;
		m_imageShape[3] = height;

		setWidth(width);
		setHeight(height);

		getWorld()->updateEntityRect(this);
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
			if (m_useImageShape)
			{
				m_image->draw(painter, x, y, m_imageShape[0], m_imageShape[1], m_imageShape[2], m_imageShape[3]);
			}
			else m_image->draw(painter, x, y);
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

		getWorld()->updateEntityRect(this);
	}

	void LevelNPC::setCode(const QString& code) 
	{ 
		m_code = code; 

		if (m_image)
		{
			auto match = m_imgPartExpression.match(code);
			if (match.hasMatch())
			{
				auto imageLeft = match.captured(2).toInt();
				auto imageTop = match.captured(3).toInt();
				auto imageWidth = match.captured(4).toInt();
				auto imageHeight = match.captured(5).toInt();
				setImageShape(imageLeft, imageTop, imageWidth, imageHeight);
			}
			else if (m_useImageShape) {
				m_useImageShape = false;
				getWorld()->updateEntityRect(this);
			}
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
		cJSON_AddStringToObject(json, "code", getCode().toLocal8Bit().data());


		return json;
	}

	void LevelNPC::deserializeJSON(cJSON* json)
	{
		
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));

		setWidth(48);
		setHeight(48);
		setImageName(jsonGetChildString(json, "image"));
		setCode(jsonGetChildString(json, "code"));
	}

	void LevelNPC::fileFailed(const QString& name)
	{
		getWorld()->getResourceManager().addFailedResource(name);
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

