#pragma once

#include <QDialog>
#include "ui_EditExternalNPC.h"
#include "LevelObjectInstance.h"
#include "IWorld.h"
#include "IExternalObjectEditor.h"
#include "AbstractExternalNPCParamRow.h"
#include "ObjectClass.h"

namespace TilesEditor
{
	class EditExternalNPC : public QDialog, public IExternalObjectEditor
	{
		Q_OBJECT
	public slots:
		void accept() override;
		void reject() override;
		void newClicked(bool checked);
		void deleteClicked(bool checked);
		void imageBrowseClicked(bool checked);
		void editClassClicked(bool checked);
		void textEdited(const QString& text);
		void deleteParamPressed();
		void makeAnonymousButtonClicked(bool checked);

	private:
		IWorld* m_world;
		bool m_modified = false;
		LevelObjectInstance* m_objectInstance;
		ObjectClass* m_objectClass = nullptr;
		void addNewParamRow(const QString& value, bool useDefaultValue = false);
		void addNewParamRow(const QString& label, AbstractExternalNPCParamRow* widget);

		Ui::EditExternalNPCClass ui;

		void populateUndoCommand(QUndoCommand* parent);

	public:
		EditExternalNPC(IWorld* world, LevelObjectInstance* instance, QWidget* parent = nullptr);
		~EditExternalNPC();

		void setModified(bool value);

		void markModified() override { setModified(true); }
		static QByteArray savedGeometry;
	};

	class EditExternalNPCParamRow : public QWidget
	{
		Q_OBJECT
	public slots:
		void editPressed();
		void deletePressed();
		void textEdited(const QString& text);

	private:
		QString m_value = "";
		QLineEdit* m_lineEdit;
		EditExternalNPC* m_parentForm;
		QFormLayout* m_parentLayout;


	public:

		EditExternalNPCParamRow(EditExternalNPC* parentForm, QFormLayout* parentLayout);

		void setValue(const QString& text);
		QString getValue() const;
	};
};
