#ifndef LEVELCONVERTERH
#define LEVELCONVERTERH

/*
Experimental...seems to work though
*/
#include <QDialog>
#include <QStringList>
#include <QSet>
#include <QAbstractItemModel>
#include "ui_LevelConverter.h"
#include "EditorTabWidget.h"
#include "ObjectManager.h"
#include "IEngine.h"

namespace TilesEditor
{
	class LevelConverter : public QDialog
	{
		Q_OBJECT

	private slots:
		void inputBrowseClicked(bool checked);
		void outputBrowseClicked(bool checked);
		void timer();

	public:
		LevelConverter(IEngine* engine, QStandardItemModel* tilesetsModel, QWidget* parent = nullptr);
		~LevelConverter();

	protected:

		void accept() override;

	private:
		QStringList		m_filters;
		QStandardItemModel* m_tilesetsModel;
		Ui::LevelConverterClass ui;
		QStringList m_files;
		QSet<QString> m_levelNames;
		EditorTabWidget* m_dummyTab;
		
		void processDirectory(const QString& dir);
	};
};

#endif
