#include <QFileDialog>
#include "ScreenshotDialog.h"
#include "Level.h"
#include "LevelNPC.h"

namespace TilesEditor
{
	QByteArray ScreenshotDialog::savedGeometry;
	ScreenshotDialog::ScreenshotDialog(IWorld* world, QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		this->setWindowFlag(Qt::Window);
		this->setWindowFlag(Qt::WindowMaximizeButtonHint);

		
		m_zoomLevel = ui.zoomLevelSpinBox->value();
		if (m_zoomLevel > 1.0)
			ui.graphicsView->setAntiAlias(false);
		else ui.graphicsView->setAntiAlias(true);


		ui.graphicsView->resetTransform();
		ui.graphicsView->scale(m_zoomLevel, m_zoomLevel);

		m_world = world;

	
		connect(ui.graphicsView, &GraphicsView::renderView, this, &ScreenshotDialog::renderScene);
		connect(ui.zoomLevelSpinBox, &QDoubleSpinBox::valueChanged, this, &ScreenshotDialog::zoomLevelChanged);
		connect(ui.saveFileButton, &QAbstractButton::clicked, this, &ScreenshotDialog::saveFileClicked);
		connect(ui.saveFolderButton, &QAbstractButton::clicked, this, &ScreenshotDialog::saveFolderClicked);
		connect(ui.graphicsView, &GraphicsView::mouseWheelEvent, this, &ScreenshotDialog::graphicsMouseWheel);
		connect(ui.showObjectCheckBox, &QAbstractButton::clicked, ui.graphicsView, &GraphicsView::redraw);
		ui.graphicsView->setSceneRect(0, 0, world->getWidth(), world->getHeight());

		if (!savedGeometry.isNull())
			restoreGeometry(savedGeometry);
	}

	ScreenshotDialog::~ScreenshotDialog()
	{
		savedGeometry = saveGeometry();
	}

	void ScreenshotDialog::graphicsMouseWheel(QWheelEvent* event)
	{
		event->ignore();
	}


	void ScreenshotDialog::zoomLevelChanged(double value)
	{
		m_zoomLevel = value;

		if (m_zoomLevel > 1.0)
			ui.graphicsView->setAntiAlias(false);
		else ui.graphicsView->setAntiAlias(true);


		ui.graphicsView->resetTransform();
		ui.graphicsView->scale(m_zoomLevel, m_zoomLevel);

		ui.graphicsView->redraw();
	}

	void ScreenshotDialog::saveFileClicked(bool checked)
	{
		auto fullPath = QFileDialog::getSaveFileName(nullptr, "Save Image", QString(), "Image Files (*.png *.jpg *jpeg)");

		if (!fullPath.isEmpty())
		{
			QFileInfo fi(fullPath);

			QPixmap output(int(m_world->getWidth() * m_zoomLevel), int(m_world->getHeight() * m_zoomLevel));

		
			QPainter* painter = new QPainter(&output);


			painter->setRenderHint(QPainter::SmoothPixmapTransform, m_zoomLevel <= 1);

			painter->scale(m_zoomLevel, m_zoomLevel);
			renderScene(painter, QRectF(0, 0, m_world->getWidth(), m_world->getHeight()));

			delete painter;

			QFile file(fullPath);
			file.open(QIODevice::WriteOnly);
			output.save(&file);
		}
	}

	void ScreenshotDialog::saveFolderClicked(bool checked)
	{
		auto folder = QFileDialog::getExistingDirectory(nullptr, "Save Images");
		if (!folder.isEmpty())
		{
			auto viewRect = Rectangle(0, 0, m_world->getWidth(), m_world->getHeight());

			auto levels = m_world->getLevelsInRect(viewRect);

			for (auto level : levels)
			{
				QPixmap output(int(level->getWidth() * m_zoomLevel), int(level->getHeight() * m_zoomLevel));

				QPainter* painter = new QPainter(&output);

				painter->setRenderHint(QPainter::SmoothPixmapTransform, m_zoomLevel <= 1);
				painter->scale(m_zoomLevel, m_zoomLevel);

				painter->translate(-level->getX(), -level->getY());
				//

				auto& layers = level->getTileLayers();

				for (auto tilemap : layers)
				{
					tilemap->draw(painter, viewRect, m_world->getTilesetImage(), tilemap->getX(), tilemap->getY());
				}

				if (ui.showObjectCheckBox->isChecked())
				{
					auto& entities = level->getObjects();
					for (auto entity : entities)
					{
						if (entity->getEntityType() == LevelEntityType::ENTITY_NPC)
						{
							auto npc = static_cast<LevelNPC*>(entity);


							entity->loadResources(m_world->getResourceManager());
							if (npc->hasValidImage())
							{
								entity->draw(painter, viewRect);
							}

						}
					}
				}
				delete painter;

				QFileInfo fi(level->getFileName());
				
				auto fileName = fi.completeBaseName() + ".png";

				auto fullPath = folder + "/" + fileName;
				QFile file(fullPath);
				file.open(QIODevice::WriteOnly);
				output.save(&file);

			}
		}
	}


	void ScreenshotDialog::renderScene(QPainter* painter, const QRectF& rect)
	{
		Rectangle viewRect(rect.x(), rect.y(), rect.width(), rect.height());

		painter->fillRect(rect, QColorConstants::Svg::black);


		auto levels = m_world->getLevelsInRect(viewRect);

	
		for (auto level : levels)
		{
			auto& layers = level->getTileLayers();

			for (auto tilemap : layers)
			{
				tilemap->draw(painter, viewRect, m_world->getTilesetImage(), tilemap->getX(), tilemap->getY());
			}
		}

		if (ui.showObjectCheckBox->isChecked())
		{
			auto entities = m_world->getEntitiesInRect(viewRect);

			for (auto entity : entities)
			{
				if (entity->getEntityType() == LevelEntityType::ENTITY_NPC)
				{
					auto npc = static_cast<LevelNPC*>(entity);


					entity->loadResources(m_world->getResourceManager());
					if (npc->hasValidImage())
					{
						entity->draw(painter, viewRect);
					}

				}
			}

		}
		


	}
};
