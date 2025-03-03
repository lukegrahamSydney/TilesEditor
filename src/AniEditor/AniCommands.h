#ifndef ANICOMMANDSH
#define ANICOMMANDSH

#include <QMap>
#include "IAniEditor.h"
#include "AbstractResourceManager.h"

namespace TilesEditor
{
	enum AniCommandType
	{
		ANI_COMMAND_SET_SPRITE_PROPERTY,
		ANI_COMMAND_MOVE_SPRITE_ATTACHMENT,
		ANI_COMMAND_MOVE_FRAME_PIECES,
		ANI_COMMAND_SET_FRAME_PROPERTY,
		ANI_COMMAND_MOVE_FRAME
	};
	class UndoCommandSetFrameAndDir :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		qsizetype m_newFrame;
		int m_newDir;
		qsizetype m_oldFrame;
		int m_oldDir;


	public:
		//SET NEW FRAME/DIR
		UndoCommandSetFrameAndDir(IAniEditor* editor, qsizetype newFrame, int dir, qsizetype oldFrame, int oldDir, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor)
		{
			m_newFrame = newFrame;
			m_newDir = dir;
			m_oldFrame = oldFrame;
			m_oldDir = oldDir;
		}

		void undo() override
		{
			m_editor->setFrameAndDir(m_oldFrame, m_oldDir);
		}

		void redo() override
		{
			m_editor->setFrameAndDir(m_newFrame, m_newDir);
		}

	};

	class UndoCommandMoveFramePieces :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		int m_frame;
		int m_dir;
		bool m_wasSelected = false;
		QMap<Ani::Frame::FramePiece*, QPointF> m_newPositions;
		QMap<Ani::Frame::FramePiece*, QPointF> m_oldPositions;



	public:
		UndoCommandMoveFramePieces(IAniEditor* editor, int frame, int dir, bool wasSelected, const QMap<Ani::Frame::FramePiece*, QPointF>& newPositions, const QMap<Ani::Frame::FramePiece*, QPointF>& oldPositions, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_frame(frame), m_dir(dir), m_wasSelected(wasSelected), m_newPositions(newPositions), m_oldPositions(oldPositions)
		{
		}

		void undo() override
		{
			m_editor->setModified();
			m_editor->setFrameAndDir(m_frame, m_dir);
			for (auto pair : m_oldPositions.asKeyValueRange())
			{
				pair.first->xoffset = pair.second.x();
				pair.first->yoffset = pair.second.y();
			}
			m_editor->setSelectedPieces(m_newPositions.keys());
		}

		void redo() override
		{
			m_editor->setModified();
			m_editor->setFrameAndDir(m_frame, m_dir);
			for (auto pair : m_newPositions.asKeyValueRange())
			{
				pair.first->xoffset = pair.second.x();
				pair.first->yoffset = pair.second.y();
			}

			m_editor->setSelectedPieces(m_newPositions.keys());
		}

		int id() const override {
			return ANI_COMMAND_MOVE_FRAME_PIECES;
		}

		bool mergeWith(const QUndoCommand* command) override
		{
			auto other = static_cast<const UndoCommandMoveFramePieces*>(command);
			
			if (this->m_newPositions.size() == other->m_newPositions.size() && this->m_newPositions.keys() == other->m_newPositions.keys())
			{
				this->m_newPositions = other->m_newPositions;
				return true;
			}
				
			return false;
		}
	};

	class UndoCommandSetFramePieceIndex :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		int m_frame;
		int m_dir;
		Ani::Frame::FramePiece* m_piece;
		qsizetype m_newIndex;
		qsizetype m_oldIndex;




	public:
		UndoCommandSetFramePieceIndex(IAniEditor* editor, int frame, int dir, Ani::Frame::FramePiece* piece, qsizetype newIndex, qsizetype oldIndex, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_frame(frame), m_dir(dir), m_piece(piece), m_newIndex(newIndex), m_oldIndex(oldIndex)
		{
		}

		void undo() override
		{
			m_editor->setModified();
			m_editor->setFrameAndDir(m_frame, m_dir);

			auto frame = m_editor->getAni().getFrame(m_frame);
			auto& pieces = frame->pieces[m_dir];
			pieces.move(m_newIndex, m_oldIndex);
			m_piece->index = m_oldIndex;
		}

		void redo() override
		{
			m_editor->setModified();
			m_editor->setFrameAndDir(m_frame, m_dir);

			auto frame = m_editor->getAni().getFrame(m_frame);
			auto& pieces = frame->pieces[m_dir];
			pieces.move(m_oldIndex, m_newIndex);
			m_piece->index = m_newIndex;
		}

	};


	class UndoCommandPopulateItems :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;



	public:
		UndoCommandPopulateItems(IAniEditor* editor, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor)
		{
		}

		void undo() override
		{
			m_editor->populateItems();
		}

		void redo() override
		{
			m_editor->populateItems();
		}

	};

	class UndoCommandDeletePiece :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::Frame* m_frame;
		int m_dir;
		Ani::Frame::FramePiece* m_piece;
		qsizetype m_insertPoint = -1;
		bool m_doDelete = true;


	public:
		UndoCommandDeletePiece(IAniEditor* editor, Ani::Frame* frame, int dir, Ani::Frame::FramePiece* piece, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor)
		{
			m_frame = frame;
			m_dir = dir;
			m_piece = piece;
		}

		~UndoCommandDeletePiece()
		{
			if (m_doDelete)
			{
				delete m_piece;
			}
		}

		void undo() override
		{
			m_editor->setModified();
			if (m_insertPoint >= 0)
			{
				m_doDelete = false;
				if (m_piece->type == Ani::Frame::PIECE_SPRITE)
				{
					auto& pieces = m_frame->pieces[m_dir];
					m_piece->index = m_insertPoint;
					pieces.insert(m_insertPoint, m_piece);
					m_editor->populateItems();
				}
				else if (m_piece->type == Ani::Frame::PIECE_SOUND)
				{
					m_piece->index = m_insertPoint;
					m_frame->sounds.insert(m_insertPoint, m_piece);
					m_editor->updateSoundsWidget(false);
				}

			}

		}

		void redo() override
		{
			m_doDelete = true;
			m_editor->setModified();
			if (m_piece->type == Ani::Frame::PIECE_SPRITE)
			{
				auto& pieces = m_frame->pieces[m_dir];

				m_insertPoint = pieces.indexOf(m_piece);
				if (m_insertPoint >= 0)
				{
					pieces.removeAt(m_insertPoint);
					m_editor->populateItems();
				}
			}
			else if (m_piece->type == Ani::Frame::PIECE_SOUND)
			{
				m_insertPoint = m_frame->sounds.indexOf(m_piece);
				if (m_insertPoint >= 0)
				{
					m_frame->sounds.removeAt(m_insertPoint);

					m_editor->updateSoundsWidget(false);
				}
			}

		}
	};

	class UndoCommandNewPiece :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::Frame* m_frame;
		int m_dir;
		Ani::Frame::FramePiece* m_piece;
		bool m_doDelete = false;


	public:
		UndoCommandNewPiece(IAniEditor* editor, Ani::Frame* frame, int dir, Ani::Frame::FramePiece* piece, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor)
		{
			m_frame = frame;
			m_dir = dir;
			m_piece = piece;
		}

		~UndoCommandNewPiece()
		{
			if (m_doDelete)
			{
				delete m_piece;
			}
		}

		void undo() override
		{
			m_editor->setModified();
			m_doDelete = true;

			if (m_piece->type == Ani::Frame::PIECE_SPRITE)
			{
				auto& pieces = m_frame->pieces[m_dir];

				pieces.removeOne(m_piece);
				m_editor->populateItems();

			}
			else if (m_piece->type == Ani::Frame::PIECE_SOUND)
			{
				m_frame->sounds.removeOne(m_piece);
				m_editor->updateSoundsWidget(false);
			}

		}

		void redo() override
		{
			m_editor->setModified();
			m_doDelete = false;

			if (m_piece->type == Ani::Frame::PIECE_SPRITE)
			{
				auto& pieces = m_frame->pieces[m_dir];
				m_piece->index = pieces.size();
				pieces.push_back(m_piece);
				m_editor->populateItems();
			}
			else if (m_piece->type == Ani::Frame::PIECE_SOUND)
			{
				m_piece->index = m_frame->sounds.size();
				m_frame->sounds.push_back(m_piece);
				m_editor->updateSoundsWidget(false);
			}

		}
	};


	class UndoCommandDeleteFrame :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::Frame* m_frame;
		qsizetype m_insertPoint = -1;
		bool m_doDelete = true;


	public:
		UndoCommandDeleteFrame(IAniEditor* editor, Ani::Frame* frame, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor)
		{
			m_frame = frame;
		}

		~UndoCommandDeleteFrame()
		{
			if (m_doDelete)
			{
				delete m_frame;
			}
		}

		void undo() override
		{
			m_editor->setModified();
			if (m_insertPoint >= 0)
			{
				m_doDelete = false;

				m_editor->insertFrame(m_insertPoint, m_frame);
			}

		}

		void redo() override
		{
			m_editor->setModified();
			m_doDelete = true;
			auto& ani = m_editor->getAni();
			m_insertPoint = ani.getFrames().indexOf(m_frame);
			if (m_insertPoint >= 0)
			{
				m_editor->removeFrame(m_insertPoint);
			}
		}
	};

	class UndoCommandInsertFrame :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::Frame* m_frame;
		qsizetype m_frameIndex = -1;
		bool m_doDelete = false;


	public:
		UndoCommandInsertFrame(IAniEditor* editor, Ani::Frame* frame, qsizetype index, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor)
		{
			m_frame = frame;
			m_frameIndex = index;
		}

		~UndoCommandInsertFrame()
		{
			if (m_doDelete)
			{
				delete m_frame;
			}
		}

		void undo() override
		{
			m_editor->setModified();
			m_doDelete = true;

			m_editor->removeFrame(m_frameIndex);

		}

		void redo() override
		{
			m_editor->setModified();
			m_doDelete = false;
			m_editor->insertFrame(m_frameIndex, m_frame);
		}
	};


	class UndoCommandMoveFrame :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		bool		m_callSetFrame = false;
		qsizetype	m_frameStart;
		qsizetype	m_moveTo;

	public:
		UndoCommandMoveFrame(IAniEditor* editor, qsizetype frameStart, qsizetype moveto, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor)
		{
			m_frameStart = frameStart;
			m_moveTo = moveto;
		}

		void undo() override
		{
			m_editor->setModified();
			if (m_moveTo != m_frameStart)
			{
				
				m_editor->getAni().getFrames().move(m_moveTo, m_frameStart);
				if (m_callSetFrame)
				{
					m_editor->setFrame(m_frameStart);
				}

			}
			m_callSetFrame = true;
		}

		void redo() override
		{
			m_editor->setModified();
			if (m_frameStart != m_moveTo)
			{
				
				m_editor->getAni().getFrames().move(m_frameStart, m_moveTo);

				if (m_callSetFrame)
				{
					m_editor->setFrame(m_moveTo);
				}

			}
			m_callSetFrame = true;
		}

		int id() const override {
			return ANI_COMMAND_MOVE_FRAME;
		}

		bool mergeWith(const QUndoCommand* command) override
		{
			auto other = static_cast<const UndoCommandMoveFrame*>(command);
			if (this->m_moveTo == other->m_frameStart)
			{
				if (this->m_frameStart == other->m_moveTo)
					this->setObsolete(true);
				this->m_moveTo = other->m_moveTo;
				return true;
			}
			return false;
		}
	};

	class UndoCommandSetSpriteProperty :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::AniSprite* m_sprite;
		QString m_propName;
		QVariant  m_newValue;
		QVariant  m_oldValue;

	public:
		UndoCommandSetSpriteProperty(IAniEditor* editor, Ani::AniSprite* sprite, const QString& propName, const QVariant& newValue, const QVariant& oldValue, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_sprite(sprite), m_propName(propName), m_newValue(newValue), m_oldValue(oldValue) {}

		void undo() override {
			m_editor->setSpriteProperty(m_sprite, m_propName, m_oldValue);
		}
		void redo() override {
			m_editor->setSpriteProperty(m_sprite, m_propName, m_newValue);
		}

		int id() const override { return ANI_COMMAND_SET_SPRITE_PROPERTY; }
		bool mergeWith(const QUndoCommand* command) override
		{
			auto other = static_cast<const UndoCommandSetSpriteProperty*>(command);

			if (this->m_sprite == other->m_sprite && other->m_propName == this->m_propName)
			{
				if (other->m_newValue == this->m_oldValue)
					this->setObsolete(true);

				this->m_newValue = other->m_newValue;

				
				return true;
			}
			return false;
		}
	};

	class UndoCommandSetFramePieceProperty :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::Frame::FramePiece* m_piece;
		QString m_propName;
		QVariant  m_newValue;
		QVariant  m_oldValue;

	public:
		UndoCommandSetFramePieceProperty(IAniEditor* editor, Ani::Frame::FramePiece* piece, const QString& propName, const QVariant& newValue, const QVariant& oldValue, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_piece(piece), m_propName(propName), m_newValue(newValue), m_oldValue(oldValue) {}

		void undo() override {
			m_editor->setFramePieceProperty(m_piece, m_propName, m_oldValue);
		}
		void redo() override {
			m_editor->setFramePieceProperty(m_piece, m_propName, m_newValue);
		}
	};


	class UndoCommandSetDefaultValue :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		QString m_propName;
		QVariant  m_newValue;
		QVariant  m_oldValue;

	public:
		UndoCommandSetDefaultValue(IAniEditor* editor, const QString& propName, const QVariant& newValue, const QVariant& oldValue, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_propName(propName), m_newValue(newValue), m_oldValue(oldValue) {}

		void undo() override {
			m_editor->setDefaultValue(m_propName, m_oldValue.toString(), true);
		}
		void redo() override {
			m_editor->setDefaultValue(m_propName, m_newValue.toString(), true);
		}
	};

	class UndoCommandSetAniProperty :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		QString m_propName;
		QVariant  m_newValue;
		QVariant  m_oldValue;

	public:
		UndoCommandSetAniProperty(IAniEditor* editor, const QString& propName, const QVariant& newValue, const QVariant& oldValue, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_propName(propName), m_newValue(newValue), m_oldValue(oldValue) {}

		void undo() override {
			m_editor->setAniProperty(m_propName, m_oldValue);
		}
		void redo() override {
			m_editor->setAniProperty(m_propName, m_newValue);
		}
	};

	class UndoCommandSetFrameProperty :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::Frame* m_frame;
		int m_dir;
		QString m_propName;
		QVariant  m_newValue;
		QVariant  m_oldValue;

	public:
		UndoCommandSetFrameProperty(IAniEditor* editor, Ani::Frame* frame, int dir, const QString& propName, const QVariant& newValue, const QVariant& oldValue, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_frame(frame), m_dir(dir), m_propName(propName), m_newValue(newValue), m_oldValue(oldValue) {}

		void undo() override {
			m_editor->setFrameProperty(m_frame, m_dir, m_propName, m_oldValue);
		}
		void redo() override {
			m_editor->setFrameProperty(m_frame, m_dir, m_propName, m_newValue);
		}

		int id() const override {
			return ANI_COMMAND_SET_FRAME_PROPERTY;
		}

		bool mergeWith(const QUndoCommand* command) override
		{
			auto other = static_cast<const UndoCommandSetFrameProperty*>(command);

			if (other->m_frame == this->m_frame && other->m_propName == this->m_propName)
			{
				m_newValue = other->m_newValue;
				return true;
			}
			return false;
		}
	};

	class UndoCommandAddSprite :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::AniSprite* m_sprite;
		AbstractResourceManager* m_resourceManager;

		bool m_doDelete = false;
	public:
		UndoCommandAddSprite(IAniEditor* editor, Ani::AniSprite* sprite, AbstractResourceManager* resourceManager, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_sprite(sprite), m_resourceManager(resourceManager){}

		~UndoCommandAddSprite()
		{
			if (m_doDelete)
			{
				m_sprite->releaseCustomImage(m_resourceManager);
				delete m_sprite;
			}
		}

		void undo() override {
			m_doDelete = true;
			m_editor->removeSprite(m_sprite);
		}

		void redo() override {
			m_doDelete = false;
			m_editor->insertNewSprite(m_sprite, -1);
		}
	};

	class UndoCommandDeleteSprite :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::AniSprite* m_sprite;
		AbstractResourceManager* m_resourceManager;
		int m_insertPos = -1;
		bool m_doDelete = true;

	public:
		UndoCommandDeleteSprite(IAniEditor* editor, Ani::AniSprite* sprite, AbstractResourceManager* resourceManager, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_sprite(sprite), m_resourceManager(resourceManager) {}

		~UndoCommandDeleteSprite()
		{
			if (m_doDelete)
			{
				m_sprite->releaseCustomImage(m_resourceManager);
				delete m_sprite;
			}
		}

		void undo() override {
			m_doDelete = false;
			m_editor->insertNewSprite(m_sprite, m_insertPos);
		}

		void redo() override {

			m_doDelete = true;
			m_insertPos = m_editor->removeSprite(m_sprite);
		}
	};

	class UndoCommandAddSpriteAttachment :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::AniSprite* m_sprite;
		int m_childIndex;
		QPointF m_pos;

	public:
		UndoCommandAddSpriteAttachment(IAniEditor* editor, Ani::AniSprite* sprite, int childIndex, const QPointF& pos, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_sprite(sprite), m_childIndex(childIndex), m_pos(pos) {}

		void undo() override {
			m_editor->setModified();
			m_sprite->attachedSprites.pop_back();
		}

		void redo() override {
			m_editor->setModified();
			m_sprite->attachedSprites.push_back(QPair<int, QPointF>(m_childIndex, m_pos));
		}
	};

	class UndoCommandDeleteSpriteAttachment :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::AniSprite* m_sprite;
		int m_childIndex;

		QPair<int, QPointF> m_attachment;
		bool incrementDrawIndex = false;

	public:
		UndoCommandDeleteSpriteAttachment(IAniEditor* editor, Ani::AniSprite* sprite, int childIndex, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_sprite(sprite), m_childIndex(childIndex) {}

		void undo() override {
			m_editor->setModified();
			
			m_sprite->attachedSprites.insert(m_childIndex, m_attachment);
			if (incrementDrawIndex)
			{
				m_sprite->m_drawIndex++;
			}
			incrementDrawIndex = false;
		}

		void redo() override {
			m_editor->setModified();
			
			if (m_childIndex < m_sprite->m_drawIndex)
			{
				m_sprite->m_drawIndex--;
				incrementDrawIndex = true;
			}
			m_attachment = m_sprite->attachedSprites[m_childIndex];
			m_sprite->attachedSprites.removeAt(m_childIndex);
		}
	};

	class UndoCommandMoveSpriteAttachment :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::AniSprite* m_sprite;
		int m_attachmentIndex;

		QPointF m_newPos;
		QPointF m_oldPos;


	public:
		UndoCommandMoveSpriteAttachment(IAniEditor* editor, Ani::AniSprite* sprite, int attachmentIndex, const QPointF& oldPos, const QPointF& newPos, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_sprite(sprite), m_attachmentIndex(attachmentIndex), m_oldPos(oldPos), m_newPos(newPos)
		{
		}

		void undo() override
		{
			m_editor->setModified();
			if (m_attachmentIndex >= 0 && m_attachmentIndex < m_sprite->attachedSprites.size())
			{
				auto& thing = m_sprite->attachedSprites[m_attachmentIndex];
				thing.second = m_oldPos;
			}
		}

		void redo() override
		{
			m_editor->setModified();
			
			if (m_attachmentIndex >= 0 && m_attachmentIndex < m_sprite->attachedSprites.size())
			{
				auto& thing = m_sprite->attachedSprites[m_attachmentIndex];
				thing.second = m_newPos;
			}
		}

		int id() const override { return ANI_COMMAND_MOVE_SPRITE_ATTACHMENT; }

		bool mergeWith(const QUndoCommand* command) override
		{
			auto other = static_cast<const UndoCommandMoveSpriteAttachment*>(command);
			if (other->m_sprite == this->m_sprite && other->m_attachmentIndex == this->m_attachmentIndex)
			{
				m_newPos = other->m_newPos;
				return true;
			}
			return false;
		}
	};


	class UndoCommandSetSpriteAttachmentIndex :
		public QUndoCommand
	{
	private:
		IAniEditor* m_editor;
		Ani::AniSprite* m_sprite;
		int m_attachmentIndex;

		int m_newIndex;


	public:
		UndoCommandSetSpriteAttachmentIndex(IAniEditor* editor, Ani::AniSprite* sprite, int attachmentIndex, int newIndex, QUndoCommand* parent) :
			QUndoCommand(parent), m_editor(editor), m_sprite(sprite), m_attachmentIndex(attachmentIndex), m_newIndex(newIndex)
		{
		}

		void undo() override
		{
			m_editor->setModified();

			m_editor->setSpriteAttachmentIndex(m_sprite, m_newIndex, m_attachmentIndex);
			
		}

		void redo() override
		{
			m_editor->setModified();
			m_editor->setSpriteAttachmentIndex(m_sprite, m_attachmentIndex, m_newIndex);
			
		}

	};
};
#endif