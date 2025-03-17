#ifndef ANIEDITORH
#define ANIEDITORH

#include <QDialog>
#include <QList>
#include <QPair>
#include <QFont>
#include <QSet>
#include <QtMultimedia/QMediaPlayer>
#include <QAudioOutput>
#include <QElapsedTimer>
#include <QUndoStack>
#include <QSettings>
#include <QSoundEffect>
#include "ui_AniEditor.h"
#include "Ani.h"
#include "AbstractResourceManager.h"
#include "Selector.h"
#include "AniEditorAddSprite.h"
#include "IAniEditor.h"

namespace TilesEditor
{
	class SpriteListWidgetItem :
		public QListWidgetItem
	{
	public:
		SpriteListWidgetItem(const QIcon& icon, const QString& text) :
			QListWidgetItem(icon, text) {}

		bool operator<(const QListWidgetItem& other) const override
		{
			return this->data(Qt::UserRole).toInt() < other.data(Qt::UserRole).toInt();
		}
	};

	class AniEditor : public QWidget, public IAniEditor
	{
		Q_OBJECT

	signals:
		void changeTabText(const QString& text);
		void setStatusText(const QString& text);

	public slots:

		//Main graphics widget
		void renderScene(QPainter* painter, const QRectF& rect);
		void graphicsMousePress(QMouseEvent* event);
		void graphicsMouseRelease(QMouseEvent* event);
		void graphicsMouseMove(QMouseEvent* event);
		void graphicsMouseWheel(QWheelEvent* event);
		void graphicsKeyPress(QKeyEvent* event);

		//Timeline graphics widget
		void renderTimeline(QPainter* painter, const QRectF& rect);
		void timelineMouseWheel(QWheelEvent* event);
		void timelineMousePress(QMouseEvent* event);
		void timelineMouseRelease(QMouseEvent* event);
		void timelineMouseMove(QMouseEvent* event);
		void timelineKeyPress(QKeyEvent* event);

		//Sprite preview graphics widget
		void renderSpritePreview(QPainter* painter, const QRectF& rect);
		void spritePreviewMouseWheel(QWheelEvent* event);
		void spritePreviewMousePress(QMouseEvent* event);
		void spritePreviewMouseMove(QMouseEvent* event);
		void spritePreviewMouseRelease(QMouseEvent* event);
		void spritePreviewKeyPress(QKeyEvent* event);

		//other
		void directionIndexChanged(int index);
		void defaultValuesCellChanged(int row, int column);
		void durationChanged(int i);
		void itemChanged(int index);
		void playStopClicked(bool checked);
		void controlsClicked(bool checked);
		void editScriptClicked(bool checked);
		void bottomControlsClicked(bool checked);
		void playTimer();
		void soundsWidgetItemTextChange(QWidget* pLineEdit);
		void aniSettingsCheckChange();
		void nextAniEditingFinished();
		void itemSpriteIDEditingFinished();

		void soundSelectionChange();
		void spriteListItemClicked(QListWidgetItem* item);
		void spriteListItemDoubleClicked(QListWidgetItem* item);
		void spritePropertyChanged();
		void spriteColorEffectClicked(bool checked);
		void newSoundClicked(bool checked);
		void deleteSoundClicked(bool checked);
		void addSpriteClicked(bool checked);
		void importSpriteClicked(bool checked);
		void framePieceXYEdited(const QString& text);
		void finishedEditingSpriteProperty();
		void timelineSliderMoved(int value);

		void spriteControlsClicked(bool checked);
		void duplicateSpriteClicked(bool checked);
		void copySpriteClicked(bool checked);
		void pasteSpriteClicked(bool checked);
		void deleteSpriteClicked(bool checked);
		void defaultsClicked(bool checked);

		void downKeyPressed();
		void upKeyPressed();
		void leftKeyPressed();
		void rightKeyPressed();

		void onionSkinClicked(bool checked);
		void centerViewClicked(bool checked);
		void undoClicked(bool checked);
		void redoClicked(bool checked);
		void deselect();

	private:
		Ui::AniEditorClass ui;
		QFont	m_fontTimeline;
		AbstractResourceManager* m_resourceManager;
		Ani m_ani;
		Ani::AniSprite* m_editingSprite = nullptr;
		Ani::Frame::FramePiece* m_insertPiece = nullptr;

		bool m_panning = false;
		bool m_draggingFrame = false;
		int m_dragFrameStartIndex = 0;
		QPointF m_mousePanStart;

		int m_zoomLevel = 3;
		int m_spritePreviewZoomLevel = 3;
		int m_frame = 0;
		int m_resizeFrame = -1;
		int m_desiredResizeFrame = -1;
		int m_resizeOffset = 0;
		int m_resizeStartDuration = 50;


		QSoundEffect m_effect;
		bool m_playing = false;
		int m_playingPosition = 0;
		int m_playFramePosition = 0;
		QElapsedTimer m_playDelta;
		Qt::MouseButton m_dragButton = Qt::MouseButton::LeftButton;
		int m_compMode = 0;
		QSet<Ani::Frame::FramePiece*> m_selectedPieces;
		int m_selectedAttachedSprite = -1;
		QPointF m_attachedSpriteStartMove;
		Selector m_selector;
		bool m_modified = false;
		bool m_firstSetFrame = true;
		QColor m_backgroundColor = QColorConstants::DarkGreen;
		QPixmap m_speakerPix;
		QPixmap m_downRightPix;
		
		QUndoStack m_undoStack;

		void updateFrame();
		qsizetype addFramePieceToItems(Ani::Frame::FramePiece * piece);
		
		void deleteSelectedFrame();

		void updateItemSettings();
		void deleteSelectedFramePieces();
		QPixmap getSpritePixmap(Ani::AniSprite* sprite);
		bool onionSkinEnabled() const;
		void setSpriteEditor(Ani::AniSprite* sprite);
		
		void setSpritePreviewZoomLevel(int level);
		void updateSpriteItemWidget(Ani::AniSprite* sprite, bool commentOnly);
		void endMovePieces();
		void deleteSprite(Ani::AniSprite* sprite);

	protected:
		bool eventFilter(QObject* object, QEvent* event) override;

	public:
		AniEditor(const QString& name, AbstractResourceManager* resourceManager, QSettings& settings, AniEditor* copyStateEditor, QWidget* parent = nullptr);
		~AniEditor();

		Ani& getAni() override { return m_ani; }
		int getDir() const;
		int getFrame() const { return m_frame; }
		void setFrame(int frame) override;
		void setFrameAndDir(int frame, int dir) override;
		void stop();
		void updateSoundsWidget(bool playSound = true) override;
		void insertNewSprite(Ani::AniSprite* sprite, int pos) override;
		int removeSprite(Ani::AniSprite* sprite) override;
		void initNewAni();
		bool loadFile(const QString& fileName, const QString& fullPath);
		void saveFile();
		void setBackgroundColour(const QColor& color);
		void applySettings(QSettings& settings);

		void saveSettings(QSettings& settings);
		void setModified() override;
		void setUnmodified() override;
		bool isModified() const { return m_modified; }
		AbstractResourceManager* getResourceManager() { return m_resourceManager; }
		void setFramePieceProperty(Ani::Frame::FramePiece* piece, const QString& propName, const QVariant& value) override;
		void setSpriteProperty(Ani::AniSprite* sprite, const QString& propName, const QVariant& value) override;
		void setDefaultValue(const QString& name, const QString& value, bool setCell = false) override;
		void setAniProperty(const QString& propName, const QVariant& value) override;
		void setFrameProperty(Ani::Frame* frame, int dir, const QString& propName, const QVariant& value) override;
		void populateItems() override;
		void addUndoCommand(QUndoCommand* command) override;
		void setSpriteAttachmentIndex(Ani::AniSprite* sprite, int oldIndex, int newIndex) override;
		void removeFrame(qsizetype index) override;
		void insertFrame(qsizetype index, Ani::Frame* frame) override;
		void setSelectedPieces(const QList<Ani::Frame::FramePiece*>& pieces) override;
	};
}

#endif
