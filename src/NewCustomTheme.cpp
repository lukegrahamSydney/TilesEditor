#include "NewCustomTheme.h"
#include "CustomListWidgetItem.h"
#include "QPushButton"

namespace TilesEditor
{

	NewCustomTheme::NewCustomTheme(QMenu& menu, QActionGroup* actionGroup, QWidget* parent)
		: QDialog(parent), m_menu(menu), m_menuGroup(actionGroup)
	{
		ui.setupUi(this);

		connect(ui.listWidgetThemes, &QListWidget::itemSelectionChanged, this, &NewCustomTheme::themesItemSelectionChanged);
		connect(ui.buttonDelete, &QPushButton::pressed, this, &NewCustomTheme::themeDeletePressed);
		connect(ui.buttonNew, &QPushButton::pressed, this, &NewCustomTheme::themeNewPressed);
		connect(ui.nameLineEdit, &QLineEdit::textChanged, this, &NewCustomTheme::themeNameChanged);
		connect(ui.darkThemeCheckBox, &QCheckBox::checkStateChanged, this, &NewCustomTheme::themeDarkModeChanged);
	}

	NewCustomTheme::~NewCustomTheme()
	{
		while (ui.listWidgetThemes->count())
		{
			delete ui.listWidgetThemes->takeItem(0);
		}
	}

	void NewCustomTheme::themeNewPressed()
	{
		auto theme = new GenericTheme("<new>", "", false);
		theme->setCheckable(true);
		auto widget = new CustomListWidgetItem<GenericTheme>();
		widget->setUserPointer(theme);
		widget->setText(theme->text());
		ui.listWidgetThemes->addItem(widget);

		auto lastAction = m_menu.actions().last();
		m_menu.insertAction(lastAction, theme);
		m_menuGroup->addAction(theme);
	}

	void NewCustomTheme::themeDeletePressed()
	{
		auto widget = static_cast<CustomListWidgetItem<GenericTheme>*>(ui.listWidgetThemes->takeItem(ui.listWidgetThemes->currentRow()));

		if (widget != nullptr)
		{
			
			auto theme = widget->getUserPointer();
			theme->deleteLater();
			delete widget;
			
		}

	}

	void NewCustomTheme::themeNameChanged(const QString& text)
	{
		auto widget = static_cast<CustomListWidgetItem<GenericTheme>*>(ui.listWidgetThemes->currentItem());
		if (widget != nullptr)
		{
			auto theme = widget->getUserPointer();
			theme->setText(text);
			widget->setText(text);
		}
	}

	void NewCustomTheme::themeStyleSheetChanged(const QString& text)
	{
		auto widget = static_cast<CustomListWidgetItem<GenericTheme>*>(ui.listWidgetThemes->currentItem());
		if (widget != nullptr)
		{
			auto theme = widget->getUserPointer();
			theme->setResourceName(text);
		}
	}

	void NewCustomTheme::themeDarkModeChanged(Qt::CheckState state)
	{
		auto widget = static_cast<CustomListWidgetItem<GenericTheme>*>(ui.listWidgetThemes->currentItem());
		if (widget != nullptr)
		{
			auto theme = widget->getUserPointer();
			theme->setDarkTheme(state == Qt::CheckState::Checked);
		}
	}

	void NewCustomTheme::addTheme(GenericTheme * theme)
	{
		auto widgetItem = new CustomListWidgetItem<GenericTheme>();
		widgetItem->setUserPointer(theme);
		widgetItem->setText(theme->text());

		ui.listWidgetThemes->addItem(widgetItem);
	}

	void NewCustomTheme::themesItemSelectionChanged()
	{
		auto widget = static_cast<CustomListWidgetItem<GenericTheme>*>(ui.listWidgetThemes->currentItem());
		if (widget != nullptr)
		{
			auto theme = widget->getUserPointer();

			ui.nameLineEdit->setText(theme->text());
			ui.styleSheetLineEdit->setText(theme->getResourceName());
			ui.darkThemeCheckBox->setChecked(theme->isDarkTheme());
		}
		else {
			ui.nameLineEdit->setText("");
			ui.styleSheetLineEdit->setText("");
			ui.darkThemeCheckBox->setChecked(false);
		}
	}
};
