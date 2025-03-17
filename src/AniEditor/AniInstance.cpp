#include "AniInstance.h"

namespace TilesEditor
{
	AniInstance::AniInstance()
	{
		m_bodyColours[Image::BODY_SLEEVE] = qRgb(255, 0, 0);
		m_bodyColours[Image::BODY_SHIRT] = qRgb(255, 255, 255);
		m_bodyColours[Image::BODY_SKIN] = qRgb(255, 173, 107);
		m_bodyColours[Image::BODY_BELT] = qRgb(0, 0, 255);
		m_bodyColours[Image::BODY_SHOES] = qRgb(206, 24, 41);
	}

	void AniInstance::freeResources(AbstractResourceManager* resourceManager)
	{
		if (m_ani)
		{
			resourceManager->freeResource(m_ani);
			m_ani = nullptr;
		}

		for (auto& pair : m_aniProperties)
		{
			if (pair.second)
				resourceManager->freeResource(pair.second);
		}
		m_aniProperties.clear();
	}

	QString AniInstance::getPropertyValue(const QString& propName)
	{
		auto it = m_aniProperties.find(propName);
		if (it != m_aniProperties.end())
			return it.value().first;
		return "";
	}

	QRectF AniInstance::getBoundingBox() const
	{
		if (m_ani && m_ani->isLoaded())
			return m_ani->getBoundingBox();
		return QRect(0, 0, 0, 0);
	}

	QRectF AniInstance::getFrameBoundingBox() const
	{
		if (m_ani && m_ani->isLoaded())
		{
			if (m_frame >= 0 && m_frame < m_ani->getFrameCount())
				return m_ani->getFrame(m_frame)->boundingBox;
		}

		return QRect(0, 0, 0, 0);
	}

	void AniInstance::setAniName(IFileRequester* requester, const QString& name, int frame, AbstractResourceManager* resourceManager)
	{
		if (m_name != name)
		{
			if (m_ani != nullptr)
			{
				resourceManager->freeResource(m_ani);
				m_ani = nullptr;
			}

			m_name = name;

			m_ani = static_cast<Ani*>(resourceManager->loadResource(requester, name, ResourceType::RESOURCE_ANI));


		}
		m_frame = frame;
	}

	Image* AniInstance::getPropertyImage(const QString& name)
	{
		auto it = m_aniProperties.find(name);
		if (it != m_aniProperties.end())
			return it.value().second;
		return nullptr;
	}

	void AniInstance::setProperty(const QString& name, const QString& value, AbstractResourceManager* resourceManager)
	{
		auto it = m_aniProperties.find(name);
		if (it != m_aniProperties.end())
		{
			auto& pair = it.value();

			if (pair.second) {
				resourceManager->freeResource(pair.second);
			}
			m_aniProperties.erase(it);
		}

		m_aniProperties[name] = QPair<QString, Image*>(value, static_cast<Image*>(resourceManager->loadResource(nullptr, value, RESOURCE_IMAGE)));
	}

	QPixmap AniInstance::getIcon()
	{
		auto it = m_aniProperties.find("HEAD");
		if (it != m_aniProperties.end())
		{
			auto image = it.value().second;

			if (image)
				return image->pixmap().copy(0, 64, 32, 32);
		}
		return QPixmap();
	}

	void AniInstance::applyBodyColours(Image* image)
	{
		for (int i = 0; i < 5; ++i)
		{
			char index = image->getBodyColourIndex(i);

			if (index >= 0)
			{
				image->image().setColor(index, m_bodyColours[i]);
			}
		}
	}

	void AniInstance::draw(int dir, double x, double y, AbstractResourceManager* resourceManager, QPainter* painter)
	{
		if (m_ani == nullptr)
			return;

		auto frame = m_ani->getFrame((size_t)m_frame);

		if (frame != nullptr)
		{
			if (m_ani->isSingleDir()) { dir = 0; }
			dir = dir < 0 ? 0 : dir;
			dir = dir > 3 ? 3 : dir;

			auto& pieces = frame->pieces[dir];

			for (auto& piece : pieces)
			{
				if (piece->type == Ani::Frame::PIECE_SPRITE)
				{ 
					auto spritePiece = static_cast<Ani::Frame::FramePieceSprite*>(piece);
					auto sprite = m_ani->getAniSprite(this, spritePiece->spriteIndex, spritePiece->spriteName);

					if (sprite && sprite->width * sprite->height > 0)
						drawSprite(sprite, x + piece->xoffset, y + piece->yoffset, resourceManager, painter, 0);
				}

			}

		}
	}

	void AniInstance::drawSprite(Ani::AniSprite* sprite, double x, double y, AbstractResourceManager* resourceManager, QPainter* painter, int level)
	{
		if (m_ani == nullptr)
			return;

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

					auto child = m_ani->getAniSprite(this, childIndex, "");
					if (child)
					{
						drawSprite(child, x + int(offsets.x()), y + int(offsets.y()), resourceManager, painter, level + 1);
					}
				}
			}

			Image* image = nullptr;
			bool drawBody = sprite->type == "BODY";


			if (sprite->type == "CUSTOM")
				image = sprite->getCustomImage(resourceManager);
			else {
				image = getPropertyImage(sprite->type);

			}
			

			if (image != nullptr)
			{
				auto opacity = painter->opacity();
				painter->save();
				painter->translate(qFloor(0.5 + x), qFloor(0.5 + y));

				if (!(sprite->rotation == 0.0 && sprite->xscale == 1.0 && sprite->yscale == 1.0))
				{
					painter->translate(sprite->width / 2, sprite->height / 2);
					painter->scale(sprite->xscale, sprite->yscale);
					painter->rotate(sprite->rotation);
					painter->translate(sprite->width / -2, sprite->height / -2);
				}

				if (sprite->colorEffectEnabled && !drawBody)
				{
					auto compMode = painter->compositionMode();
					if (sprite->colorEffect.alpha() < 255)
					{
						painter->setOpacity(opacity * sprite->colorEffect.alphaF());
						painter->setCompositionMode(QPainter::CompositionMode::CompositionMode_Plus);
					}

					if (drawBody)
					{
						//auto bodyImage = image->image();
						//applyBodyColours(image);

					}
					else painter->drawPixmap(0, 0, image->colorMod(sprite->colorEffect, QRect(sprite->left, sprite->top, sprite->width, sprite->height)));
					painter->setCompositionMode(compMode);

				}
				else {
					if (drawBody)
					{
						applyBodyColours(image);
						painter->drawImage(0, 0, image->image(), sprite->left, sprite->top, sprite->width, sprite->height);
					} else painter->drawPixmap(0, 0, image->pixmap(), sprite->left, sprite->top, sprite->width, sprite->height);
				}


				painter->restore();
				painter->setOpacity(opacity);
			}
			
			//Draw above attachments
			if (sprite->attachedSprites.size() > 0)
			{
				for (auto i = sprite->m_drawIndex; i < sprite->attachedSprites.size(); ++i)
				{
					auto& pair = sprite->attachedSprites[i];
					auto childIndex = pair.first;
					auto& offsets = pair.second;

					auto child = m_ani->getAniSprite(this, childIndex, "");
					if (child)
					{
						drawSprite(child, x + int(offsets.x()), y + int(offsets.y()), resourceManager, painter, level + 1);
					}
				}
			}
		}
	}
}
