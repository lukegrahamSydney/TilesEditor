#pragma once

#include <QDialog>
#include <QByteArray>
#include "ui_EditAnonymousNPC.h"
#include "LevelNPC.h"
#include "ResourceManager.h"

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
		void textEdited(const QString& text);

	private:


		LevelNPC* m_npc;

		bool	m_modified;
		ResourceManager& m_resourceManager;

	public:
		static QByteArray savedGeometry;

		EditAnonymousNPC(LevelNPC* npc, ResourceManager& resourceManager, QWidget* parent = nullptr);
		~EditAnonymousNPC();

		bool getModified() const { return m_modified; }

	private:
		Ui::EditAnonymousNPCClass ui;
	};
};