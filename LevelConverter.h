#ifndef LEVELCONVERTERH
#define LEVELCONVERTERH

/*
Experimental...seems to work though
*/
#include <QDialog>
#include <QStringList>
#include <QSet>
#include "ui_LevelConverter.h"
#include "EditorTabWidget.h"
#include "MainFileSystem.h"

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
		LevelConverter(QWidget* parent = nullptr);
		~LevelConverter();

	protected:

		void accept() override;

	private:
		Ui::LevelConverterClass ui;
		QStringList m_files;
		QSet<QString> m_levelNames;
		MainFileSystem m_fileSystem;
		EditorTabWidget* m_dummyTab;
		
		void processDirectory(const QString& dir);
	};
};

#endif
