#include <gs1/GS1Prototypes.h>
#include "LevelNPC.h"
#include "cJSON/JsonHelper.h"
#include "EditAnonymousNPC.h"
#include "Level.h"
#include "EditExternalNPC.h"

namespace TilesEditor
{

	QRegularExpression LevelNPC::RegExImgPart("(?:^|[^/])(?:setimgpart|setgifpart)[\\( ]+(.*?),*([\\s\\d]+)*,*([\\s\\d]+)*,*([\\s\\d]+)*,([\\s\\d]+)\\)?");
	QRegularExpression LevelNPC::RegExSetImg("(?:^|[^/])(?:setimg|setgif)[\\( ]+(.+)\\)?(?:;|$|\\S)");

	//If any of these keywords are found in the code, it will trigger one of the script parsers
	QRegularExpression LevelNPC::RegShouldUseGS1Parser("showcharacter|setcharani|setcoloreffect|setimgpart|setgifpart|addtiledef2|addtiledef");

	QRegularExpression LevelNPC::RegIsSGScript("function +oncreated|onplayerenters\\s*\\(", QRegularExpression::CaseInsensitiveOption);

	LevelNPC::LevelNPC(IWorld* world, double x, double y, int width, int height) :
		AbstractLevelEntity(world, x, y)
	{
		setWidth(width);
		setHeight(height);
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
	
		

		if (m_image)
			getWorld()->getResourceManager()->freeResource(m_image);

		if (m_aniInstance)
		{
			m_aniInstance->freeResources(getWorld()->getResourceManager());
			delete m_aniInstance;
		}
		getWorld()->getResourceManager()->removeListener(this);

	}

	AniInstance* LevelNPC::getAniInstance()
	{
		if (m_aniInstance)
			return m_aniInstance;

		m_aniInstance = new AniInstance();
		return m_aniInstance;
	}

	void LevelNPC::setDir(int dir) { 
		m_dir = dir; 
	}

	void LevelNPC::setImageShape(int left, int top, int width, int height)
	{
		m_useImageShape = true;
		m_imageShape[0] = left;
		m_imageShape[1] = top;
		m_imageShape[2] = width;
		m_imageShape[3] = height;

		calculateDimensions();
	}

	void LevelNPC::loadResources()
	{
		if (m_image == nullptr)
		{
			if (!m_loadImageFail)
			{
				m_image = static_cast<Image*>(getWorld()->getResourceManager()->loadResource(this, m_imageName, ResourceType::RESOURCE_IMAGE));

				m_loadImageFail = m_image == nullptr;
			}
		}
	}

	void LevelNPC::releaseResources()
	{
		if (m_image)
			getWorld()->getResourceManager()->freeResource(m_image);
		m_image = nullptr;
	}

	void LevelNPC::draw(QPainter* painter, const QRectF& viewRect, double x, double y)
	{
		if (m_aniInstance && m_aniInstance->aniLoaded())
		{
			m_aniInstance->draw(m_dir, getX(), getY(), getWorld()->getResourceManager(), painter);
			return;
		}

		auto oldCompositionMode = painter->compositionMode();

		auto oldOpactity = painter->opacity();
		if (m_drawAsLight)
		{
			painter->setOpacity(m_drawColour.alphaF());
			painter->setCompositionMode(QPainter::CompositionMode_Plus);
		}

		auto clipping = painter->hasClipping();
		auto drawWidth = getWidth();
		auto drawHeight = getHeight();



		painter->setClipRect(x, y, drawWidth, drawHeight, painter->hasClipping() ? Qt::IntersectClip : Qt::ReplaceClip);
		painter->setClipping(true);

		auto image = (m_image == nullptr || !m_image->isLoaded()) ? getBlankNPCImage() : m_image;
		
		if (image)
		{
			int imageLeft = 0;
			int imageTop = 0;
			int imageWidth = image->width();
			int imageHeight = image->height();
			if (m_useImageShape)
			{
				imageLeft = m_imageShape[0];
				imageTop = m_imageShape[1];
				imageWidth = m_imageShape[2];
				imageHeight = m_imageShape[3];
			}

			switch (m_renderMode)
			{
				case RenderMode::Centered:
				{
					if (imageWidth < drawWidth || imageHeight < drawHeight)
					{
						QPen pen(m_shapeColour);
						QBrush brush(QColor(m_shapeColour.red(), m_shapeColour.green(), m_shapeColour.blue(), 50));
					
						pen.setWidth(2);
						auto oldPen = painter->pen();
						auto oldBrush = painter->brush();

						painter->setPen(pen);
						painter->setBrush(brush);
						painter->drawRect(x, y, drawWidth, drawHeight);

						painter->setPen(oldPen);
						painter->setBrush(oldBrush);
					}

					if(m_drawColour == QColorConstants::White)
						image->draw(painter, x + drawWidth / 2 - imageWidth / 2, y + drawHeight / 2 - imageHeight / 2, imageLeft, imageTop, imageWidth, imageHeight);
					else
					{
						if (m_cachedColorMod.isNull())
							m_cachedColorMod = image->colorMod(m_drawColour);
						painter->drawPixmap(x + drawWidth / 2 - imageWidth / 2, y + drawHeight / 2 - imageHeight / 2, m_cachedColorMod, imageLeft, imageTop, imageWidth, imageHeight);
					}
				}
				break;

				case RenderMode::Tiled:
				{
					for (int yy = 0; yy < drawHeight; yy += imageHeight)
					{
						for (int xx = 0; xx < drawWidth; xx += imageWidth)
						{
							image->draw(painter, x + xx, y + yy, imageLeft, imageTop, imageWidth, imageHeight);
						}
					}
				}
				break;

				case RenderMode::Stretched:
				{
					image->drawStretch(painter, x, y, imageLeft, imageTop, imageWidth, imageHeight, drawWidth, drawHeight);
				}
				break;
			}
		}

		painter->setOpacity(oldOpactity);
		painter->setClipping(clipping);
		painter->setCompositionMode(oldCompositionMode);
	}

	void LevelNPC::calculateDimensions()
	{
		if (!m_hasResized)
		{
			if (m_image && m_image->isLoaded() && m_aniInstance == nullptr)
			{
				if (!m_useImageShape)
				{
					setWidth(m_image->width());
					setHeight(m_image->height());
				}
				else {
					setWidth(m_imageShape[2]);
					setHeight(m_imageShape[3]);
				}
			}
			else {
				setWidth(48);
				setHeight(48);
			}
			getWorld()->updateEntityRect(this);
		}
		
	}

	void LevelNPC::setImageName(const QString& name)
	{
		if (name == "") {
			m_imageName = "";
			if (m_image != nullptr)
				getWorld()->getResourceManager()->freeResource(m_image);
			m_image = nullptr;
		}

		else 
		{
			m_imageName = name;



			auto newImage = static_cast<Image*>(getWorld()->getResourceManager()->loadResource(this, m_imageName, ResourceType::RESOURCE_IMAGE));
			if (m_image != nullptr)
				getWorld()->getResourceManager()->freeResource(m_image);

			m_image = newImage;

			
		}

		calculateDimensions();
	}

	void LevelNPC::setCode(const QString& code)
	{
		m_code = code;
		resetCharacter();

		if (m_image)
		{
			auto match = RegExImgPart.match(code);
			if (match.hasMatch())
			{
				auto imageName = match.captured(1);


				if (imageName.trimmed() == m_imageName)
				{
					auto imageLeft = match.captured(2).toInt();
					auto imageTop = match.captured(3).toInt();
					auto imageWidth = match.captured(4).toInt();
					auto imageHeight = match.captured(5).toInt();


					setImageShape(imageLeft, imageTop, imageWidth, imageHeight);
				}
			}
			else if (m_useImageShape) {
				m_useImageShape = false;
				getWorld()->updateEntityRect(this);
			}

			if (code.indexOf("drawaslight") != -1)
			{
				m_drawAsLight = true;
			}
		}

		if (RegShouldUseGS1Parser.match(m_code).hasMatch())
		{
			auto language = detectScriptingLanguage(m_code);
			if(language == SCRIPT_GS1)
				LevelNPCGS1Parser a(m_code, this);

			else if(language == SCRIPT_SGSCRIPT)
				LevelNPCSGScriptParser a(m_code, this);
		}
	}

	void LevelNPC::setCodeRaw(const QString& code)
	{
		m_code = code;
	}

	void LevelNPC::setProperty(const QString& name, const QVariant& value)
	{
		AbstractLevelEntity::setProperty(name, value);
		if (name == "image")
			setImageName(value.toString());

		else if (name == "code")
			setCode(value.toString());
	}

	void LevelNPC::setColourEffect(double r, double g, double b, double a)
	{
		QPixmap blank;
		m_cachedColorMod.swap(blank);

		m_drawAsLight = true;
		m_drawColour = QColor(int(r * 255), int(g * 255), int(b * 255), int(a * 255));
	}


	void LevelNPC::showCharacter()
	{
		getAniInstance();
		setWidth(48);
		setHeight(48);

		getWorld()->updateEntityRect(this);
	}

	void LevelNPC::resetCharacter()
	{
		if (m_aniInstance)
		{
			m_aniInstance->freeResources(getWorld()->getResourceManager());
			delete m_aniInstance;
			m_aniInstance = nullptr;
			m_dir = 2;
		}
	}

	QRectF LevelNPC::getBoundingBox() const
	{
		if (m_aniInstance && m_aniInstance->aniLoaded())
		{
			auto aniBoundingBox = m_aniInstance->getFrameBoundingBox();
			return QRectF(aniBoundingBox.x() + getX(), aniBoundingBox.y() + getY(), qMax(48.0, aniBoundingBox.width()), qMax(48.0, aniBoundingBox.height()));

		}
		return AbstractLevelEntity::getBoundingBox();
	}

	bool LevelNPC::canAddToLevel(Level* level) {
		return level->getLevelFlags().canLayAnonymousNPC;
	}

	ScriptingLanguage LevelNPC::detectScriptingLanguage(const QString& code)
	{
		if (RegIsSGScript.match(code).hasMatch())
		{
			return SCRIPT_SGSCRIPT;
		}
		return SCRIPT_GS1;
	}

	void LevelNPC::setAniName(const QString& ani, int frame)
	{
		if (getAniInstance())
		{
			getAniInstance()->setAniName(this, ani, frame, getWorld()->getResourceManager());
			calculateDimensions();
		}
	}


	void LevelNPC::openEditor()
	{
		EditAnonymousNPC frm(this, getWorld());
		frm.exec();
	}

	QPixmap LevelNPC::getIcon()
	{
		if (m_aniInstance && m_aniInstance->aniLoaded())
			return m_aniInstance->getIcon();

		return m_image ? m_image->pixmap() : getBlankNPCImage()->pixmap();
	}

	AbstractLevelEntity* LevelNPC::duplicate() {
		auto newNPC = new LevelNPC(this->getWorld(), getX(), getY(), getWidth(), getHeight());
		newNPC->setImageName(getImageName());
		newNPC->setCode(m_code);
		//newNPC->m_code = this->m_code;
		newNPC->setLevel(getLevel());
		return newNPC;
	}

	cJSON* LevelNPC::serializeJSON(bool useLocalCoordinates)
	{
		auto json = cJSON_CreateObject();

		cJSON_AddStringToObject(json, "type", "levelNPCv1");
		cJSON_AddStringToObject(json, "image", getImageName().toLocal8Bit().data());

		if (useLocalCoordinates && getLevel()) {
			cJSON_AddNumberToObject(json, "x", getX() - getLevel()->getX());
			cJSON_AddNumberToObject(json, "y", getY() - getLevel()->getY());
		}
		else {
			cJSON_AddNumberToObject(json, "x", getX());
			cJSON_AddNumberToObject(json, "y", getY());
		}
		cJSON_AddNumberToObject(json, "layer", getLayerIndex());
		cJSON_AddStringToObject(json, "code", getCode().toLocal8Bit().data());


		return json;
	}

	void LevelNPC::deserializeJSON(cJSON* json)
	{
		
		setX(jsonGetChildDouble(json, "x"));
		setY(jsonGetChildDouble(json, "y"));
		setLayerIndex(jsonGetChildInt(json, "layer"));
		setWidth(48);
		setHeight(48);
		setImageName(jsonGetChildString(json, "image"));
		setCode(jsonGetChildString(json, "code"));
	}

	void LevelNPC::fileFailed(const QString& name, AbstractResourceManager* resourceManager)
	{
	}

	void LevelNPC::fileReady(const QString& fileName, AbstractResourceManager* resourceManager)
	{
		calculateDimensions();
	}

	void LevelNPC::fileWritten(const QString& fileName, AbstractResourceManager* resourceManager)
	{

	}

	Image* LevelNPC::getBlankNPCImage()
	{
		static Image* blankNPCImage = new Image("", QImage(":/MainWindow/icons/npc.png"));
		return blankNPCImage;
	}

	//BELOW HERE IS FOR SIMPLE PARSING OF GS1/SGSCRIPT (gs2) CODE.
	//Simple things like setting the ani, color effect, etc within the oncreated/onplayerenters event
	
	//GS1 parsing
	LevelNPCGS1Parser::LevelNPCGS1Parser(const QString& code, LevelNPC* npc):
		m_npc(npc)
	{
		auto observer = [](const gs1::Diag& d, void* userPointer) ->void
		{
		};

	
		gs1::DiagBuilder diagBuilder(observer, this);

		auto _code = code.toStdString();
		gs1::MemorySource source(_code.c_str(), _code.length());

		gs1::Lexer lexer(diagBuilder, source);

		gs1::Parser parser(diagBuilder, lexer, gs1::prototypes_cmds, gs1::prototypes_funcs);

		auto rootNode = parser.Parse();
		rootNode->Accept(this);
	}


	void LevelNPCGS1Parser::Visit(gs1::StmtIf* node)
	{
		if (m_blockLevel == 1)
		{
			if (node->cond->GetType() == "ExprId")
			{
				auto expression = static_cast<gs1::ExprId*>(node->cond);
				auto actionName = QString::fromStdString(expression->name->token.text);
				
				if (actionName == "created" || actionName == "playerenters")
				{
					if (node->thenBody)
						node->thenBody->Accept(this);
				}
			}
		}
	}

	void LevelNPCGS1Parser::Visit(gs1::StmtBlock* node)
	{
		++m_blockLevel;
		for (auto statement : node->statements)
			statement->Accept(this);
		--m_blockLevel;
	}

	void LevelNPCGS1Parser::Visit(gs1::StmtCommand* node)
	{
		QString commandName = QString::fromStdString(node->name->token.text);

		if (commandName == "showcharacter")
		{
			m_npc->showCharacter();
			m_npc->setAniName("idle.gani", 0);
			m_npc->getAniInstance()->setProperty("HEAD", "head2.png", m_npc->getWorld()->getResourceManager());
			m_npc->getAniInstance()->setProperty("BODY", "body.png", m_npc->getWorld()->getResourceManager());
		}

		else if (commandName == "setcharani")
		{
			auto args = getArguments(node->args);
			if (args.size() >= 1)
			{
				auto aniName = args[0].toString() + ".gani";
				m_npc->setAniName(aniName, 0);

				if (args.size() >= 2)
				{
					auto aniParts = args[1].toString().split(',');
					if (aniParts.size() > 0)
					{
						int param = 1;
						for (qsizetype i = 0; i < aniParts.size(); ++i)
						{

							auto paramValue = aniParts[i];
							m_npc->getAniInstance()->setProperty(QString("PARAM%1").arg(param), paramValue, m_npc->getWorld()->getResourceManager());

							++param;
						}
					}
				}

			}

		}
		else if (commandName == "setcharprop")
		{
			auto args = getArguments(node->args);
			if (args.size() >= 2)
			{
				static QMap<QString, QString> propLookup = {
					{"#3", "HEAD"},
					{"#2", "SHIELD"},
					{"#1", "SWORD"},
					{"#8", "BODY"},
					{"#5", "HORSE"},
					{"#P1", "ATTR1"},
					{"#P2", "ATTR2"},
					{"#P3", "ATTR3"},
					{"#P4", "ATTR4"},
					{"#P5", "ATTR5"},
					{"#P6", "ATTR6"},
					{"#P7", "ATTR7"},
					{"#P8", "ATTR8"},
					{"#P9", "ATTR9"},
					{"#P10", "ATTR10"}
				};
				//setcharprop #C1,black;

				static QMap<QString, int> bodyColourLookup = {
					{"#C0", Image::BODY_SKIN},
					{"#C1", Image::BODY_SHIRT},
					{"#C2", Image::BODY_SLEEVE},
					{"#C3", Image::BODY_SHOES},
					{"#C4", Image::BODY_BELT},
				};
				QString name = args[0].toString();
				QString propValue = args[1].toString();

				auto it = propLookup.find(name);
				if (it != propLookup.end())
				{
					m_npc->getAniInstance()->setProperty(it.value(), propValue, m_npc->getWorld()->getResourceManager());
				}
				else {
					auto it2 = bodyColourLookup.find(name);
					if (it2 != bodyColourLookup.end())
					{
						QColor color = QColor(propValue);
						if (color.isValid())
						{
							m_npc->getAniInstance()->setBodyColour(it2.value(), color.rgba());
						}
					}
				}
				
			}
		}
		else if (commandName == "addtiledef2")
		{
			auto args = getArguments(node->args);
			if (args.size() >= 4)
			{
				auto world = m_npc->getWorld();
				QString imageName = args[0].toString();
				QString levelName = args[1].toString();

				auto x = args[2].toInt();
				auto y = args[3].toInt();

				world->getEngine()->addTileDef2(imageName, levelName, x, y, false);
				
			}
		}
		else if (commandName == "addtiledef")
		{
			auto args = getArguments(node->args);
			if (args.size() >= 2)
			{
				auto world = m_npc->getWorld();

				QString imageName = args[0].toString();
				QString levelName = args[1].toString();

				world->getEngine()->addTileDef2(imageName, levelName, 0, 0, false);
			}
		}
		else if (commandName == "setcoloreffect")
		{
			auto args = getArguments(node->args);
			if (args.size() >= 4)
			{
				auto red = args[0].toDouble();
				auto green = args[1].toDouble();
				auto blue = args[2].toDouble();
				auto alpha = args[3].toDouble();

				m_npc->setColourEffect(red, green, blue, alpha);
			}
		}
		else if (commandName == "setgifpart" || commandName == "setimgpart")
		{
			auto args = getArguments(node->args);
			if (args.size() == 5)
			{
				m_npc->setImageShape(args[1].toInt(), args[2].toInt(), args[3].toInt(), args[4].toInt());
				m_npc->setImageName(args[0].toString());
			}
		}

		/*
		
		else if (commandName == "join")
		{
			auto& args = node->args;
			if (args.size() >= 1 && args[0] && args[0]->GetType() == "ExprStringLiteral")
			{
				auto scriptFile = QString::fromStdString(static_cast<gs1::ExprStringLiteral*>(args[0])->literal->token.text) + ".txt";

				QString fullPath;

				if (m_npc->getWorld()->getResourceManager()->locateFile(scriptFile, &fullPath))
				{
					auto script = m_npc->getWorld()->getResourceManager()->readAllToString(fullPath);

					LevelNPCGS1Parser a(script, m_npc);
				}
				
			}
		}*/
			
		
	}

	void LevelNPCGS1Parser::Visit(gs1::ExprBinaryOp* node)
	{

		if (node->op->token.text == "=")
		{
			if (node->left && node->left->GetType() == "ExprId")
			{
				auto ident = QString::fromStdString(static_cast<gs1::ExprId*>(node->left)->name->token.text);

				if (ident == "sprite")
				{
					if (node->right && node->right->GetType() == "ExprNumberLiteral")
					{
						auto value = QString::fromStdString(static_cast<gs1::ExprNumberLiteral*>(node->right)->literal->token.text).toInt();
						m_npc->getAniInstance()->setAniName(m_npc, "def.gani", value, m_npc->getWorld()->getResourceManager());
					}
				}
				else if (ident == "dir")
				{
					if (node->right && node->right->GetType() == "ExprNumberLiteral")
					{
						auto value = QString::fromStdString(static_cast<gs1::ExprNumberLiteral*>(node->right)->literal->token.text).toInt();
						m_npc->setDir(value);
					}
				}

			}
		}
	}

	QVariantList LevelNPCGS1Parser::getArguments(const std::vector<gs1::Expr*>& args)
	{
		QVariantList retval;

		for (size_t i = 0; i < args.size(); ++i)
		{
			if (args[i])
			{
				if (args[i]->GetType() == "ExprStringLiteral")
					retval.push_back(QString::fromStdString(static_cast<gs1::ExprStringLiteral*>(args[i])->literal->token.text));

				else if (args[i]->GetType() == "ExprNumberLiteral")
					retval.push_back(QString::fromStdString(static_cast<gs1::ExprNumberLiteral*>(args[i])->literal->token.text).toDouble());

			}
			else retval.push_back(QVariant());
		}
		return retval;
	}


	LevelNPCSGScriptParser::LevelNPCSGScriptParser(const QString& code, LevelNPC* npc):
		m_npc(npc)
	{

		auto C = m_npc->getWorld()->getEngine()->getScriptContext();

		C->state &= ~(uint32_t)SGS_STATE__PARSERMASK;

		sgs_TokenList tlist = sgsT_Gen(C, code.toUtf8().data(), code.length());
	
		if (tlist)
		{
			C->jt_first = C->jt_last = NULL;
			C->joinFiles = sgs_membuf_create();

			sgs_FTNode* ftree = sgsFT_Compile(C, tlist);
	
			if (ftree)
			{
				auto root = ftree->child;

				if (root && root->type == SGS_SFT_BLOCK)
				{
					for (auto child = root->child; child; child = child->next)
					{
						if (child->type == SGS_SFT_FUNC)
						{
							auto args = child->child;
							auto nclose = args->next;
							auto body = nclose->next;
							auto name = body->next;

							if (name->type == SGS_SFT_IDENT && name->token[0] == SGS_ST_IDENT)
							{
								auto functionName = QString::fromStdString(std::string((char*)&name->token[2], name->token[1])).toLower();
								
								if (functionName == "oncreated" || "onplayerenters")
								{
									if (body->type == SGS_SFT_BLOCK)
									{
										for (auto blockChild = body->child; blockChild; blockChild = blockChild->next)
										{
											if (blockChild->type == SGS_SFT_EXPLIST)
											{
												for (auto statement = blockChild->child; statement; statement = statement->next)
												{
													if (statement->type == SGS_SFT_FCALL)
													{
														parseCallNode(statement);
													}

													else if (statement->type == SGS_SFT_OPER && statement->token[0] == SGS_ST_OP_SET)
													{
														
														parseAssignmentNode(statement);
													}
												}

											}
										}
									}
								}
							}
						}
					}
				}
				sgsFT_Destroy(C, ftree);
			}
			sgs_membuf_destroy(&C->joinFiles, C);

			sgsT_Free(C, tlist);
		}

	}

	void LevelNPCSGScriptParser::parseCallNode(sgs_FTNode* node)
	{
		auto name = node->child;
		if (name->type == SGS_SFT_IDENT && name->token[0] == SGS_ST_IDENT && name->next->type == SGS_SFT_EXPLIST)
		{
			auto functionName = QString::fromStdString(std::string((char*)&name->token[2], name->token[1])).toLower();
			if (functionName == "showcharacter")
			{
				m_npc->showCharacter();
				m_npc->setAniName("idle.gani", 0);
				m_npc->getAniInstance()->setProperty("HEAD", "head2.png", m_npc->getWorld()->getResourceManager());
				m_npc->getAniInstance()->setProperty("BODY", "body.png", m_npc->getWorld()->getResourceManager());
			} else if (functionName == "setcharani")
			{
				auto arguments = getArguments(name->next);

				if (arguments.size() >= 1)
				{
					if (arguments[0].userType() == QMetaType::QString)
					{
						auto aniName = arguments[0].toString() + ".gani";
						m_npc->setAniName(aniName, 0);

						for (int paramIndex = 1; paramIndex < arguments.size(); ++paramIndex)
						{
							auto paramValue = arguments[paramIndex].toString();
							m_npc->getAniInstance()->setProperty(QString("PARAM%1").arg(paramIndex), paramValue, m_npc->getWorld()->getResourceManager());

						}
					}
				}
			}
			else if (functionName == "setcoloreffect")
			{
				auto arguments = getArguments(name->next);
				if (arguments.size() >= 4)
				{
					m_npc->setColourEffect(arguments[0].toDouble(), arguments[1].toDouble(), arguments[2].toDouble(), arguments[3].toDouble());
				}
			}
			else if (functionName == "addtiledef")
			{
				auto arguments = getArguments(name->next);
				if (arguments.size() >= 2)
				{
					m_npc->getWorld()->getEngine()->addTileDef2(arguments[0].toString(), arguments[1].toString(), 0, 0, false);
				}
			}

			else if (functionName == "addtiledef2")
			{
				auto arguments = getArguments(name->next);
				if (arguments.size() >= 4)
				{
					m_npc->getWorld()->getEngine()->addTileDef2(arguments[0].toString(), arguments[1].toString(), arguments[2].toInt(), arguments[3].toInt(), false);
				}
			}
		}
	
	}

	static void printNode(sgs_FTNode* node, QString tab)
	{
		//qDebug() << tab << "Type: " << node->type;
		//qDebug() << tab << "Token: " << node->token[0];
		for (auto child = node->child; child; child = child->next)
		{
			printNode(child, tab + "    ");
		}
	}

	void LevelNPCSGScriptParser::parseAssignmentNode(sgs_FTNode* node)
	{
		//printNode(node, "");

		if (node->child)
		{
			//this.colors[0] = "black"
			if (node->child->type == SGS_SFT_INDEX && node->child->next)
			{
				auto indexNode = node->child->child;
				auto valueNode = node->child->next;
				auto value = parseTokenValue(valueNode);

				if (indexNode->type == SGS_SFT_OPER && indexNode->token[0] == SGS_ST_OP_MMBR && indexNode->child)
				{
					auto objectName = parseTokenValue(indexNode->child).toString();

					if (objectName == "this" && indexNode->next && indexNode->child->next)
					{
						auto memberName = parseTokenValue(indexNode->child->next);
						auto keyValue = parseTokenValue(indexNode->next);
						parseThisIndexAssignment(memberName.toString(), keyValue, value);
						return;
					}
				}
			}

			//this.dir = 1;
			//Check if child is '.' operator
			if (node->child->type == SGS_SFT_OPER && node->child->token[0] == SGS_ST_OP_MMBR) //'.' operator
			{
				//Object being the left side of the '.'
				auto objectNode = node->child->child;
				if (objectNode->type == SGS_SFT_KEYWORD && objectNode->token[0] == SGS_ST_KEYWORD)
				{
					auto objectName = parseTokenValue(objectNode).toString();

					//Object is "this"
					if (objectName == "this")
					{
						auto memberName = parseTokenValue(objectNode->next).toString();

						auto valueNode = node->child->next;
						if (valueNode->type == SGS_SFT_CONST)
						{
							parseThisAssignment(memberName, parseTokenValue(valueNode));
						}

					}

				}
			}
		}
	}

	void LevelNPCSGScriptParser::parseThisAssignment(const QString& memberName, const QVariant& value)
	{
		if (memberName == "headimg")
			m_npc->getAniInstance()->setProperty("HEAD", value.toString(), m_npc->getWorld()->getResourceManager());

		else if (memberName == "shieldimg")
			m_npc->getAniInstance()->setProperty("SHIELD", value.toString(), m_npc->getWorld()->getResourceManager());

		else if (memberName == "bodyimg")
			m_npc->getAniInstance()->setProperty("BODY", value.toString(), m_npc->getWorld()->getResourceManager());

		else if (memberName == "dir")
			m_npc->setDir(value.toInt());

		else if (memberName == "sprite")
			m_npc->getAniInstance()->setAniName(m_npc, "def.gani", value.toInt(), m_npc->getWorld()->getResourceManager());

	}

	void LevelNPCSGScriptParser::parseThisIndexAssignment(const QString& memberName, const QVariant& key, const QVariant& value)
	{
		if (memberName == "colors")
		{
			auto colourIndex = key.toInt();
			QColor colour(value.toString());

			if (colour.isValid())
				m_npc->getAniInstance()->setBodyColour(colourIndex, colour.rgba());

		}
	}

	QVariantList LevelNPCSGScriptParser::getArguments(sgs_FTNode* expressionList)
	{
		QVariantList retval;

		if (expressionList->child)
		{
			for (auto arg = expressionList->child; arg; arg = arg->next)
			{
				if (arg->type == SGS_SFT_CONST)
				{
					auto value = parseTokenValue(arg);
					retval.push_back(value);
				}
				else retval.push_back(QString());

			}
		}
		return retval;
	}

	QVariant LevelNPCSGScriptParser::parseTokenValue(sgs_FTNode* node)
	{
		if (node->token[0] == SGS_ST_STRING)
		{
			int32_t arglen = 0;
			memcpy(&arglen, &node->token[1], sizeof(int32_t));
			auto value = QString::fromStdString(std::string((char*)&node->token[5], arglen));
			return value;
		}
		else if (node->token[0] == SGS_ST_NUMINT)
		{
			int64_t value = 0;
			memcpy(&value, &node->token[1], sizeof(int64_t));
			return value;
		}
		else if (node->token[0] == SGS_ST_NUMREAL)
		{
			double value = 0.0;
			memcpy(&value, &node->token[1], sizeof(double));
			return value;
		}
		else if (node->token[0] == SGS_ST_KEYWORD || node->token[0] == SGS_ST_IDENT)
		{
			int8_t arglen = 0;
			memcpy(&arglen, &node->token[1], sizeof(int8_t));
			return QString::fromStdString(std::string((char*)&node->token[2], arglen));
		}
		return QVariant();
	}
};

