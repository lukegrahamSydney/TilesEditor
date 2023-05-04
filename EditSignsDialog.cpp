#include <QDoubleValidator>
#include "EditSignsDialog.h"

namespace TilesEditor
{
	EditSignsDialog::EditSignsDialog(Level* level, IWorld* world, LevelSign* selectedSign, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		m_level = level;
		m_world = world;
		this->setWindowTitle("Edit Signs: " + level->getName());

		ui.xEdit->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, this));
		ui.yEdit->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, this));
		
		connect(ui.xEdit, &QLineEdit::textEdited, this, &EditSignsDialog::xEditChanged);
		connect(ui.yEdit, &QLineEdit::textEdited, this, &EditSignsDialog::yEditChanged);
		connect(ui.signtext, &QPlainTextEdit::textChanged, this, &EditSignsDialog::signTextEditFinished);
		connect(ui.createButton, &QAbstractButton::clicked, this, &EditSignsDialog::createSignClicked);
		connect(ui.deleteButton, &QAbstractButton::clicked, this, &EditSignsDialog::deleteSignClicked);
		connect(ui.signsTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EditSignsDialog::signSelectionChanged);
		populateTable();

		if (selectedSign != nullptr)
		{
			auto signIndex = m_level->getSigns().indexOf(selectedSign);
			if (signIndex >= 0)
				ui.signsTable->selectRow(signIndex);
		} else ui.signsTable->selectRow(0);
	}

	EditSignsDialog::~EditSignsDialog()
	{}

	void EditSignsDialog::xEditChanged(const QString& text)
	{
		if (ui.signsTable->selectionModel()->hasSelection())
		{
			m_world->setModified(m_level);

			updateSelectedSign();

			auto selectedRow = ui.signsTable->currentRow();
			if (selectedRow >= 0)
				updateRow(selectedRow);
		}
		
	}

	void EditSignsDialog::yEditChanged(const QString& text)
	{
		if (ui.signsTable->selectionModel()->hasSelection())
		{
			m_world->setModified(m_level);

			updateSelectedSign();

			auto selectedRow = ui.signsTable->currentRow();
			if (selectedRow >= 0)
				updateRow(selectedRow);
		}
	}

	void EditSignsDialog::signTextEditFinished()
	{
		if (ui.signsTable->selectionModel()->hasSelection())
		{
			m_world->setModified(m_level);
			updateSelectedSign();

			auto selectedRow = ui.signsTable->currentRow();
			if (selectedRow >= 0)
				updateRow(selectedRow);
		}

	}

	void EditSignsDialog::updateSelectedSign()
	{
		auto selectedRow = ui.signsTable->currentRow();
		if (selectedRow >= 0)
		{
			auto sign = m_level->getSigns().at(selectedRow);
			if (sign)
			{
				sign->setX(ui.xEdit->text().toDouble() * m_level->getUnitWidth());
				sign->setY(ui.yEdit->text().toDouble() * m_level->getUnitHeight());

				sign->setText(ui.signtext->toPlainText());

				m_level->updateSpatialEntity(sign);
				
			}
		}
	}

	void EditSignsDialog::updateRow(int row)
	{
		auto& signs = m_level->getSigns();

		if (row >= 0 && row < signs.count())
		{
			auto sign = signs[row];

			ui.signsTable->item(row, 0)->setText(QString::number(sign->getX() / m_level->getUnitWidth()));
			ui.signsTable->item(row, 1)->setText(QString::number(sign->getY() / m_level->getUnitHeight()));
			ui.signsTable->item(row, 2)->setText(QString(sign->getText()).replace(QChar('\n'), QChar('\t')));

		}
	}

	void EditSignsDialog::populateTable()
	{
		while (ui.signsTable->rowCount() > 0)
			ui.signsTable->removeRow(ui.signsTable->rowCount() - 1);

		auto& signs = m_level->getSigns();

		QStringList labels;
		int index = 0;
		for (auto sign : signs)
		{
			auto newIndex = ui.signsTable->rowCount();
			ui.signsTable->insertRow(newIndex);


			ui.signsTable->setItem(newIndex, 0, new QTableWidgetItem());
			ui.signsTable->setItem(newIndex, 1, new QTableWidgetItem());
			ui.signsTable->setItem(newIndex, 2, new QTableWidgetItem());

			updateRow(newIndex);

			labels.push_back(QString::number(index++));
		}
		ui.signsTable->setVerticalHeaderLabels(labels);
	}

	void EditSignsDialog::signSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
	{
		auto& signs = m_level->getSigns();
		auto row = ui.signsTable->currentRow();
		if (row >= 0 && row < signs.size())
		{
			auto sign = signs[row];

			ui.xEdit->setText(QString::number(sign->getX() / m_level->getUnitWidth()));
			ui.yEdit->setText(QString::number(sign->getY() / m_level->getUnitHeight()));

			ui.signtext->blockSignals(true);
			ui.signtext->setPlainText(sign->getText());
			ui.signtext->blockSignals(false);
		}
	}

	void EditSignsDialog::createSignClicked(bool checked)
	{
		auto signX = m_level->getX();
		auto signY = m_level->getY();

		auto sign = new LevelSign(m_level, signX, signY, 32, 16);
		m_level->addObject(sign);

		m_world->setModified(m_level);
		populateTable();
		ui.signsTable->selectRow(ui.signsTable->rowCount() - 1);

	}

	void EditSignsDialog::deleteSignClicked(bool checked)
	{
		auto& signs = m_level->getSigns();
		auto row = ui.signsTable->currentRow();
		if (row >= 0 && row < signs.size())
		{
			auto sign = signs[row];

			m_world->deleteEntity(sign);
			populateTable();
		}
	}

};
