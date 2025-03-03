#pragma once

#include <QDialog>
#include <QPixmap>
#include <QList>
#include "ui_AniEditorAddSprite.h"
#include "Ani.h"
#include "AbstractResourceManager.h"

namespace TilesEditor
{
	class AniEditor;
	class AniEditorAddSprite : public QDialog
	{
		Q_OBJECT

	public slots:
		void renderScene(QPainter* painter, const QRectF& rect);
		void graphicsMousePress(QMouseEvent* event);
		void graphicsMouseRelease(QMouseEvent* event);
		void graphicsMouseMove(QMouseEvent* event);
		void graphicsMouseWheel(QWheelEvent* event);
		void graphicsKeyPress(QKeyEvent* event);

		void sourceItemChanged(int index);
		void imageNameEditingFinished();
		void addClicked(bool checked);
		void magicWandClicked(bool checked);

	private:
		Ui::AniEditorAddSpriteClass ui;
		int m_zoomLevel = 3;
		AniEditor* m_editor;
		AbstractResourceManager* m_resourceManager;
		QPixmap m_image;
		QList<Ani::AniSprite*> m_addedSprites;
		bool m_draggingBox = false;
		QPointF m_dragOffset;

		void updatePreviewImage();
		static QByteArray savedGeometry;
		inline static bool savedMagicWand = false;

	public:
		AniEditorAddSprite(AniEditor* editor, AbstractResourceManager* resourceManager, QWidget* parent = nullptr);
		~AniEditorAddSprite();

		QList<Ani::AniSprite*>& getAddedSprites() { return m_addedSprites; }
	};
};
