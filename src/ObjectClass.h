#ifndef OBJECTCLASSH
#define OBJECTCLASSH

#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QVector>
#include <QByteArray>
#include <QSet>
#include "RefCounter.h"
#include "IFileRequester.h"
#include "RenderMode.h"
#include "ObjectClassParam.h"
#include "LoadState.h"
#include "AbstractResourceManager.h"
#include "ScriptingLanguage.h"
#include "IObjectClassInstance.h"

namespace TilesEditor
{
	class ObjectClass:
		public QObject,
		public QTreeWidgetItem,
		public RefCounter,
		public IFileRequester
	{
		Q_OBJECT
	public slots:
		void loadFileData(QByteArray fileData);
		void loadFileDataFail();


	private:
		LoadState m_loadState = LoadState::STATE_NOT_LOADED;
		LoadState m_imageLoadState = LoadState::STATE_NOT_LOADED;

		bool m_imageLoaded = false;
		QString m_fullPath;
		QString m_className;
		QString m_imageName;
		QPixmap m_image;

		QString m_clientCode = "";
		QString m_serverCode = "";

		QVector<ObjectClassParam*> m_params;
		QSet<IObjectClassInstance*> m_instances;
		bool m_useImageShape = false;
		int m_imageShape[4] = {};
		bool m_canResize = false;
		bool m_removed = false;
		bool m_embedCode = true;
		RenderMode m_renderMode = RenderMode::Centered;
		QColor m_shapeColour = QColorConstants::Blue;
		ScriptingLanguage m_scriptingLanguage = ScriptingLanguage::SCRIPT_SGSCRIPT;

	public:

		ObjectClass(const QString& className, const QString& fullPath);
		~ObjectClass();

		void addInstance(IObjectClassInstance* instance) { m_instances.insert(instance); }
		void removeInstance(IObjectClassInstance* instance) { m_instances.remove(instance); }
		void markInstancesModified();

		const QString& getFullPath() const { return m_fullPath; }
		void setRemoved(bool value) { m_removed = value; }
		bool removed() const { return m_removed; }
		bool load(AbstractResourceManager* resourceManager, bool threaded);
		bool load(AbstractResourceManager* resourceManager, QIODevice* stream);
		LoadState getLoadState() const { return m_loadState; }
		void setLoadState(LoadState state) { m_loadState = state; }

		bool imageLoaded() const { return m_imageLoaded; }
		bool loadImage(AbstractResourceManager* resourceManager);
		bool canResize() const { return m_canResize; }
		QString getName() const { return text(0); }
		QVector<ObjectClassParam*>& getParams() { return m_params; }
		const QString& getImageName() const { return m_imageName; };
		bool getImageShape(int* left, int* top, int* width, int* height) const;
		QPixmap getImage() { return m_image; }

		const QString& getClientCode() const { return m_clientCode; }
		const QString& getServerCode() const { return m_serverCode; }

		ScriptingLanguage getScriptingLanguage() const { return m_scriptingLanguage; }
		bool shouldEmbedCode() const { return m_embedCode; }
		RenderMode getRenderMode() const { return m_renderMode; }
		void setRenderMode(RenderMode mode) { m_renderMode = mode; }
		QColor getShapeColour() const { return m_shapeColour; }

		//Filename should be name part only. not FULL PATH
		void fileFailed(const QString& name, AbstractResourceManager* resourceManager) override {};
		void fileReady(const QString& fileName, AbstractResourceManager* resourceManager) override  {}
		void fileWritten(const QString& fileName, AbstractResourceManager* resourceManager) override  {};

	};
};

#endif