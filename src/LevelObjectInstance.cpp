#include "LevelObjectInstance.h"
#include "EditExternalNPC.h"
#include "Level.h"
#include "IEngine.h"
#include "cJSON/JsonHelper.h"

namespace TilesEditor
{

	LevelObjectInstance::LevelObjectInstance(IWorld* world, double x, double y, const QString& className, ObjectClass* objectClass, const QStringList& params):
		LevelNPC(world, x, y, 48, 48), m_className(className), m_objectClass(objectClass), m_params(params)
	{
		m_isObjectInstance = true;


		if (m_objectClass != nullptr)
		{
			m_objectClass->addInstance(this);
			int left, top, width, height;
			setImageName(objectClass->getImageName());

			if (objectClass->getImageShape(&left, &top, &width, &height))
				setImageShape(left, top, width, height);

			setRenderMode(m_objectClass->getRenderMode());

			setScriptingLanguage(m_objectClass->getScriptingLanguage());

		}
		m_startWidth = getWidth();
		m_startHeight = getHeight();

		parseClassCode();

	}

	LevelObjectInstance::LevelObjectInstance(IWorld* world, cJSON* json):
		LevelNPC(world, 0.0, 0.0, 48, 48)
	{
		deserializeJSON(json);
	}

	LevelObjectInstance::~LevelObjectInstance()
	{
		if (m_objectClass)
		{
			m_objectClass->removeInstance(this);
			getWorld()->getResourceManager()->getObjectManager()->releaseObject(m_objectClass);
		}

	}

	void LevelObjectInstance::setProperty(const QString& name, const QVariant& value)
	{
		LevelNPC::setProperty(name, value);

		if (name == "params")
			getParams() = value.toStringList();
	}

	void LevelObjectInstance::openEditor()
	{
		EditExternalNPC frm(getWorld(), this);
		if(frm.exec())
			parseClassCode();
	}

	AbstractLevelEntity* LevelObjectInstance::duplicate()
	{
		if (m_objectClass)
			m_objectClass->incrementRef();

		auto newNPC = new LevelObjectInstance(this->getWorld(), getX(), getY(), m_className, m_objectClass, QStringList());


		if(m_objectClass)
		{
			auto& objectParams = m_objectClass->getParams();
			qsizetype paramCount = qMax(objectParams.size(), m_params.size());

			for (qsizetype i = 0; i < paramCount; ++i)
			{
				QString value = i < m_params.size() ? m_params[i] : "";

				if (i < objectParams.size())
				{
					if (!objectParams[i]->canCopy())
						value = getWorld()->getEngine()->parseInlineString(objectParams[i]->getDefaultValue());
				}

				newNPC->getParams().push_back(value);
			}
		}
		else newNPC->getParams() = m_params;
		newNPC->setImageName(getImageName());

		if (hasResized())
		{
			newNPC->setHasResized(true);
			newNPC->setWidth(getWidth());
			newNPC->setHeight(getHeight());
		}

		newNPC->parseClassCode();
		return newNPC;
	}

	cJSON* LevelObjectInstance::serializeJSON(bool useLocalCoordinates)
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelNPCv2");
		cJSON_AddStringToObject(json, "class", m_className.toLocal8Bit().data());

		if (m_objectClass)
		{
			if (m_objectClass->getImageName() != getImageName())
				cJSON_AddStringToObject(json, "image", getImageName().toLocal8Bit().data());

		} else cJSON_AddStringToObject(json, "image", getImageName().toLocal8Bit().data());

		if (useLocalCoordinates && getLevel()) {
			cJSON_AddNumberToObject(json, "x", getX() - getLevel()->getX());
			cJSON_AddNumberToObject(json, "y", getY() - getLevel()->getY());
		}
		else {
			cJSON_AddNumberToObject(json, "x", getX());
			cJSON_AddNumberToObject(json, "y", getY());
		}
		cJSON_AddNumberToObject(json, "layer", getLayerIndex());

		if (hasResized())
		{
			cJSON_AddNumberToObject(json, "width", getWidth());
			cJSON_AddNumberToObject(json, "height", getHeight());
		}

		
		auto params = cJSON_CreateArray();
		for (auto& param : m_params)
		{
			cJSON_AddItemToArray(params, cJSON_CreateString(param.toLocal8Bit().data()));
		}
		cJSON_AddItemToObject(json, "params", params);

		return json;
	}

	void LevelObjectInstance::deserializeJSON(cJSON* json)
	{
		m_className = jsonGetChildString(json, "class", "");


		m_objectClass = getWorld()->getResourceManager()->getObjectManager()->loadObject(m_className, false);

		setWidth(48);
		setHeight(48);

		auto imageName = jsonGetChildString(json, "image");
		if(imageName != "")
			setImageName(imageName);

		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));
		setLayerIndex(jsonGetChildInt(json, "layer"));

		auto params = cJSON_GetObjectItem(json, "params");
		if (params != nullptr && params->type == cJSON_Array)
		{
			for (int i = 0; i < cJSON_GetArraySize(params); ++i)
			{
				auto param = jsonGetArrayString(params, i, "");
				m_params.push_back(param);
			}
		}

		if (m_objectClass != nullptr)
		{
			m_objectClass->addInstance(this);
			if (imageName == "")
				setImageName(m_objectClass->getImageName());

			int left, top, width, height;
			if (m_objectClass->getImageShape(&left, &top, &width, &height))
				setImageShape(left, top, width, height);
			setRenderMode(m_objectClass->getRenderMode());

			setScriptingLanguage(m_objectClass->getScriptingLanguage());
		}

		auto jsonWidth = cJSON_GetObjectItem(json, "width");
		if (jsonWidth && jsonWidth->isint) {
			setWidth(jsonWidth->valueint);
			setHasResized(true);
		}

		auto jsonHeight = cJSON_GetObjectItem(json, "height");
		if (jsonHeight && jsonHeight->isint) {
			setHeight(jsonHeight->valueint);
			setHasResized(true);
		}

		m_startWidth = getWidth();
		m_startHeight = getHeight();

		parseClassCode();
	}

	QString LevelObjectInstance::toString() const 
	{
		return QString("[Npc: %1, %2, %3]").arg(m_objectClass ? m_objectClass->getName() : getImageName()).arg(getX()).arg(getY());
	}

	void LevelObjectInstance::draw(QPainter* painter, const QRectF& viewRect, double x, double y)
	{
		LevelNPC::draw(painter, viewRect, x, y);
	}

	bool LevelObjectInstance::canResize() const
	{
		//if (m_objectClass)
		//	return m_objectClass->canResize();
		//return false;
		return true;
	}

	void LevelObjectInstance::updateResize(int edges, int mouseX, int mouseY, bool snap, double snapX, double snapY) {
	
		setHasResized(true);
		AbstractLevelEntity::updateResize(edges, mouseX, mouseY, snap, std::ceil(snapX / 16.0) * 16.0, std::ceil(snapY / 16.0) * 16.0);
	}

	void LevelObjectInstance::resetSize()
	{
		if (hasResized())
		{
			setHasResized(false);
			setWidth(m_startWidth);
			setHeight(m_startHeight);

		}
	}
	bool LevelObjectInstance::canAddToLevel(Level* level) {
		return level->getLevelFlags().canLayObjectInstance;
	}

	void LevelObjectInstance::markObjectChanged()
	{
		getWorld()->setModified(getLevel());

		parseClassCode();

	}

	void LevelObjectInstance::parseClassCode()
	{
		if (m_objectClass != nullptr)
		{
			auto code = formatGraalCode(true, false, getParams());

			if (RegShouldUseGS1Parser.match(code).hasMatch())
			{
				auto language = detectScriptingLanguage(code);
				if (language == SCRIPT_GS1)
					LevelNPCGS1Parser a(code, this);

				else if (language == SCRIPT_SGSCRIPT)
					LevelNPCSGScriptParser a(code, this);
			}
		}
	}

	QString LevelObjectInstance::formatGraalCode(bool forceEmbed, bool insertHeader, const QStringList& params) const
	{
		if (m_objectClass != nullptr)
		{
			QString header = "";
			
			if (insertHeader)
			{ 
				header += QString("//#OBJECT %1\n").arg(m_objectClass->getName());
				if (m_useImageShape)
					header += QString("//#IMAGERECT %1 %2 %3 %4\n").arg(m_imageShape[0]).arg(m_imageShape[1]).arg(m_imageShape[2]).arg(m_imageShape[3]);

				for (auto& param : params)
					header += QString("//#PARAM %1\n").arg(param);

				if (hasResized())
					header += QString("//#SIZE %1 %2\n").arg(getWidth()).arg(getHeight());
			}

			if (m_objectClass->shouldEmbedCode() || forceEmbed)
			{
				QString code = m_objectClass->getServerCode();

				auto& clientCode = m_objectClass->getClientCode();

				if (!clientCode.isEmpty())
					code += QString("\n//#CLIENTSIDE\n%1").arg(clientCode);


				int paramIndex = 0;
				for (auto& param : params)
				{
					if (paramIndex < m_objectClass->getParams().size())
					{
						if (m_objectClass->getParams()[paramIndex]->isString())
						{
							auto newValue = getWorld()->getEngine()->escapeString(param, getScriptingLanguage());
							//code = code.replace(QString("param%1").arg(++paramIndex), newValue);

							//^ why does this leave a weird character...?
							code = code.replace(QString("param") + QString::number(++paramIndex), newValue);
							continue;
						}
					}
					code = code.replace(QString("param") + QString::number(++paramIndex), param);
						//code.replace(QString("param%1").arg(++paramIndex), param);
				}

				if (header.isEmpty())
					return code;
				else return header + "\n" + code;
			}
			return header;

			
			
		}
		return QString();
	}
};