#pragma once

#include <QDialog>
#include <QStringList>
#include <QGroupBox>
#include <QMenu>
#include <QComboBox>
#include <QSyntaxStyle.hpp>
#include <QCXXHighlighter.hpp>
#include "ui_EditorObject.h"
#include "sgscript/sgscript.h"
#include "RenderMode.h"
#include "AbstractResourceManager.h"
#include "IEngine.h"

namespace TilesEditor
{
	class EditorObject : public QDialog
	{
		Q_OBJECT

	public slots:
		void buttonBoxClicked(QAbstractButton* button);
		void accept() override;
		void reject() override;
		void browseImagePressed();
		void testPressed();
		void textChanged();
		void deletePropertyPressed();

		void actionNewSizeProperty();
		void actionNewImageShapeProperty();
		void actionNewWeaponNameProperty();
		void actionNewBaddyProperty();
		void actionNewRenderModeProperty();
		void actionNewParentClassProperty();
		void actionNewDontEmbedCodeProperty();
		void actionNewScriptingLanguageProperty();

	private:
		IEngine* m_engine;

		QCXXHighlighter m_cppHighlighterClient;
		QCXXHighlighter m_cppHighlighterServer;
		Ui::EditorObjectClass ui;
		QStringList m_lines;
		QString m_className, m_fileName;
		bool m_modified = false;
		bool m_changesSaved = false;
		QMenu m_newPropertyMenu;
		QSyntaxStyle m_editorStyle;
		AbstractResourceManager* m_resourceManager;
		ScriptingLanguage m_scriptingLanguage = ScriptingLanguage::SCRIPT_SGSCRIPT;

		void save();

	public:
		EditorObject(IEngine* engine, const QString& className, const QString& fileName, AbstractResourceManager* resourceManager, QWidget* parent = nullptr);
		~EditorObject();

		void setModified(bool value);
		void addNewPropertyRow(const QString& label, QWidget* widget, bool setModified = false);
		void removePropertyRow(QWidget* item);
		void setScriptingLanguage(ScriptingLanguage language) { m_scriptingLanguage = language; }
		bool changesSaved() const { return m_changesSaved; }
		static QByteArray savedGeometry;
	};

	class AbstractEditorObjectProperty : public QGroupBox
	{
	public:
		virtual void writeData(QTextStream& stream) = 0;
		virtual QString getName() = 0;

	};

	class EditorObjectShapeProperty :
		public AbstractEditorObjectProperty
	{
		Q_OBJECT
	public slots:
		void textEdited(const QString& text);

	private:
		EditorObject* m_mainForm;
		QLineEdit* m_widthEdit;
		QLineEdit* m_heightEdit;

	public:

		EditorObjectShapeProperty(EditorObject* mainForm, int width, int height);

		void writeData(QTextStream& stream) override;
		QString getName() override { return "Size"; }
	};

	class EditorObjectWeaponNameProperty :
		public AbstractEditorObjectProperty
	{
		Q_OBJECT
	public slots:
		void textEdited(const QString& text);

	private:
		EditorObject* m_mainForm;
		QLineEdit* m_weaponName;

	public:

		EditorObjectWeaponNameProperty(EditorObject* mainForm, const QString& weaponName);

		void writeData(QTextStream& stream) override;
		QString getName() override { return "Weapon Name"; }
	};

	class EditorObjectParentProperty :
		public AbstractEditorObjectProperty
	{
		Q_OBJECT
	public slots:
		void textEdited(const QString& text);

	private:
		EditorObject* m_mainForm;
		QLineEdit* m_parentName;

	public:

		EditorObjectParentProperty(EditorObject* mainForm, const QString& parentName);

		void writeData(QTextStream& stream) override;
		QString getName() override { return "Parent Class"; }
	};

	class EditorObjectImageShapeProperty :
		public AbstractEditorObjectProperty
	{
		Q_OBJECT
	public slots:
		void textEdited(const QString& text);

	private:
		EditorObject* m_mainForm;
		QLineEdit* m_left;
		QLineEdit* m_top;
		QLineEdit* m_widthEdit;
		QLineEdit* m_heightEdit;

	public:

		EditorObjectImageShapeProperty(EditorObject* mainForm, int left, int top, int width, int height);

		void writeData(QTextStream& stream) override;
		QString getName() override { return "Image Rect"; }
	};

	class EditorObjectBaddyProperty :
		public AbstractEditorObjectProperty
	{
		Q_OBJECT

	private:
		EditorObject* m_mainForm;

	public:

		EditorObjectBaddyProperty(EditorObject* mainForm);

		void writeData(QTextStream& stream) override;
		QString getName() override { return "Baddy"; }
	};

	class EditorObjectDontEmbedCodeProperty :
		public AbstractEditorObjectProperty
	{
		Q_OBJECT

	private:
		EditorObject* m_mainForm;

	public:

		EditorObjectDontEmbedCodeProperty(EditorObject* mainForm);

		void writeData(QTextStream& stream) override;
		QString getName() override { return "Don't Embed Code"; }
	};


	class EditorObjectRenderModeProperty :
		public AbstractEditorObjectProperty
	{
		Q_OBJECT
	public slots:
		void setCurrentIndex(int index);

	private:
		EditorObject* m_mainForm;
		QComboBox* m_renderModeCombo;


	public:

		EditorObjectRenderModeProperty(EditorObject * mainForm, RenderMode renderMode);

		void writeData(QTextStream & stream) override;
		QString getName() override { return "Editor Render Mode"; }
	};

	class EditorObjectScriptingLanguageProperty :
		public AbstractEditorObjectProperty
	{
		Q_OBJECT
	public slots:
		void setCurrentIndex(int index);

	private:
		EditorObject* m_mainForm;
		QComboBox* m_scriptingLanguageCombo;


	public:

		EditorObjectScriptingLanguageProperty(EditorObject* mainForm, ScriptingLanguage scriptingLanguage);

		void writeData(QTextStream& stream) override;
		QString getName() override { return "Scripting Language"; }
	};
};
