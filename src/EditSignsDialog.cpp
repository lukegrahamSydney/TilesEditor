#include <QSignalBlocker>
#include "EditSignsDialog.h"
#include "LevelCommands.h"

namespace TilesEditor
{
	EditSignsDialog::EditSignsDialog(Level* level, IWorld* world, LevelSign* selectedSign, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		m_level = level;
		m_world = world;
		this->setWindowTitle("Edit Signs: " + level->getName());

		

		connect(ui.createButton, &QAbstractButton::clicked, this, &EditSignsDialog::createSignClicked);
		connect(ui.deleteButton, &QAbstractButton::clicked, this, &EditSignsDialog::deleteSignClicked);
		connect(ui.signsTable, &QTableWidget::currentItemChanged, this, &EditSignsDialog::signSelectionChanged);

		{
			const QSignalBlocker a(ui.signsTable);


			auto& signs = m_level->getSigns();

			int index = 0;
			for (auto sign : signs)
			{
				auto newIndex = ui.signsTable->rowCount();
				ui.signsTable->insertRow(newIndex);


				ui.signsTable->setItem(newIndex, 0, new QTableWidgetItem());
				ui.signsTable->setItem(newIndex, 1, new QTableWidgetItem());
				ui.signsTable->setItem(newIndex, 2, new QTableWidgetItem());

				updateRow(newIndex);
			}

			updateVerticalHeader();
		}

		if (selectedSign != nullptr)
		{
			auto signIndex = m_level->getSigns().indexOf(selectedSign);
			if (signIndex >= 0)
				ui.signsTable->selectRow(signIndex);
		} else ui.signsTable->selectRow(0);

		if (!savedGeometry.isNull())
			this->restoreGeometry(savedGeometry);
	}

	EditSignsDialog::~EditSignsDialog()
	{
		savedGeometry = this->saveGeometry();
	}

	void EditSignsDialog::xEditChanged(const QString& text)
	{
		//if (ui.signsTable->selectionModel()->hasSelection())
		//{
		//	m_world->setModified(m_level);

		//	updateSelectedSign();

		//	auto selectedRow = ui.signsTable->currentRow();
		//	if (selectedRow >= 0)
		//		updateRow(selectedRow);
		//}
		
	}

	void EditSignsDialog::yEditChanged(const QString& text)
	{
		if (ui.signsTable->selectionModel()->hasSelection())
		{
			//m_world->setModified(m_level);

			//updateSelectedSign();

			//auto selectedRow = ui.signsTable->currentRow();
			//if (selectedRow >= 0)
			//	updateRow(selectedRow);
		}
	}

	void EditSignsDialog::signTextEditFinished()
	{
		if (ui.signsTable->selectionModel()->hasSelection())
		{
			//m_world->setModified(m_level);
			//updateSelectedSign();

			//auto selectedRow = ui.signsTable->currentRow();
			//if (selectedRow >= 0)
			//	updateRow(selectedRow);
		}

	}

	void EditSignsDialog::accept()
	{
		updateSign(ui.signsTable->currentRow());
		QDialog::accept();
	}

	void EditSignsDialog::updateSign(int index)
	{
		if (index >= 0)
		{
			auto sign = m_level->getSigns().at(index);
			if (sign)
			{
				auto undoCommand = new QUndoCommand("Modify Sign");
				auto newX = ui.xSpinBox->value() * m_level->getUnitWidth();
				auto newY = ui.ySpinBox->value() * m_level->getUnitHeight();

				if (newX != sign->getX() || newY != sign->getY())
				{
					QMap<AbstractLevelEntity*, QRectF> oldPosition = { {sign, sign->toQRectF()} };
					QMap<AbstractLevelEntity*, QRectF> newPosition = { {sign, QRectF(newX, newY, sign->getWidth(), sign->getHeight())}};

					new CommandMoveEntities(m_world, oldPosition, newPosition, undoCommand);
				}

				if (sign->getText() != ui.signtext->toPlainText())
					new CommandSetEntityProperty(m_world, sign, "text", ui.signtext->toPlainText(), sign->getText(), undoCommand);
				
				if (undoCommand->childCount())
					m_world->addUndoCommand(undoCommand);
				else delete undoCommand;
				
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

	void EditSignsDialog::updateVerticalHeader()
	{
		QStringList labels;
		for (auto index = 0; index < ui.signsTable->rowCount(); ++index)
		{
			labels.push_back(QString::number(index));
		}
		ui.signsTable->setVerticalHeaderLabels(labels);
	}


	void EditSignsDialog::signSelectionChanged(QTableWidgetItem* current, QTableWidgetItem* previous)
	{
		if (previous)
		{
			auto previousRow = previous->row();
			updateSign(previousRow);
			updateRow(previousRow);
		}

		if (current)
		{
			auto& signs = m_level->getSigns();
			auto row = current->row();
			if (row >= 0 && row < signs.size())
			{
				auto sign = signs[row];

				ui.xSpinBox->setValue(sign->getX() / m_level->getUnitWidth());
				ui.ySpinBox->setValue(sign->getY() / m_level->getUnitHeight());

				ui.signtext->setPlainText(sign->getText());
			}
		}
		else {
			ui.xSpinBox->setValue(m_level->getX() / m_level->getUnitWidth());
			ui.ySpinBox->setValue(m_level->getY() / m_level->getUnitHeight());
			ui.signtext->setPlainText("");
		}
	}



	void EditSignsDialog::createSignClicked(bool checked)
	{
		auto signX = m_level->getX();
		auto signY = m_level->getY();

		auto sign = new LevelSign(m_world, signX, signY, 32, 16);
		sign->setLevel(m_level);

		m_world->addUndoCommand(new CommandAddEntity(m_world, sign));

		auto newIndex = ui.signsTable->rowCount();
		ui.signsTable->insertRow(newIndex);
		ui.signsTable->setItem(newIndex, 0, new QTableWidgetItem());
		ui.signsTable->setItem(newIndex, 1, new QTableWidgetItem());
		ui.signsTable->setItem(newIndex, 2, new QTableWidgetItem());

		updateRow(newIndex);
		ui.signsTable->selectRow(ui.signsTable->rowCount() - 1);

		updateVerticalHeader();

	}

	void EditSignsDialog::deleteSignClicked(bool checked)
	{

		const QSignalBlocker a(ui.signsTable);
		auto& signs = m_level->getSigns();
		auto row = ui.signsTable->currentRow();
		if (row >= 0 && row < signs.size())
		{
			auto sign = signs[row];

			m_world->deleteEntity(sign);
			ui.signsTable->removeRow(row);

			auto currentItem = ui.signsTable->currentItem();
			signSelectionChanged(currentItem, nullptr);
		}
		updateVerticalHeader();
	}

};
