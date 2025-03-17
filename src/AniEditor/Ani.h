#ifndef ANIH
#define ANIH

#include <QRandomGenerator>
#include <QString>
#include <QList>
#include <QMap>
#include <QVector>
#include <QIODevice>
#include <QPainter>
#include <QSoundEffect>
#include "cJSON/JsonHelper.h"
#include "Image.h"
#include "AbstractResourceManager.h"
#include "Resource.h"
#include "ResourceType.h"
#include "IAniInstance.h"

namespace TilesEditor
{
	class Ani:
		public Resource,
		public IAniInstance
	{
		friend class AniInstance;

	public:
		inline static const int SPRITE_INDEX_STRING = -21374783;
		class AniSprite
		{
		public:
			
			QString type;
			QPolygonF boundingBox;
			int counter = 0;
			int index;
			int left;
			int top;
			int width;
			int height;
			bool colorEffectEnabled = false;
			QColor colorEffect = QColorConstants::White;

			double rotation = 0.0;
			double xscale = 1.0;
			double yscale = 1.0;
			
			QString comment;
			QString customImageName;
			QPointF dragOffset;

			//Our draw index relative to the attachments
			//For example if the draw index is 1, than attachments with an index < 1 will
			//be draw first, then ourself, then other
			int m_drawIndex = 0;
			QVector<QPair<int, QPointF>> attachedSprites;

			AniSprite* duplicate(int newIndex);
			void updateBoundingBox();
			Image* getCustomImage(AbstractResourceManager* resouceManager);
			void releaseCustomImage(AbstractResourceManager* resouceManager);

		private:
			Image* customImage = nullptr;
			bool customImageFail = false;
		};

		struct Frame
		{
			enum FramePieceType {
				PIECE_SPRITE,
				PIECE_SOUND
			};

			struct FramePiece {
				FramePiece() {id = QRandomGenerator::global()->generate64();}
				quint64 id;
				qsizetype index = 0;
				FramePieceType type;
				double xoffset;
				double yoffset;
				QPointF dragOffset;

				virtual QString toString(Ani* ani) = 0;
				virtual void getSize(Ani* ani, int* width, int* height) = 0;
				virtual QPolygonF getBoundingBox(Ani* ani) = 0;
				virtual cJSON* serialize() = 0;
				virtual void deserialize(cJSON* json, AbstractResourceManager* resourceManager) = 0;
				virtual FramePiece* duplicate(AbstractResourceManager* resourceManager) = 0;
			};

			struct FramePieceSprite:
				FramePiece
			{
				FramePieceSprite() :FramePiece() { type = Ani::Frame::PIECE_SPRITE; }
				int spriteIndex;
				QString spriteName;

				QString toString(Ani* ani) override
				{
					auto sprite = ani->getAniSprite(spriteIndex, spriteName);
					if (sprite)
						return sprite->comment;
					return "unknown";
				}

				void getSize(Ani* ani, int* width, int* height) override
				{
					auto sprite = ani->getAniSprite(spriteIndex, spriteName);
					if (sprite)
					{
						*width = sprite->width;
						*height = sprite->height;
					}
					else {
						*width = *height = 32;
					}
				}

				QPolygonF getBoundingBox(Ani* ani) override
				{
					auto sprite = ani->getAniSprite(spriteIndex, spriteName);
					if (sprite)
					{
						return sprite->boundingBox;
					}
					else {
						return QRectF(0, 0, 16, 16);
					}
				}
				cJSON* serialize() override {
					auto obj = cJSON_CreateObject();
					cJSON_AddStringToObject(obj, "type", "sprite");
					cJSON_AddNumberToObject(obj, "x", this->xoffset);
					cJSON_AddNumberToObject(obj, "y", this->yoffset);
					cJSON_AddNumberToObject(obj, "spriteIndex", this->spriteIndex);
					cJSON_AddStringToObject(obj, "spriteName", this->spriteName.toLocal8Bit().data());
					return obj;
				}

				void deserialize(cJSON* json, AbstractResourceManager* resourceManager) override
				{
					this->xoffset = jsonGetChildDouble(json, "x");
					this->yoffset = jsonGetChildDouble(json, "y");
					this->spriteIndex = jsonGetChildInt(json, "spriteIndex");
					this->spriteName = jsonGetChildString(json, "spriteName");
				}
				FramePiece* duplicate(AbstractResourceManager* resourceManager) override;
			};

			struct FramePieceSound :
				FramePiece
			{
				QString fileName;
				QString fullPath;
				QSoundEffect soundEffect;

				FramePieceSound():FramePiece() { type = Ani::Frame::PIECE_SOUND; }

				QString toString(Ani* ani) override { return QString("Sound: %1").arg(fileName); }

				void setSoundFile(const QString& fileName, AbstractResourceManager* resourceManager)
				{
					this->fileName = fileName;

					QString fullPath;
					auto pos = this->fileName.indexOf('.');
					if (pos == -1) {
						resourceManager->locateFile(this->fileName + ".wav", &fullPath);
					}
					else resourceManager->locateFile(this->fileName, &fullPath);

					this->fullPath = fullPath;

				}

				void getSize(Ani* ani, int* width, int* height) override
				{
					*width = *height = 16;
				}

				QPolygonF getBoundingBox(Ani* ani) override
				{
					return QRectF(0, 0, 16, 16);
				}

				cJSON* serialize() override {
					auto obj = cJSON_CreateObject();
					cJSON_AddStringToObject(obj, "type", "sound");
					cJSON_AddNumberToObject(obj, "x", this->xoffset);
					cJSON_AddNumberToObject(obj, "y", this->yoffset);
					cJSON_AddStringToObject(obj, "sound", this->fileName.toLocal8Bit().data());
					return obj;
				}

				void deserialize(cJSON* json, AbstractResourceManager* resourceManager) override
				{
					this->xoffset = jsonGetChildDouble(json, "x");
					this->yoffset = jsonGetChildDouble(json, "y");

					setSoundFile(jsonGetChildString(json, "sound"), resourceManager);
				}

				FramePiece* duplicate(AbstractResourceManager* resourceManager) override;
			};

			Frame() { id = QRandomGenerator::global()->generate64(); }

			~Frame() {
				for (auto& a : pieces)
				{
					for (auto& piece : a)
						delete piece;
				}

				for (auto& piece : sounds)
					delete piece;
			}
			quint64 id;
			int duration = 50;
			QVector<FramePiece*> pieces[4];
			QVector<FramePiece*> sounds;
			QRectF boundingBox;
			//QList<IAniCommand*> commands;
		};

	private:
		QString m_script;
		QString m_fullPath;
		QString m_fileName;
		QString m_nextAni;
		QStringList m_unparsedLines;

		bool m_looped = true;
		bool m_singleDir = false;
		bool m_continous = false;
		double m_speed = 1.0;
		QMap<int, AniSprite*> m_sprites;
		QMap<QString, QPair<QString, Image*>> m_defaultImages;

		QMap<QString, Image*> m_hiddenDefaults;

		QVector<Frame*> m_frames;
		bool m_containsBodySprite;
		int m_nextSpriteIndex = 0;
		QRect m_boundingBox;


		void addFrame(Frame* aniFrame);

		static bool isCustomImage(const QString& name);

	public:
		Ani(const QString& name, AbstractResourceManager* resourceManager);
		~Ani();

		bool isLooped() const { return m_looped; }
		void setLooped(bool value) { m_looped = value; }
		bool isSingleDir() const { return m_singleDir; }
		void setSingleDir(bool value) { m_singleDir = value; }
		QVector<Frame*>& getFrames() { return m_frames; }
		bool isContinous() const { return m_continous; }
		void setContinous(bool value) { m_continous = value; }
		bool addSprite(AniSprite* aniSprite);
		void setFileName(const QString& fileName) { m_fileName = fileName; }
		const QString& getFileName() const { return m_fileName; }
		void setFullPath(const QString& fullPath) { m_fullPath = fullPath; }
		const QString& getFullPath() const { return m_fullPath; }
		void freeResources(AbstractResourceManager* resourceManager);
		const QRect& getBoundingBox() const;
		bool spriteExists(int index) const { return m_sprites.constFind(index) != m_sprites.end(); }
		Frame* getFrame(size_t index);
		AniSprite* getAniSprite(int index, const QString& name);
		AniSprite* getAniSprite(IAniInstance* propertyProvider, int index, const QString& name);
		Image* getDefaultImage(const QString& name);
		QString getDefaultImageName(const QString& name);
		QMap<int, AniSprite*>& getSprites() { return m_sprites; }

		int getTotalDuration() const;
		QString getNextAni() const { return m_nextAni; }
		void setNextAni(const QString& nextAni) { m_nextAni = nextAni; }
		const QString& getScript() const { return m_script; }
		void setScript(const QString& script) { m_script = script; }
		void insertFrame(qsizetype pos, Ani::Frame* frame);

		void removeFrame(size_t index);
		int getNextSpriteIndex(bool increment) { return increment ? m_nextSpriteIndex++ : m_nextSpriteIndex; }
		void setDefaultImage(const QString& name, const QString& value, AbstractResourceManager* resourceManager);
		Image* getHiddenDefaultImage(const QString& type, AbstractResourceManager* resourceManager);

		int getFrameCount() const { return m_frames.size(); }

		void draw(int frameIndex, int dir, double x, double y, AbstractResourceManager* resourceManager, QPainter* painter, const QRectF& rect);
		void drawSprite(Ani::AniSprite* sprite, double x, double y, AbstractResourceManager* resourceManager, QPainter* painter, const QRectF& rect, int level = 0);

		ResourceType getResourceType() const override { return RESOURCE_ANI; }
		void replace(QIODevice* stream, AbstractResourceManager* resourceManager) override;

		QString getPropertyValue(const QString& propName) override { return getDefaultImageName(propName); }
		static Ani* loadGraalAni(const QString& name, QIODevice* stream, AbstractResourceManager* resourceManager);
		static bool loadGraalAni(Ani* ani, QIODevice* stream, AbstractResourceManager* resourceManager);
		static bool saveGraalAni(Ani* ani, QIODevice* stream);
	};
};
#endif