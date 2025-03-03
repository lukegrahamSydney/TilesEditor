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
#include "LevelObjectInstance.h"
#include "EditTilesets.h"
#include "ResourceManagerFileSystem.h"

namespace TilesEditor
{
	sgs_Variable EditorTabWidget::sgs_classMembers;
	sgs_ObjInterface EditorTabWidget::sgs_interface;

	EditorTabWidget::EditorTabWidget(IEngine* engine, AbstractResourceManager* resourceManager)
		: m_engine(engine), m_fillPattern(nullptr, 0.0, 0.0, 1, 1, 0), m_resourceManager(resourceManager)
	{
		m_resourceManager->incrementRef();
		sgs_CreateObject(engine->getScriptContext(), &m_thisObject, this, &sgs_interface);
		sgs_CreateDict(engine->getScriptContext(), &m_sgsUserTable, 0);
		engine->addCPPOwnedObject(m_thisObject);

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

		m_eyeOpen = QPixmap(":/MainWindow/icons/tinycolor/icons8-eye-16.png");
		m_eyeClosed = QPixmap(":/MainWindow/icons/tinycolor/icons8-invisible-16.png");

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
		m_graphicsView->setAntiAlias(false);
		ui_objectClass.objectsTable->setModel(new ObjectListModel());
		ui_objectClass.objectsTable->setColumnWidth(2, 70);
		ui_objectClass.objectsTable->setColumnWidth(3, 70);

		

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
		//m_graphicsView->setCornerWidget(ui_tilesetsClass.tileIcon);

		//ui_tilesetsClass.tilesetsList->setModel(new QStandardItemModel());


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
		connect(ui_tilesetsClass.graphicsView, &GraphicsView::mouseWheelEvent, this, &EditorTabWidget::tilesetMouseWheel);
		connect(ui_tilesetsClass.tilesetsCombo, &QComboBox::currentIndexChanged, this, &EditorTabWidget::tilesetsIndexChanged);
		connect(ui_tilesetsClass.deleteButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetDeleteClicked);
		connect(ui_tilesetsClass.refreshButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetRefreshClicked);
		connect(ui_tilesetsClass.newButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetNewClicked);
		connect(ui_tilesetsClass.openButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetOpenClicked);
		connect(ui_tilesetsClass.editButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetEditClicked);
		connect(ui_tilesetsClass.tilesetOrderButton, &QAbstractButton::clicked, this, &EditorTabWidget::tilesetEditOrderClicked);

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
		connect(m_graphicsView, &GraphicsView::contentsScrolled, this, &EditorTabWidget::graphicsScrolled);

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

		connect(ui.fadeLayersButton, &QToolButton::clicked, m_graphicsView, &GraphicsView::redraw);
		connect(ui.hcountSpinBox, &QSpinBox::valueChanged, this, &EditorTabWidget::gridValueChanged);
		connect(ui.vcountSpinBox, &QSpinBox::valueChanged, this, &EditorTabWidget::gridValueChanged);
		connect(ui.gridButton, &QToolButton::released, m_graphicsView, &GraphicsView::redraw);

		connect(ui_tilesetsClass.tileIcon, &TilesEditor::CustomPaintWidget::mouseDoubleClick, this, &EditorTabWidget::tileIconMouseDoubleClick);
	
		auto downKeyShortcut = new QShortcut(this);
		downKeyShortcut->setKey(Qt::Key_Down);
		connect(downKeyShortcut, &QShortcut::activated, this, &EditorTabWidget::downKeyPressed);

		auto upKeyShortcut = new QShortcut(this);
		upKeyShortcut->setKey(Qt::Key_Up);
		connect(upKeyShortcut, &QShortcut::activated, this, &EditorTabWidget::upKeyPressed);

		auto leftKeyShortcut = new QShortcut(this);
		leftKeyShortcut->setKey(Qt::Key_Left);
		connect(leftKeyShortcut, &QShortcut::activated, this, &EditorTabWidget::leftKeyPressed);

		auto rightKeyShortcut = new QShortcut(this);
		rightKeyShortcut->setKey(Qt::Key_Right);
		connect(rightKeyShortcut, &QShortcut::activated, this, &EditorTabWidget::rightKeyPressed);

		auto undoKeyShortcut = new QShortcut(QKeySequence("Ctrl+Z"), this);
		connect(undoKeyShortcut, &QShortcut::activated, ui.undoButton, &QAbstractButton::click);

		auto redoKeyShortcut = new QShortcut(QKeySequence("Ctrl+Y"), this);
		connect(redoKeyShortcut, &QShortcut::activated, ui.redoButton, &QAbstractButton::click);
	}

	EditorTabWidget::~EditorTabWidget()
	{
		m_undoStack.clear();
		m_thisObject.data.O->data = nullptr;
		m_engine->removeCPPOwnedObject(m_thisObject);
		sgs_Release(m_engine->getScriptContext(), &m_thisObject);
		sgs_Release(m_engine->getScriptContext(), &m_sgsUserTable);

		m_objectsContainer->deleteLater();
		m_tileObjectsContainer->deleteLater();
		m_tilesetsContainer->deleteLater();

		if (m_tilesetImage)
		{
			m_resourceManager->freeResource(m_tilesetImage);
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

		m_resourceManager->removeListener(this);
		m_resourceManager->decrementAndDelete();

	}

	void EditorTabWidget::renderFloodFillPreview(const QPointF& point, QSet<Level*>& viewLevels, QPainter* painter, const QRectF& viewRect)
	{
		QSet<int> startTiles;
		QSet<QPair<int, int>> scannedIndexes;

		auto startTileX = int(std::floor(point.x() / 16));
		auto startTileY = int(std::floor(point.y() / 16));



		auto purpleSquareRect = QRect(startTileX * 16, startTileY * 16, m_fillPattern.getWidth(), m_fillPattern.getHeight());

		auto startTileX2 = (int)std::ceil(double(startTileX) / m_fillPattern.getHCount()) * m_fillPattern.getHCount();
		auto startTileY2 = (int)std::ceil(double(startTileY) / m_fillPattern.getVCount()) * m_fillPattern.getVCount();

		//Get a set of tiles that can be replaced
		for (auto y = 0; y < m_fillPattern.getVCount(); ++y)
		{
			for (auto x = 0; x < m_fillPattern.getHCount(); ++x)
			{
				int tile = 0;
				if (tryGetTileAt(QPointF((startTileX + x) * 16, (startTileY + y) * 16), &tile))
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
				level = getLevelAt(QPointF(nodeXPos, nodeYPos));
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
							QRectF rect(node.first * 16, node.second * 16, 16, 16);
							if (!rect.intersects(purpleSquareRect))
							{
								
								level->drawTile(node.first * 16, node.second * 16, m_tilesetImage, Tilemap::GetTileX(patternTile), Tilemap::GetTileY(patternTile), painter);
							}
								//painter->drawPixmap(node.first * 16, node.second * 16, m_tilesetImage->pixmap(), Tilemap::GetTileX(patternTile) * 16, Tilemap::GetTileY(patternTile) * 16, 16, 16);


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

	void EditorTabWidget::renderScene(QPainter * painter, const QRectF & _rect)
	{
		QRectF viewRect(std::floor(_rect.x()), std::floor(_rect.y()), _rect.width() + 2, _rect.height() + 2);

	
		//forcing the view x/y offset as a whole number prevents tile alignment errors
		auto transform = painter->transform();
		QTransform newTransform(transform.m11(), transform.m12(), transform.m21(), transform.m22(), std::floor(transform.dx()), std::floor(transform.dy()));
		painter->setTransform(newTransform);

		painter->fillRect(viewRect, QColorConstants::Black);


		auto mousePos = m_graphicsView->mapToScene(m_graphicsView->mapFromGlobal(QCursor::pos()));

	
		auto drawLevels = getLevelsInRect(QRectF(viewRect.x() - 1000, viewRect.y() - 1000, viewRect.width() + 2000, viewRect.height() + 2000));
		//Draw npcs
		QSet<AbstractLevelEntity*> drawObjects;


		if (m_tilesetImage)
		{

			//Draw tiles (and make sure level is loaded)
			for (auto level : drawLevels)
			{
				auto& layers = level->getTileLayers();

				for (auto tilemap : layers)
				{
					if(isLayerVisible(tilemap->getLayerIndex()))
					{
						drawObjects.insert(tilemap);
					}
				}
			}
		}



		if (m_overworld)
		{
			m_overworld->getEntitySpatialMap()->search(viewRect, false, drawObjects);
		}
		else if (m_level) {
			m_level->getEntitySpatialMap()->search(viewRect, false, drawObjects);
		}

		QList<AbstractLevelEntity*> sortedObjects(drawObjects.begin(), drawObjects.end());
		std::sort(sortedObjects.begin(), sortedObjects.end(), AbstractLevelEntity::sortByDepthFunc);


		bool drawnFloodFill = !ui.floodFillButton->isChecked();
		for (auto entity : sortedObjects)
		{
			if (!drawnFloodFill && entity->getRealDepth() > (double)m_selectedTilesLayer)
			{
				drawnFloodFill = true;
				renderFloodFillPreview(mousePos, drawLevels, painter, viewRect);
			}

			if (entity->getEntityType() == LevelEntityType::ENTITY_NPC && !m_showNPCs->isChecked())
				continue;

			if (entity->getEntityType() == LevelEntityType::ENTITY_LINK && !m_showLinks->isChecked())
				continue;

			if (entity->getEntityType() == LevelEntityType::ENTITY_SIGN && !m_showSigns->isChecked())
				continue;



			if (entity->getEntityType() == LevelEntityType::ENTITY_TILEMAP && m_tilesetImage)
			{
				auto tilemap = static_cast<Tilemap*>(entity);
				auto fade = m_selectedTilesLayer != tilemap->getLayerIndex() && ui.fadeLayersButton->isChecked();
				if (fade)
				{
					painter->setOpacity(0.33);

					if (tilemap->getLevel())
						tilemap->getLevel()->drawTilemap(tilemap, m_tilesetImage, tilemap->getX(), tilemap->getY(), painter, viewRect);

					painter->setOpacity(1.0);
				}
				else {
					if (tilemap->getLevel())
						tilemap->getLevel()->drawTilemap(tilemap, m_tilesetImage, tilemap->getX(), tilemap->getY(), painter, viewRect);
				}
				continue;
			}

			if (isLayerVisible(entity->getLayerIndex()))
			{
				auto fade = m_selectedTilesLayer != entity->getLayerIndex() && ui.fadeLayersButton->isChecked();
				entity->loadResources();

				if (fade)
				{
					painter->setOpacity(0.33);
					entity->draw(painter, viewRect);
					painter->setOpacity(1.0);
				} else entity->draw(painter, viewRect);
				
			}
		}


		if (!drawnFloodFill)
		{
			drawnFloodFill = true;
			renderFloodFillPreview(mousePos, drawLevels, painter, _rect);
		}
		if (m_selection != nullptr && m_selection->isVisible())
		{
			if (m_selection->getSelectionType() == SelectionType::SELECTION_TILES)
			{
				auto tileSelection = static_cast<TileSelection*>(m_selection);
				auto levels = getLevelsInRect(tileSelection->getBoundingBox(), true);
				for (auto level : levels)
				{
					if (level->getLoadState() == LoadState::STATE_LOADED)
					{
						painter->setClipping(true);
						painter->setClipRect(level->getRect());
						tileSelection->draw(level, m_tilesetImage, painter, viewRect);
						painter->setClipping(false);
					}
				}
			} else m_selection->draw(painter, viewRect);
		}

		if (m_selector.visible())
		{
			m_selector.draw(painter, viewRect, QColorConstants::White, QColor(255, 255, 255, 60));
		}

		if (selectingLevel())
		{
			auto level = getLevelAt(mousePos);
			if (level)
			{
				painter->fillRect(level->toQRectF(), QColor(128, 128, 128, 128));
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

		

		if (m_overworld != nullptr)
		{
			if (m_selection != nullptr && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS && (m_selection->resizing() || (m_selection->hasMoved() && m_selection->dragging())))
			{
				auto selectionRect = m_selection->getBoundingBox();

				auto selectionLevels = getLevelsInRect(selectionRect);

				QPen pen(QColorConstants::Magenta);
				pen.setWidth(4);
				auto oldPen = painter->pen();
				painter->setPen(pen);
				for (auto level : selectionLevels)
				{


					
					painter->drawRect(level->getX(), level->getY(), level->getWidth(), level->getHeight());

					
				}
				painter->setPen(oldPen);

			}


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

				
				auto right = std::ceil(viewRect.right() / m_gridImage.width()) * m_gridImage.width();
				auto bottom = std::ceil(viewRect.bottom() / m_gridImage.height()) * m_gridImage.height();

				for (auto left = std::floor(viewRect.x() / m_gridImage.width()) * m_gridImage.width(); left < right; left += m_gridImage.width())
				{
					for (auto top = std::floor(viewRect.y() / m_gridImage.height()) * m_gridImage.height(); top < bottom; top += m_gridImage.height())
					{
						painter->drawPixmap(left, top, m_gridImage);
					}
				}

				painter->setCompositionMode(compositionMode);
			}

		}
	}

	void EditorTabWidget::renderTilesetSelection(QPainter* painter, const QRectF& viewRect)
	{
		static const auto backColor = QColor(255, 0, 255);
		painter->fillRect(viewRect, backColor);

		if (m_tilesetImage)
		{
			auto centerPos = m_graphicsView->mapToScene(m_graphicsView->viewport()->rect().center());
			auto level = getLevelAt(centerPos);

			if (level)
				level->drawTileset(m_tilesetImage, backColor, painter, viewRect);
			else painter->drawPixmap(0, 0, m_tilesetImage->pixmap());
		}

		if (m_tilesSelector.visible())
		{
			m_tilesSelector.draw(painter, viewRect, QColorConstants::White, QColor(255, 255, 255, 60));
		}

	}

	void EditorTabWidget::renderTileObjects(QPainter* painter, const QRectF& viewRect)
	{

		//painter->fillRect(rect, QApplication::palette().color(QPalette::AlternateBase));

		if (m_tilesetImage)
		{
			auto centerPos = m_graphicsView->mapToScene(m_graphicsView->viewport()->rect().center());
			auto level = getLevelAt(centerPos);
			auto tileObject = getCurrentTileObject();
			if (tileObject)
			{
				
				//tileObject->draw(painter, viewRect, m_tilesetImage, 0.0, 0.0);

				if (level)
				{
					level->drawTilemap(tileObject, m_tilesetImage, 0.0, 0.0, painter, viewRect);

				}
				else {
					tileObject->draw(painter, viewRect, m_tilesetImage, 0.0, 0.0);
				}
			}

		}

	}

	void EditorTabWidget::paintDefaultTile(QPainter* painter, const QRectF& rect)
	{
		if (Tilemap::IsInvisibleTile(m_defaultTile))
		{
			
			QPen pen(QColorConstants::Red);
			painter->setPen(pen);
			painter->drawLine(4, 4, 12, 12);
			painter->drawLine(4, 12, 12, 4);
		}
		else {
			if (m_tilesetImage)
			{
				auto tileLeft = Tilemap::GetTileX(m_defaultTile);
				auto tileTop = Tilemap::GetTileY(m_defaultTile);

				auto centerPos = m_graphicsView->mapToScene(m_graphicsView->viewport()->rect().center());
				auto level = getLevelAt(centerPos);
				if (level)
				{
					level->drawTile(0.0, 0.0, m_tilesetImage, tileLeft, tileTop, painter);
				} else painter->drawPixmap(0, 0, m_tilesetImage->pixmap(), tileLeft * 16, tileTop * 16, 16, 16);
			}
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

			m_resourceManager->freeResource(m_tilesetImage);
		}

		m_tilesetImage = static_cast<Image*>(m_resourceManager->loadResource(this, imageName, ResourceType::RESOURCE_IMAGE));

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
			m_overworld->setTilesetImageName(tileset->getImageName());
			setModified(nullptr);
		}
		else if (m_level) {
			m_level->setTilesetName(tileset->text());
			m_level->setTilesetImageName(tileset->getImageName());
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
		auto hcount = int(rect.width() / 16);
		auto vcount = int(rect.height() / 16);
		
		auto tilemap = new Tilemap(nullptr, 0.0, 0.0, hcount, vcount, 0);

		for (int y = 0; y < vcount; ++y)
		{
			for (int x = 0; x < hcount; ++x)
			{
				auto tileLeft = int(rect.x() / 16) + x;
				auto tileTop = int(rect.y() / 16) + y;
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
			auto hcount = int(rect.width() / 16);
			auto vcount = int(rect.height() / 16);

			auto retval = new Tilemap(nullptr, 0.0, 0.0, hcount, vcount, 0);

			auto levels = getLevelsInRect(rect);

			for (auto level : levels)
			{
				auto tilemap = level->getTilemap(m_selectedTilesLayer);
				if (tilemap != nullptr)
				{
					int sourceTileX = int((rect.x() - tilemap->getX()) / 16.0);
					int sourceTileY = int((rect.y() - tilemap->getY()) / 16.0);

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

	int EditorTabWidget::getTileTranslucency() const
	{
		return ui.tileTranslucency->value();
	}

	int EditorTabWidget::getDefaultTile() const
	{
		return m_defaultTile;
	}

	void EditorTabWidget::removeTileDefs(const QString& prefix)
	{
		if (m_overworld)
		{
			for (auto level : m_overworld->getLevelList())
			{
				if (level && (prefix == "" || level->getName().startsWith(prefix)))
				{
					level->removeTileDefs();
				}
			}
		}
		else if (m_level)
		{
			if (prefix == "" || m_level->getName().startsWith(prefix))
			{
				m_level->removeTileDefs();
			}
		}
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
			image.fill(QColor(0, 0, 0, 0));
			QPen pen;
			pen.setWidth(1);
			pen.setStyle(Qt::PenStyle::DotLine);
			
			pen.setColor(QColor(255, 255, 255, 210));

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

	void EditorTabWidget::doTileSelection(bool copyOnly)
	{
		if (m_selector.visible())
		{
			auto selectionRect = m_selector.getSelection();

			auto hcount = int(selectionRect.width() / 16);
			auto vcount = int(selectionRect.height() / 16);

			if (hcount * vcount > 0)
			{
				auto tileSelection = new TileSelection(selectionRect.x(), selectionRect.y(), hcount, vcount, m_selectedTilesLayer);

				auto levels = getLevelsInRect(selectionRect);

				for (auto level : levels)
				{
					auto tilemap = level->getTilemap(m_selectedTilesLayer);
					if (tilemap != nullptr)
					{
						int sourceTileX = int((selectionRect.x() - tilemap->getX()) / 16.0);
						int sourceTileY = int((selectionRect.y() - tilemap->getY()) / 16.0);

						for (int y = 0; y < vcount; ++y)
						{
							for (int x = 0; x < hcount; ++x)
							{
								int tile = 0;

								if (tilemap->tryGetTile(x + sourceTileX, y + sourceTileY, &tile) && !Tilemap::IsInvisibleTile(tile))
								{
									//setModified(level);


									tileSelection->setTile(x, y, tile);
								}

							}
						}

					}
				}

				tileSelection->setMouseDragButton(Qt::MouseButton::LeftButton);


				if (copyOnly)
				{
					tileSelection->clipboardCopy();
					delete tileSelection;
				} else setSelection(tileSelection);

				//addUndoCommand(new CommandDeleteTiles(this, selectionRect.getX(), selectionRect.getY(), m_selectedTilesLayer, tileSelection->getTilemap(), m_defaultTile));
			}
		}
	}

	bool EditorTabWidget::doObjectSelection(int x, int y, bool allowAppend)
	{

		auto entity = getEntityAt(QPointF(x, y), true);


		if (entity != nullptr)
		{
			bool append = allowAppend && (m_selection != nullptr && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS);

			auto selection = append ? static_cast<ObjectSelection*>(m_selection) : new ObjectSelection(x, y, m_selectedTilesLayer);


			selection->addObject(entity);

			if (!append)
				setSelection(selection);


			return true;
		}

		return false;
	}

	QString EditorTabWidget::getFileName() const
	{
		if (m_overworld)
			return m_overworld->getFileName();
		else if (m_level)
			return m_level->getFileName();
		return "";
	}

	void EditorTabWidget::setSelection(AbstractSelection* newSelection)
	{
		auto hadSelection = m_selection != nullptr;
		if (m_selection != nullptr)
		{
			auto oldRect = m_selection->getBoundingBox();
			if(!m_selection->getAlternateSelectionMethod())
				m_selection->reinsertIntoWorld(this);
			m_selection->release(m_resourceManager);
			delete m_selection;
			m_selection = nullptr;

			m_graphicsView->scene()->update(oldRect.x() - 2, oldRect.y() - 2, oldRect.width() + 4, oldRect.height() + 4);
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
				m_graphicsView->scene()->update(oldRect.x() - 2, oldRect.y() - 2, oldRect.width() + 4, oldRect.height() + 4);

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

	QRectF EditorTabWidget::getViewRect() const
	{
		return m_graphicsView->mapToScene(m_graphicsView->viewport()->geometry()).boundingRect();;
	}

	QPointF EditorTabWidget::getCenterPoint() const
	{
		return m_graphicsView->mapToScene(m_graphicsView->viewport()->geometry()).boundingRect().center();
	}



	Level* EditorTabWidget::getActiveLevel()
	{
		if (m_overworld)
		{
			auto view = getViewRect();
			return m_overworld->getLevelAt(view.center());
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

		ui.undoButton->setToolTip(QString("Undo %1").arg(m_undoStack.undoText()));
		ui.redoButton->setToolTip(QString("Redo %1").arg(m_undoStack.redoText()));
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

	QSet<Level*> EditorTabWidget::getLevelsInRect(const QRectF& rect, bool threaded)
	{
		if (m_overworld)
		{
			QSet<Level*> retval;

			m_overworld->searchLevels(rect, retval);

			for (auto level : retval)
				loadLevel(level, threaded);
			return retval;
		}
		else if (m_level)
		{
			QSet<Level*> retval;
			if (rect.intersects(m_level->toQRectF()))
			{
				retval.insert(m_level);
			}
			return retval;
		}
		return QSet<Level*>();
	}

	Level* EditorTabWidget::getLevelAt(const QPointF& point)
	{
		if (m_overworld) {
			auto level = m_overworld->getLevelAt(point);
			if (level)
				loadLevel(level);
			return level;
		}
		else if(m_level) {

			return m_level;
		}
		return nullptr;
	}

	AbstractResourceManager* EditorTabWidget::getResourceManager()
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

	AbstractLevelEntity* EditorTabWidget::getEntityAt(const QPointF& point)
	{
		return getEntityAt(point, false);
	}

	AbstractLevelEntity* EditorTabWidget::getEntityAt(const QPointF& point, bool checkAllowedSelect)
	{
		QList<AbstractLevelEntity*> entities;
		QRect rect(point.x(), point.y(), 1, 1);

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

			if (entity->getLayerIndex() == m_selectedTilesLayer)
			{
				if (!checkAllowedSelect || canSelectObject(entity->getEntityType()))
					return entity;
			}
		}

		return nullptr;
	}

	QList<AbstractLevelEntity*> EditorTabWidget::getEntitiesAt(const QPointF& point)
	{
		return getEntitiesAt(point, false);
	}

	QList<AbstractLevelEntity*> EditorTabWidget::getEntitiesAt(const QPointF& point, bool checkAllowedSelect)
	{
		QList<AbstractLevelEntity*> entities;
		QRectF rect(point.x(), point.y(), 1, 1);

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

				if (entity->getLayerIndex() == m_selectedTilesLayer && canSelectObject(entity->getEntityType())) {
					++it;
				}
				else {
					it = entities.erase(it);
				}
			}
		}

		return entities;
	}

	QSet<AbstractLevelEntity*> EditorTabWidget::getEntitiesInRect(const QRectF& rect)
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



	void EditorTabWidget::applyTileDef2(const TileDef& tileDef)
	{
		if (m_overworld)
		{
			auto& levels = m_overworld->getLevelList();

			for (auto level : levels)
			{
				if (level && level->getName().startsWith(tileDef.prefix))
				{
					level->addTileDef(tileDef);
				}
			}
		}
	}

	void EditorTabWidget::setEntityProperty(AbstractLevelEntity* entity, const QString& name, const QVariant& value)
	{
		entity->setProperty(name, value);
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


				auto pasteX = centerScreen ? std::floor((viewRect.center().x() - (hcount * 16) / 2) / 16.0) * 16.0 : x;
				auto pasteY = centerScreen ? std::floor((viewRect.center().y() - (vcount * 16) / 2) / 16.0) * 16.0 : y;

				auto tileSelection = new TileSelection(pasteX, pasteY, hcount, vcount, m_selectedTilesLayer);
				
				//Do not clear the ground when we first move this selection
				tileSelection->setClearSelection(false);

				//tileSelection->setApplyTranslucency(true);
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


				setSelection(tileSelection);

				m_graphicsView->redraw();

			}
			else if (type == "objectSelection")
			{
				auto selection = new ObjectSelection(centerScreen ? viewRect.center().x() : x, centerScreen ? viewRect.center().y() : y, m_selectedTilesLayer);
				selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
				selection->deserializeJSON(json, this);


				setSelection(selection);

				m_graphicsView->redraw();
			}

			cJSON_Delete(json);
		}
	}

	void EditorTabWidget::newLevel(const QString& format, int hcount, int vcount)
	{
		auto formatObject = FileFormatManager::instance()->getFormatObject(format);

		if (formatObject)
		{
			formatObject->filterLevelSize(&hcount, &vcount);

			m_level = new Level(this, 0, 0, hcount * 16, vcount * 16, nullptr, "");
			m_level->getOrMakeTilemap(0)->clear(0);
			m_level->setLoadState(LoadState::STATE_LOADED);

			formatObject->applyFormat(m_level);

			m_graphicsView->setSceneRect(QRect(0, 0, m_level->getWidth(), m_level->getHeight()));
			m_graphicsView->redraw();
		}


	}

	void EditorTabWidget::loadLevel(const QString& name, const QString& fileName)
	{
		QFileInfo fi(fileName);

		m_level = new Level(this, 0, 0, 64 * 16, 64 * 16, nullptr, name);
		m_level->setFileName(fileName);
		m_level->loadFile(false);

		setTileset(m_level->getTilesetName());

		m_graphicsView->setSceneRect(QRect(0, 0, m_level->getWidth(), m_level->getHeight()));


	}

	void EditorTabWidget::redrawScene(const QRectF& rect)
	{
		//m_graphicsView->updateSceneRect(rect);
		m_graphicsView->redrawRect(rect);
	}

	void EditorTabWidget::redrawScene()
	{
		m_graphicsView->redraw();
	}

	void EditorTabWidget::fileFailed(const QString& name, AbstractResourceManager* resourceManager)
	{

	}

	void EditorTabWidget::fileReady(const QString& fileName, AbstractResourceManager* resourceManager)
	{
		if (m_tileset.getImageName().toLower() == fileName)
		{
			m_graphicsView->redraw();
			ui_tilesetsClass.graphicsView->redraw();
			ui_tileObjectsClass.graphicsView->redraw();
		}
		else {
			if (m_overworld)
			{
				auto level = m_overworld->getLevel(fileName);
				if (level)
				{
					//Reset the state to not loaded and try load it again
					level->setLoadState(LoadState::STATE_NOT_LOADED);
					loadLevel(level);

					//Only refresh the graphics view if the level is visible
					if(getViewRect().intersects(level->toQRectF()))
						m_graphicsView->redraw();
				}
			}
			else if (m_level && m_level->getName() == fileName)
			{
				m_level->setLoadState(LoadState::STATE_NOT_LOADED);
				loadLevel(m_level);
				m_graphicsView->redraw();
			}
		}
	}

	bool EditorTabWidget::isLayerVisible(int layer) const
	{
		auto it = m_visibleLayers.find(layer);
		if (it == m_visibleLayers.end())
			return true;

		return it.value();
	}



	void EditorTabWidget::loadLevel(Level* level, bool threaded)
	{
		while (!threaded && level->getLoadState() == LoadState::STATE_LOADING)
		{
		}

		if (level->getLoadState() == LoadState::STATE_NOT_LOADED)
		{
			level->setLoadState(LoadState::STATE_LOADING);
			QString fullPath;
			if (m_resourceManager->locateFile(level->getName(), &fullPath))
			{
				level->setFileName(fullPath);
				level->loadFile(threaded);
			}
			else
			{
				level->setLoadState(LoadState::STATE_FAILED);
				m_resourceManager->requestFile(this, level->getName());
			}
		}
	}

	void EditorTabWidget::loadOverworld(const QString& name, const QString & fileName)
	{
		m_overworld = new Overworld(this, name);
		m_overworld->setFileName(fileName);

		QFileInfo fi(fileName);


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

	bool EditorTabWidget::tryGetTileAt(const QPointF& point, int* outTile)
	{
		auto level = getLevelAt(point);
		if (level)
		{
			auto layer = level->getTilemap(m_selectedTilesLayer);

			if (layer)
			{
				if (layer->tryGetTile(int((point.x() - layer->getX()) / 16.0), int((point.y() - layer->getY()) / 16.0), outTile))
				{
					return true;
				}
			}
		}
		*outTile = 0;
		return false;
	}

	void EditorTabWidget::removeEntitySelection(AbstractLevelEntity* entity)
	{
		if (m_selection && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS)
		{
			auto objectSelection = static_cast<ObjectSelection*>(m_selection);

			objectSelection->removeEntity(entity);
			//if (objectSelection->objectCount() == 0)
			//	setSelection(nullptr);

		}
	}

	void EditorTabWidget::updateMovedEntity(AbstractLevelEntity* entity)
	{
		if (entity->getLevel())
		{
			auto overworld = entity->getLevel()->getOverworld();
			if (overworld)
			{
				//If the entity no longer intersects the level
				if (!entity->getLevel()->intersects(entity->toQRectF()))
				{
					//Find the new level
					auto newLevel = overworld->getLevelAt(entity->toQPointF());
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

			} 
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

	void EditorTabWidget::getTiles(const QPointF& point, int layer, Tilemap* output)
	{
		QRectF rect(point.x(), point.y(), output->getWidth(), output->getHeight());

		auto levels = getLevelsInRect(rect);

		for (auto level : levels)
		{
			auto tilemap = level->getTilemap(layer);
			if (tilemap != nullptr)
			{
				int sourceTileX = int((rect.x() - tilemap->getX()) / 16.0);
				int sourceTileY = int((rect.y() - tilemap->getY()) / 16.0);

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

	void EditorTabWidget::putTiles(const QPointF& point, int layer, Tilemap* input, bool ignoreInvisible, bool applyTranslucency)
	{
		QRectF rect(point.x(), point.y(), input->getWidth(), input->getHeight());

		auto levels = this->getLevelsInRect(rect);

		for (auto level : levels)
		{ 
			auto tilemap = level->getOrMakeTilemap(layer);
			if (tilemap != nullptr)
			{
				int destTileX = int((point.x() - tilemap->getX()) / 16.0);
				int destTileY = int((point.y() - tilemap->getY()) / 16.0);

				bool modified = false;
				for (int y = 0; y < input->getVCount(); ++y)
				{
					for (int x = 0; x < input->getHCount(); ++x)
					{
						int tile = applyTranslucency ? Tilemap::ReplaceTranslucency(input->getTile(x, y), this->getTileTranslucency()) :  input->getTile(x, y);


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

	void EditorTabWidget::deleteTiles(const QPointF& point, int layer, int hcount, int vcount, int replacementTile)
	{
		static int invisibleTile = Tilemap::MakeInvisibleTile(0);

		QRectF rect(point.x(), point.y(), hcount * 16, vcount * 16);

		auto levels = this->getLevelsInRect(rect);

		for (auto level : levels)
		{
			auto tilemap = level->getTilemap(layer);
			if (tilemap != nullptr)
			{
				int destTileX = int((point.x() - tilemap->getX()) / 16.0);
				int destTileY = int((point.y() - tilemap->getY()) / 16.0);

				bool modified = false;
				for (int y = 0; y < vcount; ++y)
				{
					for (int x = 0; x < hcount; ++x)
					{
						if (layer != 0) {
							tilemap->setTile(destTileX + x, destTileY + y, invisibleTile);
							modified = true;
						}
						else //if(!Tilemap::IsInvisibleTile(replacementTile))
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
				if (rect.intersects(QRectF(pos.x(), pos.y(), 1, 1)))
				{
					auto hcount = int(rect.width() / 16);
					auto vcount = int(rect.height() / 16);
					auto tileSelection = new TileSelection(-1000000, -1000000, hcount, vcount, m_selectedTilesLayer);

					if(hcount == 1 && vcount == 1)
						tileSelection->setDragOffset(tileSelection->getX(), tileSelection->getY(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
					else tileSelection->setDragOffset(tileSelection->getX() + std::floor(pos.x() / getUnitWidth()) * getUnitWidth() - rect.x(), tileSelection->getY() + std::floor(pos.y() / getUnitHeight()) * getUnitHeight() - rect.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());

					tileSelection->setApplyTranslucency(true);
					tileSelection->setMouseDragButton(Qt::MouseButton::LeftButton);

					for (int y = 0; y < vcount; ++y)
					{
						for (int x = 0; x < hcount; ++x)
						{
							auto tileLeft = int(rect.x() / 16) + x;
							auto tileTop = int(rect.y() / 16) + y;
							auto tileType = m_tileset.getTileType(tileLeft, tileTop);

							auto tile = Tilemap::MakeTile(tileLeft, tileTop, tileType);
							tileSelection->setTile(x, y, tile);
						}
					}

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
				auto tileSelection = new TileSelection(-1000000, -1000000, hcount, vcount, m_selectedTilesLayer);
				tileSelection->setApplyTranslucency(true);
				tileSelection->setMouseDragButton(Qt::MouseButton::LeftButton);
				auto tileLeft = int(pos.x() / 16);
				auto tileTop = int(pos.y() / 16);
				auto tileType = m_tileset.getTileType(tileLeft, tileTop);

				auto tile = Tilemap::MakeTile(tileLeft, tileTop, tileType);

				tileSelection->setTile(0, 0, tile);


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
		auto form = new EditTilesetDialog(&m_tileset, m_resourceManager);

		/*
		FramelessWidget frameless;

		frameless.setContent(form);
		frameless.enableDarkMode(UsingDarkMode);
		frameless.setWindowTitle(form->windowTitle());
		frameless.setWindowIcon(QIcon());
		frameless.show();

		*/
		if (form->exec() == QDialog::Accepted)
		{
			m_tileset = form->getTileset();

			
			//update this tileset globally
			auto index = ui_tilesetsClass.tilesetsCombo->findText(m_tileset.text());
			if (index >= 0)
			{
				auto currentTileset = static_cast<Tileset*>(static_cast<QStandardItemModel*>(ui_tilesetsClass.tilesetsCombo->model())->item(index));
				if (currentTileset) {
					*currentTileset = m_tileset;
				}
			
			}
			setTileset(&m_tileset);
		}

		delete form;
		
	}

	void EditorTabWidget::tilesetMouseWheel(QWheelEvent* event)
	{
		event->ignore();
	}

	void EditorTabWidget::tileObjectsMousePress(QMouseEvent* event)
	{
		auto tileObject = getCurrentTileObject();

		if (tileObject)
		{
			auto hcount = tileObject->getHCount();
			auto vcount = tileObject->getVCount();

			auto tileSelection = new TileSelection(-10000, -10000, hcount, vcount, m_selectedTilesLayer);
			tileSelection->setApplyTranslucency(true);
			tileSelection->setMouseDragButton(Qt::MouseButton::LeftButton);
			tileSelection->setDragOffset(tileSelection->getX() + tileSelection->getWidth() / 2, tileSelection->getY() + tileSelection->getHeight() / 2, !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
			
			for (int y = 0; y < vcount; ++y)
			{
				for (int x = 0; x < hcount; ++x)
				{
					int tile = tileObject->getTile(x, y);
					tileSelection->setTile(x, y, tile);
				}
			}

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
					auto level = getLevelAt(pos);
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
					auto level = getLevelAt(pos);
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

		if (event->buttons().testFlag(Qt::MouseButton::MiddleButton))
		{

			m_mousePanStart = pos;
			m_panning = true;
			m_graphicsView->setCursor(Qt::CursorShape::ClosedHandCursor);
		}

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
							m_selection->setMouseDragButton(Qt::MouseButton::LeftButton);
							m_selection->beginResize(edges, this);
							m_graphicsView->redraw();
							return;
						}
					}

					if (m_selection->pointInSelection(pos.x(), pos.y()))
					{
						m_selection->beginDrag();
						m_selection->setCopy(false);
						m_selection->setMouseDragButton(Qt::MouseButton::LeftButton);
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
						m_selection->reinsertIntoWorld(this);
					//Otherwise switch back to normal selection method (for objects)
					else m_selection->setAlternateSelectionMethod(false);

					m_graphicsView->redraw();
					return;
				}
			}

			if (m_selector.visible())
			{
				auto rect = m_selector.getSelection();
				if (rect.intersects(QRectF(pos.x(), pos.y(), 1, 1)) || m_selector.getResizeEdge(pos.x(), pos.y()))
				{
					doTileSelection(false);

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
				m_selection->setMouseDragButton(Qt::MouseButton::LeftButton);
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
				
				QRectF rect(pos.x(), pos.y(), 1, 1);
				if (rect.intersects(selection)) {
					doTileSelection(false);
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
				
				
				/*
				if (QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
				{
					
				}*/


				{
					m_selection->setDragOffset(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
					m_selection->setCopy(true);
					m_selection->setMouseDragButton(Qt::MouseButton::RightButton);
					m_lastMousePos = pos;
				}
				return;

			}
			setSelection(nullptr);

			auto entities = getEntitiesAt(pos, true);
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
					auto selection = new ObjectSelection(action->getEntity()->getX(), action->getEntity()->getY(), m_selectedTilesLayer);
					selection->setMouseDragButton(Qt::MouseButton::LeftButton);
					selection->addObject(action->getEntity());

					setSelection(selection);
					m_graphicsView->redraw();
				}
				return;
			}

			int tile = 0;
			if (tryGetTileAt(pos, &tile) && !Tilemap::IsInvisibleTile(tile))
			{
				setDefaultTile(tile);
			}


			m_graphicsView->redraw();


		}
	}

	void EditorTabWidget::graphicsMouseRelease(QMouseEvent* event)
	{
		auto pos = m_graphicsView->mapToScene(event->pos());

		if (m_panning && event->button() == Qt::MouseButton::MiddleButton)
		{
			m_panning = false;
			m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
		}



		if (event->button() == Qt::MouseButton::RightButton)
		{
			if (m_selector.visible())
			{
				m_selector.setVisible(false);
				m_graphicsView->redraw();
			}

			
			//if (m_selection && !m_selection->hasMoved() &&  !m_selection->getAlternateSelectionMethod() && m_selection->pointInSelection(pos.x(), pos.y()))
			if(m_selection && !m_selection->hasMoved())
			{
				//Right menu on selected objects
				if (m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS)
				{
					auto selection = static_cast<ObjectSelection*>(m_selection);
					QMenu contextMenu;


					auto bringFrontAction = contextMenu.addAction("Bring to Front");
					auto sendBackAction = contextMenu.addAction("Send to Back");
					contextMenu.addSeparator();

					auto resetSizeAction = contextMenu.addAction("Reset Size");
					resetSizeAction->setEnabled(false);


					
					for (auto object : selection->getObjects())
					{
						if (object->getEntityType() == LevelEntityType::ENTITY_NPC)
						{
							if (static_cast<LevelNPC*>(object)->hasResized())
							{
								resetSizeAction->setEnabled(true);
								break;
							}
						}
					}



					auto actionResult = contextMenu.exec(event->globalPosition().toPoint());
					if (actionResult != nullptr)
					{
						if (actionResult == sendBackAction || actionResult == bringFrontAction)
						{
							auto selectionRect = selection->getBoundingBox();

							//auto nearbyObjects = getEntitiesInRect(Rectangle(selectionRect.getX() - 300, selectionRect.getY() - 300, selectionRect.getWidth() + 600, selectionRect.getHeight() + 600));
							auto nearbyObjects = getEntitiesInRect(selectionRect);

							//Remove our selected objects
							for (auto entity : selection->getObjects())
								nearbyObjects.remove(entity);
							

							for (auto it = nearbyObjects.begin(); it != nearbyObjects.end();)
							{
								if ((*it)->getEntityType() != LevelEntityType::ENTITY_NPC)
									it = nearbyObjects.erase(it);
								else ++it;
							}

							QVector<AbstractLevelEntity*> sortedNearbyObjects(nearbyObjects.begin(), nearbyObjects.end());
							std::sort(sortedNearbyObjects.begin(), sortedNearbyObjects.end(), AbstractLevelEntity::sortByDepthFunc);
							
							QVector<AbstractLevelEntity*> sortedSelectedObjects(selection->getObjects().begin(), selection->getObjects().end());
							std::sort(sortedSelectedObjects.begin(), sortedSelectedObjects.end(), AbstractLevelEntity::sortByDepthFunc);
							
							auto undoCommand = new QUndoCommand("Change Object Order");

							if (sortedNearbyObjects.size() > 0)
							{
								if (actionResult == sendBackAction)
								{
									auto adjustedOrder = sortedNearbyObjects.first()->getEntityOrder() - 0.000000001;
									for(auto it = sortedSelectedObjects.rbegin(); it != sortedSelectedObjects.rend(); ++it)
									{
										auto entity = *it;
										new CommandSetEntityProperty(this, entity, "order", adjustedOrder, entity->getEntityOrder(), undoCommand);
										adjustedOrder -= 0.000000001;
									}
								}
								else if (actionResult == bringFrontAction)
								{
									auto adjustedOrder = sortedNearbyObjects.last()->getEntityOrder() + 0.000000001;
									for (auto entity : sortedSelectedObjects)
									{
										new CommandSetEntityProperty(this, entity, "order",  adjustedOrder, entity->getEntityOrder(), undoCommand);
										adjustedOrder += 0.000000001;
									}
								}
							}

							if (undoCommand->childCount() > 0)
								addUndoCommand(undoCommand);
							else undoCommand;

							m_graphicsView->redraw();
						}
						else if (actionResult == resetSizeAction)
						{
							for (auto object : selection->getObjects())
							{
								if (object->getEntityType() == LevelEntityType::ENTITY_NPC)
								{
									static_cast<LevelNPC*>(object)->resetSize();
									updateMovedEntity(object);
								}
							}

							m_graphicsView->redraw();
						}
					}



				}
			}
		}

		else if (event->button() == Qt::MouseButton::LeftButton)
		{
			if (m_selection != nullptr && m_selection->resizing())
			{
				m_selection->endResize(this);
				m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
				m_graphicsView->redraw();
				return;
			}
			else if (m_selection != nullptr && m_selection->dragging())
			{
				m_selection->endDrag(this);

				//clear the purple lines
				if(m_selection->hasMoved())
					m_graphicsView->redraw();
			}

			if (m_selector.selecting())
			{

				m_selector.endSelection(pos.x(), pos.y());

				if (QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
				{
					auto rect = m_selector.getSelection();
					QSet<AbstractLevelEntity*> entities = getEntitiesInRect(rect);
					if (entities.size() > 0)
					{
						auto selection = new ObjectSelection(rect.center().x(), rect.center().y(), m_selectedTilesLayer);
						for (auto entity : entities)
						{
							if (canSelectObject(entity->getEntityType()))
							{
								selection->addObject(entity);
							}
						}

						setSelection(selection);
					}
					m_selector.setVisible(false);
				} else selectorMade();


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

		if (m_panning && event->buttons().testFlag(Qt::MouseButton::MiddleButton))
		{
			auto delta = pos - m_mousePanStart;
			m_graphicsView->translate(delta.x(), delta.y());
			return;
		}

		//Check if the tile position of the mouse has changed
		if (oldTileXPos != tileXPos || oldTileYPos != tileYPos)
		{
			auto level = getLevelAt(pos);
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
					QTextStream(&result) << "Mouse: " << tileX << ", " << tileY << " (" << localTileX << ", " << localTileY << ", " << level->getName() << ") Tile: " << displayTile;
					emit setStatusBar(result, 0, 20000);
				}
				else {
					QString result;
					QTextStream(&result) << "Mouse: " << tileX << ", " << tileY << " Tile: " << displayTile;
					emit setStatusBar(result, 0, 20000);
				}
			}

		}

		if (m_selection)
			m_selection->setVisible(true);

		//If holding left button
		if(m_selection != nullptr && event->buttons().testFlag(m_selection->getMouseDragButton()))
		{
			if (m_selection->getAlternateSelectionMethod())
			{
				if (m_selection->getSelectionType() == SelectionType::SELECTION_TILES)
				{
					m_selection->reinsertIntoWorld(this);
				}
			}
			else {

				if (m_selection->resizing())
				{
					m_selection->updateResize(pos.x(), pos.y(), true, getSnapX(), getSnapY(), this);
					m_graphicsView->redraw();
					return;
				}

				auto oldRect = m_selection->getBoundingBox();
				auto oldSelectionLevels = getLevelsInRect(oldRect);

				bool moved = m_selection->hasMoved();
				m_selection->drag(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY(), this);
					
				auto newRect = m_selection->getBoundingBox();
				auto newSelectionLevels = getLevelsInRect(newRect);


				auto left = std::min(oldRect.x(), newRect.x()) - 4;
				auto top = std::min(oldRect.y(), newRect.y()) - 4;
				auto right = std::max(oldRect.right(), newRect.right()) + 4;
				auto bottom = std::max(oldRect.bottom(), newRect.bottom()) + 4;




				if (!m_selection->dragging())
				{
					m_selection->beginDrag();
					m_graphicsView->redraw();
				}
				else 
				{
					if (newSelectionLevels != oldSelectionLevels || (!moved && m_selection->hasMoved()))
					{
						m_graphicsView->redraw();
					} else m_graphicsView->scene()->update(left, top, right - left, bottom - top);
				}

					
				return;
			}


			
		}

		if (m_selection != nullptr)
		{

			if (m_selection->getAlternateSelectionMethod())
			{
				auto oldRect = m_selection->getBoundingBox();
				auto oldSelectionLevels = getLevelsInRect(oldRect);

				if (!m_selection->dragging())
					m_selection->beginDrag();
				m_selection->drag(pos.x(), pos.y(), true, getSnapX(), getSnapY(), this);

				auto newRect = m_selection->getBoundingBox();
				auto left = std::min(oldRect.x(), newRect.x()) - 4;
				auto top = std::min(oldRect.y(), newRect.y()) - 4;
				auto right = std::max(oldRect.right(), newRect.right()) + 4;
				auto bottom = std::max(oldRect.bottom(), newRect.bottom()) + 4;

				auto newSelectionLevels = getLevelsInRect(newRect);


				if (newSelectionLevels != oldSelectionLevels)
				{
					m_graphicsView->redraw();
				}
				else m_graphicsView->scene()->update(left, top, right - left, bottom - top);

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

			auto left = std::min(oldRect.x(), newRect.x()) - 4;
			auto top = std::min(oldRect.y(), newRect.y()) - 4;
			auto right = std::max(oldRect.right(), newRect.right()) + 4;
			auto bottom = std::max(oldRect.bottom(), newRect.bottom()) + 4;
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
				//setSelection(nullptr);

				if(entity)
				{
					//When you hold shift and double click a link, open the destination level
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

	
	void EditorTabWidget::floodFillPattern(const QPointF& point, int layer, const Tilemap* pattern, QList<TileInfo>* outputNodes)
	{
		QSet<int> startTiles;
		QSet<QPair<int, int>> scannedIndexes;
		auto startTileX = int(std::floor(point.x() / 16));
		auto startTileY = int(std::floor(point.y() / 16));

		auto startTileX2 = (int)std::ceil(double(startTileX) / pattern->getHCount()) * pattern->getHCount();
		auto startTileY2 = (int)std::ceil(double(startTileY) / pattern->getVCount()) * pattern->getVCount();

		//Get a set of tiles that can be replaced
		for (auto y = 0; y < pattern->getVCount(); ++y)
		{
			for (auto x = 0; x < pattern->getHCount(); ++x)
			{
				int tile = 0;
				if (tryGetTileAt(QPointF((startTileX + x) * 16, (startTileY + y) * 16), &tile))
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
				level = getLevelAt(QPointF(nodeXPos, nodeYPos));

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

	void EditorTabWidget::setProperty(const QString& name, const QVariant& value)
	{
		if (name == "tileset")
		{
			setTileset(value.toString());
		}
	}

	void EditorTabWidget::centerView(const QPointF& point)
	{
		m_graphicsView->centerOn(point);
	}

	void EditorTabWidget::addNewObjectClass(ObjectClass* objectClass)
	{
		auto selection = new ObjectSelection(0, 0, m_selectedTilesLayer);
		selection->setVisible(false);

		objectClass->incrementRef();

		//prepare the default param values
		QStringList params;
		for (auto& keyPair : objectClass->getParams())
		{
			params.push_back(getEngine()->parseInlineString(keyPair->getDefaultValue()));
		}

		auto npc = new LevelObjectInstance(this, 0, 0, objectClass->getName(), objectClass, params);
		npc->setLayerIndex(m_selectedTilesLayer);
		selection->addObject(npc);
		selection->setAlternateSelectionMethod(true);
		selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
		selection->setMouseDragButton(Qt::MouseButton::LeftButton);
		selection->setDragOffset(npc->getWidth() / 2, npc->getHeight() / 2, !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
		

		setSelection(selection);
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
		if (m_panning)
			return;
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

		if (event->key() == Qt::Key_0)
		{
			QFile::copy("C:/Users/Luke/source/repos/TilesEditor/level3.nw", "D:/server/world5/level3.nw");

			m_resourceManager->updateFile("level3.nw");
		}


	}

	void EditorTabWidget::graphicsScrolled()
	{
		auto centerPos = m_graphicsView->mapToScene(m_graphicsView->viewport()->rect().center());
		auto level = getLevelAt(centerPos);

		if (level != m_previousCenterLevel)
		{
			m_previousCenterLevel = level;

			ui_tilesetsClass.graphicsView->redraw();
			ui_tilesetsClass.tileIcon->update();
			ui_tileObjectsClass.graphicsView->redraw();
			
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

		m_graphicsView->setAntiAlias(zoomLevel < 1);

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

		auto rootLinkLevel = getLevelAt(QPointF(rect.x(), rect.y()));

		if (rootLinkLevel)
		{
			auto link = new LevelLink(this, rect.x(), rect.y(), rect.width(), rect.height(), false);
			link->setLevel(rootLinkLevel);
			link->setLayerIndex(m_selectedTilesLayer);

			EditLinkDialog frm(link, this, false);
			if (frm.exec() == QDialog::Accepted)
			{
				auto undoCommand = new QUndoCommand("New Link");
				auto levels = getLevelsInRect(link->toQRectF());

				for (auto level : levels)
				{
					auto newLink = link->duplicate();
					newLink->setLevel(level);

					auto rect = level->clampEntity(newLink);
					newLink->setX(rect.x());
					newLink->setY(rect.y());
					newLink->setWidth(rect.width());
					newLink->setHeight(rect.height());

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

		auto rootSignLevel = getLevelAt(QPointF(rect.x(), rect.y()));

		if (rootSignLevel)
		{
			auto sign = new LevelSign(this, rect.x(), rect.y(), 32, 16);
			sign->setLevel(rootSignLevel);
			sign->setLayerIndex(m_selectedTilesLayer);
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
		ui.undoButton->setToolTip(QString("Undo %1").arg(m_undoStack.undoText()));
		ui.redoButton->setToolTip(QString("Redo %1").arg(m_undoStack.redoText()));
		m_graphicsView->redraw();
	}

	void EditorTabWidget::redoClicked(bool checked)
	{
		setSelection(nullptr);
		m_undoStack.redo();
		ui.redoButton->setEnabled(m_undoStack.canRedo());
		ui.undoButton->setEnabled(m_undoStack.canUndo());
		ui.undoButton->setToolTip(QString("Undo %1").arg(m_undoStack.undoText()));
		ui.redoButton->setToolTip(QString("Redo %1").arg(m_undoStack.redoText()));
		m_graphicsView->redraw();
	}

	void EditorTabWidget::cutPressed()
	{
		if (m_selector.visible())
			doTileSelection(false);

		if (m_selection)
		{
			m_selection->clipboardCopy();
			m_selection->clearSelection(this);
			setSelection(nullptr);
			m_graphicsView->redraw();
		}
	}


	void EditorTabWidget::copyPressed()
	{
		if (m_selector.visible())
		{
			doTileSelection(true);
			return;
		}

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
			doTileSelection(false);

		if (m_selection)
		{
			m_selection->clearSelection(this);
			setSelection(nullptr);
			m_graphicsView->redraw();
		}
	}

	void EditorTabWidget::screenshotClicked(bool checked)
	{
		auto pos = this->getCenterPoint();
		ScreenshotDialog frm(this, pos.x(), pos.y());
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

						if (m_resourceManager->locateFile(level->getName(), &fullPath))
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

				if (m_resourceManager->locateFile(m_level->getName(), &fullPath))
				{
					QFileInfo fi(fullPath);

					if(m_resourceManager->getType() == "FileSystem")
						static_cast<ResourceManagerFileSystem*>(m_resourceManager)->addSearchDirRecursive(fi.absolutePath() + "/", 1);


					m_level->setFileName(fullPath);


				}
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
			auto fullPath = m_resourceManager->getSaveFileName("Save Level As", QString(), FileFormatManager::instance()->getLevelSaveFilters());

			if (!fullPath.isEmpty())
			{
				QFileInfo fi(fullPath);

				m_level->setName(fi.fileName());
				m_level->setFileName(fullPath);

				auto newFolder = fi.absolutePath();
				if (!newFolder.endsWith('/'))
					newFolder += '/';

				if (newFolder != m_resourceManager->getConnectionString())
				{
					if (m_resourceManager->getType() == "FileSystem")
					{
						static_cast<ResourceManagerFileSystem*>(m_resourceManager)->addSearchDirRecursive(newFolder, 1);
						m_resourceManager->getObjectManager()->mergeResourceManager(getResourceManager());
						m_resourceManager->getObjectManager()->loadAllExpanded();

					}
				}


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

		if (m_overworld)
		{
			m_overworld->setTilesetImageName(m_tileset.getImageName());
			setModified(nullptr);
		}
		else if (m_level) {
			m_level->setTilesetImageName(m_tileset.getImageName());
			setModified(m_level);
		}

		m_resourceManager->updateFile(m_tileset.getImageName());
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
		
		auto fileName = QFileDialog::getOpenFileName(nullptr, "Select Tileset", "./", "Image Files (*.png *.gif);;JSON File (*.json)");
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);
			auto directory = fi.absolutePath() + "/";

			if(m_resourceManager->getType() == "FileSystem")
				static_cast<ResourceManagerFileSystem*>(m_resourceManager)->addSearchDir(directory);

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

	void EditorTabWidget::tilesetEditOrderClicked(bool checked)
	{
		EditTilesets frm(ui_tilesetsClass.tilesetsCombo->model(), this);
		frm.exec();
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
												auto tile = Level::convertFromGraalTile(graalTile, nullptr);
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
			doTileSelection(false);

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
		auto selection = new ObjectSelection(0, 0, m_selectedTilesLayer);
		selection->setVisible(false);
		selection->setMouseDragButton(Qt::MouseButton::LeftButton);
		auto npc = new LevelNPC(this, 0, 0, 48, 48);
		npc->setLayerIndex(m_selectedTilesLayer);
		npc->setImageName("");
		selection->addObject(npc);
		selection->setAlternateSelectionMethod(true);
		selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
		selection->setDragOffset(npc->getWidth() / 2, npc->getHeight() / 2, !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
		setSelection(selection);

	}

	void EditorTabWidget::objectsNewChestClicked(bool checked)
	{
		auto selection = new ObjectSelection(0, 0, m_selectedTilesLayer);
		selection->setVisible(false);
		selection->setMouseDragButton(Qt::MouseButton::LeftButton);
		auto chest = new LevelChest(this, 0, 0, "greenrupee", -1);
		chest->setLayerIndex(m_selectedTilesLayer);
		selection->addObject(chest);
		selection->setAlternateSelectionMethod(true);
		selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
		selection->setDragOffset(chest->getWidth() / 2, chest->getHeight() / 2, !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
		setSelection(selection);

	}

	void EditorTabWidget::objectsNewBaddyClicked(bool checked)
	{
		auto selection = new ObjectSelection(0, 0, m_selectedTilesLayer);
		selection->setVisible(false);
		selection->setMouseDragButton(Qt::MouseButton::LeftButton);

		auto baddy = new LevelGraalBaddy(this, 0.0, 0.0, 0);
		baddy->setLayerIndex(m_selectedTilesLayer);
		selection->addObject(baddy);
		selection->setAlternateSelectionMethod(true);
		selection->setSelectMode(ObjectSelection::SelectMode::MODE_INSERT);
		selection->setDragOffset(baddy->getWidth() / 2, baddy->getHeight() / 2, !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), getSnapX(), getSnapY());
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
			auto selection = new ObjectSelection(entity->getX(), entity->getY(), m_selectedTilesLayer);
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
				auto undoCommand = new QUndoCommand("Delete Overworld Edge Links");
				for (auto level : m_overworld->getLevelList())
				{
					loadLevel(level, false);

					if (level->getLoadState() == LoadState::STATE_LOADED)
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

				if (undoCommand->childCount())
				{
					addUndoCommand(undoCommand);
					m_graphicsView->redraw();
				}
				else delete undoCommand;
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

		auto undoCommand = new QUndoCommand("Trim Script Endings");
		for (auto level : levels)
		{
			loadLevel(level, false);

			if (level->getLoadState() == LoadState::STATE_LOADED)
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
							new CommandSetEntityProperty(this, npc, "code", script, npc->getCode(), undoCommand);
							setModified(level);
						}

					}
				}
			}
		}

		if (undoCommand->childCount())
			addUndoCommand(undoCommand);
		else delete undoCommand;
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

		auto undoCommand = new QUndoCommand("Trim Sign Endings");
		for (auto level : levels)
		{
			loadLevel(level, true);

			if (level->getLoadState() == LoadState::STATE_LOADED)
			{
				auto& signs = level->getSigns();
				for (auto sign : signs)
				{
					auto text = rstrip(sign->getText());

					if (text != sign->getText())
					{
						new CommandSetEntityProperty(this, sign, "text", text, sign->getText(), undoCommand);
						setModified(level);
					}
				}
			}
		}
		if (undoCommand->childCount())
			addUndoCommand(undoCommand);
		else delete undoCommand;
	}

	void EditorTabWidget::tileIconMouseDoubleClick(QMouseEvent* event)
	{
		setDefaultTile(Tilemap::MakeInvisibleTile(0));
	}

	void EditorTabWidget::gridValueChanged(int)
	{
		//Set the grid image to null (this was cause it to be re-generated when we re-draw)
		QPixmap a;
		m_gridImage.swap(a);
		m_graphicsView->redraw();
	}

	void EditorTabWidget::downKeyPressed()
	{
		m_graphicsView->translate(0.0, -32);
	}

	void EditorTabWidget::upKeyPressed()
	{
		m_graphicsView->translate(0.0, 32);
	}

	void EditorTabWidget::leftKeyPressed()
	{
		m_graphicsView->translate(32, 0);
	}

	void EditorTabWidget::rightKeyPressed()
	{
		m_graphicsView->translate(-32, 0);
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
			auto fullPath = m_resourceManager->getSaveFileName("Save Level", m_resourceManager->getConnectionString(), FileFormatManager::instance()->getLevelSaveFilters());

			if (!fullPath.isEmpty())
			{
				QFileInfo fi(fullPath);

				level->setName(fi.fileName());
				level->setFileName(fullPath);

				if(m_resourceManager->getType() == "FileSystem")
					static_cast<ResourceManagerFileSystem*>(getResourceManager())->addSearchDirRecursive(fi.absolutePath() + "/", 1);
				getResourceManager()->getObjectManager()->mergeResourceManager(getResourceManager());
				getResourceManager()->getObjectManager()->loadAllExpanded();

				FileFormatManager::instance()->applyFormat(m_level);
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

	void EditorTabWidget::mark(sgs_Context* ctx)
	{
		sgs_GCMark(ctx, &m_sgsUserTable);
	}

	void EditorTabWidget::registerScriptClass(IEngine* engine)
	{
		auto ctx = engine->getScriptContext();


		auto startStackSize = sgs_StackSize(ctx);

		sgs_PushString(ctx, "createObject");
		sgs_PushCFunc(ctx, [](sgs_Context* ctx) -> int
		{
			sgs_Method(ctx);

			sgs_Real x, y;
			char* itemName;
			sgs_Int signIndex;

			EditorTabWidget* self = nullptr;
			if (sgs_LoadArgs(ctx, "@orrsi", &EditorTabWidget::sgs_interface, &self, &x, &y, &itemName, &signIndex))
			{
				if (self == nullptr)
					return 0;

				auto chest = new LevelChest(self, x, y, itemName, signIndex);
				chest->incrementRef();

				sgs_Variable obj;
				sgs_CreateObject(ctx, &obj, chest, &AbstractLevelEntity::sgs_interface);
				sgs_PushVariable(ctx, obj);
				sgs_Release(ctx, &obj);
				return 1;
				
			}

			return 0;
		});


		auto memberCount = sgs_StackSize(ctx) - startStackSize;
		sgs_CreateDict(ctx, &sgs_classMembers, memberCount);
		engine->addCPPOwnedObject(sgs_classMembers);

		auto objDestruct = [](sgs_Context* ctx, sgs_VarObj* obj) -> int
			{
				auto self = static_cast<EditorTabWidget*>(obj->data);

				if (self == nullptr)
					return 0;

				return 1;
			};


		auto objMark = [](sgs_Context* C, sgs_VarObj* obj) -> int
			{
				auto self = static_cast<EditorTabWidget*>(obj->data);
				if (self == nullptr)
					return 0;

				self->mark(C);
				return 1;
			};

		auto objGetIndex = [](sgs_Context* C, sgs_VarObj* obj) -> int
			{
				auto self = static_cast<EditorTabWidget*>(obj->data);

				if (self == nullptr)
					return 0;

				auto propName = sgs_StackItem(C, 0);


				if (sgs_PushIndex(C, EditorTabWidget::sgs_classMembers, propName, 1))
					return 1;


				if (sgs_PushIndex(C, self->m_sgsUserTable, propName, 1))
					return 1;

				return 0;
			};

		auto objSetIndex = [](sgs_Context* C, sgs_VarObj* obj) -> int
			{
				auto self = static_cast<EditorTabWidget*>(obj->data);

				if (self == nullptr)
					return 0;

				auto propName = sgs_StackItem(C, 0);


				sgs_SetIndex(C, self->m_sgsUserTable, propName, sgs_OptStackItem(C, 1), 1);
				return 0;
			};


		sgs_interface = {
			"World",
			objDestruct, objMark,  /* destruct, gcmark */
			objGetIndex, objSetIndex,  /* getindex, setindex */
			NULL, NULL, NULL, NULL, /* convert, serialize, dump, getnext */
			NULL, NULL,              /* call, expr */
			NULL, &EditorTabWidget::sgs_interface	/*proplist, parent*/
		};
	}


}
