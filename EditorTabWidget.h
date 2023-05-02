#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QMap>
#include <QFont>
#include <QStringListModel>
#include "ui_EditorTabWidget.h"
#include "ui_TilesetsWidget.h"
#include "ui_TileObjectsWidget.h"
#include "ui_ObjectsWidget.h"
#include "ResourceManager.h"
#include "Level.h"
#include "Overworld.h"
#include "GraphicsView.h"
#include "Selector.h"
#include "AbstractSelection.h"
#include "IWorld.h"
#include "TileGroupListModel.h"

namespace TilesEditor
{
	class EditorTabWidget : public QWidget, 
		public IWorld
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

		void tileObjectsMousePress(QMouseEvent* event);

		void graphicsMousePress(QMouseEvent* event);
		void graphicsMouseRelease(QMouseEvent* event);
		void graphicsMouseMove(QMouseEvent* event);
		void graphicsMouseDoubleClick(QMouseEvent* event);
		void graphicsMouseWheel(QWheelEvent* event);
		void zoomMoved(int position);
		void layerChanged(int value);
		void layerVisibilityChanged();

		void preloadOverworldClicked(bool checked);
		void editLinksClicked(bool checked);
		void newLinkClicked(bool checked);
		void newSignClicked(bool checked);
		void editSignsClicked(bool checked);
		void cutPressed();
		void copyPressed();
		void pastePressed();
		void deleteClicked(bool checked);
		void saveClicked(bool checked);
		void saveAsClicked(bool checked);
		void tilesetsIndexChanged(int index);
		void tilesetDeleteClicked(bool checked);
		void tilesetNewClicked(bool checked);
		void tileGroupNewClicked(bool checked);
		void tileGroupDeleteClicked(bool checked);
		void tileGroupImportClicked(bool checked);
		void tileGroupIndexChanged(int index);

		void tileObjectNewClicked(bool checked);
		void tileObjectDeleteClicked(bool checked);
		void tileObjectIndexChanged(int index);

		void objectsNewNPCClicked(bool checked);
		void objectsRefreshClicked(bool checked);
		void objectsDoubleClick(const QModelIndex& index);

		//When the selector goes (he we will disable the button to create link/sign)
		void selectorGone();

		//When an area is selected (here we will enable the button to create link/signs
		void selectorMade();

		//When a new selection is created (here we will enable the copy button)
		void selectionMade();

		//When there is no selection (disable copy button)
		void selectionGone();

	public:
		EditorTabWidget(QWidget* parent, ResourceManager& rootResourceManager);
		~EditorTabWidget();

	private:
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

		ResourceManager m_resourceManager;
		GraphicsView* m_graphicsView;
		Level* m_level;
		Image* m_tilesetImage;

		Overworld* m_overworld;
		QMap<int, bool>	m_visibleLayers;

		int m_selectedTilesLayer;
		bool m_selectedLayerVisible;
		int m_defaultTile;

		Selector m_selector;
		Selector m_tilesSelector;

		AbstractSelection* m_selection;


		void doTileSelection();
		bool doObjectSelection(int x, int y, bool allowAppend);
		void setSelection(AbstractSelection* newSelection);
		Rectangle getViewRect() const;
		Level* getActiveLevel();
		bool saveLevel(Level* level);
		TileObject* getCurrentTileObject();


		void loadLevel(Level* level);
		bool selectingLevel();
		void setTileset(const QString& name);
		void setUnmodified();
		void setDefaultTile(int tile);

		bool canSelectObject(LevelEntityType type) const;

	public:

		void init(QStringListModel* tilesetList, TileGroupListModel* tileGroupList);
		QWidget* getTilesetsContainer() {
			return m_tilesetsContainer;
		}

		QWidget* getTileObjectsContainer() {
			return m_tileObjectsContainer;
		}

		QWidget* getObjectsContainer() {
			return m_objectsContainer;
		}

		bool getModified() const { return m_modified; }

		QString getName() const;
		QSet<Level*> getLevelsInRect(const IRectangle& rect) override;
		Level* getLevelAt(double x, double y) override;
		ResourceManager& getResourceManager() override;
		bool containsLevel(const QString& levelName) const override;
		void centerLevel(const QString& levelName) override;
		AbstractLevelEntity* getEntityAt(double x, double y) override;
		AbstractLevelEntity* getEntityAt(double x, double y, bool checkAllowedSelect);
		QList<AbstractLevelEntity*> getEntitiesAt(double x, double y) override;
		QList<AbstractLevelEntity*> getEntitiesAt(double x, double y, bool checkAllowedSelect);
		void deleteEntity(AbstractLevelEntity* entity) override;
		void setModified(Level* level) override;
		bool tryGetTileAt(double x, double y, int* outTile) override;
		void updateMovedEntity(AbstractLevelEntity* entity) override;
		QList<Level*> getModifiedLevels() override;

		void newLevel(int hcount, int vcount);
		void loadGMap(const QString& name, const QString& fileName);
		void loadLevel(const QString& name, const QString& fileName);
	};
};
