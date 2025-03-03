#pragma once

#include <QDialog>
#include <QSyntaxStyle.hpp>
#include <QCXXHighlighter.hpp>

#include "ui_EditScriptForm.h"
namespace TilesEditor
{
	class EditScriptForm : public QDialog
	{
		Q_OBJECT

	public slots:
		void accept() override;
		void reject() override;

	public:
		EditScriptForm(const QString& code, QWidget* parent = nullptr);
		~EditScriptForm();

		const QString& getScript() const;
		void showTestControls(bool show);
		static QByteArray savedGeometry;

	private:
		Ui::EditScriptFormClass ui;
		QString m_text;

		QCXXHighlighter m_cppHighlighterClient;
		QSyntaxStyle m_editorStyle;

	};
};
