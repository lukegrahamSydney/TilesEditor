#ifndef IANIEDITORH
#define IANIEDITORH

#include <QUndoCommand>
#include "Ani.h"

namespace TilesEditor
{
	class IAniEditor
	{
	public:
		virtual Ani& getAni() = 0;
		virtual void setFrame(int frame) = 0;
		virtual void setFrameAndDir(int frame, int dir) = 0;

		virtual void addUndoCommand(QUndoCommand* undoCommand) = 0;
		virtual void setModified() = 0;
		virtual void setUnmodified() = 0;
		virtual void updateSoundsWidget(bool playSound = true) = 0;
		virtual void insertNewSprite(Ani::AniSprite* sprite, int pos) = 0;
		virtual int removeSprite(Ani::AniSprite* sprite) = 0;
		virtual void setFramePieceProperty(Ani::Frame::FramePiece* piece, const QString& propName, const QVariant& value) = 0;
		virtual void setSpriteProperty(Ani::AniSprite* sprite, const QString& propName, const QVariant& value) = 0;
		virtual void setSpriteAttachmentIndex(Ani::AniSprite* sprite, int oldIndex, int newIndex) = 0;
		virtual void setDefaultValue(const QString& name, const QString& value, bool setCell = false) = 0;
		virtual void setAniProperty(const QString& propName, const QVariant& value) = 0;
		virtual void setFrameProperty(Ani::Frame* frame, int dir, const QString& propName, const QVariant& value) = 0;
		virtual void populateItems() = 0;
		virtual void removeFrame(qsizetype index) = 0;
		virtual void insertFrame(qsizetype index, Ani::Frame* frame) = 0;
		virtual void setSelectedPieces(const QList<Ani::Frame::FramePiece*>& pieces) = 0;
	};
}
#endif