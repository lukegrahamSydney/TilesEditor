#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QPair>
#include <QStack>
#include <algorithm>

#include "EditorTabWidget.h"
#include "GraphicsView.h"
#include "TileSelection.h"
#include "ObjectSelection.h"
#include "EditAnonymousNPC.h"
#include "EditLinkDialog.h"
#include "cJSON/JsonHelper.h"
#include "ListLinksDialog.h"
#include "SaveOverworldDialog.h"
#include "EditSignsDialog.h"
#include "ObjectListModel.h"
#include "LevelCommands.h"
#include "EditTilesetDialog.h"
#include "ScreenshotDialog.h"
#include "LevelChest.h"
#include "LevelGraalBaddy.h"
#include "FileFormatManager.h"

namespace TilesEditor
{

	EditorTabWidget::EditorTabWidget(QWidget* parent, AbstractFileSystem* fileSystem)
		: QWidget(parent), m_fillPattern(nullptr, 0.0, 0.0, 1, 1, 0), m_resourceManager(fileSystem)
	{
		ui.setupUi(this);

		m_font1.setFamily("Arial");
		m_font1.setPointSizeF(12);

		//m_useFillPattern = false;
		m_tilesetsContainer = new QWidget();
		m_tileObjectsContainer = new QWidget();
		m_objectsContainer = new QWidget();

		ui_tilesetsClass.setupUi(m_tilesetsContainer);
		ui_tileObjectsClass.setupUi(m_tileObjectsContainer);
		ui_objectClass.setupUi(m_objectsContainer);

		m_modified = false;

		//QPixmap a(":/MainWindow/icons/npc.png");

		//m_resourceManager.addPersistantResource(new Image("__blankNPC", a));

		m_eyeOpen = QPixmap(":/MainWindow/icons/fugue/eye.png");
		m_eyeClosed = QPixmap(":/MainWindow/icons/fugue/eye-close.png");

		m_selection = nullptr;

		setDefaultTile(Tilemap::MakeTile(0, 0, 0));

		m_selectedTilesLayer = 0;
		m_selectedLayerVisible = true;
		m_visibleLayers[0] = true;

		auto graphicsContainer = ui.graphicsContainer;
		m_graphicsView = new GraphicsView();
		//m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
		graphicsContainer->layout()->addWidget(m_graphicsView);

		m_level = nullptr;
		m_overworld = nullptr;

		m_tilesetImage = nullptr;
		m_graphicsView->setSceneRect(QRect(0, 0, 64 * 16, 64 * 16));

		ui_objectClass.objectsTable->setModel(new ObjectListModel());
		ui_objectClass.objectsTable->setColumnWidth(2, 70);

		connect(ui_objectClass.newNPCButton, &QAbstractButton::clicked, this, &EditorTabWidget::objectsNewNPCClicked);
		connect(ui_objectClass.newChestButton, &QAbstractButton::clicked, this, &EditorTabWidget::objectsNewChestClicked);
		connect(ui_objectClass.newBaddyButton, &QAbstractButton::clicked, this, &EditorTabWidget::objectsNewBaddyClicked);

		connect(ui_objectClass.refreshButton, &QAbstractButton::clicked, this, &EditorTabWidget::objectsRefreshClicked);
		connect(ui_objectClass.objectsTable, &QTableView::doubleClicked, this, &EditorTabWidget::objectsDoubleClick);

		

		connect(ui_tilesetsClass.tileIcon, &CustomPaintWidget::paint, this, &EditorTabWidget::paintDefaultTile);
		connect(ui_tilesetsClass.graphicsView, &GraphicsView::renderView, this, &EditorTabWidget::renderTilesetSelection);
		connect(ui_tilesetsClass.graphicsView, &GraphicsView::mousePress, this, &EditorTabWidget::tilesetMousePress);
		connect(ui_tilesetsClass.graphicsView, &GraphicsView::mouseRelease, this, &EditorTabWidget::tilesetMouseRelease);
		connect(ui_tilesetsClass.graphicsView, &GraphicsView::mouseMove, this, &EditorTabWidget::tilesetMouseMove);
		connect(ui_tilesetsClass.tilesetsCombo, &QComboBox::currentIndexChanged, this, &EditorTabWidget::tilesetsIndexChanged);


		connect(ui_tilesetsClass.deleteButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetDeleteClicked);
		connect(ui_tilesetsClass.refreshButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetRefreshClicked);
		connect(ui_tilesetsClass.newButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetNewClicked);
		connect(ui_tilesetsClass.openButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetOpenClicked);
		connect(ui_tilesetsClass.editButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetEditClicked);

		connect(ui_tileObjectsClass.graphicsView, &GraphicsView::renderView, this, &EditorTabWidget::renderTileObjects);
		connect(ui_tileObjectsClass.graphicsView, &GraphicsView::mousePress, this, &EditorTabWidget::tileObjectsMousePress);

		connect(ui_tileObjectsClass.newGroupButton, &QAbstractButton::clicked, this, &EditorTabWidget::tileGroupNewClicked);
		connect(ui_tileObjectsClass.deleteGroupButton, &QAbstractButton::clicked, this, &EditorTabWidget::tileGroupDeleteClicked);


		connect(ui_tileObjectsClass.newObjectButton, &QAbstractButton::clicked, this, &EditorTabWidget::tileObjectNewClicked);
		connect(ui_tileObjectsClass.deleteObjectButton, &QAbstractButton::clicked, this, &EditorTabWidget::tileObjectDeleteClicked);

		connect(ui_tileObjectsClass.importButton, &QAbstractButton::clicked, this, &EditorTabWidget::tileGroupImportClicked);
		connect(ui_tileObjectsClass.groupCombo, &QComboBox::currentIndexChanged, this, &EditorTabWidget::tileGroupIndexChanged);
		connect(ui_tileObjectsClass.objectsCombo, &QComboBox::currentIndexChanged, this, &EditorTabWidget::tileObjectIndexChanged);



		connect(m_graphicsView, &GraphicsView::renderView, this, &EditorTabWidget::renderScene);
		connect(m_graphicsView, &GraphicsView::mousePress, this, &EditorTabWidget::graphicsMousePress);
		connect(m_graphicsView, &GraphicsView::mouseRelease, this, &EditorTabWidget::graphicsMouseRelease);
		connect(m_graphicsView, &GraphicsView::mouseMove, this, &EditorTabWidget::graphicsMouseMove);
		connect(m_graphicsView, &GraphicsView::mouseDoubleClick, this, &EditorTabWidget::graphicsMouseDoubleClick);
		connect(m_graphicsView, &GraphicsView::mouseWheelEvent, this, &EditorTabWidget::graphicsMouseWheel);
		connect(m_graphicsView, &GraphicsView::keyPress, this, &EditorTabWidget::graphicsKeyPress);

		connect(ui.zoomSlider, &QSlider::valueChanged, this, &EditorTabWidget::zoomMoved);
		connect(ui.layerWidget, &QSpinBox::valueChanged, this, &EditorTabWidget::layerChanged);
		connect(ui.showLayerButton, &QPushButton::pressed, this, &EditorTabWidget::layerVisibilityChanged);


		connect(ui.newLinkButton, &QToolButton::clicked, this, &EditorTabWidget::newLinkClicked);
		connect(ui.newSignButton, &QToolButton::clicked, this, &EditorTabWidget::newSignClicked);

		connect(ui.floodFillPatternButton, &QToolButton::clicked, this, &EditorTabWidget::floodFillPatternClicked);
		connect(ui.preloadButton, &QToolButton::clicked, this, &EditorTabWidget::preloadOverworldClicked);
		connect(ui.saveAsButton, &QToolButton::clicked, this, &EditorTabWidget::saveAsClicked);
		connect(ui.saveButton, &QToolButton::clicked, this, &EditorTabWidget::saveClicked);
		connect(ui.editLinksButton, &QToolButton::clicked, this, &EditorTabWidget::editLinksClicked);

		connect(ui.editSignsButton, &QToolButton::clicked, this, &EditorTabWidget::editSignsClicked);
		connect(ui.undoButton, &QToolButton::clicked, this, &EditorTabWidget::undoClicked);
		connect(ui.redoButton, &QToolButton::clicked, this, &EditorTabWidget::redoClicked);
		connect(ui.cutButton, &QToolButton::pressed, this, &EditorTabWidget::cutPressed);
		connect(ui.copyButton, &QToolButton::pressed, this, &EditorTabWidget::copyPressed);
		connect(ui.pasteButton, &QToolButton::pressed, this, &EditorTabWidget::pastePressed);
		connect(ui.deleteButton, &QToolButton::clicked, this, &EditorTabWidget::deleteClicked);
		connect(ui.screenshotButton, &QToolButton::clicked, this, &EditorTabWidget::screenshotClicked);


		connect(ui.hcountSpinBox, &QSpinBox::valueChanged, this, &EditorTabWidget::gridValueChanged);
		connect(ui.vcountSpinBox, &QSpinBox::valueChanged, this, &EditorTabWidget::gridValueChanged);
		connect(ui.gridButton, &QToolButton::released, m_graphicsView, &GraphicsView::redraw);

		//connect(ui.toolButton, &QToolButton::clicked, this, &EditorTabWidget::test);
		auto selectMenu = new QMenu();
		m_selectNPCs = selectMenu->addAction("Npcs");
		m_selectNPCs->setCheckable(true);
		m_selectNPCs->setChecked(true);

		m_selectLinks = selectMenu->addAction("Links");
		m_selectLinks->setCheckable(true);
		m_selectLinks->setChecked(true);

		m_selectSigns = selectMenu->addAction("Signs");
		m_selectSigns->setCheckable(true);
		m_selectSigns->setChecked(true);

		ui.selectionButton->setMenu(selectMenu);

		auto visibleObjectsMenu = new QMenu();
		m_showNPCs = visibleObjectsMenu->addAction("Npcs");
		m_showNPCs->setCheckable(true);
		m_showNPCs->setChecked(true);
		connect(m_showNPCs, &QAction::triggered, m_graphicsView, &GraphicsView::redraw);

		m_showLinks = visibleObjectsMenu->addAction("Links");
		m_showLinks->setCheckable(true);
		m_showLinks->setChecked(true);
		connect(m_showLinks, &QAction::triggered, m_graphicsView, &GraphicsView::redraw);

		m_showSigns = visibleObjectsMenu->addAction("Signs");
		m_showSigns->setCheckable(true);
		m_showSigns->setChecked(true);
		connect(m_showSigns, &QAction::triggered, m_graphicsView, &GraphicsView::redraw);

		ui.visibleObjectsButton->setMenu(visibleObjectsMenu);



		auto functionsMenu = new QMenu();
		auto deleteEdgeLinks = functionsMenu->addAction("Delete Edge Links");
		connect(deleteEdgeLinks, &QAction::triggered, this, &EditorTabWidget::deleteEdgeLinksClicked);

		auto trimScriptEndings = functionsMenu->addAction("Trim Script Endings");
		connect(trimScriptEndings, &QAction::triggered, this, &EditorTabWidget::trimScriptEndingsClicked);

		auto trimSignEndings = functionsMenu->addAction("Trim Sign Endings");
		connect(trimSignEndings, &QAction::triggered, this, &EditorTabWidget::trimSignEndingsClicked);
		ui.functionsButton->setMenu(functionsMenu);

		m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		m_graphicsView->setCornerWidget(ui_tilesetsClass.tileIcon);
	}

	EditorTabWidget::~EditorTabWidget()
	{
		m_objectsContainer->deleteLater();
		m_tileObjectsContainer->deleteLater();
		m_tilesetsContainer->deleteLater();

		if (m_tilesetImage)
		{
			m_resourceManager.freeResource(m_tilesetImage);
		}

		if (m_selection) {
			m_selection->release(m_resourceManager);
			delete m_selection;
		}

		if (m_overworld)
		{
			m_overworld->release();
			delete m_overworld;
		}

		if (m_level)
		{
			m_level->release();
			delete m_level;
		}

		m_resourceManager.getFileSystem()->removeListener(this);

	}

	void EditorTabWidget::renderScene(QPainter * painter, const QRectF & _rect)
	{
		QRectF rect(std::floor(_rect.x()), std::floor(_rect.y()), _rect.width() + 1, _rect.height() + 1);


		Rectangle viewRect(rect.x(), rect.y(), rect.width(), rect.height());
	
		//forcing the view x/y offset as a whole number prevents tile alignment errors
		auto transform = painter->transform();
		QTransform newTransform(transform.m11(), transform.m12(), transform.m21(), transform.m22(), std::floor(transform.dx()), std::floor(transform.dy()));
		painter->setTransform(newTransform);

		painter->fillRect(rect, QColorConstants::Black);




		auto mousePos = m_graphicsView->mapToScene(m_graphicsView->mapFromGlobal(QCursor::pos()));

		auto drawLevels = getLevelsInRect(viewRect);

		if (m_tilesetImage)
		{
			//Draw tiles (and make sure level is loaded)
			for (auto level : drawLevels)
			{

				auto& layers = level->getTileLayers();

				for (auto tilemap : layers)
				{
					if (m_visibleLayers.find(tilemap->getLayerIndex()) == m_visibleLayers.end() || m_visibleLayers[tilemap->getLayerIndex()])
					{
						auto fade = m_selectedTilesLayer != tilemap->getLayerIndex();
						if (fade)
						{
							painter->setOpacity(0.33);
							tilemap->draw(painter, viewRect, m_tilesetImage, tilemap->getX(), tilemap->getY());
							painter->setOpacity(1.0);
						}
						else {
							tilemap->draw(painter, viewRect, m_tilesetImage, tilemap->getX(), tilemap->getY());

							if (tilemap->getLayerIndex() == m_selectedTilesLayer && ui.floodFillButton->isChecked())
							{

								auto viewLevels = getLevelsInRect(viewRect);

								QSet<int> startTiles;
								QSet<QPair<int, int>> scannedIndexes;
								auto x = mousePos.x();
								auto y = mousePos.y();

								auto startTileX = int(std::floor(x / 16));
								auto startTileY = int(std::floor(y / 16));
								auto purpleSquareRect = Rectangle(startTileX * 16, startTileY * 16, m_fillPattern.getWidth(), m_fillPattern.getHeight());

								auto startTileX2 = (int)std::ceil(double(startTileX) / m_fillPattern.getHCount()) * m_fillPattern.getHCount();
								auto startTileY2 = (int)std::ceil(double(startTileY) / m_fillPattern.getVCount()) * m_fillPattern.getVCount();

								//Get a set of tiles that can be replaced
								for (auto y = 0; y < m_fillPattern.getVCount(); ++y)
								{
									for (auto x = 0; x < m_fillPattern.getHCount(); ++x)
									{
										int tile = 0;
										if (tryGetTileAt((startTileX + x) * 16, (startTileY + y) * 16, &tile))
											startTiles.insert(tile);
									}
								}

								QStack<QPair<int, int> > nodes;
								auto addNode = [&](int x, int y)
								{
									QPair<int, int> a(x, y);

									if (!scannedIndexes.contains(a))
									{
										scannedIndexes.insert(a);
										nodes.push(a);
									}

								};
								addNode(startTileX, startTileY);

								Level* level = nullptr;
								while (nodes.count() > 0)
								{
									auto node = nodes.pop();

									auto nodeXPos = node.first * 16.0;
									auto nodeYPos = node.second * 16.0;

									if (level == nullptr || nodeXPos < level->getX() || nodeXPos >= level->getRight() || nodeYPos < level->getY() || nodeYPos >= level->getBottom())
									{
										level = getLevelAt(nodeXPos, nodeYPos);
										if (!viewLevels.contains(level))
										{
											level = nullptr;
											continue;
										}
									}
										

									if (level != nullptr)
									{
										auto tilemap = level->getTilemap(m_selectedTilesLayer);
										if (tilemap != nullptr)
										{
											//left/top position within the destination "Tilemap"
											auto tilemapX = node.first - int(std::floor(tilemap->getX() / 16.0));
											auto tilemapY = node.second - int(std::floor(tilemap->getY() / 16.0));

											//Get the correct pattern co-ordinates
											auto deltaX = startTileX2 + (node.first - startTileX);
											auto deltaY = startTileY2 + (node.second - startTileY);

											auto patternTileX = deltaX % m_fillPattern.getHCount();
											auto patternTileY = deltaY % m_fillPattern.getVCount();


											auto patternTile = m_fillPattern.getTile(patternTileX, patternTileY);
											int tile = 0;
											if (tilemap->tryGetTile(tilemapX, tilemapY, &tile) && !Tilemap::IsInvisibleTile(tile))
											{
												if (startTiles.contains(tile))
												{
													Rectangle rect(node.first * 16, node.second * 16, 16, 16);
													if(!rect.intersects(purpleSquareRect))
														painter->drawPixmap(node.first * 16, node.second * 16, m_tilesetImage->pixmap(), Tilemap::GetTileX(patternTile) * 16, Tilemap::GetTileY(patternTile) * 16, 16, 16);


													addNode(node.first - 1, node.second);
													addNode(node.first, node.second - 1);

													addNode(node.first + 1, node.second);
													addNode(node.first, node.second + 1);


												}
											}
										}
									}
								}

							}
						}
					}
				

				}
			}
		}


		//Draw npcs
		QSet<AbstractLevelEntity*> drawObjects;
		if (m_overworld)
		{
			m_overworld->getEntitySpatialMap()->search(viewRect, false, drawObjects);
		}
		else if (m_level) {
			m_level->getEntitySpatialMap()->search(viewRect, false, drawObjects);
		}

		QList<AbstractLevelEntity*> sortedObjects(drawObjects.begin(), drawObjects.end());
		std::sort(sortedObjects.begin(), sortedObjects.end(), AbstractLevelEntity::sortByDepthFunc);

		for (auto entity : sortedObjects)
		{
			if (entity->getEntityType() == LevelEntityType::ENTITY_NPC && !m_showNPCs->isChecked())
				continue;

			if (entity->getEntityType() == LevelEntityType::ENTITY_LINK && !m_showLinks->isChecked())
				continue;

			if (entity->getEntityType() == LevelEntityType::ENTITY_SIGN && !m_showSigns->isChecked())
				continue;

			entity->loadResources();
			entity->draw(painter, viewRect);
		}

		if (m_selection != nullptr)
		{
			m_selection->draw(painter, viewRect);
		}

		if (m_selector.visible())
		{
			m_selector.draw(painter, viewRect, QColorConstants::White, QColor(255, 255, 255, 60));
		}

		if (selectingLevel())
		{
			auto level = getLevelAt(mousePos.x(), mousePos.y());
			if (level)
			{
				auto rect = QRectF(level->getX(), level->getY(), level->getWidth(), level->getHeight());
				painter->fillRect(rect, QColor(128, 128, 128, 128));
			}
		}

		if (ui.floodFillButton->isChecked())
		{
			auto tileX = std::floor(mousePos.x() / 16.0) * 16.0;
			auto tileY = std::floor(mousePos.y() / 16.0) * 16.0;

			auto rect = QRectF(tileX, tileY, m_fillPattern.getWidth(), m_fillPattern.getHeight());
			painter->fillRect(rect, QColor(255, 0, 255, 128));

		}

		if (m_overworld)
		{
			QFontMetrics fm(m_font1);

			int fontHeight = fm.height();
			painter->setFont(m_font1);

			auto compositionMode = painter->compositionMode();
			painter->setCompositionMode(QPainter::CompositionMode_Difference);
			painter->setPen(QColor(255, 255, 255));

			for (auto level : drawLevels)
			{
				int fontWidth = fm.horizontalAdvance(level->getName());


				painter->drawText(level->getCenterX() - fontWidth / 2, level->getY() + fontHeight, level->getName());

			}
			painter->setCompositionMode(compositionMode);
		}

		//Draw grid
		if (ui.gridButton->isChecked())
		{
			if (ui.hcountSpinBox->value() > 0 && ui.vcountSpinBox->value() > 0)
			{
				if (m_gridImage.isNull())
					generateGridImage(ui.hcountSpinBox->value() * 16, ui.vcountSpinBox->value() * 16);

				auto compositionMode = painter->compositionMode();
				painter->setCompositionMode(QPainter::CompositionMode_Difference);

				auto opacity = painter->opacity();
				painter->setOpacity(0.66f);
				int gridOffsetX = ((int)rect.x()) % (ui.hcountSpinBox->value() * 16);
				int gridOffsetY = ((int)rect.y()) % (ui.vcountSpinBox->value() * 16);


				for (int y = 0; y < rect.height() + m_gridImage.height(); y += m_gridImage.height())
				{


					for (int x = 0; x < rect.width() + m_gridImage.width(); x += m_gridImage.width())
					{
						float left = x - gridOffsetX + rect.x();
						float top = y - gridOffsetY + rect.y();

						painter->drawPixmap(left, top, m_gridImage);
					}
				}
				painter->setOpacity(opacity);
				painter->setCompositionMode(compositionMode);
			}

		}
	}

	void EditorTabWidget::renderTilesetSelection(QPainter* painter, const QRectF& rect)
	{
		Rectangle viewRect(rect.x(), rect.y(), rect.width(), rect.height());

		painter->fillRect(rect, QColor(255, 0, 255));

		if (m_tilesetImage)
		{
			painter->drawPixmap(0, 0, m_tilesetImage->pixmap());
		}

		if (m_tilesSelector.visible())
		{
			m_tilesSelector.draw(painter, viewRect, QColorConstants::White, QColor(255, 255, 255, 60));
		}

	}

	void EditorTabWidget::renderTileObjects(QPainter* painter, const QRectF& rect)
	{
		Rectangle viewRect(rect.x(), rect.y(), rect.width(), rect.height());


		painter->fillRect(rect, QApplication::palette().color(QPalette::Window));

		if (m_tilesetImage)
		{

			auto tileObject = getCurrentTileObject();
			if (tileObject)
			{
				tileObject->draw(painter, viewRect, m_tilesetImage, 0.0, 0.0);
			}

		}

	}

	void EditorTabWidget::paintDefaultTile(QPainter* painter, const QRectF& rect)
	{
		if (m_tilesetImage)
		{
			auto tileLeft = Tilemap::GetTileX(m_defaultTile);
			auto tileTop = Tilemap::GetTileY(m_defaultTile);

			painter->drawPixmap(0, 0, m_tilesetImage->pixmap(), tileLeft * 16, tileTop * 16, 16, 16);
		}
	}

	void EditorTabWidget::setTileset(const QString& name)
	{
		if (name == "")
			return;

		auto index = ui_tilesetsClass.tilesetsCombo->findText(name);
		if (index >= 0)
		{
			auto tileset = static_cast<Tileset*>(static_cast<QStandardItemModel*>(ui_tilesetsClass.tilesetsCombo->model())->item(index));

			if (tileset)
			{
				ui_tilesetsClass.tilesetsCombo->blockSignals(true);
				ui_tilesetsClass.tilesetsCombo->setCurrentIndex(index);
				ui_tilesetsClass.tilesetsCombo->blockSignals(false);

				setTileset(tileset);
			}

		}
		else {
			auto tileset = Tileset::loadTileset(name, m_resourceManager);
			if (tileset)
			{
				auto newIndex = static_cast<QStandardItemModel*>(ui_tilesetsClass.tilesetsCombo->model())->rowCount();
				static_cast<QStandardItemModel*>(ui_tilesetsClass.tilesetsCombo->model())->appendRow(tileset);
				ui_tilesetsClass.tilesetsCombo->blockSignals(true);
				ui_tilesetsClass.tilesetsCombo->setCurrentIndex(newIndex);
				ui_tilesetsClass.tilesetsCombo->blockSignals(false);

				setTileset(tileset);
			}
		}
	}

	void EditorTabWidget::setTileset(const Tileset* tileset)
	{

		m_tileset = *tileset;

		ui_tilesetsClass.editButton->setEnabled(tileset->hasTileTypes());
	


		auto imageName = m_tileset.getImageName();

		if (m_tilesetImage != nullptr)
		{
			if (m_tilesetImage->getName() == imageName)
				return;

			m_resourceManager.freeResource(m_tilesetImage);
		}

		m_tilesetImage = static_cast<Image*>(m_resourceManager.loadResource(this, imageName, ResourceType::RESOURCE_IMAGE));

		if (m_tilesetImage)
		{
			ui_tilesetsClass.graphicsView->setSceneRect(0, 0, m_tilesetImage->width(), m_tilesetImage->height());

		} else ui_tilesetsClass.graphicsView->setSceneRect(0, 0, 0, 0);

		ui_tileObjectsClass.graphicsView->redraw();


	}

	void EditorTabWidget::changeTileset(Tileset* tileset)
	{
		setTileset(tileset);

		if (m_overworld)
		{
			m_overworld->setTilesetName(tileset->text());
			setModified(nullptr);
		}
		else if (m_level) {
			m_level->setTilesetName(tileset->text());
			setModified(m_level);
		}
	}

	void EditorTabWidget::setUnmodified()
	{
		m_modified = false;

		if (m_overworld)
			emit changeTabText(m_overworld->getName());
		else if (m_level)
			emit changeTabText(m_level->getName());

	}

	void EditorTabWidget::setDefaultTile(int tile)
	{
		m_fillPattern = Tilemap(nullptr, 0.0, 0.0, 1, 1, 0);
		m_fillPattern.setTile(0, 0, tile);
		m_defaultTile = tile;
		ui_tilesetsClass.tileIcon->update();
	}

	bool EditorTabWidget::canSelectObject(LevelEntityType type) const
	{
		switch (type) {
			case LevelEntityType::ENTITY_NPC:
				return m_selectNPCs->isChecked();

			case LevelEntityType::ENTITY_LINK:
				return m_selectLinks->isChecked();

			case LevelEntityType::ENTITY_SIGN:
				return m_selectSigns->isChecked();
		}
		return true;
	}

	bool EditorTabWidget::hasSelectionTiles() const
	{
		return (m_tilesSelector.visible() && !m_tilesSelector.selecting()) || (m_selector.visible() && !m_selector.selecting()) || (m_selection != nullptr && m_selection->getSelectionType() == SelectionType::SELECTION_TILES);
	}

	Tilemap* EditorTabWidget::getTilesetSelection()
	{
		auto rect = m_tilesSelector.getSelection();
		auto hcount = rect.getWidth() / 16;
		auto vcount = rect.getHeight() / 16;
		
		auto tilemap = new Tilemap(nullptr, 0.0, 0.0, hcount, vcount, 0);

		for (int y = 0; y < vcount; ++y)
		{
			for (int x = 0; x < hcount; ++x)
			{
				auto tileLeft = int(rect.getX() / 16) + x;
				auto tileTop = int(rect.getY() / 16) + y;
				auto typeType = m_tileset.getTileType(tileLeft, tileTop);

				auto tile = Tilemap::MakeTile(tileLeft, tileTop, typeType);
				tilemap->setTile(x, y, tile);
			}
		}
		return tilemap;
	}

	Tilemap* EditorTabWidget::getSelectionTiles()
	{

		//Get tiles within the selector
		if (m_selector.visible() && !m_selector.selecting())
		{
			auto rect = m_selector.getSelection();
			auto hcount = rect.getWidth() / 16;
			auto vcount = rect.getHeight() / 16;

			auto retval = new Tilemap(nullptr, 0.0, 0.0, hcount, vcount, 0);

			auto levels = getLevelsInRect(rect);

			for (auto level : levels)
			{
				auto tilemap = level->getTilemap(m_selectedTilesLayer);
				if (tilemap != nullptr)
				{
					int sourceTileX = int((rect.getX() - tilemap->getX()) / 16.0);
					int sourceTileY = int((rect.getY() - tilemap->getY()) / 16.0);

					for (int y = 0; y < vcount; ++y)
					{
						for (int x = 0; x < hcount; ++x)
						{
							int tile = 0;

							if (tilemap->tryGetTile(x + sourceTileX, y + sourceTileY, &tile) && !Tilemap::IsInvisibleTile(tile))
								retval->setTile(x, y, tile);

						}
					}

				}
			}
			return retval;
		}

		//Get tiles within a m_selection
		if (m_selection != nullptr && m_selection->getSelectionType() == SelectionType::SELECTION_TILES)
		{
			auto tilesSelection = static_cast<TileSelection*>(m_selection);
			return new Tilemap(*tilesSelection->getTilemap());
		}

		//Last priority is any tiles selected in the tileset widget
		if (m_tilesSelector.visible() && !m_tilesSelector.selecting())
		{
			return getTilesetSelection();
		}

		return nullptr;
	}

	double EditorTabWidget::getSnapX() const
	{
		if (ui.snapButton->isChecked())
			return ui.hcountSpinBox->value() * 16;
		return 1.0;
	}

	double EditorTabWidget::getSnapY() const
	{
		if (ui.snapButton->isChecked())
			return ui.vcountSpinBox->value() * 16;
		return 1.0;
	}

	bool EditorTabWidget::selectingLevel()
	{
		if (ui.editLinksButton->isChecked() || ui.editSignsButton->isChecked())
			return true;
		return false;

	}

	void EditorTabWidget::generateGridImage(int width, int height)
	{
		int hcount = width <= 256 ? qFloor(256.0f / width) : 1;
		int vcount = height <= 256 ? qFloor(256.0f / height) : 1;

		if (hcount > 0 && vcount > 0)
		{
			int textureWidth = hcount * width;
			int textureHeight = vcount * height;

			QImage image(textureWidth, textureHeight, QImage::Format_ARGB32);
			image.fill(QColor(255, 255, 255, 0));
			QPen pen;
			pen.setWidth(1);
			pen.setColor(Qt::white);

			QPainter painter(&image);
			painter.setPen(pen);

			for (int y = 0; y < textureHeight; y += height)
			{
				painter.drawLine(0, y, textureWidth, y);
			}

			for (int x = 0; x < textureWidth; x += width)
			{
				painter.drawLine(x, 0, x, textureHeight);
			}
			painter.end();

			m_gridImage = QPixmap::fromImage(image);

		}
	}

	void EditorTabWidget::doTileSelection()
	{
		if (m_selector.visible())
		{
			auto selectionRect = m_selector.getSelection();

			auto hcount = int(selectionRect.getWidth() / 16);
			auto vcount = int(selectionRect.getHeight() / 16);

			if (hcount * vcount > 0)
			{
				auto tileSelection = new TileSelection(selectionRect.getX(), selectionRect.getY(), hcount, vcount);

				auto levels = getLevelsInRect(selectionRect);

				for (auto level : levels)
				{
					auto tilemap = level->getTilemap(m_selectedTilesLayer);
					if (tilemap != nullptr)
					{
						int sourceTileX = int((selectionRect.getX() - tilemap->getX()) / 16.0);
						int sourceTileY = int((selectionRect.getY() - tilemap->getY()) / 16.0);

						for (int y = 0; y < vcount; ++y)
						{
							for (int x = 0; x < hcount; ++x)
							{
								int tile = 0;

								if (tilemap->tryGetTile(x + sourceTileX, y + sourceTileY, &tile) && !Tilemap::IsInvisibleTile(tile))
								{
									setModified(level);


									tileSelection->setTile(x, y, tile);
								}

							}
						}

					}
				}

				tileSelection->setTilesetImage(m_tilesetImage);

				setSelection(tileSelection);

				addUndoCommand(new CommandDeleteTiles(this, selectionRect.getX(), selectionRect.getY(), m_selectedTilesLayer, tileSelection->getTilemap(), m_defaultTile));
			}
		}
	}

	bool EditorTabWidget::doObjectSelection(int x, int y, bool allowAppend)
	{

		auto entity = getEntityAt(x, y, true);


		if (entity != nullptr)
		{
			bool append = allowAppend && (m_selection != nullptr && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS);

			auto selection = append ? static_cast<ObjectSelection*>(m_selection) : new ObjectSelection(x, y);

			//setModified(entity->getLevel());

			if(entity->getLevel())
				entity->getLevel()->removeEntityFromSpatialMap(entity);

			selection->addObject(entity);
			entity->setStartRect(*entity);

			if (!append)
				setSelection(selection);


			return true;
		}

		return false;
	}

	void EditorTabWidget::setSelection(AbstractSelection* newSelection)
	{
		auto hadSelection = m_selection != nullptr;
		if (m_selection != nullptr)
		{
			auto oldRect = m_selection->getDrawRect();
			if(!m_selection->getAlternateSelectionMethod())
				m_selection->reinsertIntoWorld(this, m_selectedTilesLayer);
			m_selection->release(m_resourceManager);
			delete m_selection;
			m_selection = nullptr;

			m_graphicsView->scene()->update(oldRect.getX() - 2, oldRect.getY() - 2, oldRect.getWidth() + 4, oldRect.getHeight() + 4);
		}


		m_selection = newSelection;
		if (m_selection != nullptr)
		{
			//if (m_selection->getSelectionType() == SelectionType::SELECTION_TILES)
			//	ui.floodFillPatternButton->setEnabled(true);

			ui.floodFillButton->setChecked(false);
			//ui.floodFillPatternButton->setChecked(false);

			if (m_selector.visible())
			{
				auto oldRect = m_selector.getSelection();
				m_selector.setVisible(false);
				m_graphicsView->scene()->update(oldRect.getX() - 2, oldRect.getY() - 2, oldRect.getWidth() + 4, oldRect.getHeight() + 4);

				selectorGone();
			}

			if (!hadSelection)
				selectionMade();
		}

		if (m_selection == nullptr)
		{
			m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);

			if (hadSelection)
				selectionGone();
		}
	}

	Rectangle EditorTabWidget::getViewRect() const
	{
		auto a = m_graphicsView->mapToScene(m_graphicsView->viewport()->geometry()).boundingRect();
		return Rectangle(a.x(), a.y(), a.width(), a.height());
	}



	Level* EditorTabWidget::getActiveLevel()
	{
		if (m_overworld)
		{
			auto view = getViewRect();
			return m_overworld->getLevelAt(view.getCenterX(), view.getCenterY());
		}
		return m_level;
	}

	void EditorTabWidget::init(QStandardItemModel* tilesetList, TileGroupListModel* tileGroupList)
	{
		ui_tilesetsClass.tilesetsCombo->setModel(tilesetList);

		ui_tileObjectsClass.groupCombo->setModel(tileGroupList);

		setTileset(ui_tilesetsClass.tilesetsCombo->currentText());

	}

	void EditorTabWidget::addUndoCommand(QUndoCommand* command)
	{
		m_undoStack.push(command);

		ui.redoButton->setEnabled(m_undoStack.canRedo());
		ui.undoButton->setEnabled(m_undoStack.canUndo());
		//if()
	}

	QString EditorTabWidget::getName() const
	{
		if (m_overworld)
			return m_overworld->getName();
		else if (m_level)
			return m_level->getName();
		return "";
	}

	QSet<Level*> EditorTabWidget::getLevelsInRect(const IRectangle& rect)
	{
		if (m_overworld)
		{
			QSet<Level*> retval;

			m_overworld->searchLevels(rect, retval);

			for (auto level : retval)
				loadLevel(level);
			return retval;
		}
		else if (m_level)
		{
			QSet<Level*> retval;
			if (rect.intersects(*m_level))
			{
				retval.insert(m_level);
			}
			return retval;
		}
		return QSet<Level*>();
	}

	Level* EditorTabWidget::getLevelAt(double x, double y)
	{
		if (m_overworld) {
			auto level = m_overworld->getLevelAt(x, y);
			if (level)
				loadLevel(level);
			return level;
		}
		else if(m_level) {

			return m_level;
		}
		return nullptr;
	}

	ResourceManager& EditorTabWidget::getResourceManager()
	{
		return m_resourceManager;
	}

	bool EditorTabWidget::containsLevel(const QString& levelName) const
	{
		if (m_overworld)
		{
			return m_overworld->containsLevel(levelName);
		}
		else if (m_level)
		{
			return m_level->getName() == levelName;
		}
		return false;
	}

	void EditorTabWidget::centerLevel(const QString& levelName)
	{
		if (m_overworld)
		{
			auto level = m_overworld->getLevel(levelName);
			if (level != nullptr)
			{
				m_graphicsView->ensureVisible(level->getCenterX(), level->getCenterY(), 1, 1);
			}
		}
	}

	AbstractLevelEntity* EditorTabWidget::getEntityAt(double x, double y)
	{
		return getEntityAt(x, y, false);
	}

	AbstractLevelEntity* EditorTabWidget::getEntityAt(double x, double y, bool checkAllowedSelect)
	{
		QList<AbstractLevelEntity*> entities;
		Rectangle rect(x, y, 1, 1);

		if (m_overworld)
			m_overworld->getEntitySpatialMap()->search(rect, true, entities);
		else if (m_level)
			m_level->getEntitySpatialMap()->search(rect, true, entities);

		//Sort found entities
		std::sort(entities.begin(), entities.end(), AbstractLevelEntity::sortByDepthFunc);

		//iterate backwards until we find an until we are allowed to select
		for (auto it = entities.rbegin(); it != entities.rend(); ++it)
		{
			auto entity = *it;
			if (!checkAllowedSelect || canSelectObject(entity->getEntityType()))
				return entity;
		}

		return nullptr;
	}

	QList<AbstractLevelEntity*> EditorTabWidget::getEntitiesAt(double x, double y)
	{
		return getEntitiesAt(x, y, false);
	}

	QList<AbstractLevelEntity*> EditorTabWidget::getEntitiesAt(double x, double y, bool checkAllowedSelect)
	{
		QList<AbstractLevelEntity*> entities;
		Rectangle rect(x, y, 1, 1);

		if (m_overworld)
			m_overworld->getEntitySpatialMap()->search(rect, true, entities);
		else if (m_level)
			m_level->getEntitySpatialMap()->search(rect, true, entities);

		//Sort found entities
		std::sort(entities.begin(), entities.end(), AbstractLevelEntity::sortByDepthFunc);

		if (checkAllowedSelect)
		{
			for (auto it = entities.begin(); it != entities.end();)
			{
				auto entity = *it;
				if (canSelectObject(entity->getEntityType())) {
					++it;
				}
				else {
					it = entities.erase(it);
				}
			}
		}

		return entities;
	}

	QSet<AbstractLevelEntity*> EditorTabWidget::getEntitiesInRect(const IRectangle& rect)
	{
		QSet<AbstractLevelEntity*> entities;

		if (m_overworld)
			m_overworld->getEntitySpatialMap()->search(rect, true, entities);
		else if (m_level)
			m_level->getEntitySpatialMap()->search(rect, true, entities);

		return entities;
	}

	void EditorTabWidget::deleteEntity(AbstractLevelEntity* entity, QUndoCommand* parent)
	{
		auto objectListModel = static_cast<ObjectListModel*>(ui_objectClass.objectsTable->model());

		//Remove this entity from the npc search
		if(entity->getEntityType() == LevelEntityType::ENTITY_NPC)
			objectListModel->removeEntity(static_cast<LevelNPC*>(entity));

		if (parent == nullptr)
			addUndoCommand(new CommandDeleteEntity(this, entity, nullptr));
		else new CommandDeleteEntity(this, entity, parent);
	}

	void EditorTabWidget::deleteEntities(const QList<AbstractLevelEntity*>& entities, QUndoCommand* parent)
	{
		auto objectListModel = static_cast<ObjectListModel*>(ui_objectClass.objectsTable->model());

		auto undoCommand = new QUndoCommand(parent);
		for (auto entity : entities)
		{
			//Remove this entity from the npc search
			if (entity->getEntityType() == LevelEntityType::ENTITY_NPC)
				objectListModel->removeEntity(static_cast<LevelNPC*>(entity));

			new CommandDeleteEntity(this, entity, undoCommand);
		}

		if(parent == nullptr)
			addUndoCommand(undoCommand);
	}


	void EditorTabWidget::doPaste(bool centerScreen)
	{

		QClipboard* clipboard = QApplication::clipboard();

		auto text = clipboard->text();

		QByteArray ba = text.toLocal8Bit();

		auto json = cJSON_Parse(ba.data());

		if (json != nullptr)
		{
			auto viewRect = getViewRect();
			auto type = jsonGetChildString(json, "type");

			auto x = jsonGetChildDouble(json, "x");
			auto y = jsonGetChildDouble(json, "y");
			if (type == "tileSelection")
			{
				auto hcount = jsonGetChildInt(json, "hcount");
				auto vcount = jsonGetChildInt(json, "vcount");


				auto pasteX = centerScreen ? std::floor((viewRect.getCenterX() - (hcount * 16) / 2) / 16.0) * 16.0 : x;
				auto pasteY = centerScreen ? std::floor((viewRect.getCenterY() - (vcount * 16) / 2) / 16.0) * 16.0 : y;

				auto tileSelection = new TileSelection(pasteX, pasteY, hcount, vcount);

				auto tileArray = cJSON_GetObjectItem(json, "tiles");

				if (tileArray && tileArray->type == cJSON_Array)
				{
					static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
					for (int y = 0; y < cJSON_GetArraySize(tileArray); ++y)
					{
						auto arrayItem = cJSON_GetArrayItem(tileArray, y);
						if (arrayItem->type == cJSON_String)
						{
							QString line(arrayItem->valuestring);

							auto parts = line.split(' ', Qt::SkipEmptyParts);
							for (auto x = 0U; x < parts.size(); ++x)
							{
								int tile = 0;
								auto& part = parts[x];
								int bitcount = 0;


								for (auto i = part.length() - 1; i >= 0; --i) {
									auto value = base64.indexOf(part[i]);

									tile |= value << bitcount;
									bitcount += 6;

								}
								tileSelection->setTile(x, y, tile);
							}
						}
					}
				}

				if (m_tilesetImage)
					tileSelection->setTilesetImage(m_tilesetImage);
				setSelection(tileSelection);

				m_graphicsView->redraw();

			}
			else if (type == "objectSelection")
			{
				auto selection = new ObjectSelection(centerScreen ? viewRect.getCenterX() : x, centerScreen ? viewRect.getCenterY() : y);
				selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
				selection->deserializeJSON(json, this);


				setSelection(selection);

				m_graphicsView->redraw();
			}

			cJSON_Delete(json);
		}
	}

	void EditorTabWidget::newLevel(int hcount, int vcount)
	{
		m_level = new Level(this, 0, 0, hcount * 16, vcount * 16, nullptr, "");
		m_level->getOrMakeTilemap(0)->clear(0);
		m_level->setLoaded(true);
		m_graphicsView->redraw();

	}

	void EditorTabWidget::loadLevel(const QString& name, const QString& fileName)
	{
		QFileInfo fi(fileName);


		m_resourceManager.addSearchDirRecursive(fi.absolutePath());
		m_resourceManager.setRootDir(fi.absolutePath());

		m_level = new Level(this, 0, 0, 64 * 16, 64 * 16, nullptr, name);
		m_level->setFileName(fileName);
		m_level->loadFile();

		setTileset(m_level->getTilesetName());

		m_graphicsView->setSceneRect(QRect(0, 0, m_level->getWidth(), m_level->getHeight()));


	}

	void EditorTabWidget::fileFailed(const QString& name)
	{
		m_resourceManager.addFailedResource(name);
	}

	void EditorTabWidget::fileReady(const QString& fileName)
	{
		if (m_tileset.getImageName() == fileName)
		{
			tilesetRefreshClicked(true);
		}
		else {
			if (m_overworld)
			{
				auto level = m_overworld->getLevel(fileName);
				if (level)
				{
					level->setLoadFail(false);
					loadLevel(level);

					//Only refresh the graphics view if the level is visible
					auto viewRect = getViewRect();
					if(viewRect.intersects(*level))
						m_graphicsView->redraw();
				}
			}
			else if (m_level && m_level->getName() == fileName)
			{
				m_level->setLoadFail(false);
				loadLevel(m_level);
				m_graphicsView->redraw();
			}
		}
	}

	void EditorTabWidget::loadLevel(Level* level)
	{
		if (!level->getLoaded() && !level->getLoadFail())
		{
			QString fullPath;
			if (m_resourceManager.locateFile(level->getName(), &fullPath))
			{
				level->setFileName(fullPath);
				level->loadFile();
			}
			else
			{
				m_resourceManager.getFileSystem()->requestFile(this, level->getName());
			}
		}
	}

	void EditorTabWidget::loadOverworld(const QString& name, const QString & fileName)
	{
		m_overworld = new Overworld(this, name);
		m_overworld->setFileName(fileName);

		QFileInfo fi(fileName);

		m_resourceManager.addSearchDirRecursive(fi.absolutePath());
		m_resourceManager.setRootDir(fi.absolutePath());

		m_overworld->loadFile();
		
		setTileset(m_overworld->getTilesetName());

		m_graphicsView->setSceneRect(QRect(0, 0, m_overworld->getWidth(), m_overworld->getHeight()));

		ui.saveAsButton->setEnabled(false);
		

		ui.preloadButton->setEnabled(true);

	}

	void EditorTabWidget::setModified(Level* level)
	{


		if (level)
			level->setModified(true);
		if (!m_modified)
		{
			m_modified = true;
			auto tabWidget = static_cast<QTabWidget*>(this->parentWidget());


			if (m_overworld)
				emit changeTabText(m_overworld->getName() + "*");
			else if (m_level)
				emit changeTabText((m_level->getName().isEmpty() ? "new" : m_level->getName()) + "*");
		}

	}

	bool EditorTabWidget::tryGetTileAt(double x, double y, int* outTile)
	{
		auto level = getLevelAt(x, y);
		if (level)
		{
			auto layer = level->getTilemap(m_selectedTilesLayer);

			if (layer)
			{
				if (layer->tryGetTile(int((x - layer->getX()) / 16.0), int((y - layer->getY()) / 16.0), outTile))
				{
					return true;
				}
			}
		}
		*outTile = 0;
		return false;
	}

	void EditorTabWidget::updateMovedEntity(AbstractLevelEntity* entity)
	{
		if (entity->getLevel())
		{
			auto overworld = entity->getLevel()->getOverworld();
			if (overworld)
			{
				//If the entity no longer intersects the level
				if (!entity->getLevel()->intersects(*entity))
				{
					//Find the new level
					auto newLevel = overworld->getLevelAt(entity->getX(), entity->getY());
					if (newLevel && newLevel != entity->getLevel())
					{
						entity->getLevel()->removeObject(entity);
						setModified(entity->getLevel());

						newLevel->addObject(entity);
						entity->setLevel(newLevel);
						setModified(newLevel);
						return;
					}
				}

				entity->getLevel()->updateSpatialEntity(entity);
			} else
				entity->getLevel()->updateSpatialEntity(entity);

		}

	}

	void EditorTabWidget::updateEntityRect(AbstractLevelEntity* entity)
	{
		if (entity->getLevel())
		{
			entity->getLevel()->updateSpatialEntity(entity);

		}
	}

	QList<Level*> EditorTabWidget::getModifiedLevels()
	{
		if (m_overworld)
		{
			return m_overworld->getModifiedLevels();
		}
		else if (m_level && m_level->getModified())
			return QList<Level*>({ m_level });
		return QList<Level*>();
	}

	void EditorTabWidget::getTiles(double x, double y, int layer, Tilemap* output)
	{
		Rectangle rect(x, y, output->getWidth(), output->getHeight());

		auto levels = getLevelsInRect(rect);

		for (auto level : levels)
		{
			auto tilemap = level->getTilemap(layer);
			if (tilemap != nullptr)
			{
				int sourceTileX = int((rect.getX() - tilemap->getX()) / 16.0);
				int sourceTileY = int((rect.getY() - tilemap->getY()) / 16.0);

				for (int y = 0; y < output->getVCount(); ++y)
				{
					for (int x = 0; x < output->getHCount(); ++x)
					{
						int tile = 0;

						if (tilemap->tryGetTile(x + sourceTileX, y + sourceTileY, &tile) && !Tilemap::IsInvisibleTile(tile))
						{
							output->setTile(x, y, tile);
						}

					}
				}

			}
		}

	}

	void EditorTabWidget::putTiles(double x, double y, int layer, Tilemap* input, bool ignoreInvisible)
	{
		Rectangle rect(x, y, input->getWidth(), input->getHeight());

		auto levels = this->getLevelsInRect(rect);

		for (auto level : levels)
		{ 
			auto tilemap = level->getOrMakeTilemap(layer);
			if (tilemap != nullptr)
			{
				int destTileX = int((x - tilemap->getX()) / 16.0);
				int destTileY = int((y - tilemap->getY()) / 16.0);

				bool modified = false;
				for (int y = 0; y < input->getVCount(); ++y)
				{
					for (int x = 0; x < input->getHCount(); ++x)
					{
						int tile = input->getTile(x, y);


						if (!ignoreInvisible || !Tilemap::IsInvisibleTile(tile))
						{
							modified = true;
							tilemap->setTile(destTileX + x, destTileY + y, tile);
						}
					}
				}

				if (modified)
					this->setModified(level);

			}
		}
	}

	void EditorTabWidget::deleteTiles(double x, double y, int layer, int hcount, int vcount, int replacementTile)
	{
		static int invisibleTile = Tilemap::MakeInvisibleTile(0);

		Rectangle rect(x, y, hcount * 16, vcount * 16);

		auto levels = this->getLevelsInRect(rect);

		for (auto level : levels)
		{
			auto tilemap = level->getTilemap(layer);
			if (tilemap != nullptr)
			{
				int destTileX = int((x - tilemap->getX()) / 16.0);
				int destTileY = int((y - tilemap->getY()) / 16.0);

				bool modified = false;
				for (int y = 0; y < vcount; ++y)
				{
					for (int x = 0; x < hcount; ++x)
					{
						if (layer != 0) {
							tilemap->setTile(destTileX + x, destTileY + y, invisibleTile);
							modified = true;
						}
						else if(!Tilemap::IsInvisibleTile(replacementTile))
						{
							modified = true;
							tilemap->setTile(destTileX + x, destTileY + y, replacementTile);
						}
					}
				}

				if (modified)
					this->setModified(level);

			}
		}

	}



	void EditorTabWidget::tilesetMousePress(QMouseEvent* event)
	{
		auto pos = ui_tilesetsClass.graphicsView->mapToScene(event->pos());
		if (event->button() == Qt::MouseButton::LeftButton)
		{
			if (m_tilesSelector.visible())
			{
				auto rect = m_tilesSelector.getSelection();
				if (rect.intersects(Rectangle(pos.x(), pos.y(), 1, 1)))
				{
					auto hcount = rect.getWidth() / 16;
					auto vcount = rect.getHeight() / 16;
					auto tileSelection = new TileSelection(-1000000, -1000000, hcount, vcount);

					for (int y = 0; y < vcount; ++y)
					{
						for (int x = 0; x < hcount; ++x)
						{
							auto tileLeft = int(rect.getX() / 16) + x;
							auto tileTop = int(rect.getY() / 16) + y;
							auto tileType = m_tileset.getTileType(tileLeft, tileTop);

							auto tile = Tilemap::MakeTile(tileLeft, tileTop, tileType);
							tileSelection->setTile(x, y, tile);
						}
					}

					if(m_tilesetImage)
						tileSelection->setTilesetImage(m_tilesetImage);
					tileSelection->setAlternateSelectionMethod(true);

					setSelection(tileSelection);
					m_graphicsView->redraw();
					return;
				}


			}


			m_tilesSelector.setVisible(false);

			ui.floodFillPatternButton->setEnabled(hasSelectionTiles());
			auto maxWidth = (int)ui_tilesetsClass.graphicsView->sceneRect().width();
			auto maxHeight = (int)ui_tilesetsClass.graphicsView->sceneRect().height();
			m_tilesSelector.beginSelection(
				std::min(maxWidth, std::max(0, (int)pos.x())),
				std::min(maxHeight, std::max(0, (int)pos.y())),
				16, 16);
			ui_tilesetsClass.graphicsView->redraw();
		}
		else if (event->button() == Qt::MouseButton::RightButton)
		{
			m_tilesSelector.setVisible(false);
			ui.floodFillPatternButton->setEnabled(hasSelectionTiles());

			auto tileLeft = int(pos.x() / 16);
			auto tileTop = int(pos.y() / 16);
			auto tileType = m_tileset.getTileType(tileLeft, tileTop);

			setDefaultTile(Tilemap::MakeTile(tileLeft, tileTop, tileType));
			ui_tilesetsClass.graphicsView->redraw();


		}

	}

	void EditorTabWidget::tilesetMouseRelease(QMouseEvent* event)
	{
		auto pos = ui_tilesetsClass.graphicsView->mapToScene(event->pos());

		if (event->button() == Qt::MouseButton::LeftButton)
		{
			if (!m_tilesSelector.visible())
			{
				auto hcount = 1;
				auto vcount = 1;
				auto tileSelection = new TileSelection(-1000000, -1000000, hcount, vcount);

				auto tileLeft = int(pos.x() / 16);
				auto tileTop = int(pos.y() / 16);
				auto tileType = m_tileset.getTileType(tileLeft, tileTop);

				auto tile = Tilemap::MakeTile(tileLeft, tileTop, tileType);

				tileSelection->setTile(0, 0, tile);

				if (m_tilesetImage)
					tileSelection->setTilesetImage(m_tilesetImage);
				tileSelection->setAlternateSelectionMethod(true);
				m_tilesSelector.endSelection(0.0f, 0.0f);

				setSelection(tileSelection);
				m_graphicsView->redraw();
				return;
			}

			if (m_tilesSelector.selecting())
			{
				auto maxWidth = (int)ui_tilesetsClass.graphicsView->sceneRect().width();
				auto maxHeight = (int)ui_tilesetsClass.graphicsView->sceneRect().height();

				m_tilesSelector.endSelection(
					std::min(maxWidth, std::max(0, (int)pos.x())),
					std::min(maxHeight, std::max(0, (int)pos.y()))
				);

				ui.floodFillPatternButton->setEnabled(hasSelectionTiles());
			}


			ui_tilesetsClass.graphicsView->redraw();

		}
	}

	void EditorTabWidget::tilesetMouseMove(QMouseEvent* event)
	{
		auto pos = ui_tilesetsClass.graphicsView->mapToScene(event->pos());

		if (m_tilesSelector.selecting())
		{
			m_tilesSelector.setVisible(true);

			auto maxWidth = (int)ui_tilesetsClass.graphicsView->sceneRect().width();
			auto maxHeight  = (int)ui_tilesetsClass.graphicsView->sceneRect().height();

			m_tilesSelector.updateSelection(std::min(maxWidth, std::max(0, (int)pos.x())), std::min(maxHeight, std::max(0, (int)pos.y())));
			ui_tilesetsClass.graphicsView->redraw();
		}
		else if (m_tilesSelector.visible())
		{

		}

	}

	void EditorTabWidget::tilesetEditClicked(bool checked)
	{

		EditTilesetDialog dialog(&m_tileset, m_resourceManager);
		if (dialog.exec() == QDialog::Accepted)
		{
			m_tileset = dialog.getTileset();

			
			//update this tileset globally
			auto index = ui_tilesetsClass.tilesetsCombo->findText(m_tileset.text());
			if (index >= 0) {
				auto globalTileset = static_cast<Tileset*>(static_cast<QStandardItemModel*>(ui_tilesetsClass.tilesetsCombo->model())->item(index));
				if (globalTileset) {
					*globalTileset = m_tileset;
				}
			
			}
			setTileset(&m_tileset);
		}
		
	}

	void EditorTabWidget::tileObjectsMousePress(QMouseEvent* event)
	{
		auto tileObject = getCurrentTileObject();

		if (tileObject)
		{
			auto hcount = tileObject->getHCount();
			auto vcount = tileObject->getVCount();

			auto tileSelection = new TileSelection(-10000, -10000, hcount, vcount);

			for (int y = 0; y < vcount; ++y)
			{
				for (int x = 0; x < hcount; ++x)
				{
					int tile = tileObject->getTile(x, y);
					tileSelection->setTile(x, y, tile);
				}
			}

			if (m_tilesetImage)
				tileSelection->setTilesetImage(m_tilesetImage);
			tileSelection->setAlternateSelectionMethod(true);

			setSelection(tileSelection);
			m_graphicsView->redraw();
		}
	}

	void EditorTabWidget::graphicsMousePress(QMouseEvent* event)
	{
		auto pos = m_graphicsView->mapToScene(event->pos());
		if (selectingLevel())
		{
			if (ui.editLinksButton->isChecked())
			{
				if (event->button() == Qt::MouseButton::LeftButton)
				{
					auto level = getLevelAt(pos.x(), pos.y());
					if (level)
					{
						ListLinksDialog frm(level, this);
						frm.exec();
						//return;
					}
				}
				ui.editLinksButton->setChecked(false);
				m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
				m_graphicsView->redraw();
			} else if (ui.editSignsButton->isChecked())
			{
				if (event->button() == Qt::MouseButton::LeftButton)
				{
					auto level = getLevelAt(pos.x(), pos.y());
					if (level)
					{
						EditSignsDialog frm(level, this);
						frm.exec();
					}
				}
				ui.editSignsButton->setChecked(false);
				m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
				m_graphicsView->redraw();
			}
			return;
		}





		auto rect = getViewRect();

		if (event->button() == Qt::MouseButton::LeftButton)
		{
			if (ui.floodFillButton->isChecked())
			{
				addUndoCommand(new CommandFloodFillPattern(this, pos.x(), pos.y(), m_selectedTilesLayer, &m_fillPattern));
				
				m_graphicsView->redraw();
				return;
			}

			if (m_selection != nullptr)
			{
				if (!m_selection->getAlternateSelectionMethod())
				{
					if (m_selection->canResize())
					{
						int edges = m_selection->getResizeEdge(pos.x(), pos.y());

						if (edges > 0)
						{
							m_selection->beginResize(edges, this);
							return;
						}
					}

					if (m_selection->pointInSelection(pos.x(), pos.y()))
					{
						m_graphicsView->setCursor(Qt::CursorShape::OpenHandCursor);
						m_selection->setDragOffset(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
						return;
					}
					else {
						if (QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier) && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS)
						{
							doObjectSelection(pos.x(), pos.y(), true);
							m_selection->setDragOffset(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
							m_graphicsView->redraw();

							return;
						}
						setSelection(nullptr);
					}
				}
				else {
					//insert

					//If tiles then only insert the tiles and continue
					if (m_selection->getSelectionType() == SelectionType::SELECTION_TILES)
						m_selection->reinsertIntoWorld(this, m_selectedTilesLayer);
					//Otherwise switch back to normal selection method (for objects)
					else m_selection->setAlternateSelectionMethod(false);

					m_graphicsView->redraw();
					return;
				}
			}

			if (m_selector.visible())
			{
				auto rect = m_selector.getSelection();
				if (rect.intersects(Rectangle(pos.x(), pos.y(), 1, 1)) || m_selector.getResizeEdge(pos.x(), pos.y()))
				{
					doTileSelection();

					m_selector.setVisible(false);
					graphicsMousePress(event);
					return;
				}

			}

			if (doObjectSelection(pos.x(), pos.y(), false))
			{
				if (m_selection && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS)
				{
					auto objectSelection = dynamic_cast<ObjectSelection*>(m_selection);

					if(objectSelection->getEntityType() == LevelEntityType::ENTITY_NPC)
						setStatusBar("Hint: Hold 'Shift' to select multiple objects", 1, 20000);
					else if (objectSelection->getEntityType() == LevelEntityType::ENTITY_LINK)
						setStatusBar("Hint: Hold 'Shift' and double click the link to open the destination level", 1, 20000);
				}
				m_graphicsView->setCursor(Qt::CursorShape::OpenHandCursor);
				m_selection->setDragOffset(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
				m_graphicsView->redraw();
				return;
			}

			if (m_selector.visible())
			{
				m_selector.setVisible(false);
				selectorGone();
			}

			
			m_selector.beginSelection(pos.x(), pos.y(), std::ceil(getSnapX() / 16.0) * 16.0, std::ceil(getSnapY() / 16.0) * 16.0);
			m_graphicsView->redraw();
		}
		else if (event->button() == Qt::MouseButton::RightButton)
		{


			if (m_selector.visible())
			{
				auto selection = m_selector.getSelection();
				
				Rectangle rect(pos.x(), pos.y(), 1, 1);
				if (rect.intersects(selection)) {
					doTileSelection();
					m_selector.setVisible(false);
				}
				else {
					m_selector.setVisible(false);
					selectorGone();
				}
				
			}

			if (ui.floodFillButton->isChecked())
			{
				m_graphicsView->redraw();
				ui.floodFillButton->setChecked(false);
				return;
			}

			//Copy-paste via right click
			if (m_selection && !m_selection->getAlternateSelectionMethod() && m_selection->pointInSelection(pos.x(), pos.y()))
			{
				m_selection->clipboardCopy();
				doPaste(false);

				if (m_selection) {
					m_selection->setDragOffset(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
				}
				return;

			}
			setSelection(nullptr);

			auto entities = getEntitiesAt(pos.x(), pos.y(), true);
			if (!entities.empty())
			{
				QMenu contextMenu;

				class EntityAction:
					public QAction
				{
				private:
					AbstractLevelEntity* m_entity;

				public:
					EntityAction(AbstractLevelEntity* entity, const QString& text):
						QAction(text), m_entity(entity)
					{

						this->setIcon(entity->getIcon());

					}

					AbstractLevelEntity* getEntity() { return m_entity; }
				};

				for (auto entity : entities)
				{
					contextMenu.addAction(new EntityAction(entity, QString("Select '%1'").arg(entity->toString())));
				}

				auto action = dynamic_cast<EntityAction*>(contextMenu.exec(event->globalPosition().toPoint()));
				if (action) {
					auto selection = new ObjectSelection(action->getEntity()->getX(), action->getEntity()->getY());
					selection->addObject(action->getEntity());

					if (action->getEntity()->getLevel()) {
						action->getEntity()->getLevel()->removeEntityFromSpatialMap(action->getEntity());
					}
					setSelection(selection);
					m_graphicsView->redraw();
				}
				return;
			}

			int tile = 0;
			if (tryGetTileAt(pos.x(), pos.y(), &tile) && !Tilemap::IsInvisibleTile(tile))
			{
				setDefaultTile(tile);
			}


			m_graphicsView->redraw();


		}
	}

	void EditorTabWidget::graphicsMouseRelease(QMouseEvent* event)
	{
		auto pos = m_graphicsView->mapToScene(event->pos());

		if (event->button() == Qt::MouseButton::RightButton)
		{
			if (m_selector.visible())
			{
				m_selector.setVisible(false);
				m_graphicsView->redraw();
			}
		}

		else if (event->button() == Qt::MouseButton::LeftButton)
		{
			if (m_selection != nullptr && m_selection->resizing())
			{
				m_selection->endResize(this);
				m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
			}

			if (m_selector.selecting())
			{

				m_selector.endSelection(pos.x(), pos.y());

				selectorMade();


				m_graphicsView->redraw();
			}
		}
	}

	void EditorTabWidget::graphicsMouseMove(QMouseEvent* event)
	{
		auto pos = m_graphicsView->mapToScene(event->pos());
		

		auto tileXPos = int(pos.x() / 16.0);
		auto tileYPos = int(pos.y() / 16.0);

		auto oldTileXPos = int(m_lastMousePos.x() / 16);
		auto oldTileYPos = int(m_lastMousePos.y() / 16);
		m_lastMousePos = pos;

		if (selectingLevel() || ui.floodFillButton->isChecked())
		{


			//Only redraw if the tile position of the mouse has changed
			//if (oldTileXPos != tileXPos || oldTileYPos != tileYPos)
			{
				m_graphicsView->redraw();
			}
			return;
		}

		

		//Check if the tile position of the mouse has changed
		if (oldTileXPos != tileXPos || oldTileYPos != tileYPos)
		{
			auto level = getLevelAt(pos.x(), pos.y());
			if (level)
			{
				auto tilemapX = int((pos.x() - level->getX()) / level->getTileWidth());
				auto tilemapY = int((pos.y() - level->getY()) / level->getTileHeight());

				QString displayTile = "";
				auto tileMap = level->getTilemap(m_selectedTilesLayer);
				if (tileMap)
				{
					auto tile = tileMap->getTile(tilemapX, tilemapY);

					if (!Tilemap::IsInvisibleTile(tile))
						displayTile = level->getDisplayTile(tile);
					else displayTile = "";
				}

				auto tileX = (std::floor(pos.x() / level->getTileWidth()) * level->getTileWidth()) / getUnitWidth();
				auto tileY = (std::floor(pos.y() / level->getTileHeight()) * level->getTileHeight()) / getUnitHeight();
				auto localTileX = (std::floor((pos.x() - level->getX()) / level->getTileWidth()) * level->getTileWidth()) / getUnitWidth();
				auto localTileY = (std::floor((pos.y() - level->getY()) / level->getTileHeight()) * level->getTileHeight()) / getUnitHeight();

				if (m_overworld)
				{
					QString result;
					QTextStream(&result) << "Mouse: " << tileX << ", " << tileY << " (" << localTileX << ", " << localTileY << ") Tile: " << displayTile;
					emit setStatusBar(result, 0, 20000);
				}
				else {
					QString result;
					QTextStream(&result) << "Mouse: " << tileX << ", " << tileY << " Tile: " << displayTile;
					emit setStatusBar(result, 0, 20000);
				}
			}
		}

		//If holding left button
		if (event->buttons().testFlag(Qt::MouseButton::LeftButton) || event->buttons().testFlag(Qt::MouseButton::RightButton))
		{

			if (m_selection != nullptr)
			{
				if (m_selection->getAlternateSelectionMethod())
				{
					if (m_selection->getSelectionType() == SelectionType::SELECTION_TILES)
					{
						m_selection->reinsertIntoWorld(this, m_selectedTilesLayer);
					}
				}
				else {

					if (m_selection->resizing())
					{
						m_selection->updateResize(pos.x(), pos.y(), true, getSnapX(), getSnapY(), this);
						m_graphicsView->redraw();
						return;
					}

					auto oldRect = m_selection->getDrawRect();
					m_selection->drag(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY(), this);
					
					auto newRect = m_selection->getDrawRect();
					
					auto left = std::min(oldRect.getX(), newRect.getX()) - 4;
					auto top = std::min(oldRect.getY(), newRect.getY()) - 4;
					auto right = std::max(oldRect.getRight(), newRect.getRight()) + 4;
					auto bottom = std::max(oldRect.getBottom(), newRect.getBottom()) + 4;
					m_graphicsView->scene()->update(left, top, right - left, bottom - top);

					
					return;
				}


			}
		}

		if (m_selection != nullptr)
		{

			if (m_selection->getAlternateSelectionMethod())
			{
				auto oldRect = m_selection->getDrawRect();

				m_selection->drag(pos.x() - m_selection->getWidth() / 2, pos.y() - m_selection->getHeight() / 2,
					true, getSnapX(), getSnapY(), this);

				auto newRect = m_selection->getDrawRect();
				auto left = std::min(oldRect.getX(), newRect.getX()) - 4;
				auto top = std::min(oldRect.getY(), newRect.getY()) - 4;
				auto right = std::max(oldRect.getRight(), newRect.getRight()) + 4;
				auto bottom = std::max(oldRect.getBottom(), newRect.getBottom()) + 4;
				m_graphicsView->scene()->update(left, top, right - left, bottom - top);

				return;

			}
			else {
				//Set the correct cursor when it is on the edge of a selection object that can be resized
				if (m_selection->canResize())
				{
					int edges = m_selection->getResizeEdge(pos.x(), pos.y());

					if (edges > 0)
					{
						int cursors[] = { Qt::SizeHorCursor, Qt::SizeVerCursor, Qt::SizeFDiagCursor, Qt::SizeHorCursor, Qt::SizeBDiagCursor, Qt::SizeBDiagCursor, Qt::SizeVerCursor, Qt::SizeVerCursor, Qt::SizeBDiagCursor,Qt::ArrowCursor, Qt::ArrowCursor,  Qt::SizeFDiagCursor };

						if (edges < 13)
						{
							m_graphicsView->setCursor((Qt::CursorShape)cursors[edges - 1]);
						}

						return;
					}
				}

				if (m_selection->pointInSelection(pos.x(), pos.y()))
				{
					m_graphicsView->setCursor(Qt::CursorShape::OpenHandCursor);
				}
				else m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
			}
		}

		if (m_selector.selecting())
		{
			auto oldRect = m_selector.getSelection();
			m_selector.setVisible(true);
			m_selector.updateSelection(pos.x(), pos.y());
			auto newRect = m_selector.getSelection();

			auto left = std::min(oldRect.getX(), newRect.getX()) - 4;
			auto top = std::min(oldRect.getY(), newRect.getY()) - 4;
			auto right = std::max(oldRect.getRight(), newRect.getRight()) + 4;
			auto bottom = std::max(oldRect.getBottom(), newRect.getBottom()) + 4;
			m_graphicsView->scene()->update(left, top, right - left, bottom - top);
		}
		else if (m_selector.visible())
		{
			//Set the correct cursor when it is on the edge of the selector
			int edges = m_selector.getResizeEdge(pos.x(), pos.y());
			if (edges > 0)
			{
				int cursors[] = { Qt::SizeHorCursor, Qt::SizeVerCursor, Qt::SizeFDiagCursor, Qt::SizeHorCursor, Qt::SizeBDiagCursor, Qt::SizeBDiagCursor, Qt::SizeVerCursor, Qt::SizeVerCursor, Qt::SizeBDiagCursor,Qt::ArrowCursor, Qt::ArrowCursor,  Qt::SizeFDiagCursor };

				if (edges < 13)
				{
					m_graphicsView->setCursor((Qt::CursorShape)cursors[edges - 1]);
				}
				return;
			}
			else m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
		}
	}

	void EditorTabWidget::graphicsMouseDoubleClick(QMouseEvent* event)
	{
		auto pos = m_graphicsView->mapToScene(event->pos());

		if (event->button() == Qt::MouseButton::LeftButton)
		{
			if (m_selection && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS)
			{
				auto objectSelection = static_cast<ObjectSelection*>(m_selection);

				auto entity = objectSelection->getEntityAtPoint(pos.x(), pos.y());
				setSelection(nullptr);

				if(entity)
				{
					if (entity->getEntityType() == LevelEntityType::ENTITY_LINK && QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
					{
						auto link = static_cast<LevelLink*>(entity);
						emit openLevel(link->getNextLevel());

						return;
						
					}
					else {
						entity->openEditor();
						m_graphicsView->redraw();
					}
				}
			}
		}
	}

	
	void EditorTabWidget::floodFillPattern(double x, double y, int layer, const Tilemap* pattern, QList<TileInfo>* outputNodes)
	{
		QSet<int> startTiles;
		QSet<QPair<int, int>> scannedIndexes;
		auto startTileX = int(std::floor(x / 16));
		auto startTileY = int(std::floor(y / 16));

		auto startTileX2 = (int)std::ceil(double(startTileX) / pattern->getHCount()) * pattern->getHCount();
		auto startTileY2 = (int)std::ceil(double(startTileY) / pattern->getVCount()) * pattern->getVCount();

		//Get a set of tiles that can be replaced
		for (auto y = 0; y < pattern->getVCount(); ++y)
		{
			for (auto x = 0; x < pattern->getHCount(); ++x)
			{
				int tile = 0;
				if (tryGetTileAt((startTileX + x) * 16, (startTileY + y) * 16, &tile))
					startTiles.insert(tile);
			}
		}

		QStack<QPair<int, int> > nodes;
		auto addNode = [&](int x, int y)
		{
			QPair<int, int> a(x, y);

			if (!scannedIndexes.contains(a))
			{
				scannedIndexes.insert(a);
				nodes.push(a);
			}

		};
		addNode(startTileX, startTileY);

		Level* level = nullptr;
		while (nodes.count() > 0)
		{
			auto node = nodes.pop();

			auto nodeXPos = node.first * 16.0;
			auto nodeYPos = node.second * 16.0;

			if (level == nullptr || nodeXPos < level->getX() || nodeXPos >= level->getRight() || nodeYPos < level->getY() || nodeYPos >= level->getBottom())
				level = getLevelAt(nodeXPos, nodeYPos);

			if (level != nullptr)
			{
				auto tilemap = level->getTilemap(m_selectedTilesLayer);
				if (tilemap != nullptr)
				{
					//left/top position within the destination "Tilemap"
					auto tilemapX = node.first - int(std::floor(tilemap->getX() / 16.0));
					auto tilemapY = node.second - int(std::floor(tilemap->getY() / 16.0));

					//Get the correct pattern co-ordinates
					auto deltaX = startTileX2 + (node.first - startTileX);
					auto deltaY = startTileY2 + (node.second - startTileY);

					auto patternTileX = deltaX % pattern->getHCount();
					auto patternTileY = deltaY % pattern->getVCount();


					auto patternTile = pattern->getTile(patternTileX, patternTileY);
					int tile = 0;
					if (tilemap->tryGetTile(tilemapX, tilemapY, &tile) && !Tilemap::IsInvisibleTile(tile))
					{
						if (startTiles.contains(tile))
						{
							setModified(level);

							if (outputNodes)
								outputNodes->push_back(TileInfo { (unsigned short)node.first, (unsigned short)node.second, tile });

							tilemap->setTile(tilemapX, tilemapY, patternTile);


							addNode(node.first - 1, node.second);
							addNode(node.first, node.second - 1);

							addNode(node.first + 1, node.second);
							addNode(node.first, node.second + 1);


						}
					}
				}
			}
		}
		
	}

	int EditorTabWidget::getUnitWidth() const
	{
		if (m_overworld)
			return m_overworld->getUnitWidth();
		else if (m_level)
			return m_level->getUnitWidth();

		return 1;
	}

	int EditorTabWidget::getUnitHeight() const
	{
		if (m_overworld)
			return m_overworld->getUnitHeight();
		else if (m_level)
			return m_level->getUnitHeight();

		return 1;
	}

	int EditorTabWidget::getWidth() const
	{
		if (m_overworld)
			return m_overworld->getWidth();
		else if (m_level)
			return m_level->getWidth();
		return 0;
	}

	int EditorTabWidget::getHeight() const
	{
		if (m_overworld)
			return m_overworld->getHeight();
		else if (m_level)
			return m_level->getHeight();
		return 0;
	}

	void EditorTabWidget::graphicsMouseWheel(QWheelEvent* event)
	{
		
		if (QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier))
		{

			if (event->angleDelta().y() < 0)
			{
				ui.zoomSlider->setValue(ui.zoomSlider->value() - 1);
			}
			else {
				ui.zoomSlider->setValue(ui.zoomSlider->value() + 1);
			}
			m_graphicsView->redraw();
		}else event->ignore();
	}

	void EditorTabWidget::graphicsKeyPress(QKeyEvent* event)
	{

		m_graphicsView->redraw();
		if (event->key() == Qt::Key::Key_Shift)
		{
			
		}
	}

	void EditorTabWidget::zoomMoved(int position)
	{
		auto oldMousePos = m_graphicsView->mapToScene(m_graphicsView->mapFromGlobal(QCursor::pos()));

		qreal zoomFactors[] = {
			0.25,
			0.5,
			0.75,
			1.0,
			2,
			3,
			4
		};

		auto zoomLevel = zoomFactors[position];

		auto scaleX = zoomLevel;
		auto scaleY = zoomLevel;

		m_graphicsView->setAntiAlias(zoomLevel <= 1);

		m_graphicsView->resetTransform();
		m_graphicsView->scale(scaleX, scaleY);
		
		auto newMousePos = m_graphicsView->mapToScene(m_graphicsView->mapFromGlobal(QCursor::pos()));

		auto deltaMousePos = newMousePos - oldMousePos;

		m_graphicsView->translate(deltaMousePos.x(), deltaMousePos.y());

		ui.zoomLabel->setText("x" + QString::number(zoomFactors[position]));
	}

	void EditorTabWidget::layerChanged(int value)
	{
		setSelection(nullptr);
		m_selector.setVisible(false);


		m_selectedTilesLayer = value;

		auto it = m_visibleLayers.find(m_selectedTilesLayer);
		if (it == m_visibleLayers.end())
			m_visibleLayers[m_selectedTilesLayer] = true;

		m_selectedLayerVisible = m_visibleLayers[m_selectedTilesLayer];


		if(m_selectedLayerVisible)
			ui.showLayerButton->setIcon(m_eyeOpen);
		else ui.showLayerButton->setIcon(m_eyeClosed);


		m_graphicsView->redraw();
	}

	void EditorTabWidget::layerVisibilityChanged()
	{
		m_visibleLayers[m_selectedTilesLayer] = !m_visibleLayers[m_selectedTilesLayer];
		m_selectedLayerVisible = m_visibleLayers[m_selectedTilesLayer];


		if (m_selectedLayerVisible)
			ui.showLayerButton->setIcon(m_eyeOpen);
		else ui.showLayerButton->setIcon(m_eyeClosed);
		m_graphicsView->redraw();
	}

	void EditorTabWidget::floodFillPatternClicked(bool checked)
	{
		
		//Get suitabable tile pattern
		auto pattern = getSelectionTiles();
		
		if (pattern != nullptr)
		{
			if (pattern->getHCount() * pattern->getVCount() > 0)
			{
				m_fillPattern = *pattern;

				
				m_selector.setVisible(false);


				setSelection(nullptr);
				ui.floodFillPatternButton->setEnabled(hasSelectionTiles());
				ui.floodFillButton->setChecked(true);
			}
			delete pattern;
		}

		
	}

	void EditorTabWidget::preloadOverworldClicked(bool checked)
	{
		if (m_overworld)
		{
			m_overworld->preloadLevels();
		}
	}

	void EditorTabWidget::editLinksClicked(bool checked)
	{
		if (checked)
		{
			ui.editSignsButton->setChecked(false);
			if (m_overworld)
			{
				m_graphicsView->setCursor(Qt::CursorShape::OpenHandCursor);
				m_graphicsView->redraw();
			}
			else {
				ui.editLinksButton->setChecked(false);
				auto level = getActiveLevel();
				ListLinksDialog frm(level, this);
				frm.exec();


			}
		}
		else {
			m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
			m_graphicsView->redraw();
		}
	}

	void EditorTabWidget::newLinkClicked(bool checked)
	{
		auto rect = m_selector.getSelection();

		auto rootLinkLevel = getLevelAt(rect.getX(), rect.getY());

		if (rootLinkLevel)
		{
			auto link = new LevelLink(this, rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight(), false);
			link->setLevel(rootLinkLevel);

			EditLinkDialog frm(link, this);
			if (frm.exec() == QDialog::Accepted)
			{
				auto undoCommand = new QUndoCommand();
				auto levels = getLevelsInRect(*link);

				for (auto level : levels)
				{
					auto newLink = link->duplicate();
					newLink->setLevel(level);

					auto rect = level->clampEntity(newLink);
					newLink->setX(rect.getX());
					newLink->setY(rect.getY());
					newLink->setWidth(rect.getWidth());
					newLink->setHeight(rect.getHeight());

					new CommandAddEntity(this, newLink, undoCommand);

				}

				addUndoCommand(undoCommand);
			}
			delete link;
		}


	}

	void EditorTabWidget::newSignClicked(bool checked)
	{
		auto rect = m_selector.getSelection();

		auto rootSignLevel = getLevelAt(rect.getX(), rect.getY());

		if (rootSignLevel)
		{
			auto sign = new LevelSign(this, rect.getX(), rect.getY(), 32, 16);
			sign->setLevel(rootSignLevel);

			addUndoCommand(new CommandAddEntity(this, sign));

			EditSignsDialog frm(rootSignLevel, this, sign);
			if (frm.exec() == QDialog::Accepted)
			{

			}
		}

	}

	void EditorTabWidget::editSignsClicked(bool checked)
	{
		if (checked)
		{
			ui.editLinksButton->setChecked(false);
			if (m_overworld)
			{
				m_graphicsView->setCursor(Qt::CursorShape::OpenHandCursor);
				m_graphicsView->redraw();
			}
			else {
				ui.editSignsButton->setChecked(false);
				auto level = getActiveLevel();
				EditSignsDialog frm(level, this);
				frm.exec();


			}
		}
		else {
			m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
			m_graphicsView->redraw();
		}
	}

	void EditorTabWidget::undoClicked(bool checked)
	{
		setSelection(nullptr);
		m_undoStack.undo();
		ui.redoButton->setEnabled(m_undoStack.canRedo());
		ui.undoButton->setEnabled(m_undoStack.canUndo());

		m_graphicsView->redraw();
	}

	void EditorTabWidget::redoClicked(bool checked)
	{
		setSelection(nullptr);
		m_undoStack.redo();
		ui.redoButton->setEnabled(m_undoStack.canRedo());
		ui.undoButton->setEnabled(m_undoStack.canUndo());
		m_graphicsView->redraw();
	}

	void EditorTabWidget::cutPressed()
	{
		copyPressed();

		if (m_selection)
		{
			m_selection->clearSelection(this);
			setSelection(nullptr);
			m_graphicsView->redraw();
		}
	}


	void EditorTabWidget::copyPressed()
	{
		if (m_selector.visible())
			doTileSelection();

		if (m_selection != nullptr)
			m_selection->clipboardCopy();
	}

	void EditorTabWidget::pastePressed()
	{
		doPaste(true);

	}

	void EditorTabWidget::deleteClicked(bool checked)
	{
		if (m_selector.visible())
			doTileSelection();

		if (m_selection)
		{
			m_selection->clearSelection(this);
			setSelection(nullptr);
			m_graphicsView->redraw();
		}
	}

	void EditorTabWidget::screenshotClicked(bool checked)
	{
		ScreenshotDialog frm(this);
		frm.exec();
	}


	void EditorTabWidget::saveClicked(bool)
	{
		if (m_overworld)
		{

			auto levels = m_overworld->getModifiedLevels();
			if (levels.size())
			{
				for (auto level : levels)
				{
					if (level->getFileName().isEmpty())
					{
						QString fullPath;

						if (m_resourceManager.locateFile(level->getName(), &fullPath))
							level->setFileName(fullPath);


					}

				}
			}

			SaveOverworldDialog dialog(this);
			if (dialog.exec() == QDialog::Accepted)
			{
				auto modifiedLevels = dialog.getCheckedLevels();
				bool resetModification = true;

				//Save all the checked ones, and mark all the unchecked ones as saved (but dont save)
				for (auto level : levels)
				{
					if (modifiedLevels.indexOf(level) == -1)
						level->setModified(false);
					else if (saveLevel(level))
					{
						level->setModified(false);
					}
					else resetModification = false;
				}

				m_overworld->saveFile(this);
				if(resetModification)
					setUnmodified();
			}
		}
		else if (m_level)
		{
			if (m_level->getFileName().isEmpty() && m_level->getName() != "")
			{
				QString fullPath;

				if (m_resourceManager.locateFile(m_level->getName(), &fullPath))
					m_level->setFileName(fullPath);
			}
			if (saveLevel(m_level))
			{
				setUnmodified();
			}
		}
	}

	void EditorTabWidget::saveAsClicked(bool checked)
	{
		if (m_level)
		{
			auto fullPath = m_resourceManager.getFileSystem()->getSaveFileName("Save Level As", QString(), FileFormatManager::instance()->getLevelSaveFilters());

			if (!fullPath.isEmpty())
			{
				QFileInfo fi(fullPath);

				m_level->setName(fi.fileName());
				m_level->setFileName(fullPath);

				FileFormatManager::instance()->applyFormat(m_level);
				if (saveLevel(m_level))
					setUnmodified();

				m_graphicsView->redraw();
			}
		}
	}

	void EditorTabWidget::tilesetsIndexChanged(int index)
	{
		if (index >= 0)
		{
			changeTileset(static_cast<Tileset*>(static_cast<QStandardItemModel*>(ui_tilesetsClass.tilesetsCombo->model())->item(index)));
		}

		if (m_tilesetImage)
			ui_tilesetsClass.graphicsView->setSceneRect(0, 0, m_tilesetImage->width(), m_tilesetImage->height());

		m_graphicsView->redraw();
		ui_tilesetsClass.graphicsView->redraw();


	}



	void EditorTabWidget::tilesetDeleteClicked(bool checked)
	{
		auto currentIndex = ui_tilesetsClass.tilesetsCombo->currentIndex();
		if(currentIndex >= 0)
			ui_tilesetsClass.tilesetsCombo->removeItem(currentIndex);
	}

	void EditorTabWidget::tilesetRefreshClicked(bool checked)
	{
		auto tilesetName = ui_tilesetsClass.tilesetsCombo->currentText();

		setTileset(tilesetName);

		m_resourceManager.updateResource(m_tileset.getImageName());
		ui_tilesetsClass.graphicsView->redraw();
		m_graphicsView->redraw();
	}

	void EditorTabWidget::tilesetNewClicked(bool checked)
	{
		auto tilesetName = QInputDialog::getText(nullptr, "Add new tileset", "Tileset Name: ");

		if (!tilesetName.isEmpty())
		{
			auto tileset = new Tileset();
			tileset->setText(tilesetName);

			tileset->setFileName("./" + tilesetName + ".json");

			if (tileset)
			{
				EditTilesetDialog frm(tileset, m_resourceManager);
				if (frm.exec() == QDialog::Accepted)
				{
					*tileset = frm.getTileset();

					auto model = static_cast<QStandardItemModel*>(ui_tilesetsClass.tilesetsCombo->model());
					model->appendRow(tileset);

					if (ui_tilesetsClass.tilesetsCombo->model()->rowCount() == 1)
						ui_tilesetsClass.tilesetsCombo->setCurrentIndex(0);
				}
				else delete tileset;
			}
		}
	}

	void EditorTabWidget::tilesetOpenClicked(bool checked)
	{
		
		auto fileName = QFileDialog::getOpenFileName(nullptr, "Select Tileset", m_resourceManager.getRootDir(), "Image Files (*.png *.gif);;JSON File (*.json)");
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);
			auto directory = fi.absolutePath() + "/";

			m_resourceManager.addSearchDir(directory);

			QString name = fi.fileName();

			if (fi.suffix() == "json")
				name = fi.baseName();


			auto tileset = Tileset::loadTileset(name, m_resourceManager);

			if (tileset)
			{
				auto model = static_cast<QStandardItemModel*>(ui_tilesetsClass.tilesetsCombo->model());
				model->appendRow(tileset);

				if (ui_tilesetsClass.tilesetsCombo->model()->rowCount() == 1)
					ui_tilesetsClass.tilesetsCombo->setCurrentIndex(0);
			}
		}
	}

	void EditorTabWidget::tileGroupNewClicked(bool checked)
	{
		auto groupName = QInputDialog::getText(nullptr, "Add new group", "Name: ");
		if (!groupName.isEmpty())
		{
			auto group = new TileGroupModel();
			group->setName(groupName);

			static_cast<TileGroupListModel*>(ui_tileObjectsClass.groupCombo->model())->addTileGroup(group);

		}
	}

	void EditorTabWidget::tileGroupDeleteClicked(bool checked)
	{
		auto groupListModel = static_cast<TileGroupListModel*>(ui_tileObjectsClass.groupCombo->model());
		if (groupListModel)
		{
			groupListModel->removeTileGroup(ui_tileObjectsClass.groupCombo->currentIndex());
		}
	}

	void EditorTabWidget::tileGroupImportClicked(bool checked)
	{
		static const QString base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		auto fileName = QFileDialog::getOpenFileName(nullptr, "Select GRAAL tile object", QString(), "Text File (*.txt)");
		if (!fileName.isEmpty())
		{
			auto groupName = QInputDialog::getText(nullptr, "Add new group", "Name: ");
			if (!groupName.isEmpty())
			{


				QFile file(fileName);

				if (file.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text))
				{
					auto group = new TileGroupModel();
					group->setName(groupName);

					QTextStream textStream(&file);

					if (textStream.readLine() == "GOBJSET01")
					{
						while (!textStream.atEnd())
						{
							auto line = textStream.readLine();

							auto words = line.split(' ');
							if (words.count() >= 1)
							{
								if (words[0] == "OBJECT" && words.count() >= 3)
								{
									auto hcount = words[1].toInt();
									auto vcount = words[2].toInt();

									QString name = words[3];
									for (auto i = 4U; i < words.count(); ++i)
										name += words[i] + " ";
									name = name.trimmed();

									auto tileObject = new TileObject(hcount, vcount);
									tileObject->setName(name);
									for(int y = 0; !textStream.atEnd(); ++y)
									{
										auto tileData = textStream.readLine();
										if (tileData == "OBJECTEND")
											break;

										if (tileData.length() >= hcount * 2)
										{
											for (size_t ii = 0u; ii < hcount; ++ii)
											{
												char byte1 = tileData[ii * 2].unicode();
												char byte2 = tileData[1 + ii * 2].unicode();

												auto graalTile = (int)((base64.indexOf(byte1) << 6) + base64.indexOf(byte2));
												auto tile = Level::convertFromGraalTile(graalTile);
												tileObject->setTile(ii, y, tile);

											}
										}
									}
					
									group->addTileObject(tileObject);


								}
							}
						}
					}
					static_cast<TileGroupListModel*>(ui_tileObjectsClass.groupCombo->model())->addTileGroup(group);

				}
			}
		}
	}

	void EditorTabWidget::tileGroupIndexChanged(int index)
	{
		auto tileGroupList = static_cast<TileGroupListModel*>(ui_tileObjectsClass.groupCombo->model());
		if (tileGroupList)
		{
			if (index >= 0)
			{
				auto tileGroup = tileGroupList->at(index);
				if (tileGroup != nullptr)
				{
					ui_tileObjectsClass.objectsCombo->setModel(tileGroup);
				}
			} else ui_tileObjectsClass.objectsCombo->setModel(nullptr);
		}
	}

	void EditorTabWidget::tileObjectNewClicked(bool checked)
	{
		if (m_selector.visible())
			doTileSelection();

		if (m_selection != nullptr && m_selection->getSelectionType() == SelectionType::SELECTION_TILES)
		{
			auto tileSelection = static_cast<TileSelection*>(m_selection);

			auto objectName = QInputDialog::getText(nullptr, "Add new object", "Name: ");
			if (!objectName.isEmpty())
			{
				auto tileObject = new TileObject(tileSelection->getHCount(), tileSelection->getVCount());
				tileObject->setName(objectName);

				for (int y = 0; y < tileSelection->getVCount(); ++y)
				{
					for (int x = 0; x < tileSelection->getHCount(); ++x)
					{
						int tile = tileSelection->getTile(x, y);
						tileObject->setTile(x, y, tile);
					}
				}
				auto tileGroupModel = static_cast<TileGroupModel*>(ui_tileObjectsClass.objectsCombo->model());
				if (tileGroupModel)
				{
					tileGroupModel->addTileObject(tileObject);
				}
			}
		}
	}

	void EditorTabWidget::tileObjectDeleteClicked(bool checked)
	{
		auto tileGroupModel = static_cast<TileGroupModel*>(ui_tileObjectsClass.objectsCombo->model());
		if (tileGroupModel)
		{
			auto tileObjectIndex = ui_tileObjectsClass.objectsCombo->currentIndex();
			tileGroupModel->removeTileObject(tileObjectIndex);
		}
	}

	void EditorTabWidget::tileObjectIndexChanged(int index)
	{
		auto tileObject = getCurrentTileObject();
		if (tileObject) {
			ui_tileObjectsClass.graphicsView->setSceneRect(0, 0, tileObject->getWidth(), tileObject->getHeight());
		}
		ui_tileObjectsClass.graphicsView->redraw();
	}

	void EditorTabWidget::objectsNewNPCClicked(bool checked)
	{
		auto selection = new ObjectSelection(0, 0);

		auto npc = new LevelNPC(this, 0, 0, 48, 48);
		
		npc->setImageName("");
		selection->addObject(npc);
		selection->setAlternateSelectionMethod(true);
		selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
		setSelection(selection);

	}

	void EditorTabWidget::objectsNewChestClicked(bool checked)
	{
		auto selection = new ObjectSelection(0, 0);

		auto chest = new LevelChest(this, 0, 0, "greenrupee", -1);
		selection->addObject(chest);
		selection->setAlternateSelectionMethod(true);
		selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
		setSelection(selection);

	}

	void EditorTabWidget::objectsNewBaddyClicked(bool checked)
	{
		auto selection = new ObjectSelection(0, 0);

		auto chest = new LevelGraalBaddy(this, 0.0, 0.0, 0);
		selection->addObject(chest);
		selection->setAlternateSelectionMethod(true);
		selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
		setSelection(selection);
	}


	void EditorTabWidget::objectsRefreshClicked(bool checked)
	{
		auto model = static_cast<ObjectListModel*>(ui_objectClass.objectsTable->model());
		model->clear();


		auto viewRect = getViewRect();

		auto levels = getLevelsInRect(viewRect);
		for (auto level : levels)
		{
			auto& objects = level->getObjects();

			for (auto entity : objects)
			{
				if (entity->getEntityType() == LevelEntityType::ENTITY_NPC)
				{
					auto npc = static_cast<LevelNPC*>(entity);
					model->push_back(npc);
				}
			}
		}
		model->reset();

	}

	void EditorTabWidget::objectsDoubleClick(const QModelIndex& index)
	{
		auto row = index.row();
		auto model = static_cast<ObjectListModel*>(ui_objectClass.objectsTable->model());

		auto entity = model->at(row);
		if (entity != nullptr)
		{
			auto selection = new ObjectSelection(entity->getX(), entity->getY());
			selection->addObject(entity);

			setSelection(selection);
			m_graphicsView->centerOn(entity->getCenterX(), entity->getCenterY());
			m_graphicsView->redraw();

		}

	}

	void EditorTabWidget::deleteEdgeLinksClicked(bool checked)
	{
		if (!m_overworld)
		{
			QMessageBox::information(nullptr, "", "This function is for overworlds only");
		}
		else {
			if (QMessageBox::question(nullptr, "Warning", "Are you sure you want to remove all edge links from all levels in the overworld?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
			{
				auto undoCommand = new QUndoCommand("Delete overworld edge links");
				for (auto level : m_overworld->getLevelList())
				{
					loadLevel(level);

					if (level->getLoaded())
					{
						auto& links = level->getLinks();

						for (auto link : links) {
							if (link->isPossibleEdgeLink() && m_overworld->getLevelList().contains(link->getNextLevel()))
							{
								deleteEntity(link, undoCommand);
							}
						}
					}
				}

				addUndoCommand(undoCommand);
				m_graphicsView->redraw();
			}
		}
	}

	void EditorTabWidget::trimScriptEndingsClicked(bool checked)
	{
		if (QMessageBox::question(nullptr, "Warning", "This function will trim the endings of all NPC scripts. Do you wish to proceed?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
			return;

		QList<Level*> levels;

		if (m_overworld)
			levels = m_overworld->getLevelList().values();
		else if (m_level) {
			levels.push_back(m_level);
		}

		auto rstrip = [](const QString& str) -> QString {
			int n = str.size() - 1;
			for (; n >= 0; --n) {
				if (!str.at(n).isSpace()) {
					return str.left(n + 1);
				}
			}
			return "";
		};

		for (auto level : levels)
		{
			loadLevel(level);

			if (level->getLoaded())
			{
				auto& objects = level->getObjects();
				for (auto object : objects)
				{
					if (object->getEntityType() == LevelEntityType::ENTITY_NPC)
					{
						auto npc = static_cast<LevelNPC*>(object);

						auto script = rstrip(npc->getCode());

						if (script != npc->getCode())
						{
							npc->setCode(script);
							setModified(level);
						}

					}
				}
			}
		}
	}
	
	void EditorTabWidget::trimSignEndingsClicked(bool checked)
	{
		if (QMessageBox::question(nullptr, "Warning", "This function will trim the endings of all Signs. Do you wish to proceed?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
			return;

		QList<Level*> levels;

		if (m_overworld)
			levels = m_overworld->getLevelList().values();
		else if (m_level) {
			levels.push_back(m_level);
		}

		auto rstrip = [](const QString& str) -> QString {
			int n = str.size() - 1;
			for (; n >= 0; --n) {
				if (!str.at(n).isSpace()) {
					return str.left(n + 1);
				}
			}
			return "";
		};

		for (auto level : levels)
		{
			loadLevel(level);

			if (level->getLoaded())
			{
				auto& signs = level->getSigns();
				for (auto sign : signs)
				{
					auto text = rstrip(sign->getText());

					if (text != sign->getText())
					{
						sign->setText(text);
						setModified(level);
					}
				}
			}
		}
	}

	void EditorTabWidget::gridValueChanged(int)
	{
		QPixmap a;
		m_gridImage.swap(a);
		m_graphicsView->redraw();
	}

	void EditorTabWidget::selectorGone()
	{
		ui.newLinkButton->setEnabled(false);
		ui.newSignButton->setEnabled(false);
		ui.copyButton->setEnabled(false);
		ui.cutButton->setEnabled(false);
		ui.floodFillPatternButton->setEnabled(hasSelectionTiles());
	}

	void EditorTabWidget::selectorMade()
	{
		if (m_selector.visible())
		{
			ui.newLinkButton->setEnabled(true);
			ui.newSignButton->setEnabled(true);
			ui.copyButton->setEnabled(true);
			ui.cutButton->setEnabled(true);

			ui.floodFillPatternButton->setEnabled(true);
		}
	}

	void EditorTabWidget::selectionMade()
	{
		ui.copyButton->setEnabled(true);
		ui.cutButton->setEnabled(true);
	}

	void EditorTabWidget::selectionGone()
	{
		ui.copyButton->setEnabled(false);
		ui.cutButton->setEnabled(false);
		ui.floodFillPatternButton->setEnabled(hasSelectionTiles());
	}

	bool EditorTabWidget::saveLevel(Level* level)
	{
		if (level->getFileName().isEmpty())
		{
			auto fullPath = m_resourceManager.getFileSystem()->getSaveFileName("Save Level", QString(), FileFormatManager::instance()->getLevelSaveFilters());

			if (!fullPath.isEmpty())
			{
				QFileInfo fi(fullPath);

				level->setName(fi.fileName());
				level->setFileName(fullPath);
			}
		}

		if (level->getFileName().isEmpty())
		{
			QMessageBox::critical(nullptr, "Unable to save file", "Invalid file path");
		}
		else {
			level->saveFile(this);
			return true;
		}
		return false;
	}

	TileObject* EditorTabWidget::getCurrentTileObject()
	{
		auto tileGroupModel = static_cast<TileGroupModel*>(ui_tileObjectsClass.objectsCombo->model());
		if (tileGroupModel)
		{
			auto tileObjectIndex = ui_tileObjectsClass.objectsCombo->currentIndex();
			if (tileObjectIndex >= 0)
				return static_cast<TileObject*>(tileGroupModel->at(tileObjectIndex));
		}
		return nullptr;
	}

}
