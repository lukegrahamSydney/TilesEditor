#ifndef OBJECTCLASSPARAMH
#define OBJECTCLASSPARAMH

#include <QString>
#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QList>
#include <QComboBox>
#include <QDoubleValidator>
#include <QIntValidator>
#include "IExternalObjectEditor.h"
#include "AbstractExternalNPCParamRow.h"
#include "EditScriptForm.h"

namespace TilesEditor
{
	class ObjectClassParam
	{
	private:
		QString m_label;
		QString m_defaultValue;
		bool m_canCopy = true;

	public:

		ObjectClassParam(const QString& label) :
			m_label(label) {}

		
		const QString& getLabel() const { return m_label; }
		const QString& getDefaultValue() const { return m_defaultValue; }

		void setDefaultValue(const QString& value) { m_defaultValue = value; }

		bool canCopy() const { return m_canCopy; }
		void setCanCopy(bool value) { m_canCopy = value; }
		virtual AbstractExternalNPCParamRow* createParamRow(IExternalObjectEditor* editor) = 0;
		virtual bool isString() const { return false; }
	};

	class ObjectClassParamExpressionWidget :
		public AbstractExternalNPCParamRow
	{
		Q_OBJECT
	public slots:
		inline void editPressed()
		{
			EditScriptForm form(this->m_lineEdit->text());
			if (form.exec() == QDialog::Accepted)
			{
				setValue(form.getScript());
				m_editor->markModified();
			}
		}

		inline void textEdited(const QString& text)
		{
			m_editor->markModified();
		}

	private:
		QLineEdit* m_lineEdit;
		IExternalObjectEditor* m_editor;


	public:

		inline ObjectClassParamExpressionWidget(IExternalObjectEditor* editor):
			m_editor(editor)
		{
			m_lineEdit = new QLineEdit(this);
			auto editButton = new QToolButton();

			editButton->setIcon(QIcon(":/MainWindow/icons/tinycolor/icons8-edit2-16.png"));

			auto layout = new QHBoxLayout();
			layout->addWidget(m_lineEdit);
			layout->addWidget(editButton);
			layout->setSpacing(2);
			layout->setContentsMargins(0, 0, 0, 0);

			this->setLayout(layout);

			connect(editButton, &QPushButton::pressed, this, &ObjectClassParamExpressionWidget::editPressed);
			connect(m_lineEdit, &QLineEdit::textEdited, this, &ObjectClassParamExpressionWidget::textEdited);
		}

		void setValue(const QString& text) override {
			m_lineEdit->setText(text);
		}

		QString getValue() const override {
			return m_lineEdit->text();
		}
	};

	class ObjectClassParamExpression :
		public ObjectClassParam
	{
	private:

	public:
		ObjectClassParamExpression(const QString& label) :
			ObjectClassParam(label)
		{}

		AbstractExternalNPCParamRow* createParamRow(IExternalObjectEditor* editor) override
		{
			return new ObjectClassParamExpressionWidget(editor);
		}
	};

	enum ObjectClassParamTextType {
		Real,
		Int,
		String
	};
	class ObjectClassParamTextWidget :
		public AbstractExternalNPCParamRow
	{
		Q_OBJECT
	public slots:
		inline void textEdited(const QString& text)
		{
			m_editor->markModified();
		}

	private:
		QLineEdit* m_lineEdit;
		IExternalObjectEditor* m_editor;
		ObjectClassParamTextType m_textType = ObjectClassParamTextType::Int;

	public:

		inline ObjectClassParamTextWidget(IExternalObjectEditor* editor, ObjectClassParamTextType textType) :
			m_editor(editor), m_textType(textType)
		{
			m_lineEdit = new QLineEdit(this);


			auto layout = new QHBoxLayout();
			layout->addWidget(m_lineEdit);
			layout->setSpacing(2);
			layout->setContentsMargins(0, 0, 0, 0);

			this->setLayout(layout);

			if(textType == ObjectClassParamTextType::Real)
				m_lineEdit->setValidator(new QDoubleValidator(-2000000000, 2000000000, 4, this));
			else if (textType == ObjectClassParamTextType::Int)
				m_lineEdit->setValidator(new QIntValidator(-2000000000, 2000000000, this));

			connect(m_lineEdit, &QLineEdit::textEdited, this, &ObjectClassParamTextWidget::textEdited);
		}

		void setValue(const QString& text) override {
			m_lineEdit->setText(text);
		}

		QString getValue() const override {
			return m_lineEdit->text();
		}


	};

	class ObjectClassParamText :
		public ObjectClassParam
	{
	public:

	private:
		ObjectClassParamTextType m_textType = ObjectClassParamTextType::Int;

	public:
		ObjectClassParamText(const QString& label, ObjectClassParamTextType textType) :
			ObjectClassParam(label), m_textType(textType)
		{}

		AbstractExternalNPCParamRow* createParamRow(IExternalObjectEditor* editor) override
		{
			return new ObjectClassParamTextWidget(editor, m_textType);
		}

		bool isString() const override { return m_textType == ObjectClassParamTextType::String; }
	};

	class ObjectClassParamListWidget :
		public AbstractExternalNPCParamRow
	{
		Q_OBJECT
	public slots:


		inline void itemIndexChanged(int)
		{
			m_editor->markModified();
		}

	private:
		QComboBox* m_comboBox;
		IExternalObjectEditor* m_editor;


	public:

		inline ObjectClassParamListWidget(IExternalObjectEditor* editor) :
			m_editor(editor)
		{
			m_comboBox = new QComboBox(this);

			auto layout = new QHBoxLayout();
			layout->addWidget(m_comboBox);
			layout->setSpacing(2);
			layout->setContentsMargins(0, 0, 0, 0);

			this->setLayout(layout);
			connect(m_comboBox, &QComboBox::currentIndexChanged, this, &ObjectClassParamListWidget::itemIndexChanged);
			//connect(editButton, &QPushButton::pressed, this, &ObjectClassParamExpressionWidget::editPressed);
			//connect(m_lineEdit, &QLineEdit::textEdited, this, &ObjectClassParamExpressionWidget::textEdited);
		}

		void addItem(const QString& name, const QString& value)
		{
			auto blocked = m_comboBox->blockSignals(true);
			m_comboBox->addItem(name, value);
			m_comboBox->blockSignals(blocked);
		}

		void setValue(const QString& value) override {

			for (int i = 0; i < m_comboBox->count(); ++i)
			{
				if (m_comboBox->itemData(i).toString() == value)
				{
					auto blocked = m_comboBox->blockSignals(true);
					m_comboBox->setCurrentIndex(i);
					m_comboBox->blockSignals(blocked);
					return;
				}
			}
		}

		QString getValue() const override {
			return m_comboBox->itemData(m_comboBox->currentIndex()).toString();
		}
	};

	class ObjectClassParamList :
		public ObjectClassParam
	{
	private:
		QList<QPair<QString, QString> > m_listItems;

	public:
		ObjectClassParamList(const QString& label) :
			ObjectClassParam(label)
		{}

		void addItem(const QString& name, const QString& value)
		{
			m_listItems.push_back(QPair<QString, QString>(name, value));
		}

		AbstractExternalNPCParamRow* createParamRow(IExternalObjectEditor* editor) override
		{
			auto rowWidget = new ObjectClassParamListWidget(editor);

			for (auto& pair : m_listItems)
			{
				rowWidget->addItem(pair.first, pair.second);

			}
			return rowWidget;
		}
	};

	class ObjectClassParamFactory
	{
	public:
		static ObjectClassParam* createParam(const QString& type, const QString& label)
		{
			if (type == "INT")
				return new ObjectClassParamText(label, ObjectClassParamTextType::Int);
			else if (type == "REAL")
				return new ObjectClassParamText(label, ObjectClassParamTextType::Real);
			else if (type == "STRING")
				return new ObjectClassParamText(label, ObjectClassParamTextType::String);

			else if (type == "EXPRESSION")
				return new ObjectClassParamExpression(label);

			else if (type == "COMBO")
				return new ObjectClassParamList(label);
			return nullptr;
		}
	};
};

#endif

