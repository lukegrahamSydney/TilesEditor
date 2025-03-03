#ifndef EDITANONYMOUSNPCH
#define EDITANONYMOUSNPCH


#include <QActionGroup>
#include <QMenu>
#include <QDialog>
#include <QByteArray>
#include <QSyntaxStyle.hpp>
#include <QCXXHighlighter.hpp>
#include "ui_EditAnonymousNPC.h"
#include "LevelNPC.h"
#include "IWorld.h"
#include "IEngine.h"

namespace TilesEditor
{
	class EditAnonymousNPC : public QDialog
	{
		Q_OBJECT
	public slots:
		//void dialogAccepted();
		//void dialogRejected();
		void accept() override;
		void reject() override;
		void imageBrowseClicked(bool checked);
		void testPressed();

		void positionChanged();
		void deletePressed();
		void convertPressed();
		void revertPressed();
		void textEdited(const QString& text);
		void textChanged();

	private:

		QCXXHighlighter m_cppHighlighterClient;
		QSyntaxStyle m_editorStyle;
		LevelNPC* m_npc;
		IWorld* m_world;
		bool	m_modified;
		QMenu	m_testScriptTypes;
		QActionGroup m_testScriptTypesGroup;

	public:
		static QByteArray savedGeometry;
		EditAnonymousNPC(LevelNPC* npc, IWorld* world, QWidget* parent = nullptr);
		~EditAnonymousNPC();

		bool getModified() const { return m_modified; }

		

	private:
		Ui::EditAnonymousNPCClass ui;

		ScriptingLanguage getSelectedLanguage();
	};
};

#endif