#include <QTextStream>
#include "Ani.h"

namespace TilesEditor
{
	Ani::AniSprite* Ani::AniSprite::duplicate(int newIndex)
	{
		auto retval = new Ani::AniSprite();
		retval->index = newIndex;
		retval->attachedSprites = this->attachedSprites;
		retval->colorEffect = this->colorEffect;
		retval->colorEffectEnabled = this->colorEffectEnabled;
		retval->comment = this->comment;
		retval->type = this->type;
		retval->customImageName = this->customImageName;
		retval->left = this->left;
		retval->top = this->top;
		retval->width = this->width;
		retval->height = this->height;

		retval->rotation = this->rotation;
		retval->xscale = this->xscale;
		retval->yscale = this->yscale;
		retval->boundingBox = this->boundingBox;
		return retval;
	}
	void Ani::AniSprite::updateBoundingBox()
	{
		//rotate bounding box

		QTransform t;

		t.translate(this->width / 2, this->height / 2);
		t.scale(this->xscale, this->yscale);
		t.rotate(this->rotation);
		
		t.translate(this->width / -2, this->height / -2);
		this->boundingBox = t.mapToPolygon(QRect(0, 0, this->width + 1, this->height + 1));
	}

	Image* Ani::AniSprite::getCustomImage(AbstractResourceManager* resouceManager)
	{
		if (customImage == nullptr && !customImageFail)
		{
			customImage = static_cast<Image*>(resouceManager->loadResource(nullptr, customImageName, ResourceType::RESOURCE_IMAGE));
			customImageFail = customImage == nullptr;

			if (customImageFail)
			{

			}
		}
		return customImage;
	}

	void Ani::AniSprite::releaseCustomImage(AbstractResourceManager* resouceManager)
	{
		if (customImage != nullptr) {
			resouceManager->freeResource(customImage);
			customImage = nullptr;
		}
		customImageFail = false;
	}

	//Ani
	Ani::Ani(const QString& name, AbstractResourceManager* resourceManager):
		Resource(name)
	{
		m_looped = false;
		m_singleDir = false;
		m_speed = 1.0;
		m_containsBodySprite = false;
		setDefaultImage("SPRITES", "sprites.png", resourceManager);
		setDefaultImage("PICS", "pics1.png", resourceManager);
	}

	Ani::~Ani()
	{
		for (auto sprite : m_sprites)
		{
			delete sprite;

		}

		
		for (auto frame : m_frames)
		{
			delete frame;
		}

	}

	void Ani::freeResources(AbstractResourceManager* resourceManager)
	{
		for (auto sprite : m_sprites)
		{
			sprite->releaseCustomImage(resourceManager);
			delete sprite;

		}
		m_sprites.clear();
		
		for (auto& image : m_hiddenDefaults)
		{
			if(image)
				resourceManager->freeResource(image);
		}
		m_hiddenDefaults.clear();

		for (auto pair : m_defaultImages)
		{
			if (pair.second != nullptr)
			{
				resourceManager->freeResource(pair.second);
			}
		}
		m_defaultImages.clear();
	}

	bool Ani::addSprite(AniSprite* aniSprite)
	{
		auto& thing = m_sprites[aniSprite->index];
		bool retval = thing == nullptr;

		thing = aniSprite;

		if (aniSprite->index >= m_nextSpriteIndex)
			m_nextSpriteIndex = aniSprite->index + 1;
		return retval;
	}

	void Ani::addFrame(Frame* aniFrame)
	{
		m_frames.push_back(aniFrame);
	}

	Image* Ani::getHiddenDefaultImage(const QString& type, AbstractResourceManager* resourceManager)
	{
		static QMap<QString, QString> defaultFiles = {
			{"HEAD", "head1.png"},
			{"BODY", "body.png"},
			{"SWORD", "sword1.png"},
			{"SHIELD", "shield1.png"},
			{"HORSE", "ride.png"},
			{"ATTR1", "hat0.png"}
		};

		auto it = m_hiddenDefaults.find(type);
		if (it != m_hiddenDefaults.end())
			return it.value();

		auto it2 = defaultFiles.find(type);
		if (it2 != defaultFiles.end())
		{
			auto image = m_hiddenDefaults[type] = static_cast<Image*>(resourceManager->loadResource(nullptr, it2.value(), ResourceType::RESOURCE_IMAGE));

			if (!image)
			{

			}
			return image;
		}
		return nullptr;
	}

	bool Ani::isCustomImage(const QString& name)
	{
		static QSet<QString> internalTypes = {
			"HEAD",
			"BODY",
			"SWORD",
			"SHIELD",
			"SPRITES",
			"HORSE",
			"PICS",
			"ATTR1",
			"ATTR2",
			"ATTR3",
			"ATTR4",
			"ATTR5",
			"ATTR6",
			"ATTR7",
			"ATTR8",
			"ATTR9",
			"ATTR10",
			"PARAM1",
			"PARAM2",
			"PARAM3",
			"PARAM4",
			"PARAM5",
			"PARAM6",
			"PARAM7",
			"PARAM8",
			"PARAM9",
			"PARAM10"
		};

		return !internalTypes.contains(name);
	}

	const QRect& Ani::getBoundingBox() const
	{
		return m_boundingBox;
	}

	Ani::Frame* Ani::getFrame(size_t index)
	{
		if (index < m_frames.size())
			return m_frames[index];
		return nullptr;
	}
	 
	Ani::AniSprite* Ani::getAniSprite(int index, const QString& name)
	{
		return getAniSprite(this, index, name);
	}

	Ani::AniSprite* Ani::getAniSprite(IAniInstance* propertyProvider, int index, const QString& name)
	{
		if (index == SPRITE_INDEX_STRING)
		{
			bool ok = false;
			int newIndex = propertyProvider->getPropertyValue(name).toInt(&ok);
			if (ok) {
				auto it = m_sprites.find(newIndex);
				if (it != m_sprites.end())
					return it.value();

			}
			return nullptr;
		}
		auto it = m_sprites.find(index);
		if (it != m_sprites.end())
			return it.value();
		return nullptr;
	}

	Image* Ani::getDefaultImage(const QString& name)
	{
		auto it = m_defaultImages.find(name);
		if (it != m_defaultImages.end())
			return it.value().second;
		return nullptr;
	}

	QString Ani::getDefaultImageName(const QString& name)
	{
		auto it = m_defaultImages.find(name);
		if (it != m_defaultImages.end())
			return it.value().first;
		return "";
	}

	int Ani::getTotalDuration() const
	{
		int retval = 0;
		for (auto& frame : m_frames)
			retval += frame->duration;
		return retval;
	}

	void Ani::insertFrame(qsizetype pos, Ani::Frame* frame)
	{
		m_frames.insert(pos, frame);
	}

	void Ani::removeFrame(size_t index)
	{
		if (index < m_frames.size())
		{
			//delete m_frames[index];
			m_frames.remove(index, 1);
		}
	}

	void Ani::setDefaultImage(const QString& name, const QString& value, AbstractResourceManager* resourceManager)
	{
		auto it = m_defaultImages.find(name);
		if (it != m_defaultImages.end())
		{
			if (it.value().second)
			{
				resourceManager->freeResource(it.value().second);
				it.value().second = nullptr;
			}
			it.value().first = value;
			it.value().second = static_cast<Image*>(resourceManager->loadResource(nullptr, value, ResourceType::RESOURCE_IMAGE));
		}
		else {
			m_defaultImages[name] = QPair<QString, Image*>(value, static_cast<Image*>(resourceManager->loadResource(nullptr, value, ResourceType::RESOURCE_IMAGE)));
		}

	}

	void Ani::draw(int frameIndex, int dir, double x, double y, AbstractResourceManager* resourceManager, QPainter* painter, const QRectF& rect)
	{
		auto frame = this->getFrame((size_t)frameIndex);


		if (frame != nullptr)
		{
			if (this->m_singleDir) { dir = 0; }
			dir = dir < 0 ? 0 : dir;
			dir = dir > 3 ? 3 : dir;

			auto& pieces = frame->pieces[dir];

			for (auto& piece : pieces)
			{
				if (piece->type == Ani::Frame::PIECE_SPRITE)
				{
					auto spritePiece = static_cast<Ani::Frame::FramePieceSprite*>(piece);
					AniSprite* sprite = this->getAniSprite(spritePiece->spriteIndex, spritePiece->spriteName);

					if(sprite && sprite->width * sprite->height > 0)
						drawSprite(sprite, x + piece->xoffset, y + piece->yoffset, resourceManager, painter, rect, 0);
				}
				
			}

		}
	}

	void Ani::drawSprite(Ani::AniSprite* sprite, double x, double y, AbstractResourceManager* resourceManager, QPainter* painter, const QRectF& rect, int level)
	{
		if (level > 3)
			return;
		if (sprite != nullptr)
		{
			//Draw behind attachments first
			if (sprite->attachedSprites.size() > 0)
			{
				for (auto i = 0; i < sprite->m_drawIndex && i < sprite->attachedSprites.size(); ++i)
				{
					auto& pair = sprite->attachedSprites[i];
					auto childIndex = pair.first;
					auto& offsets = pair.second;

					auto child = this->getAniSprite(childIndex, "");
					if (child)
					{
						drawSprite(child, x + int(offsets.x()), y + int(offsets.y()), resourceManager, painter, rect, level + 1);
					}
				}
			}

			Image* image = nullptr;

			if (sprite->type == "CUSTOM")
				image = sprite->getCustomImage(resourceManager);
			else {
				image = getDefaultImage(sprite->type);

				if (image == nullptr)
					image = getHiddenDefaultImage(sprite->type, resourceManager);
			}

			if (image != nullptr)
			{
				auto opacity = painter->opacity();
				painter->save();
				painter->translate(int(x), int(y));

				if (!(sprite->rotation == 0.0 && sprite->xscale == 1.0 && sprite->yscale == 1.0))
				{
					painter->translate(sprite->width / 2, sprite->height / 2);
					painter->scale(sprite->xscale, sprite->yscale);
					painter->rotate(sprite->rotation);
					painter->translate(sprite->width / -2, sprite->height / -2);
				}

				if (sprite->colorEffectEnabled)
				{
					auto compMode = painter->compositionMode();
					if (sprite->colorEffect.alpha() < 255)
					{
						painter->setOpacity(opacity * sprite->colorEffect.alphaF());
						painter->setCompositionMode(QPainter::CompositionMode::CompositionMode_Plus);
					}
					painter->drawPixmap(0, 0, image->colorMod(sprite->colorEffect, QRect(sprite->left, sprite->top, sprite->width, sprite->height)));
					painter->setCompositionMode(compMode);

				}else painter->drawPixmap(0, 0, image->pixmap(), sprite->left, sprite->top, sprite->width, sprite->height);

				
				painter->restore();
				painter->setOpacity(opacity);
			}
			else {
				painter->save();
				painter->translate(int(x), int(y));

				if (!(sprite->rotation == 0.0 && sprite->xscale == 1.0 && sprite->yscale == 1.0))
				{
					painter->translate(sprite->width / 2, sprite->height / 2);
					painter->scale(sprite->xscale, sprite->yscale);
					painter->rotate(sprite->rotation);
					painter->translate(sprite->width / -2, sprite->height / -2);
				}

				auto opacity = painter->opacity();
				painter->setOpacity(0.65);
				auto oldPen = painter->pen();
				painter->setPen(QPen(QColorConstants::Black, 1.0, Qt::SolidLine, Qt::PenCapStyle::FlatCap, Qt::PenJoinStyle::MiterJoin));
				painter->fillRect(QRectF(0, 0, sprite->width, sprite->height), QColorConstants::White);
				painter->drawRect(QRectF(0, 0, sprite->width, sprite->height));

				painter->setPen(QPen(QColorConstants::Red, 1.0));

				painter->drawLine(QPointF(2, 2), QPointF(sprite->width - 2, sprite->height - 2));
				painter->drawLine(QPointF(2, sprite->height - 2), QPointF(sprite->width - 2, 2));
				painter->restore();
				painter->setOpacity(opacity);
			}

			//Draw above attachments
			if (sprite->attachedSprites.size() > 0)
			{
				for(auto i = sprite->m_drawIndex; i < sprite->attachedSprites.size(); ++i)
				{
					auto& pair = sprite->attachedSprites[i];
					auto childIndex = pair.first;
					auto& offsets = pair.second;

					auto child = this->getAniSprite(childIndex, "");
					if (child)
					{
						drawSprite(child, x + int(offsets.x()), y + int(offsets.y()), resourceManager, painter, rect, level + 1);
					}
				}
			}
		}
	}

	void Ani::replace(QIODevice* stream, AbstractResourceManager* resourceManager)
	{
		loadGraalAni(this, stream, resourceManager);
	}

	Ani* Ani::loadGraalAni(const QString& name, QIODevice* stream, AbstractResourceManager* resourceManager)
	{
		Ani* retval = new Ani(name, resourceManager);
		if (retval->loadGraalAni(retval, stream, resourceManager))
			return retval;

		delete retval;
		return nullptr;
	}

	bool Ani::loadGraalAni(Ani* ani, QIODevice* stream, AbstractResourceManager* resourceManager)
	{
		QStringList lines;

		QTextStream textStream(stream);
		for (QString line = textStream.readLine(); !line.isNull(); line = textStream.readLine())
			lines.push_back(line);

		int left = 0;
		int top = 0;
		int right = 0;
		int bottom = 0;

		int dirCount = 4;
		for (auto i = 0U; i < lines.size(); ++i)
		{
			auto& line = lines[i];

			QStringList words = line.split(" ", Qt::SplitBehaviorFlags::SkipEmptyParts);
			if (words.size() > 0)
			{
				QString& word1 = words[0];

				if (word1 == "SPRITE" && words.size() >= 7)
				{
					auto aniSprite = new Ani::AniSprite();
					aniSprite->type = words[2];

					if (aniSprite->type == "BODY")
					{
						ani->m_containsBodySprite = true;
					}
					else if (isCustomImage(aniSprite->type))
					{
						aniSprite->customImageName = aniSprite->type;
						aniSprite->type = "CUSTOM";

					}


					aniSprite->index = words[1].toInt();
					aniSprite->left = words[3].toInt();
					aniSprite->top =  words[4].toInt();
					aniSprite->width =  words[5].toInt();
					aniSprite->height =  words[6].toInt();
					aniSprite->boundingBox = QRectF(0, 0, aniSprite->width, aniSprite->height);
					aniSprite->updateBoundingBox();
					if (words.size() >= 8)
						aniSprite->comment = words.mid(7).join(" ");

	
					ani->addSprite(aniSprite);
				}
				else if (word1 == "STRETCHXEFFECT" && words.size() >= 3)
				{
					auto spriteIndex = words[1].toInt();
					auto sprite = ani->getAniSprite(spriteIndex, "");
					if (sprite)
					{
						sprite->xscale = words[2].toDouble();
						sprite->updateBoundingBox();
					}
				}
				else if (word1 == "STRETCHYEFFECT" && words.size() >= 3)
				{
					auto spriteIndex = words[1].toInt();
					auto sprite = ani->getAniSprite(spriteIndex, "");
					if (sprite)
					{
						sprite->yscale = words[2].toDouble();
						sprite->updateBoundingBox();
					}
				}
				else if (word1 == "ROTATEEFFECT" && words.size() >= 3)
				{
					auto spriteIndex = words[1].toInt();
					auto sprite = ani->getAniSprite(spriteIndex, "");
					if (sprite)
					{
						sprite->rotation = words[2].toDouble() / (M_PI / 180);
						sprite->updateBoundingBox();
					}
				}
				else if ((word1 == "ATTACHSPRITE" || word1 == "ATTACHSPRITE2") && words.size() >= 5)
				{
					auto behind = word1 == "ATTACHSPRITE2";

					auto parentIndex = words[1].toInt();
					auto childIndex = words[2].toInt();
					auto xoffset = words[3].toDouble();
					auto yoffset = words[4].toDouble();

					auto parent = ani->getAniSprite(parentIndex, "");
					if (parent)
					{
						if (behind)
						{
							parent->attachedSprites.insert(parent->m_drawIndex, QPair<int, QPointF>(childIndex, QPointF(xoffset, yoffset)));
							++parent->m_drawIndex;
						}else parent->attachedSprites.push_back(QPair<int, QPointF>(childIndex, QPointF(xoffset, yoffset)));
					}
				}
				else if (word1 == "COLOREFFECT" && words.size() >= 6)
				{
					auto spriteIndex = words[1].toInt();

					auto sprite = ani->getAniSprite(spriteIndex, "");
					if (sprite)
					{
						sprite->colorEffectEnabled = true;
						sprite->colorEffect = QColor(int(words[2].toDouble() * 255),
							int(words[3].toDouble() * 255),
							int(words[4].toDouble() * 255),
							int(words[5].toDouble() * 255));
					}
				}
				else if (word1 == "ZOOMEFFECT" && words.size() >= 3)
				{
					auto spriteIndex = words[1].toInt();

					auto sprite = ani->getAniSprite(spriteIndex, "");
					if (sprite)
					{
						sprite->xscale = sprite->yscale = words[2].toDouble();
						sprite->updateBoundingBox();
					}
				}
				else if (word1 == "LOOP")
				{
					ani->setLooped(true);
				}
				else if (word1 == "CONTINUOUS")
				{
					ani->setContinous(true);
				}
				else if (word1 == "SETBACKTO" && words.size() >= 2)
				{
					ani->m_nextAni = words[1];
				}
				else if (word1 == "SINGLEDIR" || word1 == "SINGLEDIRECTION")
				{
					ani->m_singleDir = true;
					dirCount = 1;
				}

				else if (words[0].startsWith("DEFAULT") && words.size() >= 2)
				{
					auto type = words[0].mid(strlen("DEFAULT"));

					ani->setDefaultImage(type, words[1], resourceManager);
					
				}
				else if (word1 == "ANI")
				{
					for (++i; i < lines.size(); ++i)
					{
						line = lines[i];

						if (line != "ANIEND")
						{
							auto frame = new Frame();
							int frameLeft = 0;
							int frameTop = 0;
							int frameRight = 0;
							int frameBottom = 0;
							for (int dir = 0; dir < dirCount && i + 1 < lines.size(); ++dir)
							{
							
								QStringList offsets = line.split(",", Qt::SplitBehaviorFlags::SkipEmptyParts);
								if (offsets.size() >= 1)
								{
									auto& frameSprites = frame->pieces[dir];

									for (auto& offset : offsets)
									{
										QStringList parts = offset.split(" ", Qt::SplitBehaviorFlags::SkipEmptyParts);
										if (parts.size() >= 3)
										{
											Frame::FramePieceSprite* frameSprite = new Frame::FramePieceSprite();
											frameSprite->type = Frame::PIECE_SPRITE;
											frameSprite->id = QRandomGenerator::global()->generate64();
											frameSprite->index = frameSprites.size();

											bool isValidSpriteIndex = false;
										
											frameSprite->spriteName = parts[0];
											frameSprite->spriteIndex = frameSprite->spriteName.toInt(&isValidSpriteIndex);

											//If the first part isn't a valid integer, it's probably something like PARAM1.
											if (!isValidSpriteIndex)
												frameSprite->spriteIndex = SPRITE_INDEX_STRING;
											

											frameSprite->xoffset = parts[1].toDouble();
											frameSprite->yoffset = parts[2].toDouble();

											//Global bounding box
											if (frameSprite->xoffset < left)
												left = frameSprite->xoffset;


											if (frameSprite->yoffset < top)
												top = frameSprite->yoffset;

											//Frame bounding box
											if (frameSprite->xoffset < frameLeft)
												frameLeft = frameSprite->xoffset;

											if (frameSprite->yoffset < frameTop)
												frameTop = frameSprite->yoffset;

											auto sprite = ani->getAniSprite(frameSprite->spriteIndex, frameSprite->spriteName);
											if (sprite != nullptr)
											{
												//Global bounding box
												if (frameSprite->xoffset + sprite->width > right)
													right = frameSprite->xoffset + sprite->width;
												if (frameSprite->yoffset + sprite->height > bottom)
													bottom = frameSprite->yoffset + sprite->height;

												//Frame bounding box
												if (frameSprite->xoffset + sprite->width > frameRight)
													frameRight = frameSprite->xoffset + sprite->width;

												if (frameSprite->yoffset + sprite->height > frameBottom)
													frameBottom = frameSprite->yoffset + sprite->height;

											}
											frameSprites.push_back(frameSprite);
										}
									}
								}

								line = lines[++i];
								if (ani->m_singleDir)
									break;
							}

							frame->boundingBox = QRectF(frameLeft, frameTop, frameRight - frameLeft, frameBottom - frameTop);

							for (; i < lines.size(); ++i)
							{
								line = lines[i];
								if (line == "")
									break;

								if (line == "ANIEND")
								{
									--i;
									break;
								}
								QStringList parts = line.split(" ", Qt::SplitBehaviorFlags::SkipEmptyParts);
								auto wordCount = parts.size();

								if (wordCount == 0)
									continue;

								QString commandName = parts[0];

								if (commandName == "PLAYSOUND" && wordCount >= 4)
								{
									auto& soundFile = parts[1];

									auto frameSound = new Ani::Frame::FramePieceSound();
									frameSound->type = Ani::Frame::PIECE_SOUND;
									frameSound->id = QRandomGenerator::global()->generate64();
									
									frameSound->xoffset = parts[2].toDouble() * 16;
									frameSound->yoffset = parts[3].toDouble() * 16;

									frameSound->setSoundFile(soundFile, resourceManager);
									frame->sounds.push_back(frameSound);
									
								}
								else if (commandName == "WAIT" && wordCount >= 2)
								{
									auto msecs = parts[1].toInt() * 50.0;
									frame->duration += msecs;
								}


							}

							ani->addFrame(frame);
						}
						else break;
					}
				}
				else if (word1 == "SCRIPT")
				{
					QString script;
					bool firstLine = true;
					for (++i; i < lines.size(); ++i)
					{
						line = lines[i];

						if (line != "SCRIPTEND")
						{
							if (!firstLine)
								script += "\n";
							firstLine = false;
							script += line;
						}
						else break;
					}
					ani->setScript(script);
				}
				else if(i > 0) {
					ani->m_unparsedLines.push_back(line);
				}
			}

		}

		ani->m_boundingBox = QRect(left, top, right - left, bottom - top);

		ani->setLoaded(true);
		return true;
	}

	bool Ani::saveGraalAni(Ani* ani, QIODevice* _stream)
	{
		QTextStream stream(_stream);

		stream << "GANI0001" << Qt::endl;

		QStringList otherCommands;
		auto& sprites = ani->getSprites();

		for (auto& sprite : sprites)
		{
			stream << "SPRITE " << QString("%1").arg(QString::number(sprite->index), 4, ' ') << " ";

			if (sprite->type == "CUSTOM")
			{
				stream << QString("%1").arg(sprite->customImageName, 15, ' ');
			}
			else stream << QString("%1").arg(sprite->type, 15, ' ');

			stream << " " << QString("%1").arg(QString::number(sprite->left), 4, ' ') << " " << QString("%1").arg(QString::number(sprite->top), 4, ' ') << " " << QString("%1").arg(QString::number(sprite->width), 4, ' ') << " " << QString("%1").arg(QString::number(sprite->height), 4, ' ') << " " << sprite->comment << Qt::endl;

			int attachIndex = 0;
			for (auto& attachedSprite : sprite->attachedSprites)
			{
				QString line = QString("%1 %2 %3 %4 %5").arg(attachIndex < sprite->m_drawIndex ? "ATTACHSPRITE2" : "ATTACHSPRITE").arg(QString::number(sprite->index), 4).arg(QString::number(attachedSprite.first), 4).arg(QString::number(int(attachedSprite.second.x())), 4).arg(QString::number(int(attachedSprite.second.y())), 4);
				otherCommands.push_back(line);

				++attachIndex;
			}

			if (sprite->xscale != 1.0)
			{
				QString line = QString("STRETCHXEFFECT %1 %2").arg(QString::number(sprite->index), 4).arg(QString::number(sprite->xscale), 4);
				otherCommands.push_back(line);
			}

			if (sprite->yscale != 1.0)
			{
				QString line = QString("STRETCHYEFFECT %1 %2").arg(QString::number(sprite->index), 4).arg(QString::number(sprite->yscale), 4);
				otherCommands.push_back(line);
			}

			if (sprite->rotation != 0.0)
			{
				QString line = QString("ROTATEEFFECT %1 %2").arg(QString::number(sprite->index), 4).arg(QString::number(sprite->rotation * M_PI / 180), 4);
				otherCommands.push_back(line);
				
			}

			if (sprite->colorEffectEnabled)
			{
				QString line = QString("COLOREFFECT %1 %2 %3 %4 %5").arg(sprite->index).arg(QString::number(sprite->colorEffect.red() / 255.0, 103, 3)).arg(QString::number(sprite->colorEffect.green() / 255.0, 103, 3)).arg(QString::number(sprite->colorEffect.blue() / 255.0, 103, 3)).arg(QString::number(sprite->colorEffect.alpha () / 255.0, 103, 3));
				otherCommands.push_back(line);
			}
		}


		if (otherCommands.size())
		{
			for (auto& line : otherCommands)
				stream << line << Qt::endl;
			stream << Qt::endl;
		}
		
		if (ani->isLooped())
			stream << "LOOP" << Qt::endl;

		if (ani->isContinous())
			stream << "CONTINUOUS" << Qt::endl;

		if (!ani->getNextAni().isEmpty())
			stream << "SETBACKTO " << ani->getNextAni() << Qt::endl;

		if (ani->isSingleDir())
			stream << "SINGLEDIRECTION" << Qt::endl;

		for (auto& line : ani->m_unparsedLines)
			stream << line << Qt::endl;


		auto& defaults = ani->m_defaultImages;

		bool hasDefaults = false;
		for (auto it = defaults.begin(); it != defaults.end(); ++it)
		{
			auto type = it.key();

			if (type == "SPRITES" || type == "PICS")
				continue;

			auto fileName = it.value().first;

			if (!fileName.isEmpty())
			{
				stream << "DEFAULT" << type << " " << fileName << Qt::endl;
				hasDefaults = true;
			}
		}

		if (hasDefaults)
			stream << Qt::endl;

		auto& frames = ani->getFrames();

		if (frames.size() > 0)
		{
			stream << "ANI" << Qt::endl;
			int dirCount = ani->isSingleDir() ? 1 : 4;

			for (auto& frame : frames)
			{
				for (int dir = 0; dir < dirCount; ++dir)
				{
					auto& parts = frame->pieces[dir];

					QStringList partsList;

					for (auto& part : parts)
					{
						if (part->type == Ani::Frame::PIECE_SPRITE)
						{
							auto spritePart = static_cast<Ani::Frame::FramePieceSprite*>(part);

							if(spritePart->spriteIndex != SPRITE_INDEX_STRING)
								partsList.push_back(QString("%1 %2 %3").arg(QString::number(spritePart->spriteIndex), 4).arg(QString::number(int(spritePart->xoffset)), 3).arg(QString::number(int(spritePart->yoffset)), 3));
							else partsList.push_back(QString("%1 %2 %3").arg(spritePart->spriteName).arg(QString::number(int(spritePart->xoffset)), 3).arg(QString::number(int(spritePart->yoffset)), 3));
						}
					}

					auto line = partsList.join(", ");
					stream << line << Qt::endl;
				}

				if (frame->duration > 50)
					stream << "WAIT " << (frame->duration / 50) - 1 << Qt::endl;

				for (auto piece : frame->sounds)
				{
					if (piece->type == Ani::Frame::PIECE_SOUND)
					{
						auto soundPiece = static_cast<Ani::Frame::FramePieceSound*>(piece);
						stream << "PLAYSOUND " << soundPiece->fileName << " " << QString::number(soundPiece->xoffset / 16.0, 'g', 4) << " " << QString::number(soundPiece->yoffset / 16.0, 'g', 4) << Qt::endl;
					}
					
				}
				stream << Qt::endl;
			}
			stream << "ANIEND" << Qt::endl;

			auto& script = ani->getScript();

			if (!script.isEmpty())
			{
				stream << Qt::endl;
				stream << "SCRIPT" << Qt::endl;
				stream << script;

				if (!script.endsWith("\n\n"))
					stream << Qt::endl;
				stream << "SCRIPTEND" << Qt::endl;
			}
		}
		return true;
	}

	Ani::Frame::FramePiece* Ani::Frame::FramePieceSprite::duplicate(AbstractResourceManager* resourceManager) {
		auto retval = new FramePieceSprite();
		retval->type = Ani::Frame::PIECE_SPRITE;
		retval->id = QRandomGenerator::global()->generate64();
		retval->xoffset = this->xoffset;
		retval->yoffset = this->yoffset;
		retval->spriteIndex = this->spriteIndex;
		retval->spriteName = this->spriteName;
		return retval;

	}

	Ani::Frame::FramePiece* Ani::Frame::FramePieceSound::duplicate(AbstractResourceManager* resourceManager)
	{
		auto retval = new FramePieceSound();
		retval->type = Ani::Frame::PIECE_SOUND;
		retval->id = QRandomGenerator::global()->generate64();
		retval->xoffset = this->xoffset;
		retval->yoffset = this->yoffset;
		retval->setSoundFile(this->fileName, resourceManager);
		return retval;
	}
};