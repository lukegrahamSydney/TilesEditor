#include "ListLinksDialog.h"
#include "EditLinkDialog.h"

namespace TilesEditor
{


	ListLinksDialog::ListLinksDialog(Level* level, IWorld* world, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		m_level = level;
		m_world = world;


		connect(ui.deleteButton, &QAbstractButton::clicked, this, &ListLinksDialog::deleteClicked);
		connect(ui.editButton, &QAbstractButton::clicked, this, &ListLinksDialog::editClicked);

		this->setWindowTitle("Edit Links: " + level->getName());
		populateTable();

	}

	ListLinksDialog::~ListLinksDialog()
	{}

	void ListLinksDialog::populateTable()
	{
		while (ui.linksTable->rowCount() > 0)
			ui.linksTable->removeRow(ui.linksTable->rowCount() - 1);

		auto& links = m_level->getLinks();

		for (auto link : links)
		{
			auto newIndex = ui.linksTable->rowCount();
			ui.linksTable->insertRow(newIndex);

			ui.linksTable->setItem(newIndex, 0, new QTableWidgetItem(QString::number(link->getX())));
			ui.linksTable->setItem(newIndex, 1, new QTableWidgetItem(QString::number(link->getY())));
			ui.linksTable->setItem(newIndex, 2, new QTableWidgetItem(QString::number(link->getWidth())));
			ui.linksTable->setItem(newIndex, 3, new QTableWidgetItem(QString::number(link->getHeight())));
			ui.linksTable->setItem(newIndex, 4, new QTableWidgetItem(link->getNextX()));
			ui.linksTable->setItem(newIndex, 5, new QTableWidgetItem(link->getNextY()));
			ui.linksTable->setItem(newIndex, 6, new QTableWidgetItem(link->getNextLevel()));
		}
	}

	void ListLinksDialog::deleteClicked(bool)
	{
		if (ui.linksTable->selectedItems().count() > 0)
		{
			auto selectedRow = ui.linksTable->currentRow();

			auto& links = m_level->getLinks();
			if (selectedRow >= 0 && selectedRow < links.size())
			{
				auto link = links.at(selectedRow);
				if (link != nullptr)
				{
					m_level->removeObject(link);
					delete link;
					m_world->setModified(m_level);
					populateTable();
					
				}
			}
		}
	}

	void ListLinksDialog::editClicked(bool)
	{
		if (ui.linksTable->selectedItems().count() > 0)
		{
			auto selectedRow = ui.linksTable->currentRow();

			auto& links = m_level->getLinks();
			if (selectedRow >= 0 && selectedRow < links.size())
			{
				auto link = links.at(selectedRow);

				if (link)
				{
					EditLinkDialog frm(link);
					if (frm.exec() == -1)
					{
						m_level->removeObject(link);
						delete link;
						m_world->setModified(m_level);
					}else if (frm.getModified())
					{
						m_world->setModified(m_level);
					}


					populateTable();
				}
			}
		}
	}
};
