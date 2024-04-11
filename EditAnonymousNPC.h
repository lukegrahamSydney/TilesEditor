#pragma once

#include <QDialog>
#include <QByteArray>
#include "ui_EditAnonymousNPC.h"
#include "LevelNPC.h"
#include "IWorld.h"

namespace TilesEditor
{
	class EditAnonymousNPC : public QDialog
	{
		Q_OBJECT
	public slots:
		void editorLineChanged(QTextBlock block);
		//void dialogAccepted();
		//void dialogRejected();

		void accept() override;
		void reject() override;
		void imageBrowsePressed();
		void positionChanged();
		void deletePressed();
		void convertPressed();
		void revertPressed();
		void textEdited(const QString& text);

	private:


		LevelNPC* m_npc;
		IWorld* m_world;
		bool	m_modified;
		

	public:
		static QByteArray savedGeometry;

		EditAnonymousNPC(LevelNPC* npc, IWorld* world, QWidget* parent = nullptr);
		~EditAnonymousNPC();

		bool getModified() const { return m_modified; }

	

	private:
		Ui::EditAnonymousNPCClass ui;
	};
};