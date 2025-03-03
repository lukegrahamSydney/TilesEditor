#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QMap>
#include <QFont>
#include <QStringListModel>
#include <QUndoStack>
#include <QSet>
#include <qtreewidget.h>
#include "ui_EditorTabWidget.h"
#include "ui_TilesetsWidget.h"
#include "ui_TileObjectsWidget.h"
#include "ui_ObjectsWidget.h"
#include "AbstractResourceManager.h"
#include "Level.h"
#include "Overworld.h"
#include "GraphicsView.h"
#include "Selector.h"
#include "AbstractSelection.h"
#include "IWorld.h"
#include "TileGroupListModel.h"
#include "Tileset.h"
#include "IFileRequester.h"
#include "ObjectClass.h"
#include "ObjectManager.h"
#include "IEngine.h"
#include "TileDefs.h"

namespace TilesEditor
{
	class EditorTabWidget : 
		public QWidget, 
		public IWorld,
		public IFileRequester
	{
		Q_OBJECT

	signals:
		void openLevel(const QString& levelName);
		void changeTabText(const QString& text);
		void setStatusBar(const QString& text, int section, int msecs);


	public slots:
		void renderScene(QPainter* painter, const QRectF& rect);
		void renderTilesetSelection(QPainter* painter, const QRectF& rect);
		void renderTileObjects(QPainter* painter, const QRectF& rect);
		void paintDefaultTile(QPainter* painter, const QRectF& rect);

		void tilesetMousePress(QMouseEvent* event);
		void tilesetMouseRelease(QMouseEvent* event);
		void tilesetMouseMove(QMouseEvent* event);
		void tilesetEditClicked(bool checked);
		void tilesetMouseWheel(QWheelEvent* event);


		void tileObjectsMousePress(QMouseEvent* event);

		void graphicsMousePress(QMouseEvent* event);
		void graphicsMouseRelease(QMouseEvent* event);
		void graphicsMouseMove(QMouseEvent* event);
		void graphicsMouseDoubleClick(QMouseEvent* event);
		void graphicsMouseWheel(QWheelEvent* event);
		void graphicsKeyPress(QKeyEvent* event);
		void graphicsScrolled();

		void zoomMoved(int position);
		void layerChanged(int value);
		void layerVisibilityChanged();

		void floodFillPatternClicked(bool checked);
		void preloadOverworldClicked(bool checked);
		void editLinksClicked(bool checked);
		void newLinkClicked(bool checked);
		void newSignClicked(bool checked);
		void editSignsClicked(bool checked);
		void undoClicked(bool checked);
		void redoClicked(bool checked);
		void cutPressed();
		void copyPressed();
		void pastePressed();
		void deleteClicked(bool checked);
		void screenshotClicked(bool checked);
		void saveClicked(bool checked);
		void saveAsClicked(bool checked);
		void tilesetsIndexChanged(int index);
		
		void tilesetDeleteClicked(bool checked);
		void tilesetRefreshClicked(bool checked);
		void tilesetNewClicked(bool checked);
		void tilesetOpenClicked(bool checked);
		void tilesetEditOrderClicked(bool checked);

		void tileGroupNewClicked(bool checked);
		void tileGroupDeleteClicked(bool checked);
		void tileGroupImportClicked(bool checked);
		void tileGroupIndexChanged(int index);

		void tileObjectNewClicked(bool checked);
		void tileObjectDeleteClicked(bool checked);
		void tileObjectIndexChanged(int index);

		void objectsNewNPCClicked(bool checked);
		void objectsNewChestClicked(bool checked);
		void objectsNewBaddyClicked(bool checked);
		void objectsRefreshClicked(bool checked);
		void objectsDoubleClick(const QModelIndex& index);

		void deleteEdgeLinksClicked(bool checked);
		void trimScriptEndingsClicked(bool checked);
		void trimSignEndingsClicked(bool checked);
		void tileIconMouseDoubleClick(QMouseEvent* event);
		void gridValueChanged(int);
	
		void downKeyPressed();
		void upKeyPressed();
		void leftKeyPressed();
		void rightKeyPressed();

		//When the selector goes (he we will disable the button to create link/sign)
		void selectorGone();

		//When an area is selected (here we will enable the button to create link/signs
		void selectorMade();

		//When a new selection is created (here we will enable the copy button)
		void selectionMade();

		//When there is no selection (disable copy button)
		void selectionGone();


	private:
		IEngine* m_engine;
		sgs_Variable m_thisObject;
		sgs_Variable m_sgsUserTable;

		Level* m_previousCenterLevel = nullptr;
		Ui::EditorTabWidgetClass ui;
		Ui::TilesetsWidgetClass ui_tilesetsClass;
		Ui::TileObjectsWidgetClass ui_tileObjectsClass;
		Ui::ObjectsWidgetClass ui_objectClass;

		QWidget* m_tilesetsContainer;
		QWidget* m_tileObjectsContainer;
		QWidget* m_objectsContainer;
		QFont	m_font1;
		bool m_modified;
		QPixmap	m_eyeOpen;
		QPixmap m_eyeClosed;

		QAction* m_selectNPCs;
		QAction* m_selectLinks;
		QAction* m_selectSigns;

		QAction* m_showNPCs;
		QAction* m_showLinks;
		QAction* m_showSigns;

		AbstractResourceManager* m_resourceManager;
		GraphicsView* m_graphicsView;
		Level* m_level;

		Tileset m_tileset;
		Image* m_tilesetImage;

		Overworld* m_overworld;
		QMap<int, bool>	m_visibleLayers;
		QPointF m_lastMousePos;

		QPixmap m_gridImage;

		int m_selectedTilesLayer;
		bool m_selectedLayerVisible;
		int m_defaultTile;

		Selector m_selector;
		Selector m_tilesSelector;

		AbstractSelection* m_selection;

		Tilemap	m_fillPattern;
		QUndoStack m_undoStack;

		bool m_panning = false;
		QPointF m_mousePanStart;



		void generateGridImage(int width, int height);
		void renderFloodFillPreview(const QPointF& point, QSet<Level*>& viewLevels, QPainter* painter, const QRectF& _rect);

		void doTileSelection(bool copyOnly);
		bool doObjectSelection(int x, int y, bool allowAppend);

		

		bool saveLevel(Level* level);
		TileObject* getCurrentTileObject();


		void loadLevel(Level* level, bool threaded = true);
		bool selectingLevel();
		void setTileset(const Tileset* tileset);
		void setTileset(const QString& name);
		void changeTileset(Tileset* tileset);
		void setUnmodified();
		void setDefaultTile(int tile);


		bool canSelectObject(LevelEntityType type) const;

		bool hasSelectionTiles() const;
		Tilemap* getTilesetSelection();
		Tilemap* getSelectionTiles();

		double getSnapX() const;
		double getSnapY() const;


	public:
		EditorTabWidget(IEngine* engine, AbstractResourceManager* resourceManager);
		~EditorTabWidget();

		void init(QStandardItemModel* tilesetList, TileGroupListModel* tileGroupList);
		QWidget* getTilesetsContainer() {
			return m_tilesetsContainer;
		}

		QWidget* getTileObjectsContainer() {
			return m_tileObjectsContainer;
		}

		QWidget* getObjectsContainer() {
			return m_objectsContainer;
		}

		Level* getActiveLevel();

		QString getFileName() const;
		void setSelection(AbstractSelection* newSelection);
		bool getModified() const { return m_modified; }
		QRectF getViewRect() const;
		QPointF getCenterPoint() const;
		void addUndoCommand(QUndoCommand* command) override;
		QString getName() const;
		QSet<Level*> getLevelsInRect(const QRectF& rect, bool threaded = true) override;
		Level* getLevelAt(const QPointF& point) override;
		AbstractResourceManager* getResourceManager() override;
		bool containsLevel(const QString& levelName) const override;
		void centerLevel(const QString& levelName) override;
		AbstractLevelEntity* getEntityAt(const QPointF& point) override;
		AbstractLevelEntity* getEntityAt(const QPointF& point, bool checkAllowedSelect);
		QList<AbstractLevelEntity*> getEntitiesAt(const QPointF& point) override;
		QList<AbstractLevelEntity*> getEntitiesAt(const QPointF& point, bool checkAllowedSelect);
		QSet<AbstractLevelEntity*> getEntitiesInRect(const QRectF& rect) override;
		
		void deleteEntity(AbstractLevelEntity* entity, QUndoCommand* parent = nullptr) override;
		void deleteEntities(const QList<AbstractLevelEntity*>& entities, QUndoCommand* parent = nullptr) override;
		void setModified(Level* level) override;
		bool tryGetTileAt(const QPointF& point, int* outTile) override;
		void removeEntitySelection(AbstractLevelEntity* entity)  override;
		void updateMovedEntity(AbstractLevelEntity* entity) override;
		void updateEntityRect(AbstractLevelEntity* entity) override;
		QList<Level*> getModifiedLevels() override;


		void getTiles(const QPointF& point, int layer, Tilemap* output) override;
		void putTiles(const QPointF& point, int layer, Tilemap* input, bool ignoreInvisible, bool applyTranslucency) override;
		void deleteTiles(const QPointF& point, int layer, int hcount, int vcount, int replacementTile) override;

		void floodFillPattern(const QPointF& point, int layer, const Tilemap* pattern, QList<TileInfo>* outputNodes = nullptr) override;
		void setProperty(const QString& name, const QVariant& value) override;
		void centerView(const QPointF& point);
		void addNewObjectClass(ObjectClass* objectClass);
		int getUnitWidth() const override;
		int getUnitHeight() const override;
		int getWidth() const override;
		int getHeight() const override;
		int getTileTranslucency() const override;
		int getDefaultTile() const override;

		void removeTileDefs(const QString& prefix) override;
		Image* getTilesetImage() override { return m_tilesetImage; }

		void applyTileDef2(const TileDef& tileDef);

		void setEntityProperty(AbstractLevelEntity* entity, const QString& name, const QVariant& value);
		void doPaste(bool centerScreen);
		void newLevel(const QString& format, int hcount, int vcount);
		void loadOverworld(const QString& name, const QString& fileName);
		void loadLevel(const QString& name, const QString& fileName);

		void redrawScene(const QRectF& rect) override;
		void redrawScene() override;
		void fileFailed(const QString& name, AbstractResourceManager* resourceManager) override;
		void fileReady(const QString& fileName, AbstractResourceManager* resourceManager) override;
		void fileWritten(const QString& fileName, AbstractResourceManager* resourceManager) override {}
		bool isLayerVisible(int layer) const;

		IEngine* getEngine() override { return m_engine; }

		void mark(sgs_Context* ctx);
		sgs_Variable& getScriptThisObject() { return m_thisObject; }
		static sgs_Variable sgs_classMembers;
		static sgs_ObjInterface sgs_interface;
		static void registerScriptClass(IEngine* engine);
	};
};
