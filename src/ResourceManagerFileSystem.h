#ifndef RESOURCEMANAGERFILESYSTEMH
#define RESOURCEMANAGERFILESYSTEMH

#include <QString>
#include <QSet>
#include <QStringList>
#include <QMap>
#include <QFileInfo>
#include <QFileDialog>
#include "AbstractResourceManager.h"

namespace TilesEditor
{
	class ObjectManager;
	class ResourceManagerFileSystem :
		public AbstractResourceManager
	{
	private:
		QString			m_rootDir;
		QSet<QString>	m_searchDirectories;
		QStringList		m_searchDirectoriesList;

		QMap<QString, QString>	m_fileNameCache;

		void searchDirectory(const QString& searchPath, int level, int maxLevel);

	protected:
		void requestFile(const QString& fileName) override {
			requestFileFailed(fileName);
		}

	public:
		ResourceManagerFileSystem(const QString& rootDir, ObjectManager* objectManager);

		void setRootDir(const QString& dir);
		void addSearchDir(const QString& dir);
		void addSearchDirRecursive(const QString& dir, int max);

		const QString& getType() const override {
			static QString type = "FileSystem";
			return type;
		}

		QString getOpenFileName(const QString& caption, const QString& dir, const QString& filter) override
		{
			return QFileDialog::getOpenFileName(nullptr, caption, dir, filter);
		}

		QStringList getOpenFileNames(const QString& caption, const QString& dir, const QString& filter) override
		{
			return QFileDialog::getOpenFileNames(nullptr, caption, dir, filter);
		}

		QString getSaveFileName(const QString& caption, const QString& dir, const QString& filter) override
		{
			return QFileDialog::getSaveFileName(nullptr, caption, dir, filter);
		}

		bool isCompatible(const QString& connectionString) override;
		QString getConnectionString() const override { return m_rootDir; }
		void mergeResourceManager(AbstractResourceManager* other) override;
		bool locateFile(const QString& name, QString* outPath = nullptr) override;
		QIODevice* openStream(const QString& fileName, QIODeviceBase::OpenModeFlag mode, QString* fullpath = nullptr) override;
		QIODevice* openStreamFullPath(const QString& fullPath, QIODeviceBase::OpenModeFlag mode) override;

		//Request file does nothing
		void requestFile(IFileRequester* listener, const QString& fileName) override {}

		void endWrite(IFileRequester* requester, const QString& fileName, QIODevice* stream) override
		{
			delete stream;
		}
	};
};
#endif