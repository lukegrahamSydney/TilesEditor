#pragma once

#include <QDialog>
#include "ui_EditLinkDialog.h"
#include "LevelLink.h"

namespace TilesEditor
{
	class EditLinkDialog : public QDialog
	{
		Q_OBJECT

	public slots:
		void accept() override;
		void reject() override;
		void deletePressed();
		void textEdited(const QString& text);

	private:
		LevelLink* m_link;
		bool m_modified;

	public:
		EditLinkDialog(LevelLink* link, QWidget* parent = nullptr);
		~EditLinkDialog();

		bool getModified() const { return m_modified; }

	private:
		Ui::EditLinkDialogClass ui;
	};
};
