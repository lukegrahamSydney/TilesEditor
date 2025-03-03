#include <QFile>
#include <QSyntaxStyle.hpp>
#include <QCXXHighlighter.hpp>
#include <QGuiApplication>
#include <QStyleHints>
#include "EditScriptForm.h"


namespace TilesEditor
{
	QByteArray EditScriptForm::savedGeometry;
	EditScriptForm::EditScriptForm(const QString& code, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		m_text = code;

		QFile fl(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark ? ":/styles/drakula.xml" : ":/default_style.xml");

		if (fl.open(QIODevice::ReadOnly))
		{
			if (m_editorStyle.load(fl.readAll()))
			{
				ui.scriptEditor->setSyntaxStyle(&m_editorStyle);
			}
		}

		ui.scriptEditor->setHighlighter(&m_cppHighlighterClient);

		ui.scriptEditor->setPlainText(code);

		if (!savedGeometry.isNull())
			restoreGeometry(savedGeometry);

	}

	EditScriptForm::~EditScriptForm()
	{
		savedGeometry = saveGeometry();
	
	}

	void EditScriptForm::accept()
	{
		m_text = ui.scriptEditor->toPlainText();
		QDialog::accept();
	}

	void EditScriptForm::reject()
	{
		QDialog::reject();
	}

	const QString& EditScriptForm::getScript() const
	{
		return m_text;
	}

	void EditScriptForm::showTestControls(bool show)
	{
		ui.testContainer->setVisible(show);
	}
};
