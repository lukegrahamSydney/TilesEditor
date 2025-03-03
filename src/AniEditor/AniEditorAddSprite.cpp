#include <QMessageBox>
#include <QStack>
#include <QSignalBlocker>
#include "AniEditorAddSprite.h"

#include "AniEditor.h"
#include "AniCommands.h"

namespace TilesEditor
{
	QByteArray AniEditorAddSprite::savedGeometry;

	AniEditorAddSprite::AniEditorAddSprite(AniEditor* editor, AbstractResourceManager* resourceManager, QWidget* parent)
		: QDialog(parent), m_editor(editor), m_resourceManager(resourceManager)
	{
		ui.setupUi(this);

		this->setWindowFlag(Qt::Window);
		this->setWindowFlag(Qt::WindowMaximizeButtonHint);

		ui.splitter->setStretchFactor(0, 0);
		ui.splitter->setStretchFactor(1, 1);

		QList<int> sizes = QList<int>{ 220, 10 };
		ui.splitter->setSizes(sizes);

		ui.spiteIndexSpinBox->setValue(editor->getAni().getNextSpriteIndex(false));

		connect(ui.graphicsView, &GraphicsView::renderView, this, &AniEditorAddSprite::renderScene);
		connect(ui.graphicsView, &GraphicsView::mousePress, this, &AniEditorAddSprite::graphicsMousePress);
		connect(ui.graphicsView, &GraphicsView::mouseMove, this, &AniEditorAddSprite::graphicsMouseMove);
		connect(ui.graphicsView, &GraphicsView::mouseRelease, this, &AniEditorAddSprite::graphicsMouseRelease);
		connect(ui.graphicsView, &GraphicsView::mouseWheelEvent, this, &AniEditorAddSprite::graphicsMouseWheel);
		connect(ui.graphicsView, &GraphicsView::keyPress, this, &AniEditorAddSprite::graphicsKeyPress);

		connect(ui.imageSourceComboBox, &QComboBox::currentIndexChanged, this, &AniEditorAddSprite::sourceItemChanged);

		connect(ui.imageFileLineEdit, &QLineEdit::editingFinished, this, &AniEditorAddSprite::imageNameEditingFinished);

		connect(ui.leftSpinBox, &QSpinBox::valueChanged, ui.graphicsView, &GraphicsView::redraw);
		connect(ui.topSpinBox, &QSpinBox::valueChanged, ui.graphicsView, &GraphicsView::redraw);

		connect(ui.heightSpinBox, &QSpinBox::valueChanged, ui.graphicsView, &GraphicsView::redraw);
		connect(ui.widthSpinBox, &QSpinBox::valueChanged, ui.graphicsView, &GraphicsView::redraw);
		connect(ui.gridColumnsSpinBox, &QSpinBox::valueChanged, ui.graphicsView, &GraphicsView::redraw);
		connect(ui.gridRowsSpinBox, &QSpinBox::valueChanged, ui.graphicsView, &GraphicsView::redraw);
		connect(ui.columnSeparationSpinBox, &QSpinBox::valueChanged, ui.graphicsView, &GraphicsView::redraw);
		connect(ui.rowSeparationSpinBox, &QSpinBox::valueChanged, ui.graphicsView, &GraphicsView::redraw);
		connect(ui.addButton, &QAbstractButton::clicked, this, &AniEditorAddSprite::addClicked);
		connect(ui.magicWandButton, &QAbstractButton::clicked, this, &AniEditorAddSprite::magicWandClicked);


		ui.magicWandButton->setChecked(savedMagicWand);
		ui.graphicsView->setCursor(ui.magicWandButton->isChecked() ? Qt::CrossCursor : Qt::ArrowCursor);
		if (!savedGeometry.isNull())
			restoreGeometry(savedGeometry);
	}

	AniEditorAddSprite::~AniEditorAddSprite()
	{
		savedGeometry = saveGeometry();
		savedMagicWand = ui.magicWandButton->isChecked();
	}


	void AniEditorAddSprite::updatePreviewImage()
	{
		QString fullPath;

		m_image = QPixmap();
		if (m_resourceManager->locateFile(ui.imageFileLineEdit->text(), &fullPath))
		{
			m_image.load(fullPath);
		}
		ui.graphicsView->setSceneRect(0, 0, m_image.width(), m_image.height());
		ui.graphicsView->redraw();
	}

	void AniEditorAddSprite::renderScene(QPainter* painter, const QRectF& viewRect)
	{

		painter->drawPixmap(0, 0, m_image);


		QPen newPen(QColorConstants::White, 1);
		newPen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
		painter->setPen(newPen);

		painter->translate(0.5 + ui.leftSpinBox->value(), 0.5 + ui.topSpinBox->value());
		if (ui.columnSeparationSpinBox->value() == 0 && ui.rowSeparationSpinBox->value() == 0)
		{
			auto compositionMode = painter->compositionMode();
			painter->setCompositionMode(QPainter::CompositionMode_Difference);



			

			int height = ui.gridRowsSpinBox->value() * ui.heightSpinBox->value();
			for (int column = 0; column <= ui.gridColumnsSpinBox->value(); ++column)
			{
				painter->drawLine(column * ui.widthSpinBox->value(), 0, column * ui.widthSpinBox->value(), height);
			}

			int width = ui.gridColumnsSpinBox->value() * ui.widthSpinBox->value();
			for (int row = 0; row <= ui.gridRowsSpinBox->value(); ++row)
			{
				painter->drawLine(1, row * ui.heightSpinBox->value(), width-1, row * ui.heightSpinBox->value());
			}
		}
		else {
			auto cellWidth = ui.widthSpinBox->value() + ui.columnSeparationSpinBox->value();
			auto cellHeight = ui.heightSpinBox->value() + ui.rowSeparationSpinBox->value();
			for (int column = 0; column < ui.gridColumnsSpinBox->value(); ++column)
			{
				for (int row = 0; row < ui.gridRowsSpinBox->value(); ++row)
				{
					QRect rect(column * cellWidth, row * cellHeight, ui.widthSpinBox->value(), ui.heightSpinBox->value());

					painter->drawRect(rect);

				}
			}
		}
	}

	void AniEditorAddSprite::graphicsMousePress(QMouseEvent* event)
	{
		auto mousePos = ui.graphicsView->mapToScene(event->pos());

		if (ui.magicWandButton->isChecked())
		{
			//QPixmap to QImage
			QImage image = m_image.toImage();
			
			QRect imageRect = image.rect();
			auto imageWidth = imageRect.width();
			auto imageHeight = imageRect.height();

			auto imageSize = imageWidth * imageHeight;

			if (imageSize > 0)
			{
				auto marked = new unsigned char[imageSize];
				memset(marked, 0, imageSize);

				QRect rect(mousePos.x(), mousePos.y(), 1, 1);
				QStack<QPoint> queue;
				
				auto addNode = [&](const QPoint& point)
				{
					if (imageRect.contains(point))
					{
						if (!marked[point.x() + point.y() * imageWidth] && image.pixelColor(point).alpha() > 0)
						{
							marked[point.x() + point.y() * imageWidth] = 1;
							queue.push(point);
						}
					}

				};

				addNode(QPoint(mousePos.x(), mousePos.y()));

				if (queue.size())
				{
					while (queue.size())
					{
						while (queue.size())
						{
							auto pos = queue.pop();
							if (pos.x() < rect.left())
								rect.setLeft(pos.x());
							if (pos.y() < rect.top())
								rect.setTop(pos.y());
							if (pos.x() > rect.right())
								rect.setRight(pos.x());
							if (pos.y() > rect.bottom())
								rect.setBottom(pos.y());

							addNode(QPoint(pos.x(), pos.y() - 1));
							addNode(QPoint(pos.x() - 1, pos.y()));
							addNode(QPoint(pos.x(), pos.y() + 1));
							addNode(QPoint(pos.x() + 1, pos.y()));
						}

						//Check the borders
						for (auto y = rect.y() - 1; y <= rect.bottom(); ++y)
						{
							addNode(QPoint(rect.x() - 1, y));
							addNode(QPoint(rect.right() + 1, y));
						}

						for (auto x = rect.x() - 1; x <= rect.right(); ++x)
						{
							addNode(QPoint(x, rect.y() - 1));
							addNode(QPoint(x, rect.bottom() + 1));
						}

					}

					

					const QSignalBlocker blocker1(ui.leftSpinBox);
					const QSignalBlocker blocker2(ui.topSpinBox);
					const QSignalBlocker blocker3(ui.widthSpinBox);
					const QSignalBlocker blocker4(ui.heightSpinBox);
					const QSignalBlocker blocker5(ui.gridColumnsSpinBox);
					const QSignalBlocker blocker6(ui.gridRowsSpinBox);
					ui.leftSpinBox->setValue(rect.left());
					ui.topSpinBox->setValue(rect.top());
					ui.widthSpinBox->setValue(rect.width());
					ui.heightSpinBox->setValue(rect.height());

					ui.gridColumnsSpinBox->setValue(1);
					ui.gridRowsSpinBox->setValue(1);
					ui.graphicsView->redraw();
				}
				delete[] marked;
			}
		}
		else {
			QRect rect(ui.leftSpinBox->value(), ui.topSpinBox->value(),
				(ui.widthSpinBox->value() + ui.columnSeparationSpinBox->value()) * ui.gridColumnsSpinBox->value(),
				(ui.heightSpinBox->value() + ui.rowSeparationSpinBox->value()) * ui.gridRowsSpinBox->value());

			m_draggingBox = false;
			if (rect.contains(mousePos.x(), mousePos.y()))
			{
				m_draggingBox = true;
				m_dragOffset = QPointF(mousePos.x() - rect.x(), mousePos.y() - rect.y());
				ui.graphicsView->redraw();
			}
		}
	}

	void AniEditorAddSprite::graphicsMouseRelease(QMouseEvent* event)
	{
		auto pos = ui.graphicsView->mapToScene(event->pos());
		m_draggingBox = false;
	}

	void AniEditorAddSprite::graphicsMouseMove(QMouseEvent* event)
	{
		auto pos = ui.graphicsView->mapToScene(event->pos());

		if (m_draggingBox)
		{
			ui.leftSpinBox->setValue(pos.x() - m_dragOffset.x());
			ui.topSpinBox->setValue(pos.y() - m_dragOffset.y());
			ui.graphicsView->redraw();
		}
	}

	void AniEditorAddSprite::graphicsMouseWheel(QWheelEvent* event)
	{
		auto oldMousePos = ui.graphicsView->mapToScene(ui.graphicsView->mapFromGlobal(QCursor::pos()));

		if (QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier))
		{

			if (event->angleDelta().y() < 0)
			{
				m_zoomLevel = qMax(0, m_zoomLevel - 1);
			}
			else {
				m_zoomLevel = qMin(7, m_zoomLevel + 1);
			}

			static qreal zoomFactors[] = {
				0.25,
				0.5,
				0.75,
				1.0,
				2,
				3,
				4,
				8
			};

			auto zoomLevel = zoomFactors[m_zoomLevel];

			auto scaleX = zoomLevel;
			auto scaleY = zoomLevel;

			ui.graphicsView->setAntiAlias(zoomLevel < 1);

			ui.graphicsView->resetTransform();
			ui.graphicsView->scale(scaleX, scaleY);

			auto newMousePos = ui.graphicsView->mapToScene(ui.graphicsView->mapFromGlobal(QCursor::pos()));

			auto deltaMousePos = newMousePos - oldMousePos;

			ui.graphicsView->translate(deltaMousePos.x(), deltaMousePos.y());
		}
		else event->ignore();
	}

	void AniEditorAddSprite::graphicsKeyPress(QKeyEvent* event)
	{
		if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
			ui.addButton->click();
	}

	void AniEditorAddSprite::sourceItemChanged(int index)
	{
		auto type = ui.imageSourceComboBox->currentText();

		
		if (type != "CUSTOM")
		{
			ui.imageFileLineEdit->setText(m_editor->getAni().getDefaultImageName(type));

			updatePreviewImage();
		}
	}
	void AniEditorAddSprite::imageNameEditingFinished()
	{
		updatePreviewImage();
	}

	void AniEditorAddSprite::addClicked(bool checked)
	{
		auto cellWidth = ui.widthSpinBox->value() + ui.columnSeparationSpinBox->value();
		auto cellHeight = ui.heightSpinBox->value() + ui.rowSeparationSpinBox->value();
		bool added = false;
		int spriteIndex = ui.spiteIndexSpinBox->value();

		int count = ui.gridColumnsSpinBox->value() * ui.gridRowsSpinBox->value();

		auto undoCommand = new QUndoCommand();
		QMessageBox::ButtonRole existsOptionAll = QMessageBox::ButtonRole::NoRole;
		for (int row = 0; row < ui.gridRowsSpinBox->value(); ++row)
		{
			for (int column = 0; column < ui.gridColumnsSpinBox->value(); ++column)
			{
				QRect rect(ui.leftSpinBox->value() + column * cellWidth, ui.topSpinBox->value() + row * cellHeight, ui.widthSpinBox->value(), ui.heightSpinBox->value());

				QMessageBox::ButtonRole existsOption = QMessageBox::ButtonRole::NoRole;
				if (m_editor->getAni().spriteExists(spriteIndex))
				{
					if (existsOptionAll == QMessageBox::ButtonRole::NoRole)
					{
						QMessageBox b;

						b.setText("This sprite index is already being used. What would you like to do?");


						b.addButton("New Sprite Index", QMessageBox::ButtonRole::ActionRole);
						b.addButton("Skip", QMessageBox::ButtonRole::RejectRole);


						b.setCheckBox(new QCheckBox("Apply to all occurances"));
						b.exec();

						existsOption = b.buttonRole(b.clickedButton());
						if (b.checkBox()->checkState() == Qt::CheckState::Checked)
							existsOptionAll = existsOption;
					}
					else existsOption = existsOptionAll;
				}
				
				//skip or cancel
				if (existsOption == QMessageBox::ButtonRole::RejectRole)
				{
					++spriteIndex;
					continue;
				}

				//New id
				if (existsOption == QMessageBox::ButtonRole::ActionRole)
					spriteIndex = m_editor->getAni().getNextSpriteIndex(true);

				auto sprite = new Ani::AniSprite;
				sprite->index = spriteIndex;

				sprite->comment = count == 1 ? ui.commentLineEdit->text() :  QString("%1 (%2, %3)").arg(ui.commentLineEdit->text()).arg(column).arg(row);
				sprite->left = rect.left();
				sprite->top = rect.top();
				sprite->width = rect.width();
				sprite->height = rect.height();
				sprite->type = ui.imageSourceComboBox->currentText();
				if (sprite->type == "CUSTOM")
				{
					sprite->customImageName = ui.imageFileLineEdit->text();
				}

				sprite->updateBoundingBox();

				
				new UndoCommandAddSprite(m_editor, sprite, m_resourceManager, undoCommand);
				added = true;
				++spriteIndex;


			}
		}

		if (added)
		{
			m_editor->addUndoCommand(undoCommand);
		}
		else delete undoCommand;
		ui.spiteIndexSpinBox->setValue(m_editor->getAni().getNextSpriteIndex(false));
		if (added)
		{
			QMessageBox::information(nullptr, "Success", "Sprites added sucessfully");
		}


		//this->accept();
	}
	void AniEditorAddSprite::magicWandClicked(bool checked)
	{
		ui.graphicsView->setCursor(checked ? Qt::CrossCursor : Qt::ArrowCursor);
	}
}
