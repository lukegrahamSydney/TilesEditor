#include "EditExternalNPC.h"
#include "EditScriptForm.h"
#include <qtoolbutton.h>
#include <QMessageBox>
#include <qfileinfo.h>
#include "ImageDimensions.h"
#include "ObjectClassParam.h"
#include "EditorObject.h"
#include "LevelCommands.h"


namespace TilesEditor
{
	QByteArray EditExternalNPC::savedGeometry;
	EditExternalNPC::EditExternalNPC(IWorld* world, LevelObjectInstance* instance, QWidget* parent)
		: QDialog(parent), m_world(world), m_objectInstance(instance)
	{
		ui.setupUi(this);

		ui.imageLineEdit->setText(instance->getImageName());

		m_objectClass = instance->getObjectClass();

		if (m_objectClass)
		{
			m_objectClass->incrementRef();
		}

		auto& instanceParams = instance->getParams();
		ui.classLineEdit->setText(instance->getClassName());

		int paramIndex = 0;
		for (auto& param : instanceParams)
		{
			addNewParamRow(param);
		}
		connect(ui.newButton, &QPushButton::clicked, this, &EditExternalNPC::newClicked);
		connect(ui.deleteButton, &QPushButton::clicked, this, &EditExternalNPC::deleteClicked);
		connect(ui.browseButton, &QPushButton::clicked, this, &EditExternalNPC::imageBrowseClicked);
		connect(ui.imageLineEdit, &QLineEdit::textEdited, this, &EditExternalNPC::textEdited);
		connect(ui.toolButtonEditClass, &QPushButton::clicked, this, &EditExternalNPC::editClassClicked);
		if (!savedGeometry.isNull())
			restoreGeometry(savedGeometry);
	}

	EditExternalNPC::~EditExternalNPC()
	{
		savedGeometry = saveGeometry();

		if (m_objectClass)
		{
			m_world->getResourceManager()->getObjectManager()->releaseObject(m_objectClass);
		}
	}

	void EditExternalNPC::setModified(bool value)
	{
		if (value && !m_modified)
		{
			this->setWindowTitle("Edit Object Instance*");
		}
		m_modified = value;
	}

	void EditExternalNPC::addNewParamRow(const QString& value, bool useDefaultValue)
	{
		int paramIndex = ui.paramsLayout->rowCount();

		QString label = "Param";

		if (m_objectClass)
		{
			auto& paramTemplate = m_objectClass->getParams();
			if (paramIndex < paramTemplate.size())
			{
				auto& classParam = paramTemplate.at(paramIndex);

				auto rowWidget = classParam->createParamRow(this);
				rowWidget->setValue(useDefaultValue ? m_world->getEngine()->parseInlineString(classParam->getDefaultValue()) : value);
				addNewParamRow(classParam->getLabel(), rowWidget);

				return;
			}
		}

		auto rowWidget = new ObjectClassParamExpressionWidget(this);
		rowWidget->setValue(value);

		addNewParamRow("Param", rowWidget);
		

	}

	void EditExternalNPC::addNewParamRow(const QString& label, AbstractExternalNPCParamRow* widget)
	{
		auto parentWidget = new QWidget();
		auto deleteButton = new QToolButton(parentWidget);
		deleteButton->setIcon(QIcon(":/MainWindow/icons/tinycolor/icons8-trash-16.png"));

		auto layout = new QHBoxLayout();
		layout->addWidget(widget);
		layout->addWidget(deleteButton);
		layout->setSpacing(2);
		layout->setContentsMargins(0, 0, 0, 0);
		parentWidget->setLayout(layout);


		ui.paramsLayout->addRow(label + ":", parentWidget);

		connect(deleteButton, &QPushButton::pressed, this, &EditExternalNPC::deleteParamPressed);
	}


	void EditExternalNPC::imageBrowseClicked(bool checked)
	{
		auto fileName = m_world->getResourceManager()->getOpenFileName("Select Image", m_world->getResourceManager()->getConnectionString(), "Image Files (*.png *.gif)");
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);
			auto directory = fi.absolutePath() + "/";

			if(m_world->getResourceManager()->getType() == "FileSystem")
				static_cast<ResourceManagerFileSystem*>(m_world->getResourceManager())->addSearchDir(directory);

			ui.imageLineEdit->setText(fi.fileName());

			int width, height;

			if (ImageDimensions::getImageFileDimensions(fileName, &width, &height))
			{
				//ui.widthText->setText(QString::number(width));
				//ui.heightText->setText(QString::number(height));
			}
			m_modified = true;

		}

	}

	void EditExternalNPC::editClassClicked(bool checked)
	{
		if (m_objectClass != nullptr)
		{
			EditorObject form(m_world->getEngine(), m_objectClass->getName(), m_objectClass->getFullPath(), m_world->getResourceManager());

			form.exec();

			if (form.changesSaved())
			{
				m_objectClass->load(m_world->getResourceManager(), false);
				m_objectClass->markInstancesModified();
				markModified();
			}
		}
	}

	void EditExternalNPC::deleteClicked(bool checked)
	{
		if (QMessageBox::question(nullptr, "Warning", "Are you sure you want to delete this NPC?") == QMessageBox::Yes)
		{
			m_modified = false;
			m_world->deleteEntity(m_objectInstance);
			QDialog::reject();

		}
	}

	void EditExternalNPC::textEdited(const QString& text)
	{
		setModified(true);
	}

	void EditExternalNPC::deleteParamPressed()
	{
		auto deleteButton = static_cast<QToolButton*>(this->sender());
		ui.paramsLayout->removeRow(deleteButton->parentWidget());

		markModified();
	}

	void EditExternalNPC::accept()
	{
		if (m_modified)
		{
			auto undoCommand = new QUndoCommand("Edit NPC");

			new CommandSetEntityProperty(m_world, m_objectInstance, "image", ui.imageLineEdit->text(), m_objectInstance->getImageName(), undoCommand);

			QStringList params;

			for (int paramIndex = 0; paramIndex < ui.paramsLayout->rowCount(); ++paramIndex)
			{
				auto paramRow = static_cast<AbstractExternalNPCParamRow*>(ui.paramsLayout->itemAt(paramIndex, QFormLayout::FieldRole)->widget()->layout()->itemAt(0)->widget());
				params.push_back(paramRow->getValue());
			}

			new CommandSetEntityProperty(m_world, m_objectInstance, "params", params, m_objectInstance->getParams(), undoCommand);
			m_world->setModified(m_objectInstance->getLevel());

			if (undoCommand->childCount())
				m_world->addUndoCommand(undoCommand);
			else delete undoCommand;
		}
		QDialog::accept();
	}


	void EditExternalNPC::reject()
	{
		if (m_modified)
		{
			if (QMessageBox::question(nullptr, "Changes made", "You have made changes to the NPC. Are you sure you want to close?") == QMessageBox::Yes)
			{
				QDialog::reject();
			}
		}
		else QDialog::reject();
	}

	void EditExternalNPC::newClicked(bool checked)
	{
		addNewParamRow("", true);


		setModified(true);
	}


	EditExternalNPCParamRow::EditExternalNPCParamRow(EditExternalNPC* parentForm, QFormLayout* parentLayout) :
		QWidget(), m_parentForm(parentForm), m_parentLayout(parentLayout)
	{

		m_lineEdit = new QLineEdit(this);
		auto deleteButton = new QToolButton();
		auto editButton = new QToolButton();

		editButton->setIcon(QIcon(":/MainWindow/icons/fugue/script--pencil.png"));
		deleteButton->setIcon(QIcon(":/MainWindow/icons/fugue/cross-script.png"));

		auto layout = new QHBoxLayout();
		layout->addWidget(m_lineEdit);
		layout->addWidget(editButton);
		layout->addWidget(deleteButton);
		layout->setSpacing(2);
		layout->setContentsMargins(0, 0, 0, 0);

		this->setLayout(layout);

		connect(editButton, &QPushButton::pressed, this, &EditExternalNPCParamRow::editPressed);
		connect(deleteButton, &QPushButton::pressed, this, &EditExternalNPCParamRow::deletePressed);
		connect(m_lineEdit, &QLineEdit::textEdited, this, &EditExternalNPCParamRow::textEdited);
	}

	void EditExternalNPCParamRow::setValue(const QString& text)
	{
		m_value = text;
		m_lineEdit->setText(text);

	}

	QString EditExternalNPCParamRow::getValue() const
	{
		return m_value;
	}

	void EditExternalNPCParamRow::editPressed()
	{

		EditScriptForm form(this->m_lineEdit->text());
		if (form.exec() == QDialog::Accepted)
		{
			m_value = form.getScript();
			m_lineEdit->setText(m_value);
			m_parentForm->setModified(true);
		}

	}

	void EditExternalNPCParamRow::deletePressed()
	{
		m_parentForm->setModified(true);
		m_parentLayout->removeRow(this);

	}

	void EditExternalNPCParamRow::textEdited(const QString& text)
	{
		m_parentForm->setModified(true);
		m_value = m_lineEdit->text();
	}


};

