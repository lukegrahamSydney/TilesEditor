#ifndef LEVELNPCH
#define LEVELNPCH

#include <QString>
#include <QRegularExpression>
#include <QColor>
#include <QPixmap>
#include <gs1/parse/Parser.hpp>
#include <gs1/parse/SyntaxTreeVisitor.hpp>
#include "sgscript/sgs_int.h"
#include "AbstractLevelEntity.h"
#include "LevelEntityType.h"
#include "Image.h"
#include "IFileRequester.h"
#include "RenderMode.h"
#include "IEngine.h"
#include "AniEditor/AniInstance.h"

namespace TilesEditor
{
	

	class LevelNPC:
		public AbstractLevelEntity,
		public IFileRequester
	{
	public:
		static QRegularExpression RegExImgPart;
		static QRegularExpression RegExSetImg;
		static QRegularExpression RegShouldUseGS1Parser;
		static QRegularExpression RegIsSGScript;

	private:

		bool m_loadImageFail;

		QString m_imageName;
		QString m_code;
		Image* m_image;
		QPixmap m_cachedColorMod;


		bool m_hasResized = false;
		bool m_drawAsLight = false;
		QColor m_drawColour = QColorConstants::White;

		RenderMode m_renderMode = RenderMode::Centered;
		QColor m_shapeColour = QColorConstants::Blue;
		ScriptingLanguage m_scriptingLanguage = ScriptingLanguage::SCRIPT_UNDEFINED;
		AniInstance* m_aniInstance = nullptr;
		int m_dir = 2;

	private:
		void calculateDimensions();

	protected:
		bool m_isObjectInstance = false;
		bool m_useImageShape;
		int m_imageShape[4];

	public:

		LevelNPC(IWorld* world, double x, double y, int width, int height);
		LevelNPC(IWorld* world, cJSON* json);
		virtual ~LevelNPC();
	
		bool isObjectInstance() const { return m_isObjectInstance; }
		virtual QString getClassName() const { return ""; }
		LevelEntityType getEntityType() const override { return LevelEntityType::ENTITY_NPC; }

		void setAniName(const QString& ani, int frame);
		AniInstance* getAniInstance();
		void setDir(int dir);
		void setScriptingLanguage(ScriptingLanguage language) { m_scriptingLanguage = language; }
		ScriptingLanguage getScriptingLanguage() const { return m_scriptingLanguage; }
		void setImageShape(int left, int top, int width, int height);
		void loadResources() override;
		void releaseResources() override;
		void draw(QPainter* painter, const QRectF& viewRect, double x, double y) override;

		void setImageName(const QString& name);
		const QString& getImageName() const { return m_imageName; }
		void setCode(const QString& code);
		void setCodeRaw(const QString& code);

		void setProperty(const QString& name, const QVariant& value) override;
		void setColourEffect(double r, double g, double b, double a);
		const QString& getCode() const { return m_code; }

		void showCharacter();
		void resetCharacter();
		QRectF getBoundingBox() const override;

		bool hasValidImage() const {
			return m_imageName != "" && !m_loadImageFail;
		}

		bool canAddToLevel(Level* level) override;

		ScriptingLanguage detectScriptingLanguage(const QString& code);

		RenderMode getRenderMode() const { return m_renderMode; }
		void setRenderMode(RenderMode mode) { m_renderMode = mode; }

		void openEditor() override;

		bool hasResized() const { return m_hasResized; }
		void setHasResized(bool value) { m_hasResized = value; }

		
		virtual void resetSize() {}
		QString toString() const override { return QString("[Npc: %1, %2, %3]").arg(getImageName()).arg(getX()).arg(getY()); }
		QPixmap getIcon() override;

		AbstractLevelEntity* duplicate() override;

		cJSON* serializeJSON(bool useLocalCoordinates = false) override;
		void deserializeJSON(cJSON* json) override;

		void fileFailed(const QString& name, AbstractResourceManager* resourceManager) override;
		void fileReady(const QString& fileName, AbstractResourceManager* resourceManager) override;
		void fileWritten(const QString& fileName, AbstractResourceManager* resourceManager) override;
		
		static Image* getBlankNPCImage();

	};


	class LevelNPCGS1Parser :
		public gs1::SyntaxTreeVisitor
	{
	private:
		LevelNPC* m_npc = nullptr;
		int m_blockLevel = 0;

	public:
		LevelNPCGS1Parser(const QString& code, LevelNPC* npc);

	protected:

		void Visit(struct gs1::StmtIf* node) override;
		void Visit(struct gs1::StmtBlock* node) override;
		void Visit(struct gs1::StmtCommand* node) override;
		void Visit(struct gs1::ExprBinaryOp* node) override;

		QVariantList getArguments(const std::vector<gs1::Expr*>& args);

	};

	class LevelNPCSGScriptParser :
		public gs1::SyntaxTreeVisitor
	{
	private:
		LevelNPC* m_npc = nullptr;

	public:
		LevelNPCSGScriptParser(const QString& code, LevelNPC* npc);

	private:
		void parseCallNode(sgs_FTNode* node);
		void parseAssignmentNode(sgs_FTNode* node);
		void parseThisAssignment(const QString& memberName, const QVariant& value);
		void parseThisIndexAssignment(const QString& memberName, const QVariant& key, const QVariant& value);
		QVariantList getArguments(sgs_FTNode* expressionList);
		QVariant parseTokenValue(sgs_FTNode* node);
	};
};

#endif

