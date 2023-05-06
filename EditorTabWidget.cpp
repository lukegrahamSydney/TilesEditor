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

namespace TilesEditor
{

	EditorTabWidget::EditorTabWidget(QWidget* parent, ResourceManager& resourceManager)
		: QWidget(parent), m_fillPattern(nullptr, 0.0, 0.0, 1, 1, 0)
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
		m_resourceManager.mergeSearchDirectories(resourceManager);

		QPixmap a(":/MainWindow/icons/npc.png");

		m_resourceManager.addPersistantResource(new Image("__blankNPC", a));

		m_eyeOpen = QPixmap(":/MainWindow/icons/fugue/eye.png");
		m_eyeClosed = QPixmap(":/MainWindow/icons/fugue/eye-close.png");

		m_selection = nullptr;

		setDefaultTile(Tilemap::MakeTile(0, 0, 0));

		m_selectedTilesLayer = 0;
		m_selectedLayerVisible = true;
		m_visibleLayers[0] = true;

		auto graphicsContainer = ui.graphicsContainer;
		m_graphicsView = new GraphicsView();

		graphicsContainer->layout()->addWidget(m_graphicsView);

		m_level = nullptr;
		m_overworld = nullptr;

		m_tilesetImage = nullptr;
		m_graphicsView->setSceneRect(QRect(0, 0, 64 * 16, 64 * 16));

		ui_objectClass.objectsTable->setModel(new ObjectListModel());
		ui_objectClass.objectsTable->setColumnWidth(2, 70);

		connect(ui_objectClass.newNPCButton, &QAbstractButton::clicked, this, &EditorTabWidget::objectsNewNPCClicked);
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

		auto menu = new QMenu();
		m_selectNPCs = menu->addAction("Npcs");
		m_selectNPCs->setCheckable(true);
		m_selectNPCs->setChecked(true);

		m_selectLinks = menu->addAction("Links");
		m_selectLinks->setCheckable(true);
		m_selectLinks->setChecked(true);

		m_selectSigns = menu->addAction("Signs");
		m_selectSigns->setCheckable(true);
		m_selectSigns->setChecked(true);

		ui.selectionButton->setMenu(menu);
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
			m_overworld->release(m_resourceManager);
			delete m_overworld;
		}

		if (m_level)
		{
			m_level->release(m_resourceManager);
			delete m_level;
		}

	}

	void EditorTabWidget::renderScene(QPainter * painter, const QRectF & rect)
	{
		Rectangle viewRect(rect.x(), rect.y(), rect.width(), rect.height());
		auto mousePos = m_graphicsView->mapToScene(m_graphicsView->mapFromGlobal(QCursor::pos()));

		painter->fillRect(rect, QColorConstants::Svg::black);

		QSet<Level*> drawLevels;
		if (m_overworld)
		{
			m_overworld->searchLevels(viewRect, drawLevels);
		}
		else if(m_level)
			drawLevels.insert(m_level);

		if (m_tilesetImage)
		{
			//Draw tiles (and make sure level is loaded)
			for (auto level : drawLevels)
			{
				loadLevel(level);

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

							if (ui.floodFillButton->isChecked()/* && QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier)*/)
							{

								
								QSet<QPair<int, int> > scannedIndexes;
								auto x = mousePos.x();
								auto y = mousePos.y();


								auto startTileX = int(std::floor(x / 16));
								auto startTileY = int(std::floor(y / 16));

								auto startTileX2 = (int)std::ceil(double(startTileX) / m_fillPattern.getHCount()) * m_fillPattern.getHCount();
								auto startTileY2 = (int)std::ceil(double(startTileY) / m_fillPattern.getVCount()) * m_fillPattern.getVCount();
								auto startTile = 0;



								if (tryGetTileAt(startTileX * 16, startTileY * 16, &startTile))
								{
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
												auto tileX = node.first - int(std::floor(tilemap->getX() / 16.0));
												auto tileY = node.second - int(std::floor(tilemap->getY() / 16.0));

												auto deltaX = startTileX2 + (node.first - startTileX);
												auto deltaY = startTileY2 + (node.second - startTileY);
												auto patternTileX = deltaX % m_fillPattern.getHCount();
												auto patternTileY = deltaY % m_fillPattern.getVCount();

													
												auto patternTile = m_fillPattern.getTile(patternTileX, patternTileY);
												int tile = 0;
												if (tilemap->tryGetTile(tileX, tileY, &tile) && !Tilemap::IsInvisibleTile(tile))
												{
													if (tile == startTile)
													{
														painter->drawPixmap(node.first * 16, node.second * 16, m_tilesetImage->pixmap(), Tilemap::GetTileX(patternTile) * 16, Tilemap::GetTileY(patternTile) * 16, 16, 16);

														//if (nodes.count() < 3000)
														//{
															addNode(node.first - 1, node.second);
															addNode(node.first, node.second - 1);
	
															addNode(node.first + 1, node.second);
															addNode(node.first, node.second + 1);

														//}
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
			entity->loadResources(m_resourceManager);
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

			auto rect = QRectF(tileX, tileY, 16, 16);
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


			//Draw tiles (and make sure level is loaded)
			for (auto level : drawLevels)
			{
				int fontWidth = fm.horizontalAdvance(level->getName());


				painter->drawText(level->getCenterX() - fontWidth / 2, level->getY() + fontHeight, level->getName());

			}
			painter->setCompositionMode(compositionMode);
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
		if (m_tilesetImage != nullptr)
		{
			if (m_tilesetImage->getName() == name)
				return;

			m_resourceManager.freeResource(m_tilesetImage);
		}

		m_tilesetImage = static_cast<Image*>(m_resourceManager.loadResource(name, ResourceType::RESOURCE_IMAGE));

		if (m_tilesetImage)
		{
			ui_tilesetsClass.graphicsView->setSceneRect(0, 0, m_tilesetImage->width(), m_tilesetImage->height());

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
		return false;
	}

	bool EditorTabWidget::selectingLevel()
	{
		if (ui.editLinksButton->isChecked() || ui.editSignsButton->isChecked())
			return true;
		return false;

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

		AbstractLevelEntity* entity = getEntityAt(x, y, true);


		if (entity != nullptr)
		{
			bool append = allowAppend && (m_selection != nullptr && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS);

			auto selection = append ? static_cast<ObjectSelection*>(m_selection) : new ObjectSelection(x, y);

			//setModified(entity->getLevel());
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
			if(!m_selection->getAlternateSelectionMethod())
				m_selection->reinsertIntoWorld(this, m_selectedTilesLayer);
			m_selection->release(m_resourceManager);
			delete m_selection;
		}


		m_selection = newSelection;
		if (m_selection != nullptr)
		{
			ui.floodFillButton->setChecked(false);
			//ui.floodFillPatternButton->setChecked(false);

			if (m_selector.visible())
			{
				m_selector.setVisible(false);
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

	void EditorTabWidget::init(QStringListModel* tilesetList, TileGroupListModel* tileGroupList)
	{
		ui_tilesetsClass.tilesetsCombo->setModel(tilesetList);

		ui_tileObjectsClass.groupCombo->setModel(tileGroupList);

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


	void EditorTabWidget::newLevel(int hcount, int vcount)
	{
		m_level = new Level(0, 0, hcount * 16, vcount * 16, nullptr, "");
		m_level->getOrMakeTilemap(0, m_resourceManager)->clear(0);
		m_level->setLoaded(true);
		m_graphicsView->redraw();

		qDebug() << "A: " << m_level->getFileName();
	}

	void EditorTabWidget::loadLevel(const QString& name, const QString& fileName)
	{
		QFileInfo fi(fileName);

		m_resourceManager.addSearchDirRecursive(fi.absolutePath());
		m_resourceManager.setRootDir(fi.absolutePath());

		m_level = new Level(0, 0, 64 * 16, 64 * 16, nullptr, name);
		m_level->setFileName(fileName);
		m_level->loadNWFile(m_resourceManager);


	}

	void EditorTabWidget::loadLevel(Level* level)
	{
		if (!level->getLoaded())
		{
			QString fullPath;
			if (m_resourceManager.locateFile(level->getName(), &fullPath))
			{
				level->setFileName(fullPath);
				level->loadNWFile(m_resourceManager);
			}
		}
	}

	void EditorTabWidget::loadGMap(const QString& name, const QString & fileName)
	{
		m_overworld = new Overworld(name);

		QFileInfo fi(fileName);

		m_resourceManager.addSearchDirRecursive(fi.absolutePath());
		m_resourceManager.setRootDir(fi.absolutePath());

		m_overworld->loadGMap(fileName, m_resourceManager);
		m_graphicsView->setSceneRect(QRect(0, 0, m_overworld->getWidth(), m_overworld->getHeight()));

		ui.saveAsButton->setEnabled(false);
		setTileset("pics1.png");

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
		if (m_overworld)
		{
			if (entity->getLevel() && !entity->getLevel()->intersects(*entity))
			{
				auto newLevel = this->getLevelAt(entity->getX(), entity->getY());
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
			m_overworld->getEntitySpatialMap()->updateEntity(entity);
		}
		else if (m_level)
		{
			m_level->getEntitySpatialMap()->updateEntity(entity);
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
			auto tilemap = level->getOrMakeTilemap(layer, this->getResourceManager());
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
					auto viewRect = getViewRect();
					auto hcount = rect.getWidth() / 16;
					auto vcount = rect.getHeight() / 16;
					auto tileSelection = new TileSelection(viewRect.getRight(), viewRect.getY(), hcount, vcount);

					for (int y = 0; y < vcount; ++y)
					{
						for (int x = 0; x < hcount; ++x)
						{
							auto tileLeft = int(rect.getX() / 16) + x;
							auto tileTop = int(rect.getY() / 16) + y;

							auto tile = Tilemap::MakeTile(tileLeft, tileTop, 0);
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
			setDefaultTile(Tilemap::MakeTile(int(pos.x() / 16), int(pos.y() / 16), 0));
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

				auto tile = Tilemap::MakeTile(int(pos.x() / 16.0), int(pos.y() / 16.0), 0);

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

		if(m_selector.visible())
			selectorGone();



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
						m_selection->setDragOffset(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier));
						return;
					}
					else {
						if (QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier) && m_selection->getSelectionType() == SelectionType::SELECTION_OBJECTS)
						{
							doObjectSelection(pos.x(), pos.y(), true);
							m_selection->setDragOffset(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier));
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
				m_selection->setDragOffset(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier));
				m_graphicsView->redraw();
				return;
			}

			m_selector.setVisible(false);
			m_selector.beginSelection(pos.x(), pos.y(), 16, 16);
			m_graphicsView->redraw();
		}
		else if (event->button() == Qt::MouseButton::RightButton)
		{


			if (m_selector.visible())
				m_selector.setVisible(false);

			if (ui.floodFillButton->isChecked())
			{
				m_graphicsView->redraw();
				ui.floodFillButton->setChecked(false);
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
						if(entity->getIcon())
							this->setIcon(entity->getIcon()->pixmap());

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
			if (oldTileXPos != tileXPos || oldTileYPos != tileYPos)
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
				auto localX = int((pos.x() - level->getX()) / 16.0);
				auto localY = int((pos.y() - level->getY()) / 16.0);

				int tileIndex = 0;
				auto tileMap = level->getTilemap(m_selectedTilesLayer);
				if (tileMap)
				{
					auto tile = tileMap->getTile(localX, localY);
					auto tileLeft = Tilemap::GetTileX(tile);
					auto tileTop = Tilemap::GetTileY(tile);

					//Gonstruct: Converts tile positions (lef/top) to graal tile index
					tileIndex = tileLeft / 16 * 512 + tileLeft % 16 + tileTop * 16;
				}

				if (m_overworld)
				{
					QString result;
					QTextStream(&result) << "Tile: " << tileXPos << ", " << tileYPos << " (" << localX << ", " << localY << "): " << tileIndex;
					emit setStatusBar(result, 0, 20000);
				}
				else {
					QString result;
					QTextStream(&result) << "Tile: " << tileXPos << ", " << tileYPos << ": " << tileIndex;
					emit setStatusBar(result, 0, 20000);
				}
			}
		}

		//If holding left button
		if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
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
						m_selection->updateResize(pos.x(), pos.y(), true, this);
						m_graphicsView->redraw();
						return;
					}

					m_selection->drag(pos.x(), pos.y(), !QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier), this);
					m_graphicsView->redraw();
					return;
				}


			}
		}

		if (m_selection != nullptr)
		{

			if (m_selection->getAlternateSelectionMethod())
			{
				//m_selection->setX(std::floor((pos.x() - m_selection->getWidth() / 2) / 16.0) * 16.0);
				//m_selection->setY(std::floor((pos.y() - m_selection->getHeight() / 2) / 16.0) * 16.0);

				m_selection->drag(
					std::floor((pos.x() - m_selection->getWidth() / 2) / 16.0) * 16.0,
					std::floor((pos.y() - m_selection->getHeight() / 2) / 16.0) * 16.0,
					true, this);

				m_graphicsView->redraw();
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
			m_selector.setVisible(true);
			m_selector.updateSelection(pos.x(), pos.y());
			m_graphicsView->redraw();
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
					if (entity->getEntityType() == LevelEntityType::ENTITY_NPC)
					{

						EditAnonymousNPC frm(static_cast<LevelNPC*>(entity), m_resourceManager);
						if (frm.exec() == QDialog::Accepted)
						{
							if (frm.getModified())
							{
								setModified(entity->getLevel());

								updateMovedEntity(entity);
							}
						}
						else if (frm.result() == -1)
						{
							deleteEntity(entity);

							m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
							m_graphicsView->redraw();
							return;
						}

					}

					else if (entity->getEntityType() == LevelEntityType::ENTITY_LINK)
					{
						auto link = static_cast<LevelLink*>(entity);
						if (QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
						{
							emit openLevel(link->getNextLevel());

							return;
						}

						EditLinkDialog frm(link);
						if (frm.exec() == QDialog::Accepted)
						{
							if (frm.getModified())
							{
								setModified(entity->getLevel());
								updateMovedEntity(entity);
							}
						}
						else if (frm.result() == -1)
						{
							//setModified(entity->getLevel());
							deleteEntity(entity);

							m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
							m_graphicsView->redraw();
							return;
						}
					}
					else if (entity->getEntityType() == LevelEntityType::ENTITY_SIGN)
					{
						auto sign = static_cast<LevelSign*>(entity);

						EditSignsDialog frm(sign->getLevel(), this, sign);
						if (frm.exec() == QDialog::Accepted)
						{
						}
						else if (frm.result() == -1)
						{
							deleteEntity(entity);
							m_graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
							m_graphicsView->redraw();
							return;
						}
					}


				}
			}


			
		}
	}

	int EditorTabWidget::floodFillPattern(double x, double y, int layer, const Tilemap* pattern, QList<QPair<unsigned short, unsigned short>>* outputNodes)
	{
		QSet<QPair<int, int>> scannedIndexes;
		auto startTileX = int(std::floor(x / 16));
		auto startTileY = int(std::floor(y / 16));

		auto startTileX2 = (int)std::ceil(double(startTileX) / pattern->getHCount()) * pattern->getHCount();
		auto startTileY2 = (int)std::ceil(double(startTileY) / pattern->getVCount()) * pattern->getVCount();
		
		auto startTile = 0;
		if (tryGetTileAt(startTileX * 16, startTileY * 16, &startTile))
		{
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
							if (tile == startTile)
							{
								setModified(level);

								if (outputNodes)
									outputNodes->push_back(QPair<unsigned short, unsigned short>((unsigned short)node.first, (unsigned short)node.second));

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

		return startTile;
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

		qreal zoomFactors[] = {
			0.25,
			0.5,
			0.75,
			1.0,
			2,
			3,
			4
		};

		qreal scaleX = zoomFactors[position];
		qreal scaleY = zoomFactors[position];

		if (zoomFactors[position] > 1.0)
			m_graphicsView->setAntiAlias(false);
		else m_graphicsView->setAntiAlias(true);

		
		m_graphicsView->resetTransform();
		m_graphicsView->scale(scaleX, scaleY);

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

		if (m_selector.visible())
			doTileSelection();

		if (m_selection && m_selection->getSelectionType() == SelectionType::SELECTION_TILES)
		{
			auto tileSelection = static_cast<TileSelection*>(m_selection);

			m_fillPattern = *tileSelection->getTilemap();
			setSelection(nullptr);
			ui.floodFillButton->setChecked(true);

		}
		
	}

	void EditorTabWidget::preloadOverworldClicked(bool checked)
	{
		if (m_overworld)
		{
			m_overworld->preloadLevels(m_resourceManager);
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
			auto link = new LevelLink(rootLinkLevel, rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());

			EditLinkDialog frm(link);
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
			auto sign = new LevelSign(rootSignLevel, rect.getX(), rect.getY(), 32, 16);
			
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
		QClipboard* clipboard = QApplication::clipboard();

		auto text = clipboard->text();

		QByteArray ba = text.toLocal8Bit();

		auto json = cJSON_Parse(ba.data());

		if (json != nullptr)
		{
			auto viewRect = getViewRect();
			auto type = jsonGetChildString(json, "type");
			if (type == "tileSelection")
			{
				auto hcount = jsonGetChildInt(json, "hcount");
				auto vcount = jsonGetChildInt(json, "vcount");



				auto tileSelection = new TileSelection(std::floor((viewRect.getCenterX() - (hcount * 16) / 2) / 16.0) * 16.0, std::floor((viewRect.getCenterY() - (vcount * 16) / 2) / 16.0) * 16.0, hcount, vcount);

				auto tileArray = cJSON_GetObjectItem(json, "tiles");

				if (tileArray && tileArray->type == cJSON_Array)
				{
					for (int y = 0; y < cJSON_GetArraySize(tileArray); ++y)
					{
						auto arrayItem = cJSON_GetArrayItem(tileArray, y);
						if (arrayItem->type == cJSON_String)
						{
							QString line(arrayItem->valuestring);

							auto parts = line.split(' ', Qt::SkipEmptyParts);
							for (auto x = 0U; x < parts.size(); ++x)
							{
								int tile = parts[x].toInt(nullptr, 16);
								tileSelection->setTile(x, y, tile);
							}
						}
					}
				}

				if(m_tilesetImage)
					tileSelection->setTilesetImage(m_tilesetImage);
				setSelection(tileSelection);

				m_graphicsView->redraw();

			}
			else if (type == "objectSelection")
			{
				auto selection = new ObjectSelection(viewRect.getCenterX(), viewRect.getCenterY());
				selection->deserializeJSON(json, this);


				setSelection(selection);

				m_graphicsView->redraw();
			}

			cJSON_Delete(json);
		}
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
			auto fullPath = QFileDialog::getSaveFileName(nullptr, "Save Level As", QString(), "All Level Files (*.nw)");

			if (!fullPath.isEmpty())
			{
				QFileInfo fi(fullPath);

				m_level->setName(fi.fileName());
				m_level->setFileName(fullPath);

				if (saveLevel(m_level))
					setUnmodified();
			}
		}
	}

	void EditorTabWidget::tilesetsIndexChanged(int index)
	{
		auto imageName = ui_tilesetsClass.tilesetsCombo->currentText();

		setTileset(imageName);

		m_graphicsView->redraw();
		ui_tilesetsClass.graphicsView->redraw();

		if (m_tilesetImage)
			ui_tilesetsClass.graphicsView->setSceneRect(0, 0, m_tilesetImage->width(), m_tilesetImage->height());
	}

	void EditorTabWidget::tilesetDeleteClicked(bool checked)
	{
		auto currentIndex = ui_tilesetsClass.tilesetsCombo->currentIndex();
		if(currentIndex >= 0)
			ui_tilesetsClass.tilesetsCombo->removeItem(currentIndex);
	}

	void EditorTabWidget::tilesetRefreshClicked(bool checked)
	{
		auto imageName = ui_tilesetsClass.tilesetsCombo->currentText();

		m_resourceManager.updateResource(imageName);
		ui_tilesetsClass.graphicsView->redraw();
		m_graphicsView->redraw();
	}

	void EditorTabWidget::tilesetNewClicked(bool checked)
	{
		auto imageName = QInputDialog::getText(nullptr, "Add new tileset", "Image File: ");

		if (!imageName.isEmpty())
		{

			ui_tilesetsClass.tilesetsCombo->model()->insertRow(ui_tilesetsClass.tilesetsCombo->model()->rowCount());
			ui_tilesetsClass.tilesetsCombo->model()->setData(ui_tilesetsClass.tilesetsCombo->model()->index(ui_tilesetsClass.tilesetsCombo->model()->rowCount() - 1, 0), imageName);

			if(ui_tilesetsClass.tilesetsCombo->model()->rowCount() == 1)
				tilesetsIndexChanged(0);
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
		auto viewRect = getViewRect();

		auto selection = new ObjectSelection(0, 0);

		auto npc = new LevelNPC(nullptr, 0, 0, 48, 48);
		npc->setImageName("", m_resourceManager);
		selection->addObject(npc);
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

	void EditorTabWidget::selectorGone()
	{
		ui.newLinkButton->setEnabled(false);
		ui.newSignButton->setEnabled(false);
		ui.copyButton->setEnabled(false);
		ui.cutButton->setEnabled(false);
		ui.floodFillPatternButton->setEnabled(false);
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
	}

	bool EditorTabWidget::saveLevel(Level* level)
	{
		qDebug() << level->getFileName();
		if (level->getFileName().isEmpty())
		{
			auto fullPath = QFileDialog::getSaveFileName(nullptr, "Save Level", QString(), "All Level Files (*.nw)");

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
			level->saveNWFile();
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
