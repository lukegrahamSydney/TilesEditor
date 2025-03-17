#include <QFile>
#include <QValidator>
#include <QMessageBox>
#include <QFileDialog>
#include <QStyleHints>
#include "EditorObject.h"
#include "QSyntaxStyle.hpp"
#include "QCXXHighlighter.hpp"
#include <QToolButton>


namespace TilesEditor
{
	static QString rstrip(const QString& str) {
		int n = str.size() - 1;
		for (; n >= 0; --n) {
			if (!str.at(n).isSpace()) {
				return str.left(n + 1);
			}
		}
		return "";
	}



	QByteArray EditorObject::savedGeometry;
	EditorObject::EditorObject(IEngine* engine, const QString& className, const QString& fileName, AbstractResourceManager* resourceManager, QWidget* parent)
		: QDialog(parent), m_engine(engine), m_className(className), m_fileName(fileName), m_editorStyle(this), m_resourceManager(resourceManager)
	{
		ui.setupUi(this);

		this->setWindowTitle("Edit Object: " + className);

		this->setWindowFlag(Qt::Window);
		this->setWindowFlag(Qt::WindowMaximizeButtonHint);
		ui.splitter->setStretchFactor(1, 0);
		ui.splitter->setStretchFactor(0, 1);


		m_newPropertyMenu.addAction("Size", this, &EditorObject::actionNewSizeProperty);
		m_newPropertyMenu.addAction("Image Rect", this, &EditorObject::actionNewImageShapeProperty);
		m_newPropertyMenu.addAction("Weapon Name", this, &EditorObject::actionNewWeaponNameProperty);
		m_newPropertyMenu.addAction("Baddy", this, &EditorObject::actionNewBaddyProperty);
		m_newPropertyMenu.addAction("Editor Render Mode", this, &EditorObject::actionNewRenderModeProperty);
		m_newPropertyMenu.addAction("Parent Class", this, &EditorObject::actionNewParentClassProperty);
		m_newPropertyMenu.addAction("Don't Embed Code", this, &EditorObject::actionNewDontEmbedCodeProperty);
		m_newPropertyMenu.addAction("Scripting Language", this, &EditorObject::actionNewScriptingLanguageProperty);
		ui.newPropertyButton->setMenu(&m_newPropertyMenu);

		QFile fl(QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark ? ":/styles/drakula.xml" : ":/default_style.xml");

		if (fl.open(QIODevice::ReadOnly))
		{
			if (m_editorStyle.load(fl.readAll()))
			{
				ui.clientCodeEdit->setSyntaxStyle(&m_editorStyle);
				ui.serverCodeEdit->setSyntaxStyle(&m_editorStyle);
			}
		}

		ui.clientCodeEdit->setHighlighter(&m_cppHighlighterClient);
		ui.serverCodeEdit->setHighlighter(&m_cppHighlighterServer);

		ui.fileLineEdit->setText(fileName);

		QFile textFile(fileName);
		if (textFile.open(QIODeviceBase::ReadOnly))
		{
			QString properties;
			QTextStream textStream(&textFile);

			if (rstrip(textStream.readLine()) == "NPC001")
			{
				while (!textStream.atEnd())
				{
					QString line = rstrip(textStream.readLine());
					QStringList words = line.split(' ');


					if (words.size() > 0)
					{
						if (words[0] == "IMAGE")
						{
							if (words.size() >= 2)
								ui.imageLineEdit->setText(words[1].trimmed());
						}
						else if (words[0] == "CLIENTCODE")
						{
							if (!textStream.atEnd())
							{
								QString code = "";
								for (auto codeLine = rstrip(textStream.readLine()); !textStream.atEnd(); codeLine = rstrip(textStream.readLine()))
								{
									if (codeLine == "CLIENTCODEEND")
									{
										break;
									}
									else code += codeLine.replace('\t', "    ") + "\n";
								}

								ui.clientCodeEdit->setText(code);
							}
						}
						else if (words[0] == "SERVERCODE")
						{
							if (!textStream.atEnd())
							{
								QString code = "";
								for (auto codeLine = rstrip(textStream.readLine()); !textStream.atEnd(); codeLine = rstrip(textStream.readLine()))
								{
									if (codeLine == "SERVERCODEEND")
									{
										break;
									}
									else code += codeLine.replace('\t', "    ") + "\n";
								}

								ui.serverCodeEdit->setText(code);
							}
						}
						else if (words[0] == "SHAPE" && words.size() >= 3)
						{
							int width = words[1].toInt();
							int height = words[2].toInt();

							addNewPropertyRow("Size", new EditorObjectShapeProperty(this, width, height));

						}
						else if (words[0] == "IMAGESHAPE" && words.size() >= 5)
						{
							int left = words[1].toInt();
							int top = words[2].toInt();
							int width = words[3].toInt();
							int height = words[4].toInt();

							addNewPropertyRow("Image Rect", new EditorObjectImageShapeProperty(this, left, top, width, height));

						}
						else if (words[0] == "WEAPONNAME")
						{
							QString name = line.mid(strlen("WEAPONNAME ")).trimmed();
							addNewPropertyRow("Weapon Name", new EditorObjectWeaponNameProperty(this, name));
						}
						else if (words[0] == "BADDY")
						{
							addNewPropertyRow("Baddy", new EditorObjectBaddyProperty(this));
						}
						else if (words[0] == "DONTEMBEDCODE")
						{
							addNewPropertyRow("Don't Embed Code", new EditorObjectDontEmbedCodeProperty(this));
						}
						else if (words[0] == "PARENT" && words.size() >= 2)
						{
							addNewPropertyRow("Parent Class", new EditorObjectParentProperty(this, words[1]));
						}
						else if (words[0] == "EDITORRENDERMODE" && words.size() >= 2)
						{
							RenderMode renderMode = RenderMode::Centered;
							auto modeName = words[1].trimmed().toLower();

							if (modeName == "centered")
								renderMode = RenderMode::Centered;
							else if (modeName == "stretched")
								renderMode = RenderMode::Stretched;
							else if (modeName == "tiled")
								renderMode = RenderMode::Tiled;
							else if (modeName == "rect") {
								renderMode = RenderMode::Rect;
								if (words.size() >= 3)
								{

								}
							}
							else continue;

							addNewPropertyRow("Editor Render Mode", new EditorObjectRenderModeProperty(this, renderMode));


						}

						else if (words[0] == "SCRIPTINGLANGUAGE" && words.size() >= 2)
						{
							auto languageName = words[1].trimmed().toLower();

							if (languageName == "gs1script")
								m_scriptingLanguage = ScriptingLanguage::SCRIPT_GS1;
							else if (languageName == "sgscript")
								m_scriptingLanguage = ScriptingLanguage::SCRIPT_SGSCRIPT;

							addNewPropertyRow("Scripting Langauge", new EditorObjectScriptingLanguageProperty(this, m_scriptingLanguage));
						}
						else properties += line + "\n";
					}
				}
			}

			ui.propertiesEdit->setText(rstrip(properties));
		} else QMessageBox::warning(this, "Error reading object file", "This object file may no longer exist.");
		connect(ui.browseButton, &QAbstractButton::pressed, this, &EditorObject::browseImagePressed);
		connect(ui.testButton, &QPushButton::pressed, this, &EditorObject::testPressed);
		connect(ui.clientCodeEdit, &QCodeEditor::textChanged, this, &EditorObject::textChanged);
		connect(ui.serverCodeEdit, &QCodeEditor::textChanged, this, &EditorObject::textChanged);
		connect(ui.imageLineEdit, &QLineEdit::textChanged, this, &EditorObject::textChanged);

		connect(ui.propertiesEdit, &QTextEdit::textChanged, this, &EditorObject::textChanged);
		connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &EditorObject::buttonBoxClicked);

		if (!savedGeometry.isNull())
			restoreGeometry(savedGeometry);




	}

	EditorObject::~EditorObject()
	{
		savedGeometry = saveGeometry();
	}


	void EditorObject::save()
	{
		QFile textFile(m_fileName);
		if (textFile.open(QIODeviceBase::WriteOnly))
		{
			QString properties;
			QTextStream textStream(&textFile);

			textStream << "NPC001" << Qt::endl;
			textStream << "IMAGE " << ui.imageLineEdit->text() << Qt::endl;

			for (int i = 0; i < ui.propertiesLayout->count(); ++i)
			{
				auto item = ui.propertiesLayout->itemAt(i);
				auto propertyWidget = (AbstractEditorObjectProperty*)(item->widget()->layout()->itemAt(0)->widget());
				propertyWidget->writeData(textStream);
			}

			auto otherProperties = ui.propertiesEdit->toPlainText();

			if (!otherProperties.isEmpty())
				textStream << rstrip(otherProperties) << Qt::endl;

			auto clientCode = ui.clientCodeEdit->toPlainText();
			if (!clientCode.isEmpty())
			{
				textStream << "CLIENTCODE" << Qt::endl;
				textStream << rstrip(clientCode) << Qt::endl;
				textStream << "CLIENTCODEEND" << Qt::endl;
			}

			auto serverCode = ui.serverCodeEdit->toPlainText();
			if (!serverCode.isEmpty())
			{
				textStream << "SERVERCODE" << Qt::endl;
				textStream << rstrip(serverCode) << Qt::endl;
				textStream << "SERVERCODEEND" << Qt::endl;
			}

			m_modified = false;
			m_changesSaved = true;
			this->setWindowTitle("Edit Object: " + m_className);
		}
	}

	void EditorObject::actionNewSizeProperty()
	{
		addNewPropertyRow("Size", new EditorObjectShapeProperty(this, 32, 32), true);
	}

	void EditorObject::actionNewImageShapeProperty()
	{
		addNewPropertyRow("Image Rect", new EditorObjectImageShapeProperty(this, 0, 0, 32, 32), true);
	}

	void EditorObject::actionNewWeaponNameProperty()
	{
		addNewPropertyRow("Weapon Name", new EditorObjectWeaponNameProperty(this, ""), true);
	}

	void EditorObject::actionNewBaddyProperty()
	{
		addNewPropertyRow("Baddy", new EditorObjectBaddyProperty(this), true);
	}

	void EditorObject::actionNewRenderModeProperty()
	{
		addNewPropertyRow("Editor Render Mode", new EditorObjectRenderModeProperty(this, RenderMode::Centered), true);
	}

	void EditorObject::actionNewParentClassProperty()
	{
		addNewPropertyRow("Parent Class", new EditorObjectParentProperty(this, ""), true);
	}

	void EditorObject::actionNewDontEmbedCodeProperty()
	{
		addNewPropertyRow("Don't Embed Code", new EditorObjectDontEmbedCodeProperty(this), true);
	}

	void EditorObject::actionNewScriptingLanguageProperty()
	{
		addNewPropertyRow("Scripting Language", new EditorObjectScriptingLanguageProperty(this, ScriptingLanguage::SCRIPT_SGSCRIPT), true);
	}


	void EditorObject::buttonBoxClicked(QAbstractButton* button)
	{
		if (button == ui.buttonBox->button(QDialogButtonBox::StandardButton::Ok))
			this->accept();

		else if (button == ui.buttonBox->button(QDialogButtonBox::StandardButton::Cancel))
			this->reject();

		else if (button == ui.buttonBox->button(QDialogButtonBox::StandardButton::Save))
			this->save();
	}

	void EditorObject::accept()
	{
		if (m_modified)
		{
			save();

		}

		QDialog::accept();
	}

	void EditorObject::reject()
	{
		if (m_modified)
		{
			if (QMessageBox::question(nullptr, "Changes made", "You have made changes to the file. Are you sure you want to close?") == QMessageBox::Yes)
			{
				QDialog::reject();
			}
		}
		else QDialog::reject();

	}

	void EditorObject::browseImagePressed()
	{
		auto fileName = QFileDialog::getOpenFileName(nullptr, "Select Image", QString(), "Image Files (*.png *.gif)");
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);

			auto fname = fi.fileName();
			if (fname != ui.imageLineEdit->text())
			{
				ui.imageLineEdit->setText(fname);
				setModified(true);
			}
		}
	}


	void EditorObject::setModified(bool value)
	{
		if (value && !m_modified)
		{
			this->setWindowTitle("Edit Object: " + m_className + "*");
		}
		m_modified = value;
	}

	void EditorObject::addNewPropertyRow(const QString& label, QWidget* widget, bool setModified)
	{
		auto parent = new QWidget();
		parent->setProperty("propName", label);
		parent->setLayout(new QHBoxLayout());
		parent->layout()->setContentsMargins(0, 0, 0, 0);

		auto deleteButton = new QToolButton(parent);
		deleteButton->setIcon(QIcon(":/MainWindow/icons/tinycolor/icons8-trash-16.png"));



		parent->layout()->addWidget(widget);
		parent->layout()->addWidget(deleteButton);

		ui.propertiesLayout->addWidget(parent);
		connect(deleteButton, &QPushButton::pressed, this, &EditorObject::deletePropertyPressed);

		if (setModified)
			this->setModified(true);

		for (auto action : m_newPropertyMenu.actions())
		{
			if (action->text() == label)
			{
				action->setEnabled(false);
				action->setProperty("count", action->property("count").toInt() + 1);
			}
		}
	}

	void EditorObject::removePropertyRow(QWidget* item)
	{
		auto propName = item->property("propName").toString();
		for (auto action : m_newPropertyMenu.actions())
		{
			if (action->text() == propName)
			{
				auto count = action->property("count").toInt() - 1;
				action->setProperty("count", count);

				if (count == 0)
					action->setEnabled(true);
			}
		}
		setModified(true);
		item->deleteLater();
	}



	void EditorObject::testPressed()
	{

		auto code = ui.scriptsTab->currentWidget() == ui.tab ? ui.clientCodeEdit->toPlainText() : ui.serverCodeEdit->toPlainText();

		QString error = "";
		if (!m_engine->testCodeForErrors(code, &error, m_scriptingLanguage))
		{
			ui.testEdit->setPlainText(error);
		}
		else ui.testEdit->setPlainText("No syntax errors found.");


	}

	void EditorObject::textChanged()
	{
		setModified(true);
		m_modified = true;
	}

	void EditorObject::deletePropertyPressed()
	{
		auto deleteButton = static_cast<QToolButton*>(this->sender());
		this->removePropertyRow(deleteButton->parentWidget());
	}


	//SHAPE PROPERTY
	EditorObjectShapeProperty::EditorObjectShapeProperty(EditorObject* mainForm, int width, int height) :
		m_mainForm(mainForm)
	{
		this->setTitle("Size");

		m_widthEdit = new QLineEdit(QString("%1").arg(width));
		m_heightEdit = new QLineEdit(QString("%1").arg(height));

		auto layout = new QHBoxLayout();
		layout->addWidget(new QLabel("W:"));
		layout->addWidget(m_widthEdit);
		layout->addWidget(new QLabel("H:"));
		layout->addWidget(m_heightEdit);
		layout->setSpacing(2);
		layout->setContentsMargins(0, 10, 0, 5);

		this->setLayout(layout);
		m_widthEdit->setValidator(new QIntValidator(0, 1000000000, this));
		m_heightEdit->setValidator(new QIntValidator(0, 1000000000, this));

		connect(m_widthEdit, &QLineEdit::textEdited, this, &EditorObjectShapeProperty::textEdited);
		connect(m_heightEdit, &QLineEdit::textEdited, this, &EditorObjectShapeProperty::textEdited);


	}


	void EditorObjectShapeProperty::textEdited(const QString& text)
	{
		m_mainForm->setModified(true);
	}

	void EditorObjectShapeProperty::writeData(QTextStream& stream)
	{
		stream << "SHAPE " << m_widthEdit->text() << " " << m_heightEdit->text() << Qt::endl;
	}

	//WEAPON NAME PROPERTY

	EditorObjectWeaponNameProperty::EditorObjectWeaponNameProperty(EditorObject* mainForm, const QString& weaponName) :
		m_mainForm(mainForm)
	{
		this->setTitle("Weapon Name");
		m_weaponName = new QLineEdit(weaponName);

		auto layout = new QHBoxLayout();
		layout->addWidget(m_weaponName);
		layout->setSpacing(2);
		layout->setContentsMargins(0, 10, 0, 5);

		this->setLayout(layout);

		connect(m_weaponName, &QLineEdit::textEdited, this, &EditorObjectWeaponNameProperty::textEdited);
	}


	void EditorObjectWeaponNameProperty::textEdited(const QString& text)
	{
		m_mainForm->setModified(true);
	}

	void EditorObjectWeaponNameProperty::writeData(QTextStream& stream)
	{
		stream << "WEAPONNAME " << m_weaponName->text() << Qt::endl;
	}

	//PARENT NAME
	EditorObjectParentProperty::EditorObjectParentProperty(EditorObject* mainForm, const QString& parentName) :
		m_mainForm(mainForm)
	{
		this->setTitle("Parent Class");
		m_parentName = new QLineEdit(parentName);

		auto layout = new QHBoxLayout();
		layout->addWidget(m_parentName);
		layout->setSpacing(2);
		layout->setContentsMargins(0, 10, 0, 5);

		this->setLayout(layout);

		connect(m_parentName, &QLineEdit::textEdited, this, &EditorObjectParentProperty::textEdited);
	}

	void EditorObjectParentProperty::textEdited(const QString& text)
	{
		m_mainForm->setModified(true);
	}

	void EditorObjectParentProperty::writeData(QTextStream& stream)
	{
		stream << "PARENT " << m_parentName->text() << Qt::endl;
	}

	//IMAGE SHAPE PROPERTY

	EditorObjectImageShapeProperty::EditorObjectImageShapeProperty(EditorObject* mainForm, int left, int top, int width, int height) :
		m_mainForm(mainForm)
	{
		this->setTitle("Image Rect");

		m_left = new QLineEdit(QString("%1").arg(left));
		m_top = new QLineEdit(QString("%1").arg(top));
		m_widthEdit = new QLineEdit(QString("%1").arg(width));
		m_heightEdit = new QLineEdit(QString("%1").arg(height));

		auto layout = new QHBoxLayout();

		auto gridLayout = new QGridLayout();
		gridLayout->addWidget(new QLabel("X:"), 0, 0);
		gridLayout->addWidget(m_left, 0, 1);
		gridLayout->addWidget(new QLabel("Y:"), 0, 2);
		gridLayout->addWidget(m_top, 0, 3);

		gridLayout->addWidget(new QLabel("W:"), 1, 0);
		gridLayout->addWidget(m_widthEdit, 1, 1);

		gridLayout->addWidget(new QLabel("H:"), 1, 2);
		gridLayout->addWidget(m_heightEdit, 1, 3);
		gridLayout->setSpacing(2);
		gridLayout->setContentsMargins(0, 0, 0, 0);

		layout->addLayout(gridLayout);
		layout->setSpacing(2);
		layout->setContentsMargins(0, 10, 0, 5);

		this->setLayout(layout);

		m_left->setValidator(new QIntValidator(0, 1000000000, this));
		m_top->setValidator(new QIntValidator(0, 1000000000, this));
		m_widthEdit->setValidator(new QIntValidator(0, 1000000000, this));
		m_heightEdit->setValidator(new QIntValidator(0, 1000000000, this));

		connect(m_left, &QLineEdit::textEdited, this, &EditorObjectImageShapeProperty::textEdited);
		connect(m_top, &QLineEdit::textEdited, this, &EditorObjectImageShapeProperty::textEdited);
		connect(m_widthEdit, &QLineEdit::textEdited, this, &EditorObjectImageShapeProperty::textEdited);
		connect(m_heightEdit, &QLineEdit::textEdited, this, &EditorObjectImageShapeProperty::textEdited);
	}


	void EditorObjectImageShapeProperty::textEdited(const QString& text)
	{
		m_mainForm->setModified(true);
	}

	void EditorObjectImageShapeProperty::writeData(QTextStream& stream)
	{
		stream << "IMAGESHAPE " << m_left->text() << " " << m_top->text() << " " << m_widthEdit->text() << " " << m_heightEdit->text() << Qt::endl;
	}

	EditorObjectBaddyProperty::EditorObjectBaddyProperty(EditorObject* mainForm) :
		m_mainForm(mainForm)
	{
		this->setTitle("Baddy");
		auto layout = new QHBoxLayout();
		layout->addWidget(new QLabel("This object is a baddy"));
		layout->setSpacing(2);
		layout->setContentsMargins(0, 10, 0, 5);

		this->setLayout(layout);
	}

	void EditorObjectBaddyProperty::writeData(QTextStream& stream)
	{
		stream << "BADDY" << Qt::endl;
	}


	EditorObjectDontEmbedCodeProperty::EditorObjectDontEmbedCodeProperty(EditorObject* mainForm) :
		m_mainForm(mainForm)
	{
		this->setTitle(getName());
		auto layout = new QHBoxLayout();
		layout->addWidget(new QLabel("Do not embed the code in GRAAL\nlevel files"));
		layout->setSpacing(2);
		layout->setContentsMargins(0, 10, 0, 5);

		this->setLayout(layout);
	}

	void EditorObjectDontEmbedCodeProperty::writeData(QTextStream& stream)
	{
		stream << "DONTEMBEDCODE" << Qt::endl;
	}


	EditorObjectRenderModeProperty::EditorObjectRenderModeProperty(EditorObject* mainForm, RenderMode renderMode) :
		m_mainForm(mainForm)
	{
		this->setTitle("Editor Render Mode");
		auto layout = new QHBoxLayout();

		m_renderModeCombo = new QComboBox(this);

		m_renderModeCombo->addItem("Centered", QVariant(RenderMode::Centered));
		m_renderModeCombo->addItem("Stretched", QVariant(RenderMode::Stretched));
		m_renderModeCombo->addItem("Tiled", QVariant(RenderMode::Tiled));
		m_renderModeCombo->addItem("Rect", QVariant(RenderMode::Rect));

		m_renderModeCombo->setCurrentIndex((int)renderMode);
		connect(m_renderModeCombo, &QComboBox::currentIndexChanged, this, &EditorObjectRenderModeProperty::setCurrentIndex);
		layout->addWidget(m_renderModeCombo);
		layout->setSpacing(2);
		layout->setContentsMargins(0, 10, 0, 5);

		this->setLayout(layout);
	}

	void EditorObjectRenderModeProperty::setCurrentIndex(int index)
	{
		m_mainForm->setModified(true);

	}
	void EditorObjectRenderModeProperty::writeData(QTextStream& stream)
	{
		stream << "EDITORRENDERMODE " << m_renderModeCombo->itemText(m_renderModeCombo->currentIndex()) << Qt::endl;
	}


	EditorObjectScriptingLanguageProperty::EditorObjectScriptingLanguageProperty(EditorObject* mainForm, ScriptingLanguage scriptingLanguage) :
		m_mainForm(mainForm)
	{
		this->setTitle("Scripting Language");
		auto layout = new QHBoxLayout();

		m_scriptingLanguageCombo = new QComboBox(this);

		m_scriptingLanguageCombo->addItem("SGScript", QVariant(ScriptingLanguage::SCRIPT_SGSCRIPT));
		m_scriptingLanguageCombo->addItem("GS1Script", QVariant(ScriptingLanguage::SCRIPT_GS1));

		m_scriptingLanguageCombo->setCurrentIndex((int)scriptingLanguage);

		connect(m_scriptingLanguageCombo, &QComboBox::currentIndexChanged, this, &EditorObjectScriptingLanguageProperty::setCurrentIndex);
		layout->addWidget(m_scriptingLanguageCombo);
		layout->setSpacing(2);
		layout->setContentsMargins(0, 10, 0, 5);

		this->setLayout(layout);
	}

	void EditorObjectScriptingLanguageProperty::setCurrentIndex(int index)
	{
		m_mainForm->setModified(true);
		m_mainForm->setScriptingLanguage((ScriptingLanguage)index);
	
	}

	void EditorObjectScriptingLanguageProperty::writeData(QTextStream& stream)
	{
		stream << "SCRIPTINGLANGUAGE " << m_scriptingLanguageCombo->itemText(m_scriptingLanguageCombo->currentIndex()) << Qt::endl;
	}



};