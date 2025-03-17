#include <QPainterPath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QClipboard>
#include <QStandardItem>
#include <QMessageBox>
#include <QClipboard>
#include <QColorDialog>
#include <QShortcut>
#include <algorithm>
#include <QAudioFormat>
#include "AniEditor.h"
#include "EditScriptForm.h"
#include "cJSON/JsonHelper.h"
#include "AniCommands.h"
#include <QSoundEffect>

namespace TilesEditor
{
	
	static QVariantList intListToVariantList(const QList<int>& list)
	{
		QVariantList retval;
		for (auto value : list)
			retval.push_back(value);
		return retval;
	}

	static QList<int> variantListToIntList(const QVariantList& list)
	{
		QList<int> retval;
		for (auto value : list)
			retval.push_back(value.toInt());
		return retval;
	}

	AniEditor::AniEditor(const QString& name, AbstractResourceManager* resourceManager, QSettings& settings, AniEditor* copyStateEditor, QWidget* parent)
		: QWidget(parent), m_ani(name, resourceManager), m_resourceManager(resourceManager)
	{
		this->setFocusPolicy(Qt::StrongFocus);
		m_resourceManager->incrementRef();
		ui.setupUi(this);

		m_speakerPix = QPixmap(":/MainWindow/icons/tinycolor/icons8-speaker-16.png");
		m_downRightPix = QPixmap(":/MainWindow/icons/tinycolor/icons8-down-right-16.png");

		ui.splitterLeft->setStretchFactor(0, 0);
		ui.splitterLeft->setStretchFactor(1, 1);

		ui.splitterRightSettings->setStretchFactor(0, 0);
		ui.splitterRightSettings->setStretchFactor(1, 1);
		
		ui.splitterRight->setStretchFactor(1, 0);
		ui.splitterRight->setStretchFactor(0, 1);



		ui.bottomLeftSplitter->setStretchFactor(1, 0);
		ui.bottomLeftSplitter->setStretchFactor(0, 1);

		ui.splitterSprites->setStretchFactor(1, 0);
		ui.splitterSprites->setStretchFactor(0, 1);


		ui.splitterSpriteSettings->setStretchFactor(1, 1);


		if (copyStateEditor)
		{
			//Copy the splitter sizes and state (such as collapsed) from another tab
			ui.splitterLeft->restoreState(copyStateEditor->ui.splitterLeft->saveState());
			ui.splitterLeft->setSizes(copyStateEditor->ui.splitterLeft->sizes());
			ui.splitterRight->restoreState(copyStateEditor->ui.splitterRight->saveState());
			ui.splitterRight->setSizes(copyStateEditor->ui.splitterRight->sizes());
			ui.bottomLeftSplitter->restoreState(copyStateEditor->ui.bottomLeftSplitter->saveState());
			ui.bottomLeftSplitter->setSizes(copyStateEditor->ui.bottomLeftSplitter->sizes());
			ui.splitterSprites->restoreState(copyStateEditor->ui.splitterSprites->saveState());
			ui.splitterSprites->setSizes(copyStateEditor->ui.splitterSprites->sizes());
			ui.splitterRightSettings->restoreState(copyStateEditor->ui.splitterRightSettings->saveState());
			ui.splitterRightSettings->setSizes(copyStateEditor->ui.splitterRightSettings->sizes());
			
			ui.splitterSpriteSettings->restoreState(copyStateEditor->ui.splitterSpriteSettings->saveState());
			ui.splitterSpriteSettings->setSizes(copyStateEditor->ui.splitterSpriteSettings->sizes());

			ui.onionSkinButton->setChecked(copyStateEditor->ui.onionSkinButton->isChecked());

			ui.defaultsTableSpacer->changeSize(0, 0, QSizePolicy::Minimum, copyStateEditor->ui.defaultsTable->isVisible() ? QSizePolicy::Maximum : QSizePolicy::Expanding);
			ui.defaultsTable->setVisible(copyStateEditor->ui.defaultsTable->isVisible());

		}
		else {
			//Otherwise initialize the sizes to these values
			QList<int> sizes = { 220, 10 };
			ui.splitterLeft->setSizes(sizes);

			sizes = { 10, 200 };
			ui.splitterRight->setSizes(sizes);

			sizes = { 10, 140 };
			ui.bottomLeftSplitter->setSizes(sizes);

			sizes = { 10, 300 };
			ui.splitterSprites->setSizes(sizes);

			if (settings.contains("GaniEditor/TabState"))
			{
				auto tabSettings = settings.value("GaniEditor/TabState").toMap();
				ui.splitterLeft->restoreState(tabSettings.value("splitterLeftState").toByteArray());
				ui.splitterLeft->setSizes(variantListToIntList(tabSettings.value("splitterLeftSizes").toList()));

				ui.splitterRight->restoreState(tabSettings.value("splitterRightState").toByteArray());
				ui.splitterRight->setSizes(variantListToIntList(tabSettings.value("splitterRightSizes").toList()));

				ui.bottomLeftSplitter->restoreState(tabSettings.value("bottomLeftSplitterState").toByteArray());
				ui.bottomLeftSplitter->setSizes(variantListToIntList(tabSettings.value("bottomLeftSplitterSizes").toList()));

				ui.splitterSprites->restoreState(tabSettings.value("splitterSpritesState").toByteArray());
				ui.splitterSprites->setSizes(variantListToIntList(tabSettings.value("splitterSpritesSizes").toList()));

				ui.splitterRightSettings->restoreState(tabSettings.value("splitterRightSettingsState").toByteArray());
				ui.splitterRightSettings->setSizes(variantListToIntList(tabSettings.value("splitterRightSettingsSizes").toList()));

				ui.splitterSpriteSettings->restoreState(tabSettings.value("splitterSpriteSettingsState").toByteArray());
				ui.splitterSpriteSettings->setSizes(variantListToIntList(tabSettings.value("splitterSpriteSettingsSizes").toList()));

				ui.onionSkinButton->setChecked(tabSettings.value("onionSkin").toBool());
				
				ui.defaultsTableSpacer->changeSize(0, 0, QSizePolicy::Minimum, tabSettings.value("defaultsVisible").toBool() ? QSizePolicy::Maximum : QSizePolicy::Expanding);
				ui.defaultsTable->setVisible(tabSettings.value("defaultsVisible").toBool());

			}
		}
		ui.graphicsView->setSceneRect(QRect(-5000, -5000, 10000, 10000));
		ui.graphicsView->centerOn(0, 0);
		ui.graphicsView->setAntiAlias(false);

		ui.graphicsViewTimeline->setSceneRect(QRect(-2, 0, 10000, 116));
		ui.graphicsViewTimeline->translate(2, 0);

		ui.graphicsViewTimeline->installEventFilter(this);
		ui.changeColorEffectButton->installEventFilter(this);
		ui.spritesListWidget->installEventFilter(this);

		m_fontTimeline.setFamily("Arial");
		m_fontTimeline.setPointSizeF(8);


		ui.spritesListWidget->setDragDropMode(QAbstractItemView::NoDragDrop);
		//Connects
		connect(ui.graphicsView, &GraphicsView::renderView, this, &AniEditor::renderScene);
		connect(ui.graphicsView, &GraphicsView::mousePress, this, &AniEditor::graphicsMousePress);
		connect(ui.graphicsView, &GraphicsView::mouseRelease, this, &AniEditor::graphicsMouseRelease);
		connect(ui.graphicsView, &GraphicsView::mouseMove, this, &AniEditor::graphicsMouseMove);
		connect(ui.graphicsView, &GraphicsView::mouseWheelEvent, this, &AniEditor::graphicsMouseWheel);
		connect(ui.graphicsView, &GraphicsView::keyPress, this, &AniEditor::graphicsKeyPress);

		connect(ui.graphicsViewTimeline, &GraphicsView::renderView, this, &AniEditor::renderTimeline);
		connect(ui.graphicsViewTimeline, &GraphicsView::mouseWheelEvent, this, &AniEditor::timelineMouseWheel);
		connect(ui.graphicsViewTimeline, &GraphicsView::mousePress, this, &AniEditor::timelineMousePress);
		connect(ui.graphicsViewTimeline, &GraphicsView::mouseRelease, this, &AniEditor::timelineMouseRelease);
		connect(ui.graphicsViewTimeline, &GraphicsView::mouseMove, this, &AniEditor::timelineMouseMove);
		connect(ui.graphicsViewTimeline, &GraphicsView::keyPress, this, &AniEditor::timelineKeyPress);
		


		connect(ui.graphicsViewSprite, &GraphicsView::renderView, this, &AniEditor::renderSpritePreview);
		connect(ui.graphicsViewSprite, &GraphicsView::mouseWheelEvent, this, &AniEditor::spritePreviewMouseWheel);
		connect(ui.graphicsViewSprite, &GraphicsView::mousePress, this, &AniEditor::spritePreviewMousePress);
		connect(ui.graphicsViewSprite, &GraphicsView::mouseRelease, this, &AniEditor::spritePreviewMouseRelease);
		
		connect(ui.graphicsViewSprite, &GraphicsView::mouseMove, this, &AniEditor::spritePreviewMouseMove);
		connect(ui.graphicsViewSprite, &GraphicsView::keyPress, this, &AniEditor::spritePreviewKeyPress);

		
		connect(ui.directionComboBox, &QComboBox::currentIndexChanged, this, &AniEditor::directionIndexChanged);

		connect(ui.defaultsTable, &QTableWidget::cellChanged, this, &AniEditor::defaultValuesCellChanged);
		
		connect(ui.durationWidget, &QSpinBox::valueChanged, this, &AniEditor::durationChanged);

		connect(ui.itemsComboWidget, &QComboBox::activated, this, &AniEditor::itemChanged);


		connect(ui.playButton, &QAbstractButton::clicked, this, &AniEditor::playStopClicked);
		connect(ui.stopButton, &QAbstractButton::clicked, this, &AniEditor::playStopClicked);

		connect(ui.itemLeftButton, &QAbstractButton::clicked, this, &AniEditor::controlsClicked);
		connect(ui.itemUpButton, &QAbstractButton::clicked, this, &AniEditor::controlsClicked);
		connect(ui.itemRightButton, &QAbstractButton::clicked, this, &AniEditor::controlsClicked);
		connect(ui.itemDownButton, &QAbstractButton::clicked, this, &AniEditor::controlsClicked);
		connect(ui.itemBackButton, &QAbstractButton::clicked, this, &AniEditor::controlsClicked);
		connect(ui.itemForwardButton, &QAbstractButton::clicked, this, &AniEditor::controlsClicked);

		connect(ui.soundsWidget->itemDelegate(), &QAbstractItemDelegate::commitData, this, &AniEditor::soundsWidgetItemTextChange);

		connect(ui.singleDirCheckBox, &QCheckBox::clicked, this, &AniEditor::aniSettingsCheckChange);
		connect(ui.loopedCheckBox, &QCheckBox::clicked, this, &AniEditor::aniSettingsCheckChange);
		connect(ui.continousCheckBox, &QCheckBox::clicked, this, &AniEditor::aniSettingsCheckChange);
		connect(ui.nextAniLineEdit, &QLineEdit::editingFinished, this, &AniEditor::nextAniEditingFinished);

		connect(ui.soundsWidget, &QListWidget::itemSelectionChanged, this, &AniEditor::soundSelectionChange);
		
		connect(ui.newFrameButton, &QAbstractButton::clicked, this, &AniEditor::bottomControlsClicked);
		connect(ui.deleteFrameButton, &QAbstractButton::clicked, this, &AniEditor::bottomControlsClicked);
		connect(ui.copyFrameButton, &QAbstractButton::clicked, this, &AniEditor::bottomControlsClicked);
		connect(ui.pasteBeforeButton, &QAbstractButton::clicked, this, &AniEditor::bottomControlsClicked);
		connect(ui.pasteAfterButton, &QAbstractButton::clicked, this, &AniEditor::bottomControlsClicked);
		connect(ui.reverseFrameButtons, &QAbstractButton::clicked, this, &AniEditor::bottomControlsClicked);

		connect(ui.spritesListWidget, &QListWidget::itemClicked, this, &AniEditor::spriteListItemClicked);
		connect(ui.spritesListWidget, &QListWidget::itemDoubleClicked, this, &AniEditor::spriteListItemDoubleClicked);

		connect(ui.onionSkinButton, &QAbstractButton::clicked, ui.graphicsView, &GraphicsView::redraw);
		connect(ui.centerViewButton, &QAbstractButton::clicked, this, &AniEditor::centerViewClicked);

		connect(ui.commentLineEdit, &QLineEdit::editingFinished, this, &AniEditor::spritePropertyChanged);
		connect(ui.xScaleDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &AniEditor::spritePropertyChanged);
		connect(ui.yScaleDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &AniEditor::spritePropertyChanged);
		connect(ui.rotationDoubleSpinBox, &QDoubleSpinBox::valueChanged, this, &AniEditor::spritePropertyChanged);
		connect(ui.rotationSlider, &QSlider::valueChanged, this, &AniEditor::spritePropertyChanged);
		connect(ui.xScaleSlider, &QSlider::valueChanged, this, &AniEditor::spritePropertyChanged);
		connect(ui.yScaleSlider, &QSlider::valueChanged, this, &AniEditor::spritePropertyChanged);
		connect(ui.colorEffectCheckBox, &QCheckBox::clicked, this, &AniEditor::spritePropertyChanged);
		connect(ui.spriteLeftSpinBox, &QSpinBox::editingFinished, this, &AniEditor::spritePropertyChanged);
		connect(ui.spriteTopSpinBox, &QSpinBox::editingFinished, this, &AniEditor::spritePropertyChanged);
		connect(ui.spriteWidthSpinBox, &QSpinBox::editingFinished, this, &AniEditor::spritePropertyChanged);
		connect(ui.spriteHeightSpinBox, &QSpinBox::editingFinished, this, &AniEditor::spritePropertyChanged);
		connect(ui.spriteSourceComboBox, &QComboBox::currentIndexChanged, this, &AniEditor::spritePropertyChanged);
		connect(ui.spriteImageLineEdit, &QLineEdit::editingFinished, this, &AniEditor::spritePropertyChanged);

		connect(ui.editScriptButton, &QAbstractButton::clicked, this, &AniEditor::editScriptClicked);
		connect(ui.addSoundButton, &QAbstractButton::clicked, this, &AniEditor::newSoundClicked);
		connect(ui.deleteSoundButton, &QAbstractButton::clicked, this, &AniEditor::deleteSoundClicked);
		connect(ui.addSpriteButton, &QAbstractButton::clicked, this, &AniEditor::addSpriteClicked);
		connect(ui.importSpritesButton, &QAbstractButton::clicked, this, &AniEditor::importSpriteClicked);

		connect(ui.deleteSpriteButton, &QAbstractButton::clicked, this, &AniEditor::spriteControlsClicked);
		connect(ui.attachmentBackButton, &QAbstractButton::clicked, this, &AniEditor::spriteControlsClicked);
		connect(ui.attachmentForwardButton, &QAbstractButton::clicked, this, &AniEditor::spriteControlsClicked);

		connect(ui.undoButton, &QAbstractButton::clicked, this, &AniEditor::undoClicked);
		connect(ui.redoButton, &QAbstractButton::clicked, this, &AniEditor::redoClicked);


		connect(ui.xLineEdit, &QLineEdit::textEdited, this, &AniEditor::framePieceXYEdited);
		connect(ui.yLineEdit, &QLineEdit::textEdited, this, &AniEditor::framePieceXYEdited);
		connect(ui.itemSpriteIDEdit, &QLineEdit::editingFinished, this, &AniEditor::itemSpriteIDEditingFinished);

		


		connect(ui.xScaleDoubleSpinBox, &QAbstractSpinBox::editingFinished, this, &AniEditor::finishedEditingSpriteProperty);
		connect(ui.yScaleDoubleSpinBox, &QAbstractSpinBox::editingFinished, this, &AniEditor::finishedEditingSpriteProperty);
		connect(ui.rotationDoubleSpinBox, &QAbstractSpinBox::editingFinished, this, &AniEditor::finishedEditingSpriteProperty);

		connect(ui.xScaleSlider, &QSlider::sliderReleased, this, &AniEditor::finishedEditingSpriteProperty);
		connect(ui.yScaleSlider, &QSlider::sliderReleased, this, &AniEditor::finishedEditingSpriteProperty);
		connect(ui.rotationSlider, &QSlider::sliderReleased, this, &AniEditor::finishedEditingSpriteProperty);
		connect(ui.commentLineEdit, &QLineEdit::editingFinished, this, &AniEditor::finishedEditingSpriteProperty);
		connect(ui.timelineSlider, &QAbstractSlider::valueChanged, this, &AniEditor::timelineSliderMoved);
		connect(ui.timelineSlider, &QAbstractSlider::sliderMoved, this, &AniEditor::timelineSliderMoved);
		connect(ui.duplicateSpriteButton, &QAbstractButton::clicked, this, &AniEditor::duplicateSpriteClicked);

		connect(ui.copySpriteButton, &QAbstractButton::clicked, this, &AniEditor::copySpriteClicked);
		connect(ui.pasteSpriteButton, &QAbstractButton::clicked, this, &AniEditor::pasteSpriteClicked);
		connect(ui.deleteSprite2Button, &QAbstractButton::clicked, this, &AniEditor::deleteSpriteClicked);

		connect(ui.defaultsButton, &QAbstractButton::clicked, this, &AniEditor::defaultsClicked);
		ui.durationWidget->setStepType(QAbstractSpinBox::StepType::DefaultStepType);
		//test
		ui.spriteEditPanel->setEnabled(false);
		

		ui.undoButton->setEnabled(false);
		ui.redoButton->setEnabled(false);

		ui.xLineEdit->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, ui.xLineEdit));
		ui.yLineEdit->setValidator(new QDoubleValidator(-1000000000, 1000000000, 2, ui.yLineEdit));
		setSpritePreviewZoomLevel(4);

		auto downKeyShortcut = new QShortcut(this);
		downKeyShortcut->setKey(Qt::Key_Down);
		connect(downKeyShortcut, &QShortcut::activated, this, &AniEditor::downKeyPressed);

		auto upKeyShortcut = new QShortcut(this);
		upKeyShortcut->setKey(Qt::Key_Up);
		connect(upKeyShortcut, &QShortcut::activated, this, &AniEditor::upKeyPressed);

		auto leftKeyShortcut = new QShortcut(this);
		leftKeyShortcut->setKey(Qt::Key_Left);
		connect(leftKeyShortcut, &QShortcut::activated, this, &AniEditor::leftKeyPressed);

		auto rightKeyShortcut = new QShortcut(this);
		rightKeyShortcut->setKey(Qt::Key_Right);
		connect(rightKeyShortcut, &QShortcut::activated, this, &AniEditor::rightKeyPressed);


		//Using QShortcut instead of the built-in shortcut in QAbstractButton since that's
		//Too slow with the key-repeat
		auto sKeyShortCut = new QShortcut(this);
		sKeyShortCut->setKey(Qt::Key_S);
		connect(sKeyShortCut, &QShortcut::activated, ui.itemDownButton, &QAbstractButton::click);

		auto aKeyShortCut = new QShortcut(this);
		aKeyShortCut->setKey(Qt::Key_A);
		connect(aKeyShortCut, &QShortcut::activated, ui.itemLeftButton, &QAbstractButton::click);

		auto dKeyShortCut = new QShortcut(this);
		dKeyShortCut->setKey(Qt::Key_D);
		connect(dKeyShortCut, &QShortcut::activated, ui.itemRightButton, &QAbstractButton::click);

		auto wKeyShortCut = new QShortcut(this);
		wKeyShortCut->setKey(Qt::Key_W);
		connect(wKeyShortCut, &QShortcut::activated, ui.itemUpButton, &QAbstractButton::click);

		applySettings(settings);

		ui.spritesListWidget->setSortingEnabled(true);

	}

	void AniEditor::updateFrame()
	{
		populateItems();

		m_selectedPieces.clear();



	}
	qsizetype AniEditor::addFramePieceToItems(Ani::Frame::FramePiece* piece)
	{
		const QSignalBlocker blocker(ui.itemsComboWidget);
		auto newIndex = ui.itemsComboWidget->count();
		ui.itemsComboWidget->addItem(piece->toString(&m_ani), QVariant(piece->type));
		ui.itemsComboWidget->setItemData(newIndex, QVariant(piece->id), Qt::UserRole + 1);

		return newIndex;
	}

	void AniEditor::populateItems()
	{
		const QSignalBlocker blocker(ui.itemsComboWidget);
		auto selectedItemID = ui.itemsComboWidget->currentData(Qt::UserRole + 1).toULongLong();
		ui.itemsComboWidget->clear();
		ui.itemsComboWidget->addItem("", QVariant(-1));

		auto frame = m_ani.getFrame(m_frame);
		if (frame)
		{
			auto& pieces = frame->pieces[getDir()];

			for (qsizetype i = 0; i < pieces.size(); ++i)
			{
				auto& piece = pieces[i];
				
				auto newIndex = addFramePieceToItems(piece); 
				
				if (piece->id == selectedItemID)
				{
					
					ui.itemsComboWidget->setCurrentIndex(newIndex);
					
				}
			}


		}
		
	}


	void AniEditor::deleteSelectedFrame()
	{
		//Make sure to clear the stack first
		auto frame = m_ani.getFrame(getFrame());
		if (frame)
		{
			qsizetype newFrameIndex = m_frame > 0 ? m_frame - 1 : m_frame;

			auto undoCommand = new QUndoCommand(nullptr);
			new UndoCommandSetFrameAndDir(this, newFrameIndex, getDir(), getFrame(), getDir(), undoCommand);
			new UndoCommandDeleteFrame(this, frame, undoCommand);
			addUndoCommand(undoCommand);

			ui.graphicsView->redraw();
			ui.graphicsViewTimeline->redraw();
		}
	}

	void AniEditor::newSoundClicked(bool checked)
	{
		auto frame = m_ani.getFrame(getFrame());

		if (frame)
		{
			auto piece = new Ani::Frame::FramePieceSound();
			piece->xoffset = 24;
			piece->yoffset = 32;
			piece->fileName = "new";

			auto undoCommand = new QUndoCommand(nullptr);
			new UndoCommandSetFrameAndDir(this, getFrame(), getDir(), getFrame(), getDir(), undoCommand);
			new UndoCommandNewPiece(this, frame, getDir(), piece, undoCommand);
			addUndoCommand(undoCommand);
			ui.graphicsView->redraw();
			ui.graphicsViewTimeline->redraw();
		}
	}

	void AniEditor::deleteSoundClicked(bool checked)
	{
		auto soundItem = ui.soundsWidget->currentItem();
		if (soundItem)
		{
			auto soundID = soundItem->data(Qt::UserRole).toULongLong();

			auto frame = m_ani.getFrame(getFrame());
			if (frame)
			{
				for (auto sound : frame->sounds)
				{
					if (sound->id == soundID)
					{
						deselect();

						auto undoCommand = new QUndoCommand(nullptr);
						new UndoCommandSetFrameAndDir(this, getFrame(), getDir(), getFrame(), getDir(), undoCommand);
						new UndoCommandDeletePiece(this, frame, getDir(), sound, undoCommand);
						addUndoCommand(undoCommand);

						ui.graphicsView->redraw();
						ui.graphicsViewTimeline->redraw();
						break;
					}
				}
			}
		}
	}



	void AniEditor::addSpriteClicked(bool checked)
	{
		AniEditorAddSprite form(this, m_resourceManager);
		form.exec();
	}

	void AniEditor::insertNewSprite(Ani::AniSprite* sprite, int pos)
	{
		this->setModified();
		m_ani.addSprite(sprite);
		auto pixmap = getSpritePixmap(sprite);
		auto item = new SpriteListWidgetItem(pixmap, QString::number(sprite->index));
		
		item->setData(Qt::UserRole, QVariant(sprite->index));
		item->setData(Qt::UserRole + 1, QVariant(sprite->type));
		item->setToolTip(sprite->comment);

		if (pos == -1)
			ui.spritesListWidget->addItem(item);
		else ui.spritesListWidget->insertItem(pos, item);
	}

	int AniEditor::removeSprite(Ani::AniSprite* sprite)
	{
		this->setModified();

		m_ani.getSprites().remove(sprite->index);

		if (sprite == m_editingSprite)
		{
			m_editingSprite = nullptr;
			setSpriteEditor(nullptr);
		}

		for (int i = 0; i < ui.spritesListWidget->count(); ++i)
		{
			auto item = ui.spritesListWidget->item(i);

			if (item->data(Qt::UserRole).toInt() == sprite->index)
			{
				delete ui.spritesListWidget->takeItem(i);
				return i;
			}
		}
		return -1;
	}

	void AniEditor::importSpriteClicked(bool checked)
	{
		auto fullPath = m_resourceManager->getOpenFileName("Open File", m_resourceManager->getConnectionString(), "*.gani");

		if (!fullPath.isEmpty())
		{
			QFile file(fullPath);

			if (file.open(QIODeviceBase::ReadOnly))
			{
				Ani importAni(file.fileName(), m_resourceManager);

				if (Ani::loadGraalAni(&importAni, &file, m_resourceManager))
				{
					auto undoCommand = new QUndoCommand(nullptr);

					//Import default image names if ours are blank
					for (int i = 0; i < ui.defaultsTable->verticalHeader()->count(); ++i)
					{
						QString keyName = ui.defaultsTable->verticalHeaderItem(i)->text().toUpper();

						auto value = m_ani.getDefaultImageName(keyName);
						if (value.isEmpty())
						{
							auto newValue = importAni.getDefaultImageName(keyName);

							if (!newValue.isEmpty())
							{
								const QSignalBlocker blocker(ui.defaultsTable);
								ui.defaultsTable->setItem(i, 0, new QTableWidgetItem(newValue));
								new UndoCommandSetDefaultValue(this, keyName, newValue, "", undoCommand);
							}
						}
					}

					QMessageBox::ButtonRole existsOptionAll = QMessageBox::ButtonRole::NoRole;
					for (auto& sprite : importAni.getSprites())
					{
						int spriteIndex = sprite->index;

						QMessageBox::ButtonRole existsOption = QMessageBox::ButtonRole::NoRole;
						if (getAni().spriteExists(spriteIndex))
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
							continue;

						//New id
						if (existsOption == QMessageBox::ButtonRole::ActionRole)
							spriteIndex = getAni().getNextSpriteIndex(true);

						auto newSprite = new Ani::AniSprite();
						newSprite->index = spriteIndex;
						newSprite->comment = sprite->comment;
						newSprite->type = sprite->type;
						newSprite->customImageName = sprite->customImageName;
						newSprite->left = sprite->left;
						newSprite->top = sprite->top;
						newSprite->width = sprite->width;
						newSprite->height = sprite->height;
						newSprite->rotation = sprite->rotation;
						newSprite->xscale = sprite->xscale;
						newSprite->yscale = sprite->yscale;
						
						newSprite->updateBoundingBox();

						new UndoCommandAddSprite(this, newSprite, m_resourceManager, undoCommand);
					}

					addUndoCommand(undoCommand);
				}
			}
		}
	}

	void AniEditor::framePieceXYEdited(const QString& text)
	{
		
		if (m_selectedPieces.size() > 0)
		{
			auto piece = *m_selectedPieces.begin();

			QMap<Ani::Frame::FramePiece*, QPointF> newPositions;
			QMap<Ani::Frame::FramePiece*, QPointF> oldPositions;

			newPositions[piece] = QPointF(ui.xLineEdit->text().toDouble(), ui.yLineEdit->text().toDouble());
			oldPositions[piece] = QPointF(piece->xoffset, piece->yoffset);

			//Do it like this to prevent the undo command from merging
			auto undoCommand = new QUndoCommand(nullptr);
			new UndoCommandMoveFramePieces(this, getFrame(), getDir(), true, newPositions, oldPositions, undoCommand);

			addUndoCommand(undoCommand);
			ui.graphicsView->redraw();
		}

	}

	void AniEditor::stop()
	{
		m_playing = false;
		//ui.playStopButton->setIcon(QIcon(":/MainWindow/icons/tinycolor/icons8-play-16.png"));
		ui.graphicsViewTimeline->redraw();
	}

	void AniEditor::finishedEditingSpriteProperty()
	{
		if (m_editingSprite)
		{
			/*
			if (sender() == ui.xScaleDoubleSpinBox || sender() == ui.xScaleSlider)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "xscale", m_editingSprite->xscale, m_editingSprite->startXScale, nullptr));
			}
			else if (sender() == ui.yScaleDoubleSpinBox || sender() == ui.yScaleSlider)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "yscale", m_editingSprite->yscale, m_editingSprite->startYScale, nullptr));
			} else if (sender() == ui.rotationDoubleSpinBox || sender() == ui.rotationSlider)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "rotation", m_editingSprite->rotation, m_editingSprite->startRotation, nullptr));
			}
			else */
			
			/*
			if (sender() == ui.commentLineEdit)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "comment", m_editingSprite->comment, m_editingSprite->comment, nullptr));
			}*/
			
		}
	}

	void AniEditor::timelineSliderMoved(int value)
	{
		setFrame(value);

		double x = 0.0;

		for (qsizetype i = 0; i < m_ani.getFrameCount(); ++i)
		{
			auto frame = m_ani.getFrame(i);

			int width = frame->duration;
			int right = x + width;

			if (i == getFrame())
			{
				ui.graphicsViewTimeline->ensureVisible(x + width / 2, 0.0, width, 0, 100, 0);
				break;
			}


			x = right;
			
		}
		
		ui.graphicsViewTimeline->redraw();
		ui.graphicsView->redraw();
	}


	void AniEditor::spriteControlsClicked(bool checked)
	{
		if (sender() == ui.deleteSpriteButton)
		{
			if (m_editingSprite)
			{
				deselect();

				deleteSprite(m_editingSprite);


				m_editingSprite = nullptr;
				setSpriteEditor(nullptr);

				ui.graphicsView->redraw();
			}
		}
		else if (sender() == ui.attachmentBackButton)
		{
			if (m_editingSprite && m_selectedAttachedSprite >= 0 && m_selectedAttachedSprite < m_editingSprite->attachedSprites.size())
			{
				auto undoCommand = new QUndoCommand();

				if (m_selectedAttachedSprite == m_editingSprite->m_drawIndex)
				{
					new UndoCommandSetSpriteProperty(this, m_editingSprite, "drawIndex", m_editingSprite->m_drawIndex + 1, m_editingSprite->m_drawIndex, undoCommand);

				}
				else {

					if (m_selectedAttachedSprite > 0)
					{
						new UndoCommandSetSpriteAttachmentIndex(this, m_editingSprite, m_selectedAttachedSprite, m_selectedAttachedSprite - 1, undoCommand);
					}
				}



				if (undoCommand->childCount())
					addUndoCommand(undoCommand);
				else delete undoCommand;
			}
		}
		else if (sender() == ui.attachmentForwardButton)
		{
			if (m_editingSprite && m_selectedAttachedSprite >= 0 && m_selectedAttachedSprite < m_editingSprite->attachedSprites.size())
			{
				auto undoCommand = new QUndoCommand();

				if (m_selectedAttachedSprite + 1 == m_editingSprite->m_drawIndex)
				{
					new UndoCommandSetSpriteProperty(this, m_editingSprite, "drawIndex", m_editingSprite->m_drawIndex - 1, m_editingSprite->m_drawIndex, undoCommand);

				}
				else {

					if (m_selectedAttachedSprite < m_editingSprite->attachedSprites.size() - 1)
					{
						new UndoCommandSetSpriteAttachmentIndex(this, m_editingSprite, m_selectedAttachedSprite, m_selectedAttachedSprite + 1, undoCommand);
					}
				}



				if (undoCommand->childCount())
					addUndoCommand(undoCommand);
				else delete undoCommand;
			}
		}
	}


	void AniEditor::duplicateSpriteClicked(bool checked)
	{
		if (m_selectedPieces.size() > 0)
		{
			auto frame = m_ani.getFrame(getFrame());

			if (frame)
			{
				auto undoCommand = new QUndoCommand(nullptr);

				Ani::AniSprite* newEditorSprite = nullptr;
				for (auto piece : m_selectedPieces)
				{
					if (piece->type == Ani::Frame::PIECE_SPRITE)
					{
						auto spritePiece = static_cast<Ani::Frame::FramePieceSprite*>(piece);

						if (spritePiece->spriteIndex != Ani::SPRITE_INDEX_STRING)
						{
							auto sprite = m_ani.getAniSprite(spritePiece->spriteIndex, "");
							if (sprite)
							{
								auto newSpriteIndex = m_ani.getNextSpriteIndex(true);

								auto newSprite = sprite->duplicate(newSpriteIndex);

								new UndoCommandAddSprite(this, newSprite, m_resourceManager, undoCommand);
								new UndoCommandSetFramePieceProperty(this, spritePiece, "spriteIndex", QString::number(newSpriteIndex), spritePiece->spriteIndex == -1 ? spritePiece->spriteName : QString::number(spritePiece->spriteIndex), undoCommand);

								if (m_editingSprite == sprite)
								{
									newEditorSprite = newSprite;
								}
							}
						}
					}
				}

				if (undoCommand->childCount())
					addUndoCommand(undoCommand);
				else delete undoCommand;

				if (newEditorSprite)
				{

					setSpriteEditor(newEditorSprite);

					for (qsizetype i = 0; i < ui.spritesListWidget->count(); ++i)
					{
						auto item = ui.spritesListWidget->item(i);
						if (item->data(Qt::UserRole).toInt() == newEditorSprite->index)
						{
							const QSignalBlocker blocker(ui.spritesListWidget);
							ui.spritesListWidget->setCurrentRow(i);
							break;
						}
					}
				}
			}
		}
	}

	void AniEditor::copySpriteClicked(bool checked)
	{
		auto item = ui.spritesListWidget->currentItem();
		if (item)
		{
			auto sprite = m_ani.getAniSprite(item->data(Qt::UserRole).toInt(), "");

			auto json = cJSON_CreateObject();
			cJSON_AddStringToObject(json, "type", "sprite");
			
			cJSON_AddStringToObject(json, "spriteType", sprite->type.toLocal8Bit().data());
			if(sprite->type == "CUSTOM")
				cJSON_AddStringToObject(json, "image", sprite->customImageName.toLocal8Bit().data());

			cJSON_AddNumberToObject(json, "left", sprite->left);
			cJSON_AddNumberToObject(json, "top", sprite->top);
			cJSON_AddNumberToObject(json, "width", sprite->width);
			cJSON_AddNumberToObject(json, "height", sprite->height);
			cJSON_AddNumberToObject(json, "rotation", sprite->rotation);
			cJSON_AddNumberToObject(json, "xscale", sprite->xscale);
			cJSON_AddNumberToObject(json, "yscale", sprite->yscale);
			cJSON_AddStringToObject(json, "colorEffect", sprite->colorEffect.name().toLocal8Bit());
			cJSON_AddStringToObject(json, "comment", sprite->comment.toLocal8Bit());

			auto attachments = cJSON_CreateArray();
			for (qsizetype i = 0; i < sprite->attachedSprites.size(); ++i)
			{
				auto& attachment = sprite->attachedSprites[i];
				auto jsonAttachment = cJSON_CreateObject();
				cJSON_AddNumberToObject(jsonAttachment, "index", attachment.first);
				cJSON_AddNumberToObject(jsonAttachment, "x", attachment.second.x());
				cJSON_AddNumberToObject(jsonAttachment, "y", attachment.second.y());

				cJSON_AddItemToArray(attachments, jsonAttachment);
			}

			cJSON_AddItemToObject(json, "attachments", attachments);
			auto buffer = cJSON_Print(json);
			QApplication::clipboard()->setText(QString(buffer));
			free(buffer);
			cJSON_Delete(json);
		}
	}

	void AniEditor::pasteSpriteClicked(bool checked)
	{
		auto text = QApplication::clipboard()->text();

		QByteArray ba = text.toLocal8Bit();

		auto json = cJSON_Parse(ba.data());
		if (json != nullptr)
		{
			auto type = jsonGetChildString(json, "type");
			if (type == "sprite")
			{
				auto sprite = new Ani::AniSprite();
				sprite->index = m_ani.getNextSpriteIndex(false);

				sprite->type = jsonGetChildString(json, "spriteType");
				if(sprite->type == "CUSTOM")
					sprite->customImageName = jsonGetChildString(json, "image");

				sprite->left = jsonGetChildInt(json, "left");
				sprite->top = jsonGetChildInt(json, "top");
				sprite->width = jsonGetChildInt(json, "width");
				sprite->height = jsonGetChildInt(json, "height");

				sprite->rotation = jsonGetChildDouble(json, "rotation");
				sprite->xscale = jsonGetChildDouble(json, "xscale");
				sprite->yscale = jsonGetChildDouble(json, "yscale");

				sprite->comment = jsonGetChildString(json, "comment");
				sprite->colorEffectEnabled = false;
				if (cJSON_GetObjectItem(json, "colorEffect"))
				{
					sprite->colorEffectEnabled = true;
					sprite->colorEffect = QColor::fromString(jsonGetChildString(json, "colorEffect"));
				}

				auto jsonAttachments = cJSON_GetObjectItem(json, "attachments");
				if (jsonAttachments && jsonAttachments->type == cJSON_Array)
				{
					for (int i = 0; i < cJSON_GetArraySize(jsonAttachments); ++i)
					{
						auto jsonAttachment = cJSON_GetArrayItem(jsonAttachments, i);
						QPointF pos(jsonGetChildDouble(jsonAttachment, "x"), jsonGetChildDouble(jsonAttachment, "y"));
						sprite->attachedSprites.push_back(QPair<int, QPointF>(jsonGetChildInt(jsonAttachment, "index"), pos));
					}
				}

				sprite->updateBoundingBox();
				addUndoCommand(new UndoCommandAddSprite(this, sprite, m_resourceManager, nullptr));
				for (qsizetype i = 0; i < ui.spritesListWidget->count(); ++i)
				{
					auto item = ui.spritesListWidget->item(i);
					if (item->data(Qt::UserRole).toInt() == sprite->index)
					{
						const QSignalBlocker blocker(ui.spritesListWidget);
						ui.spritesListWidget->setCurrentRow(i);
						break;
					}
				}

			}
		}
	}

	void AniEditor::deleteSpriteClicked(bool checked)
	{
		auto item = ui.spritesListWidget->currentItem();

		if (item)
		{
			auto spriteIndex = item->data(Qt::UserRole).toInt();
			
			auto sprite = m_ani.getAniSprite(spriteIndex, "");
			if (sprite)
			{
				addUndoCommand(new UndoCommandDeleteSprite(this, sprite, m_resourceManager, nullptr));
			}
		}
	}

	void AniEditor::defaultsClicked(bool checked)
	{
		ui.defaultsTable->setVisible(!ui.defaultsTable->isVisible());
		ui.defaultsTableSpacer->changeSize(0, 0, QSizePolicy::Minimum, ui.defaultsTable->isVisible() ? QSizePolicy::Maximum : QSizePolicy::Expanding);
	}

	void AniEditor::downKeyPressed()
	{
		ui.directionComboBox->setCurrentIndex(2);
	}

	void AniEditor::upKeyPressed()
	{
		ui.directionComboBox->setCurrentIndex(0);
	}

	void AniEditor::leftKeyPressed()
	{
		ui.directionComboBox->setCurrentIndex(1);
	}

	void AniEditor::rightKeyPressed()
	{
		ui.directionComboBox->setCurrentIndex(3);
	}

	void AniEditor::updateItemSettings()
	{
		const QSignalBlocker blocker1(ui.xLineEdit);
		const QSignalBlocker blocker2(ui.yLineEdit);
		const QSignalBlocker blocker3(ui.itemSpriteIDEdit);
		const QSignalBlocker blocker4(ui.itemsComboWidget);
		if (m_selectedPieces.size() > 0)
		{
			auto item = *m_selectedPieces.begin();
	
			ui.xLineEdit->setText(QString::number(item->xoffset));
			ui.yLineEdit->setText(QString::number(item->yoffset));
			ui.itemSpriteIDEdit->setText("");

			if (item->type == Ani::Frame::PIECE_SOUND)
			{
				for (qsizetype i = 0; i < ui.soundsWidget->count(); ++i)
				{
					if (ui.soundsWidget->item(i)->data(Qt::UserRole).toULongLong() == item->id)
					{
						const QSignalBlocker blocker(ui.soundsWidget);
						ui.soundsWidget->setCurrentRow(i);
						return;
					}
				}
			}
			else if (item->type == Ani::Frame::PIECE_SPRITE)
			{
				ui.itemSpriteIDEdit->setEnabled(true);
				auto itemPieceSprite = static_cast<Ani::Frame::FramePieceSprite*>(item);

				setSpriteEditor(m_ani.getAniSprite(itemPieceSprite->spriteIndex, itemPieceSprite->spriteName));
				ui.itemSpriteIDEdit->setText(itemPieceSprite->spriteIndex == Ani::SPRITE_INDEX_STRING ? itemPieceSprite->spriteName : QString::number(itemPieceSprite->spriteIndex));

				for (qsizetype i = 0; i < ui.itemsComboWidget->count(); ++i)
				{
					if (ui.itemsComboWidget->itemData(i, Qt::UserRole + 1).toULongLong() == item->id)
					{
						
						
						ui.itemsComboWidget->setCurrentIndex(i);
						break;
					}
				}
			} else ui.itemSpriteIDEdit->setEnabled(false);
			ui.soundsWidget->setCurrentRow(-1);
		}
		else {

			ui.xLineEdit->setText("");
			ui.yLineEdit->setText("");
			ui.itemSpriteIDEdit->setText("");
			ui.soundsWidget->setCurrentRow(-1);
			ui.itemsComboWidget->setCurrentIndex(-1);
		}


	}

	void AniEditor::deleteSelectedFramePieces()
	{

		if (m_selectedPieces.size() > 0)
		{
			endMovePieces();

			QUndoCommand* undoCommand = new QUndoCommand(nullptr);
			auto frame = m_ani.getFrame(getFrame());
			for (auto piece : m_selectedPieces)
			{
				new UndoCommandDeletePiece(this, frame, getDir(), piece, undoCommand);
			}

			addUndoCommand(undoCommand);
		}
		m_selectedPieces.clear();
	}

	QPixmap AniEditor::getSpritePixmap(Ani::AniSprite* sprite)
	{
		Image* image = nullptr;
		if (sprite->type == "CUSTOM")
			image = sprite->getCustomImage(m_resourceManager);
		else {
			image = m_ani.getDefaultImage(sprite->type);

			if(image == nullptr)
				image = m_ani.getHiddenDefaultImage(sprite->type, m_resourceManager);
		}

		if (image)
		{
			QRect rect(sprite->left, sprite->top, sprite->width, sprite->height);
			auto pixmap = image->pixmap();
			QRect clippedRect = pixmap.rect().intersected(rect);

			pixmap = image->pixmap().copy(clippedRect);

			QTransform t;
			t.scale(sprite->xscale, sprite->yscale);
			t.translate(clippedRect.width() / 2, clippedRect.height() / 2);
			t.rotate(sprite->rotation);
			t.translate(clippedRect.width() / -2, clippedRect.height() / -2);

			return pixmap.transformed(t);
			
		}
		return QPixmap();
	}

	bool AniEditor::onionSkinEnabled() const
	{
		return ui.onionSkinButton->isChecked();
	}

	void AniEditor::saveSettings(QSettings& settings)
	{
		QVariantMap state;
		state["splitterLeftState"] = this->ui.splitterLeft->saveState();
		state["splitterLeftSizes"] = intListToVariantList(this->ui.splitterLeft->sizes());

		state["splitterRightState"] = this->ui.splitterRight->saveState();
		state["splitterRightSizes"] = intListToVariantList(this->ui.splitterRight->sizes());

		state["bottomLeftSplitterState"] = this->ui.bottomLeftSplitter->saveState();
		state["bottomLeftSplitterSizes"] = intListToVariantList(this->ui.bottomLeftSplitter->sizes());

		state["splitterSpritesState"] = this->ui.splitterSprites->saveState();
		state["splitterSpritesSizes"] = intListToVariantList(this->ui.splitterSprites->sizes());

		state["splitterSpriteSettingsState"] = this->ui.splitterSpriteSettings->saveState();
		state["splitterSpriteSettingsSizes"] = intListToVariantList(this->ui.splitterSpriteSettings->sizes());

		state["splitterRightSettingsState"] = this->ui.splitterRightSettings->saveState();
		state["splitterRightSettingsSizes"] = intListToVariantList(this->ui.splitterRightSettings->sizes());
		state["onionSkin"] = ui.onionSkinButton->isChecked();
		state["defaultsVisible"] = ui.defaultsTable->isVisible();

		settings.setValue("GaniEditor/TabState", state);

	}

	void AniEditor::addUndoCommand(QUndoCommand* command)
	{
		m_undoStack.push(command);

		ui.undoButton->setEnabled(m_undoStack.canUndo());
		ui.redoButton->setEnabled(m_undoStack.canRedo());

		if (m_undoStack.isClean())
		{
			setUnmodified();

		}
		
		
	}

	void AniEditor::setUnmodified()
	{
		if (m_modified)
		{
			m_modified = false;
			emit changeTabText((m_ani.getFileName().isEmpty() ? QString("new") : m_ani.getFileName()));
			m_undoStack.setClean();
		}
	}

	void AniEditor::setSpriteAttachmentIndex(Ani::AniSprite* sprite, int oldIndex, int newIndex)
	{
		if (sprite == m_editingSprite)
		{
			if (m_selectedAttachedSprite == oldIndex)
				m_selectedAttachedSprite = newIndex;
		}
		sprite->attachedSprites.move(oldIndex, newIndex);

		if (sprite == m_editingSprite)
			ui.graphicsViewSprite->redraw();

		
	}

	void AniEditor::setSelectedPieces(const QList<Ani::Frame::FramePiece*>& pieces)
	{
		if (m_selectedPieces.values() != pieces)
		{
			m_selectedPieces.clear();
			for (auto piece : pieces)
				m_selectedPieces.insert(piece);

			updateItemSettings();
		}
	}

	void AniEditor::removeFrame(qsizetype index)
	{
		m_ani.removeFrame(index);
		const QSignalBlocker blocker(ui.timelineSlider);
		ui.timelineSlider->setMaximum(qMax(0, m_ani.getFrameCount() - 1));
		ui.totalDurationLabel->setText(QString("%1s").arg(QString::number(m_ani.getTotalDuration() / 1000.0, 103, 3)));
		ui.frameCountLabel->setText(QString("%1/%2").arg(getFrame() + 1).arg(m_ani.getFrameCount()));
	}

	void AniEditor::applySettings(QSettings& settings)
	{

	}

	void AniEditor::insertFrame(qsizetype index, Ani::Frame* frame)
	{
		m_ani.insertFrame(index, frame);

		const QSignalBlocker blocker(ui.timelineSlider);
		ui.timelineSlider->setMaximum(qMax(0, m_ani.getFrameCount() - 1));
		ui.totalDurationLabel->setText(QString("%1s").arg(QString::number(m_ani.getTotalDuration() / 1000.0, 103, 3)));
		ui.frameCountLabel->setText(QString("%1/%2").arg(getFrame() + 1).arg(m_ani.getFrameCount()));
	}


	void AniEditor::deselect()
	{
		endMovePieces();

		if (m_selectedPieces.size())
		{

			const QSignalBlocker blocker(ui.itemsComboWidget);
			ui.itemsComboWidget->setCurrentIndex(0);

			updateItemSettings();
			m_selectedPieces.clear();

			ui.graphicsViewTimeline->redraw();

		
		}
	}

	//Randomly asked deepseek to add comments to this method lol
	void AniEditor::deleteSprite(Ani::AniSprite* sprite)
	{
		// Create a new QUndoCommand to group all undoable actions related to deleting the sprite.
		auto undoCommand = new QUndoCommand(nullptr);

		// Iterate through all frames in the animation.
		for (auto& frame : m_ani.getFrames())
		{
			int dir = 0; // Initialize direction counter for the current frame.

			// Iterate through all direction-specific pieces in the current frame.
			for (auto& dirPieces : frame->pieces)
			{
				// Iterate through each piece in the current direction.
				for (auto& piece : dirPieces)
				{
					// Check if the piece is of type "PIECE_SPRITE".
					if (piece->type == Ani::Frame::PIECE_SPRITE)
					{
						// Cast the piece to a FramePieceSprite to access sprite-specific data.
						auto spritePiece = static_cast<Ani::Frame::FramePieceSprite*>(piece);

						// Check if the sprite index of the piece matches the sprite being deleted.
						if (spritePiece->spriteIndex == sprite->index)
						{
							// Create an undo command to delete this specific piece from the frame.
							new UndoCommandDeletePiece(this, frame, dir, piece, undoCommand);
						}
					}
				}
				++dir; // Increment the direction counter for the next set of pieces.
			}
		}

		// Create an undo command to delete the sprite itself from the ani.
		new UndoCommandDeleteSprite(this, sprite, m_resourceManager, undoCommand);

		// Add the grouped undo command to the undo stack for potential undo/redo operations.
		addUndoCommand(undoCommand);
	}

	bool AniEditor::eventFilter(QObject* object, QEvent* event)
	{
		if (object == ui.graphicsViewTimeline)
		{
			if (event->type() == QEvent::FocusOut || event->type() == QEvent::FocusIn)
			{
				//Redraw the timeline if we focus out of it (to clear the rectangle selection)
				ui.graphicsViewTimeline->redraw();
			}
		}
		else if (object == ui.changeColorEffectButton && ui.changeColorEffectButton->isEnabled())
		{
			if (event->type() == QEvent::MouseButtonPress)
			{
				if (m_editingSprite)
				{
					QColorDialog dialog(m_editingSprite->colorEffect);
					dialog.setOption(QColorDialog::ColorDialogOption::ShowAlphaChannel, true);

					if (dialog.exec())
					{
						auto undoCommand = new UndoCommandSetSpriteProperty(this, m_editingSprite, "color", dialog.selectedColor().rgba(), m_editingSprite->colorEffect.rgba(), nullptr);

						addUndoCommand(undoCommand);
					}
				}
			}
		}
		else if (object == ui.spritesListWidget)
		{
			if (event->type() == QEvent::KeyPress)
			{
				auto keyEvent = static_cast<QKeyEvent*>(event);
				if (keyEvent->key() == Qt::Key::Key_Delete)
				{
					auto selectedItem = ui.spritesListWidget->currentItem();
					if (selectedItem)
					{
						auto spriteIndex = selectedItem->data(Qt::UserRole).toInt();
						auto sprite = m_ani.getAniSprite(spriteIndex, "");

						deleteSprite(sprite);
						ui.graphicsView->redraw();
					}
				} else 	if (keyEvent->key() == Qt::Key::Key_C && (QApplication::keyboardModifiers() & Qt::ControlModifier))
				{
					ui.copySpriteButton->click();
					return true;
				}
				else 	if (keyEvent->key() == Qt::Key::Key_V && (QApplication::keyboardModifiers() & Qt::ControlModifier))
				{
					ui.pasteSpriteButton->click();
					return true;
				}
			}
		}
		return false;
	}


	void AniEditor::graphicsMousePress(QMouseEvent* event)
	{
		
		auto pos = ui.graphicsView->mapToScene(event->pos());

		if (m_insertPiece)
		{
			if (event->buttons().testFlag(Qt::LeftButton))
			{
				auto frame = m_ani.getFrame(getFrame());
				if (frame)
				{
					auto undoCommand = new QUndoCommand(nullptr);
					new UndoCommandSetFrameAndDir(this, getFrame(), getDir(), getFrame(), getDir(), undoCommand);
					new UndoCommandNewPiece(this, frame, getDir(), m_insertPiece, undoCommand);
					addUndoCommand(undoCommand);

					
					m_selectedPieces.clear();
					m_selectedPieces.insert(m_insertPiece);

					{
						const QSignalBlocker blocker(ui.itemsComboWidget);
						ui.itemsComboWidget->setCurrentIndex(addFramePieceToItems(m_insertPiece));
					}
					updateItemSettings();
					m_dragButton = Qt::MouseButton::LeftButton;
					m_insertPiece = nullptr;
					ui.graphicsView->redraw();
					return;
				}
				
			}
			delete m_insertPiece;
			m_insertPiece = nullptr;
			ui.graphicsView->redraw();
			return;
		}


		if (event->buttons().testFlag(Qt::RightButton) && m_dragButton == Qt::MouseButton::NoButton && m_insertPiece)
		{
			delete m_insertPiece;
			m_insertPiece = nullptr;
			ui.graphicsView->redraw();
		}
			
		if(event->buttons().testFlag(Qt::MouseButton::MiddleButton))
		{

			m_mousePanStart = pos;
			m_panning = true;
			ui.graphicsView->setCursor(Qt::CursorShape::ClosedHandCursor);
		}
		
		else if (event->buttons().testFlag(Qt::LeftButton))
		{
			if (m_selectedPieces.size())
			{
				bool found = false;
				//If clicking within an already selected piece, return
				for (auto piece : m_selectedPieces)
				{
					auto boundingBox = piece->getBoundingBox(&m_ani);
					boundingBox.translate(piece->xoffset, piece->yoffset);

					if (boundingBox.intersects(QRectF(pos.x(), pos.y(), 1, 1)))
					{
						found = true;
						break;
					}
				}

				if (found)
				{
					for (auto piece : m_selectedPieces)
					{
						piece->dragOffset = QPointF(pos.x() - piece->xoffset, pos.y() - piece->yoffset);
					}
					m_dragButton = Qt::MouseButton::LeftButton;
					return;
				}
			}

			if (!QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
				deselect();
			
			auto frame = m_ani.getFrame((size_t)getFrame());

			int dir = getDir();
			if (frame != nullptr)
			{
				if (m_ani.isSingleDir()) { dir = 0; }
				dir = dir < 0 ? 0 : dir;
				dir = dir > 3 ? 3 : dir;

				QVector<Ani::Frame::FramePiece*>* sources[] = {
					&frame->sounds,
					&frame->pieces[dir]
				};

				bool found = false;
				for (size_t i = 0; i < sizeof(sources) / sizeof(sources[0]) && !found; ++i)
				{
					auto pieces = sources[i];

					for (auto it = pieces->rbegin(); it != pieces->rend(); ++it)
					{
						auto& piece = *it;

						auto boundingBox = piece->getBoundingBox(&m_ani);
						boundingBox.translate(piece->xoffset, piece->yoffset);

						if (boundingBox.intersects(QRectF(pos.x(), pos.y(), 1, 1)))
						{
							m_selectedPieces.insert(piece);
							piece->dragOffset = QPointF(pos.x() - piece->xoffset, pos.y() - piece->yoffset);
							found = true;
							break;
						}
					}
				}

				if (found && QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ShiftModifier))
				{
					for (auto piece : m_selectedPieces)
					{
						piece->dragOffset = QPointF(pos.x() - piece->xoffset, pos.y() - piece->yoffset);
					}
				}

				if (m_selectedPieces.size() == 1)
				{
					auto first = *m_selectedPieces.begin();

					for (int i = 0; i < ui.itemsComboWidget->count(); ++i)
					{
						if (ui.itemsComboWidget->itemData(i, Qt::UserRole).toInt() == first->type && ui.itemsComboWidget->itemData(i, Qt::UserRole + 1).toULongLong() == first->id)
						{
							ui.itemsComboWidget->setCurrentIndex(i);
							break;
						}
					}
				}

				if(found)
					m_dragButton = Qt::MouseButton::LeftButton;

			}

			if (m_selectedPieces.size() == 0)
			{
				setSpriteEditor(nullptr);
				m_selector.setVisible(true);
				m_selector.beginSelection(pos.x(), pos.y(), 1.0, 1.0);
			}

			ui.graphicsView->redraw();
			updateItemSettings();
			return;
		}
		else if (event->buttons().testFlag(Qt::RightButton))
		{
			if (m_selectedPieces.size())
			{
				deselect();
				ui.graphicsView->redraw();
				return;
			}
		}
	}

	void AniEditor::graphicsMouseRelease(QMouseEvent* event)
	{
		auto pos = ui.graphicsView->mapToScene(event->pos());
		if (m_panning && event->button() == Qt::MouseButton::MiddleButton)
		{
			m_panning = false;
			ui.graphicsView->setCursor(Qt::CursorShape::ArrowCursor);
		}
		
		
		if (m_selector.visible())
		{

			m_selector.setVisible(false);

			m_selector.endSelection(pos.x(), pos.y());

			auto selectRect = m_selector.getSelection();
			auto frame = m_ani.getFrame((size_t)getFrame());

			int dir = getDir();
			if (frame != nullptr)
			{
				if (m_ani.isSingleDir()) { dir = 0; }
				dir = dir < 0 ? 0 : dir;
				dir = dir > 3 ? 3 : dir;

				QVector<Ani::Frame::FramePiece*>* sources[] = {
					&frame->sounds,
					&frame->pieces[dir]
				};

				bool found = false;
				for (size_t i = 0; i < sizeof(sources) / sizeof(sources[0]); ++i)
				{
					auto pieces = sources[i];

					for (auto it = pieces->rbegin(); it != pieces->rend(); ++it)
					{
						auto& piece = *it;

						auto boundingBox = piece->getBoundingBox(&m_ani);
						boundingBox.translate(piece->xoffset, piece->yoffset);

						if (boundingBox.intersects(selectRect))
						{
							m_selectedPieces.insert(piece);
							piece->dragOffset = QPointF(pos.x() - piece->xoffset, pos.y() - piece->yoffset);
						}
					}
				}
			}

			if (m_selectedPieces.size())
				updateItemSettings();

			m_dragButton = Qt::MouseButton::LeftButton;
			ui.graphicsView->redraw();
		}
	}

	void AniEditor::graphicsMouseMove(QMouseEvent* event)
	{
		auto pos = ui.graphicsView->mapToScene(event->pos());


		if (m_panning && event->buttons().testFlag(Qt::MouseButton::MiddleButton))
		{
			auto delta = pos - m_mousePanStart;
			ui.graphicsView->translate(delta.x(), delta.y());
		}
		if (m_insertPiece)
		{
			m_insertPiece->xoffset = qFloor(0.5 + pos.x() - m_insertPiece->dragOffset.x());
			m_insertPiece->yoffset = qFloor(0.5 + pos.y() - m_insertPiece->dragOffset.y());
			ui.graphicsView->redraw();
			return;
		}
		else if (m_selector.visible())
		{
			m_selector.updateSelection(pos.x(), pos.y());
			ui.graphicsView->redraw();
		}
		else 
		if (event->buttons().testFlag(m_dragButton))
		{
			if (m_selectedPieces.size())
			{
				QMap<Ani::Frame::FramePiece*, QPointF> newPositions;
				QMap<Ani::Frame::FramePiece*, QPointF> oldPositions;
				for (auto piece : m_selectedPieces)
				{
					oldPositions[piece] = QPointF(piece->xoffset, piece->yoffset);
					newPositions[piece] = QPointF(qFloor(0.5 + pos.x() - piece->dragOffset.x()), qFloor(0.5 + pos.y() - piece->dragOffset.y()));
				}

				addUndoCommand(new UndoCommandMoveFramePieces(this, getFrame(), getDir(), true, newPositions, oldPositions, nullptr));
				ui.graphicsView->redraw();

				updateItemSettings();
			}
		}
	}

	void AniEditor::graphicsMouseWheel(QWheelEvent* event)
	{
		if (m_panning)
			return;

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

	void AniEditor::graphicsKeyPress(QKeyEvent* event)
	{
		if (event->key() == Qt::Key::Key_V && (QApplication::keyboardModifiers() & Qt::ControlModifier))
		{
			auto text = QApplication::clipboard()->text();

			QByteArray ba = text.toLocal8Bit();

			auto json = cJSON_Parse(ba.data());
			if (json != nullptr)
			{
				auto type = jsonGetChildString(json, "type");
				if (type == "framePieces")
				{
					auto pieces = cJSON_GetObjectItem(json, "pieces");
					if (pieces && pieces->type == cJSON_Array)
					{
						auto frame = m_ani.getFrame(getFrame());
						if (frame)
						{
							QList<Ani::Frame::FramePiece*> newPieces;
							auto undoCommand = new QUndoCommand(nullptr);
							new UndoCommandSetFrameAndDir(this, getFrame(), getDir(), getFrame(), getDir(), undoCommand);
							for (int i = 0; i < cJSON_GetArraySize(pieces); ++i)
							{
								auto piece = cJSON_GetArrayItem(pieces, i);
								auto pieceType = jsonGetChildString(piece, "type");
								if (pieceType == "sprite")
								{
									auto spritePiece = new Ani::Frame::FramePieceSprite();
									spritePiece->deserialize(piece, m_resourceManager);

									new UndoCommandNewPiece(this, frame, getDir(), spritePiece, undoCommand);

									newPieces.push_back(spritePiece);
								}
								else if (pieceType == "sound")
								{
									auto soundPiece = new Ani::Frame::FramePieceSound();
									soundPiece->deserialize(piece, m_resourceManager);
									new UndoCommandNewPiece(this, frame, getDir(), soundPiece, undoCommand);
									newPieces.push_back(soundPiece);
								}
							}
							addUndoCommand(undoCommand);
							m_selectedPieces.clear();
							for (auto piece : newPieces)
								m_selectedPieces.insert(piece);
							ui.graphicsView->redraw();
						}
					}
				}
				cJSON_Delete(json);
			}

		}


		if (m_selectedPieces.size())
		{
			if ((event->key() == Qt::Key::Key_C || event->key() == Qt::Key_X) && (QApplication::keyboardModifiers() & Qt::ControlModifier))
			{
				auto frame = m_ani.getFrame(getFrame());

				if (frame)
				{
					auto& framePieces = frame->pieces[getDir()];

					auto json = cJSON_CreateObject();
					cJSON_AddStringToObject(json, "type", "framePieces");
					auto pieces = cJSON_CreateArray();

					for (auto piece : m_selectedPieces)
						piece->index = framePieces.indexOf(piece);

					static auto sortByIndex = [](const Ani::Frame::FramePiece* v1, const Ani::Frame::FramePiece* v2)
					{
						return v1->index < v2->index;
					};

					QList<Ani::Frame::FramePiece*> sortedPieces(m_selectedPieces.begin(), m_selectedPieces.end());
					std::sort(sortedPieces.begin(), sortedPieces.end(), sortByIndex);

					for(auto piece: sortedPieces)
						cJSON_AddItemToArray(pieces, piece->serialize());

					cJSON_AddItemToObject(json, "pieces", pieces);

					auto buffer = cJSON_Print(json);
					QApplication::clipboard()->setText(QString(buffer));
					free(buffer);
					cJSON_Delete(json);

					if (event->key() == Qt::Key_X)
					{
						deleteSelectedFramePieces();
						ui.graphicsView->redraw();
					}
				}
				
			}
			
			
		}

		if (event->key() == Qt::Key::Key_Delete)
		{
			deleteSelectedFramePieces();
			ui.graphicsView->redraw();
		}
	}


	AniEditor::~AniEditor()
	{
		m_undoStack.clear();

		m_ani.freeResources(m_resourceManager);
		//m_resourceManager->getFileSystem()->removeListener(this);
		m_resourceManager->decrementAndDelete();

	}

	int AniEditor::getDir() const {
		if (m_ani.isSingleDir())
			return 0;
		return ui.directionComboBox->currentIndex();
	}

	void AniEditor::renderScene(QPainter* painter, const QRectF& viewRect)
	{
		painter->fillRect(viewRect, m_backgroundColor);

		QPen oldPen = painter->pen();
		QPen pen;
		pen.setWidth(1);
		pen.setStyle(Qt::PenStyle::DotLine);

		pen.setColor(QColor(255, 255, 255, 60));

		painter->setPen(pen);


		painter->drawLine(QPointF(0.5, -4999.5), QPointF(0.5, 10000.0));
		painter->drawLine(QPointF( -4999.5, 0.5), QPointF(10000, 0.5));


		painter->drawLine(QPointF(0.5 + 48, 1.5), QPointF(0.5 + 48, 0.5 + 48 ));
		painter->drawLine(QPointF(0.5 + 1, 0.5 + 48), QPointF(0.5 + 48.0, 0.5 + 48));
		painter->setPen(oldPen);

		if (onionSkinEnabled() && getFrame() > 0)
		{
			auto compMode = painter->compositionMode();
			//painter->setCompositionMode((QPainter::CompositionMode)m_compMode);
			painter->setOpacity(0.25);
			m_ani.draw(getFrame() - 1, getDir(), 0, 0, m_resourceManager, painter, viewRect);
			painter->setOpacity(1.0);
			painter->setCompositionMode(compMode);
		}

		auto p = painter->compositionMode();
		painter->setCompositionMode((QPainter::CompositionMode)m_compMode);
		m_ani.draw(getFrame(), getDir(), 0.0, 0.0, m_resourceManager, painter, viewRect);
		painter->setCompositionMode(p);
		if (m_insertPiece && ui.graphicsView->underMouse())
		{
			if (m_insertPiece->type == Ani::Frame::PIECE_SPRITE)
			{
				auto piece = static_cast<Ani::Frame::FramePieceSprite*>(m_insertPiece);

				auto sprite = m_ani.getAniSprite(piece->spriteIndex, piece->spriteName);
				if (sprite)
				{
					m_ani.drawSprite(sprite, piece->xoffset, piece->yoffset, m_resourceManager, painter, viewRect, 0);
				}
			}
		}
		auto frame = m_ani.getFrame(getFrame());
		if (frame)
		{
			for (auto soundPiece : frame->sounds)
			{
				painter->drawPixmap(qRound(soundPiece->xoffset), qRound(soundPiece->yoffset), m_speakerPix);
			}

		}

		if (m_selectedPieces.size())
		{
			auto compositionMode = painter->compositionMode();
			painter->setCompositionMode(QPainter::CompositionMode_Difference);
			
			QPen newPen(QColorConstants::White, 1);
			newPen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
			painter->setPen(newPen);
			for (auto piece : m_selectedPieces)
			{
				auto boundingBox = piece->getBoundingBox(&m_ani);

	
				boundingBox.translate(qRound(piece->xoffset) - 0.5, qRound(piece->yoffset) - 0.5);

				painter->drawPolygon(boundingBox);


			}
			painter->setPen(oldPen);
			painter->setCompositionMode(compositionMode);
			
		}

		if (m_selector.visible())
		{
			m_selector.draw(painter, viewRect, QColorConstants::White, QColor(255, 255, 255, 0));
		}
	}

	void AniEditor::renderTimeline(QPainter* painter, const QRectF& rect)
	{
		painter->setRenderHint(QPainter::Antialiasing);

		int totalTime = 0;
		static const int headerHeight = 20;


		QFontMetrics fm(m_fontTimeline);

		painter->setFont(m_fontTimeline);

		static int buttonHeight = 96;
		double x = 0.0;
		QPen pen(Qt::black, 1);
		painter->setPen(pen);

		if (rect.bottom() > headerHeight)
		{

			for (size_t i = 0; i < m_ani.getFrameCount(); ++i)
			{

				auto frame = m_ani.getFrame(i);

				int width = frame->duration;
				int right = x + width;

				if (right > rect.x() || x < rect.right())
				{
					bool selected = i == getFrame();
					QPainterPath path;
					path.addRoundedRect(QRectF(2 + x + 0.5, headerHeight + 0.5, width - 4, buttonHeight), 10, 10);
					QPen pen(Qt::black, 1);
					painter->setPen(pen);
					painter->fillPath(path, selected ? Qt::darkGreen : Qt::red);
					painter->drawPath(path);

					painter->setClipping(true);
					painter->setClipRect(QRect(2 + x + 0.5, headerHeight + 14, width - 4, buttonHeight - 20));
					m_ani.draw(i, getDir(), 2 + x + 0.5 + width / 2 - 24, headerHeight + 20, m_resourceManager, painter, rect);
					painter->setClipping(false);

					QString text = QString("%1").arg(i);
					painter->drawText(x + 0.5 + width / 2 - (fm.horizontalAdvance(text) / 2), headerHeight + 12, text);

					text = QString("%1 ms").arg(frame->duration);
					painter->drawText(x + 0.5 + width / 2 - (fm.horizontalAdvance(text) / 2), headerHeight + buttonHeight - 4, text);


					if (frame->sounds.size())
					{
						painter->drawPixmap(x + 0.5 + width / 2 - m_speakerPix.width() / 2, headerHeight + buttonHeight - 32, m_speakerPix);
					}

					if (selected)
					{
						if (ui.graphicsViewTimeline->hasFocus())
						{
							QPen pen(Qt::black, 1);
							painter->setPen(pen);
							painter->drawRect(QRectF(x, headerHeight + 0.5, width, buttonHeight));
						}
					}
				}

				x = right;
				totalTime += frame->duration;
			}
		}
		


		x = qFloor(rect.x() / 50) * 50;
		painter->fillRect(rect.x(), 0, rect.width(), headerHeight, QColorConstants::Gray);
		for (int time = x; time <= rect.right() + 100; time += 50)
		{
			QString timeStr = QString::number(time / 1000.0, 103, 3);
			int fontWidth = fm.horizontalAdvance(timeStr);



			painter->drawLine(x, 11, x, headerHeight - 1);

			painter->drawText( x - fontWidth / 2, 10, timeStr);

			x += 50;
		}

		if (m_playing)
		{
			auto pos = qMin(m_playingPosition, totalTime);

			QRectF rect = QRectF(pos - 5, 11, 10, 10);

			QPainterPath path;
			path.moveTo(rect.left() + (rect.width() / 2), rect.bottom());
			path.lineTo(rect.topLeft());
			path.lineTo(rect.topRight());
			path.lineTo(rect.left() + (rect.width() / 2), rect.bottom());

			painter->fillPath(path, QBrush(QColor("blue")));

			//painter->drawLine(pos, 0, pos, headerHeight);
		}


		if (totalTime >= ui.graphicsViewTimeline->sceneRect().width() || totalTime < ui.graphicsViewTimeline->sceneRect().width() + 5000)
			ui.graphicsViewTimeline->setSceneRect(QRect(-2, 0, totalTime + 5000, 116));

	}

	void AniEditor::timelineMouseWheel(QWheelEvent* event)
	{
		if (event->angleDelta().y() < 0)
		{
			ui.timelineSlider->setValue(ui.timelineSlider->value() - 1);
		}
		else {
			ui.timelineSlider->setValue(ui.timelineSlider->value() + 1);

		}
	}

	void AniEditor::timelineMouseRelease(QMouseEvent* event)
	{
		m_draggingFrame = false;
		ui.graphicsViewTimeline->setCursor(Qt::CursorShape::ArrowCursor);
		if (m_resizeFrame >= 0 && event->button() == Qt::MouseButton::LeftButton)
		{

			m_resizeFrame = m_desiredResizeFrame  =-1;
			
		}
	}

	void AniEditor::timelineMousePress(QMouseEvent* event)
	{
		if (m_playing)
			stop();

		auto pos = ui.graphicsViewTimeline->mapToScene(event->pos());

		if (pos.y() < 20)
			return;

		if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
		{
			if (m_desiredResizeFrame >= 0)
			{
				m_resizeFrame = m_desiredResizeFrame;
			}
		}
		double x = 0.0;
		for (size_t i = 0; i < m_ani.getFrameCount(); ++i)
		{
			auto frame = m_ani.getFrame(i);

			int width = frame->duration;

			if (pos.x() > x + 2 && pos.x() < x + width - 4)
			{
				setFrame(i);
				ui.graphicsView->redraw();
				ui.graphicsViewTimeline->redraw();
				m_draggingFrame = true;

				ui.graphicsViewTimeline->setCursor(Qt::CursorShape::ClosedHandCursor);
				return;

			}

			x += width;
		}

	}

	void AniEditor::timelineMouseMove(QMouseEvent* event)
	{
		auto pos = ui.graphicsViewTimeline->mapToScene(event->pos());

		if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
		{
			if (m_draggingFrame)
			{
				double x = 0.0;
				for (size_t i = 0; i < m_ani.getFrameCount(); ++i)
				{
					auto frame = m_ani.getFrame(i);

					int width = frame->duration;

					if (i != m_frame)
					{
						if ((m_frame > i && pos.x() < x + 15) || (m_frame < i && pos.x() > x + width - 15))
						{
							addUndoCommand(new UndoCommandMoveFrame(this, m_frame, i, nullptr));
							m_frame = i;
							const QSignalBlocker blocker(ui.timelineSlider);
							ui.timelineSlider->setValue(m_frame);
							ui.graphicsViewTimeline->redraw();


							return;
						}

					}

					x += width;
				}
				return;
			}
			if (m_resizeFrame >= 0)
			{
				auto frame = m_ani.getFrame(m_resizeFrame);
				if (frame != nullptr)
				{
					double newDurationUnfiltered = m_resizeStartDuration + (pos.x() - m_resizeOffset);


					auto newDuration = int(qRound(newDurationUnfiltered / 50) * 50);
					newDuration = qMax(newDuration, 50);

					if (frame->duration != newDuration)
					{
						addUndoCommand(new UndoCommandSetFrameProperty(this, frame, getDir(), "duration", newDuration, frame->duration, nullptr));
						this->setModified();
					}
						

					ui.graphicsViewTimeline->redraw();
					
					if (m_resizeFrame == getFrame()) {
						const QSignalBlocker blocker(ui.durationWidget);
						ui.durationWidget->setValue(frame->duration);
					}

					return;
				}
			}
		}
		else {
			double x = 0.0;
			for (size_t i = 0; i < m_ani.getFrameCount(); ++i)
			{
				auto frame = m_ani.getFrame(i);

				int width = frame->duration;
				int right = x + width - 2;

				if (pos.x() >= right && pos.x() <= right + 4)
				{
					ui.graphicsViewTimeline->setCursor(Qt::SizeHorCursor);
					m_desiredResizeFrame = int(i);
					m_resizeOffset = pos.x();
					m_resizeStartDuration = frame->duration;
					return;

				}

				x += width;
			}
		}
		m_desiredResizeFrame = -1;
		ui.graphicsViewTimeline->setCursor(Qt::CursorShape::ArrowCursor);
	}

	void AniEditor::timelineKeyPress(QKeyEvent* event)
	{
		if (event->key() == Qt::Key::Key_Delete)
		{
			deleteSelectedFrame();
		}
		else if (event->key() == Qt::Key_C && QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier))
		{
			ui.copyFrameButton->click();
		}
		else if (event->key() == Qt::Key_V && QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier))
		{
			ui.pasteAfterButton->click();
		}
	}

	void AniEditor::renderSpritePreview(QPainter* painter, const QRectF& rect)
	{
		painter->fillRect(rect, m_backgroundColor);
		if (m_editingSprite != nullptr)
		{

			m_ani.drawSprite(m_editingSprite, 0.0, 0.0, m_resourceManager, painter, rect, 0);

			if (m_selectedAttachedSprite >= 0 && m_selectedAttachedSprite < m_editingSprite->attachedSprites.size())
			{
				auto& attachmentPair = m_editingSprite->attachedSprites[m_selectedAttachedSprite];
				auto attachedSprite = m_ani.getAniSprite(attachmentPair.first, "");
				if (attachedSprite)
				{
					auto boundingBox = attachedSprite->boundingBox;

					auto& attachmentOffset = attachmentPair.second;
					boundingBox.translate(attachmentOffset.x() - 0.5, attachmentOffset.y() - 0.5);

					auto compositionMode = painter->compositionMode();
					painter->setCompositionMode(QPainter::CompositionMode_Difference);

					auto oldPen = painter->pen();
					QPen newPen(QColorConstants::White, 1);
					newPen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
					painter->setPen(newPen);
					painter->drawPolygon(boundingBox);
					painter->setPen(oldPen);
					painter->setCompositionMode(compositionMode);
				}
			}

			if (m_insertPiece && ui.graphicsViewSprite->underMouse())
			{
				if (m_insertPiece->type == Ani::Frame::PIECE_SPRITE)
				{
					auto piece = static_cast<Ani::Frame::FramePieceSprite*>(m_insertPiece);

					auto sprite = m_ani.getAniSprite(piece->spriteIndex, piece->spriteName);
					if (sprite)
					{
						m_ani.drawSprite(sprite, piece->xoffset, piece->yoffset, m_resourceManager, painter, rect, 0);
					}
				}
			}


			
		}
		
	}

	void AniEditor::spritePreviewMouseWheel(QWheelEvent* event)
	{
		

		if (QGuiApplication::keyboardModifiers().testFlag(Qt::KeyboardModifier::ControlModifier))
		{
			int newLevel = m_spritePreviewZoomLevel;
			if (event->angleDelta().y() < 0)
			{
				newLevel = qMax(0, m_spritePreviewZoomLevel - 1);
			}
			else {
				newLevel = qMin(7, m_spritePreviewZoomLevel + 1);
			}

			setSpritePreviewZoomLevel(newLevel);


		}
		else event->ignore();
	}

	void AniEditor::spritePreviewMousePress(QMouseEvent* event)
	{
		auto pos = ui.graphicsViewSprite->mapToScene(ui.graphicsViewSprite->mapFromGlobal(QCursor::pos()));

		if (event->buttons().testFlag(Qt::MouseButton::RightButton) && m_insertPiece)
		{
			delete m_insertPiece;
			m_insertPiece = nullptr;
			ui.graphicsViewSprite->redraw();
		}
		if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
		{
			if (m_editingSprite)
			{
				if (m_insertPiece)
				{

					if (m_insertPiece->type == Ani::Frame::PIECE_SPRITE)
					{
						this->setModified();
						auto spritePiece = static_cast<Ani::Frame::FramePieceSprite*>(m_insertPiece);

						addUndoCommand(new UndoCommandAddSpriteAttachment(this, m_editingSprite, spritePiece->spriteIndex, QPointF(m_insertPiece->xoffset, m_insertPiece->yoffset), nullptr));
					}


					delete m_insertPiece;
					m_selectedAttachedSprite = -1;
					m_insertPiece = nullptr;
					ui.graphicsViewSprite->redraw();
					ui.graphicsView->redraw();
					return;
				}

				//If we are selecting the attachment that is already selected, do nothing
				if (m_selectedAttachedSprite >= 0 && m_selectedAttachedSprite < m_editingSprite->attachedSprites.size())
				{
					auto& attachmentPair = m_editingSprite->attachedSprites[m_selectedAttachedSprite];
					auto attachmentSpriteIndex = attachmentPair.first;
					auto attachmentSprite = m_ani.getAniSprite(attachmentSpriteIndex, "");
					auto& attachmentOffset = attachmentPair.second;

					if (attachmentSprite)
					{
						auto boundingBox = attachmentSprite->boundingBox;
						boundingBox.translate(attachmentOffset.x(), attachmentOffset.y());

						if (boundingBox.intersects(QRectF(pos.x(), pos.y(), 1, 1)))
						{
							m_attachedSpriteStartMove = attachmentOffset;
							attachmentSprite->dragOffset = QPointF(pos.x() - attachmentOffset.x(), pos.y() - attachmentOffset.y());
							return;
						}
					}

				}
				

				if (m_editingSprite->attachedSprites.size())
				{
					for (qsizetype i = m_editingSprite->attachedSprites.size() - 1; i >= 0; --i)
					{
						auto& attachmentPair = m_editingSprite->attachedSprites[i];
						auto attachmentSpriteIndex = attachmentPair.first;
						auto attachmentSprite = m_ani.getAniSprite(attachmentSpriteIndex, "");
						auto& attachmentOffset = attachmentPair.second;

						if (attachmentSprite)
						{
							auto boundingBox = attachmentSprite->boundingBox;
							boundingBox.translate(attachmentOffset.x(), attachmentOffset.y());

							if (boundingBox.intersects(QRectF(pos.x(), pos.y(), 1, 1)))
							{
								m_selectedAttachedSprite = i;
								m_attachedSpriteStartMove = attachmentOffset;
								attachmentSprite->dragOffset = QPointF(pos.x() - attachmentOffset.x(), pos.y() - attachmentOffset.y());
								ui.graphicsViewSprite->redraw();
								return;
							}

						}
					}
				}
			}
			m_selectedAttachedSprite = -1;

			ui.graphicsViewSprite->redraw();
		}
	}



	void AniEditor::spritePreviewMouseMove(QMouseEvent* event)
	{
		auto pos = ui.graphicsViewSprite->mapToScene(ui.graphicsViewSprite->mapFromGlobal(QCursor::pos()));

		if (m_insertPiece)
		{
			m_insertPiece->xoffset = pos.x() - m_insertPiece->dragOffset.x();
			m_insertPiece->yoffset = pos.y() - m_insertPiece->dragOffset.y();
			ui.graphicsViewSprite->redraw();
			return;
		}
		if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
		{
			if (m_editingSprite && m_selectedAttachedSprite >= 0 && m_selectedAttachedSprite < m_editingSprite->attachedSprites.size())
			{
				auto& attachmentPair = m_editingSprite->attachedSprites[m_selectedAttachedSprite];

				auto attachmentSprite = m_ani.getAniSprite(attachmentPair.first, "");
				auto& attachmentOffset = attachmentPair.second;
				if (attachmentSprite)
				{
					this->setModified();
					QPointF offsets = QPointF(qFloor(0.5 + pos.x() - attachmentSprite->dragOffset.x()), qFloor(0.5 + pos.y() - attachmentSprite->dragOffset.y()));

					addUndoCommand(new UndoCommandMoveSpriteAttachment(this, m_editingSprite, m_selectedAttachedSprite, attachmentOffset, offsets, nullptr));


					//attachmentOffset = QPointF(qFloor(0.5 + offsets.x()), qFloor(0.5 + offsets.y()));
					ui.graphicsViewSprite->redraw();
					ui.graphicsView->redraw();
				}

			}
		}

	}

	void AniEditor::spritePreviewMouseRelease(QMouseEvent* event)
	{
		if (event->button() == Qt::MouseButton::LeftButton)
		{

		}
	}

	void AniEditor::spritePreviewKeyPress(QKeyEvent* event)
	{
		if (m_editingSprite)
		{
			if (event->key() == Qt::Key::Key_Delete)
			{
				if (m_selectedAttachedSprite >= 0 && m_selectedAttachedSprite < m_editingSprite->attachedSprites.size())
				{
					addUndoCommand(new UndoCommandDeleteSpriteAttachment(this, m_editingSprite, m_selectedAttachedSprite, nullptr));
					ui.graphicsViewSprite->redraw();
					ui.graphicsView->redraw();
				}
			}
		}
	}

	void AniEditor::directionIndexChanged(int index)
	{
		updateFrame();
		ui.graphicsView->redraw();
		ui.graphicsViewTimeline->redraw();
	}

	void AniEditor::defaultValuesCellChanged(int row, int column)
	{
		auto propertyName = ui.defaultsTable->verticalHeaderItem(row)->text().toUpper();
		auto value = ui.defaultsTable->item(row, 0)->text();

		this->setModified();

		addUndoCommand(new UndoCommandSetDefaultValue(this, propertyName, value, m_ani.getDefaultImageName(propertyName), nullptr));
		//setDefaultValue(propertyName, value, false);

	}

	void AniEditor::durationChanged(int i)
	{
		double x = i;
		x = qMax(50.0, (double)(qFloor(x / 50) * 50));

		auto frame = m_ani.getFrame(m_frame);
		if (frame)
		{
			addUndoCommand(new UndoCommandSetFrameProperty(this, frame, getDir(), "duration", int(x), frame->duration, nullptr));
			ui.graphicsViewTimeline->redraw();
		}
	}

	void AniEditor::itemChanged(int index)
	{
		deselect();
		if (index >= 0)
		{
			int type = ui.itemsComboWidget->itemData(index, Qt::UserRole).toInt();

			if (type == int(Ani::Frame::PIECE_SPRITE))
			{
				quint64 id = ui.itemsComboWidget->itemData(index, Qt::UserRole + 1).toULongLong();

				auto frame = m_ani.getFrame(m_frame);
				if (frame)
				{
					auto& pieces = frame->pieces[getDir()];
					for (auto& piece : pieces)
					{
						if (piece->id == id)
						{
							m_selectedPieces.insert(piece);

							ui.graphicsView->setFocus();
							break;
						}
					}
				}
			}
		}
		updateItemSettings();
		ui.graphicsView->redraw();
	}

	void AniEditor::playStopClicked(bool checked)
	{
		if (sender() == ui.stopButton && m_playing)
		{
			stop();

		}
		else if(sender() == ui.playButton && !m_playing)
		{
			//ui.playStopButton->setIcon(QIcon(":/MainWindow/icons/tinycolor/icons8-stop-16.png"));
			m_playing = true;
			m_playingPosition = m_playFramePosition = 0;
			setFrame(0);
			ui.graphicsView->redraw();
			ui.graphicsViewTimeline->redraw();
			m_playDelta.start();

			QTimer::singleShot(20, this, &AniEditor::playTimer);
		}
	}

	void AniEditor::controlsClicked(bool checked)
	{
		bool redraw = false;

		if (m_selectedPieces.size())
		{
			if (sender() == ui.itemLeftButton || sender() == ui.itemRightButton || sender() == ui.itemDownButton || sender() == ui.itemUpButton)
			{
				QMap<Ani::Frame::FramePiece*, QPointF> newPositions;
				QMap<Ani::Frame::FramePiece*, QPointF> oldPositions;
				this->setModified();
				redraw = true;

				for (auto piece : m_selectedPieces)
					oldPositions[piece] = newPositions[piece] = QPointF(piece->xoffset, piece->yoffset);
				
				if (sender() == ui.itemLeftButton)
				{
					for (auto piece : m_selectedPieces)
						newPositions[piece].setX(piece->xoffset - 1);
				}
				else if (sender() == ui.itemRightButton)
				{
					for (auto piece : m_selectedPieces)
						newPositions[piece].setX(piece->xoffset + 1);
				}
				else if (sender() == ui.itemDownButton)
				{
					for (auto piece : m_selectedPieces)
						newPositions[piece].setY(piece->yoffset + 1);
				}
				else if (sender() == ui.itemUpButton)
				{
					for (auto piece : m_selectedPieces)
						newPositions[piece].setY(piece->yoffset - 1);
				}
				addUndoCommand(new UndoCommandMoveFramePieces(this, getFrame(), getDir(), true, newPositions, oldPositions, nullptr));
				updateItemSettings();

			}

			else if (sender() == ui.itemBackButton || sender() == ui.itemForwardButton)
			{
				auto frame = m_ani.getFrame(getFrame());
				if (frame != nullptr)
				{
					auto undoCommand = new QUndoCommand();
					auto& framePieces = frame->pieces[getDir()];
					for (auto piece : m_selectedPieces)
					{
						if (piece->type == Ani::Frame::PIECE_SPRITE)
						{
							auto index = framePieces.indexOf(piece);
							if (index >= 0)
							{
								if (sender() == ui.itemBackButton && index > 0)
								{
									new UndoCommandSetFramePieceIndex(this, getFrame(), getDir(), piece, index - 1, index, undoCommand);
									redraw = true;
								}
								else if (sender() == ui.itemForwardButton && index < framePieces.size() - 1)
								{
									new UndoCommandSetFramePieceIndex(this, getFrame(), getDir(), piece, index + 1, index, undoCommand);
									redraw = true;
								}
							}
						}
					}

					if (undoCommand->childCount() > 0)
					{
						new UndoCommandPopulateItems(this, undoCommand);
						addUndoCommand(undoCommand);
					}
					else delete undoCommand;
				}
				redraw = true;
			}
		}

		if (redraw)
			ui.graphicsView->redraw();
	}

	void AniEditor::editScriptClicked(bool checked)
	{
		EditScriptForm form(m_ani.getScript());
		form.showTestControls(false);

		if (form.exec())
		{
			addUndoCommand(new UndoCommandSetAniProperty(this, "script", form.getScript(), m_ani.getScript(), nullptr));
		}


	}

	void AniEditor::bottomControlsClicked(bool checked)
	{
		stop();
		if (sender() == ui.deleteFrameButton)
		{
			deleteSelectedFrame();
		}
		else if (sender() == ui.newFrameButton)
		{
			auto newFrame = new Ani::Frame;
			newFrame->duration = 50;

			auto newIndex = m_ani.getFrameCount();
			//m_ani.insertFrame(newIndex, newFrame);

			auto undoCommand = new UndoCommandInsertFrame(this, newFrame, newIndex, nullptr);
			addUndoCommand(undoCommand);
			setFrame(newIndex);
			ui.graphicsView->redraw();
			ui.graphicsViewTimeline->redraw();
		}
		else if (sender() == ui.copyFrameButton)
		{
			auto frame = m_ani.getFrame(getFrame());
			if (frame)
			{
				auto jsonObject = cJSON_CreateObject();

				cJSON_AddStringToObject(jsonObject, "type", "frame");
				cJSON_AddNumberToObject(jsonObject, "duration", frame->duration);
				 
				auto jsonPiecesAll = cJSON_CreateArray();
				for (int dir = 0; dir < (m_ani.isSingleDir() ? 1 : 4); ++dir)
				{
					auto& framePieces = frame->pieces[dir];

					auto pieces = cJSON_CreateArray();

					for (qsizetype i = 0; i < framePieces.size(); ++i)
						framePieces[i]->index = i;

					static auto sortByIndex = [](const Ani::Frame::FramePiece* v1, const Ani::Frame::FramePiece* v2)
						{
							return v1->index < v2->index;
						};

					QList<Ani::Frame::FramePiece*> sortedPieces(framePieces.begin(), framePieces.end());
					std::sort(sortedPieces.begin(), sortedPieces.end(), sortByIndex);

					for (auto piece : sortedPieces)
						cJSON_AddItemToArray(pieces, piece->serialize());

				
					cJSON_AddItemToArray(jsonPiecesAll, pieces);
				}

				cJSON_AddItemToObject(jsonObject, "pieces", jsonPiecesAll);

				auto jsonSounds = cJSON_CreateArray();
				for(auto sound: frame->sounds)
					cJSON_AddItemToArray(jsonSounds, sound->serialize());

				cJSON_AddItemToObject(jsonObject, "sounds", jsonSounds);

				auto buffer = cJSON_Print(jsonObject);
				QApplication::clipboard()->setText(QString(buffer));
				free(buffer);
				cJSON_Delete(jsonObject);

				ui.pasteAfterButton->setEnabled(true);
				ui.pasteBeforeButton->setEnabled(true);
			}
		}
		else if (sender() == ui.pasteAfterButton || sender() == ui.pasteBeforeButton)
		{
			auto text = QApplication::clipboard()->text();

			QByteArray ba = text.toLocal8Bit();

			auto json = cJSON_Parse(ba.data());
			if (json)
			{ 
				auto type = jsonGetChildString(json, "type");

				if (type == "frame")
				{
					auto frame = new Ani::Frame();
					frame->duration = jsonGetChildInt(json, "duration");

					auto pieces = cJSON_GetObjectItem(json, "pieces");
					if (pieces && pieces->type == cJSON_Array)
					{
						for (int dir = 0; dir < (m_ani.isSingleDir() ? 1 : 4) && dir < cJSON_GetArraySize(pieces); ++dir)
						{
							auto piecesDir = cJSON_GetArrayItem(pieces, dir);
							if (piecesDir && piecesDir->type == cJSON_Array)
							{
								for (int i = 0; i < cJSON_GetArraySize(piecesDir); ++i)
								{
									auto jsonPiece = cJSON_GetArrayItem(piecesDir, i);
									if (jsonPiece && jsonPiece->type == cJSON_Object)
									{
										if (jsonGetChildString(jsonPiece, "type") == "sprite")
										{
											auto piece = new Ani::Frame::FramePieceSprite();
											piece->deserialize(jsonPiece, m_resourceManager);

											frame->pieces[dir].push_back(piece);
										}
										
									}
								}
							}
						}
					}

					auto jsonSounds = cJSON_GetObjectItem(json, "sounds");
					if (jsonSounds && jsonSounds->type == cJSON_Array)
					{
						for (int i = 0; i < cJSON_GetArraySize(jsonSounds); ++i)
						{
							auto jsonSound = cJSON_GetArrayItem(jsonSounds, i);
							if (jsonSound && jsonSound->type == cJSON_Object)
							{
								if (jsonGetChildString(jsonSound, "type") == "sound")
								{
									auto sound = new Ani::Frame::FramePieceSound();
									sound->deserialize(jsonSound, m_resourceManager);

									frame->sounds.push_back(sound);
								}
							}
						}
					}
					
					auto newIndex = sender() == ui.pasteAfterButton ? getFrame() + 1 : getFrame();
					newIndex = qMin(newIndex, int(m_ani.getFrameCount()));

					addUndoCommand(new UndoCommandInsertFrame(this, frame, newIndex, nullptr));
					setFrame(newIndex);
					ui.graphicsView->redraw();
					ui.graphicsViewTimeline->redraw();
				}
				cJSON_Delete(json);
			}
		}
		else if (sender() == ui.reverseFrameButtons)
		{
			auto oldSelectedFrameIndex = getFrame();

			auto undoCommand = new QUndoCommand(nullptr);

			int count = m_ani.getFrameCount();
			for (int i = 0; i < count; ++i)
			{
				new UndoCommandMoveFrame(this, 0, count-i-1, undoCommand);
			}


			new UndoCommandSetFrameAndDir(this, count - oldSelectedFrameIndex - 1, getDir(), oldSelectedFrameIndex, getDir(), undoCommand);
			
			addUndoCommand(undoCommand);
			ui.graphicsViewTimeline->redraw();
		}
	}

	void AniEditor::playTimer()
	{
		if (m_playing)
		{
			auto delta = m_playDelta.elapsed();
			m_playDelta.restart();

			m_playingPosition += delta;
			m_playFramePosition += delta;

			bool redrawTimeline = false;
			auto frame = m_ani.getFrame(getFrame());
			if (frame)
			{
				if (m_playFramePosition >= frame->duration)
				{
					m_playFramePosition = m_playFramePosition - frame->duration;

					auto nextFrame = getFrame() + 1;
					if (nextFrame < m_ani.getFrameCount())
					{
						setFrame(nextFrame);
						ui.graphicsView->redraw();
						
					}
					else {
						if (m_ani.isLooped())
						{
							setFrame(0);
							m_playingPosition = m_playFramePosition = 0;
							ui.graphicsView->redraw();
							redrawTimeline = true;
						}
						else {
							stop();
						}
					}
					redrawTimeline = true;
				}

				auto rect = ui.graphicsViewTimeline->mapToScene(ui.graphicsViewTimeline->viewport()->geometry()).boundingRect();

				if (m_playingPosition > rect.right() || m_playingPosition < rect.left())
				{
					redrawTimeline = true;
					ui.graphicsViewTimeline->ensureVisible(m_playingPosition, 0, 1, 0, rect.width() / 2);
				}
				redrawTimeline = true;
				if(redrawTimeline)
					ui.graphicsViewTimeline->redraw();

			}
		}

		if (m_playing)
		{
			
			QTimer::singleShot(20, this, &AniEditor::playTimer);
		}
	}

	void AniEditor::soundsWidgetItemTextChange(QWidget* pLineEdit)
	{
		auto frame = m_ani.getFrame(getFrame());

		if (frame)
		{
			auto currentItem = ui.soundsWidget->currentItem();
			if (currentItem != nullptr)
			{
				auto id = currentItem->data(Qt::UserRole).toULongLong();

				for (auto piece : frame->sounds)
				{
					if (piece->id == id)
					{
						auto soundPiece = static_cast<Ani::Frame::FramePieceSound*>(piece);

						addUndoCommand(new UndoCommandSetFramePieceProperty(this, soundPiece, "soundFile", currentItem->text(), soundPiece->fileName, nullptr));
						break;
					}
				}
			}
		}
	}

	void AniEditor::aniSettingsCheckChange()
	{
		if (sender() == ui.loopedCheckBox)
			addUndoCommand(new UndoCommandSetAniProperty(this, "loop", ui.loopedCheckBox->isChecked(), m_ani.isLooped(), nullptr));

		else if (sender() == ui.continousCheckBox)
			addUndoCommand(new UndoCommandSetAniProperty(this, "continous", ui.continousCheckBox->isChecked(), m_ani.isContinous(), nullptr));

		else if (sender() == ui.singleDirCheckBox)
		{
			addUndoCommand(new UndoCommandSetAniProperty(this, "singledir", ui.singleDirCheckBox->isChecked(), m_ani.isSingleDir(), nullptr));
			ui.graphicsView->redraw();
			ui.graphicsViewTimeline->redraw();
		}

	}

	void AniEditor::nextAniEditingFinished()
	{
		addUndoCommand(new UndoCommandSetAniProperty(this, "nextani", ui.nextAniLineEdit->text(), m_ani.getNextAni(), nullptr));
	}

	void AniEditor::itemSpriteIDEditingFinished()
	{
		if (m_selectedPieces.size())
		{
			auto piece = *m_selectedPieces.begin();

			if (piece->type == Ani::Frame::PIECE_SPRITE)
			{
				auto spritePiece = static_cast<Ani::Frame::FramePieceSprite*>(piece);
				addUndoCommand(new UndoCommandSetFramePieceProperty(this, spritePiece, "spriteIndex", ui.itemSpriteIDEdit->text(), spritePiece->spriteIndex == Ani::SPRITE_INDEX_STRING ? spritePiece->spriteName : QString::number(spritePiece->spriteIndex), nullptr));
			}
			
		}
		
	}

	void AniEditor::soundSelectionChange()
	{
		deselect();

		auto frame = m_ani.getFrame(getFrame());
		if (frame)
		{
			auto selectedItem = ui.soundsWidget->currentItem();
			if (selectedItem)
			{
				auto id = selectedItem->data(Qt::UserRole).toULongLong();

				for (auto soundPiece : frame->sounds)
				{
					if (soundPiece->id == id)
					{
						m_selectedPieces.insert(soundPiece);
						ui.graphicsView->redraw();
						return;
					}
				}
			}
		}
	}

	void AniEditor::spriteListItemClicked(QListWidgetItem* item)
	{
		if (m_selectedPieces.size() && m_dragButton == Qt::MouseButton::NoButton)
		{
			if (m_insertPiece)
			{
				delete m_insertPiece;
				m_insertPiece = nullptr;
			}
		}
		auto pos = ui.spritesListWidget->mapFromGlobal(QCursor::pos());
		auto rect = ui.spritesListWidget->visualItemRect(item);
		deselect();

		auto spriteID = item->data(Qt::UserRole).toInt();

		auto sprite = m_ani.getAniSprite(spriteID, "");
		if (sprite)
		{
			auto piece = new Ani::Frame::FramePieceSprite;
			piece->type = Ani::Frame::PIECE_SPRITE;
			piece->spriteIndex = sprite->index;
			piece->spriteName = QString::number(sprite->index);
			piece->xoffset = -5000;
			piece->yoffset = -5000;
			piece->dragOffset = QPointF((sprite->width * sprite->xscale) / 2, (sprite->height * sprite->yscale) / 2);
			m_insertPiece = piece;


			m_dragButton = Qt::MouseButton::NoButton;
		}

	}


	void AniEditor::spriteListItemDoubleClicked(QListWidgetItem* item)
	{
		if (m_insertPiece)
		{
			delete m_insertPiece;
			m_insertPiece = nullptr;
		}
		auto spriteID = item->data(Qt::UserRole).toInt();
		setSpriteEditor(m_ani.getAniSprite(spriteID, ""));
	}


	void AniEditor::setSpriteEditor(Ani::AniSprite* sprite)
	{
		m_selectedAttachedSprite = -1;
		m_editingSprite = sprite;
		
		const QSignalBlocker blocker1(ui.commentLineEdit);
		const QSignalBlocker blocker2(ui.rotationDoubleSpinBox);
		const QSignalBlocker blocker3(ui.rotationSlider);
		const QSignalBlocker blocker4(ui.xScaleSlider);
		const QSignalBlocker blocker5(ui.yScaleSlider);
		const QSignalBlocker blocker6(ui.xScaleDoubleSpinBox);
		const QSignalBlocker blocker7(ui.yScaleDoubleSpinBox);
		const QSignalBlocker blocker8(ui.colorEffectCheckBox);
		const QSignalBlocker blocker9(ui.spriteLeftSpinBox);
		const QSignalBlocker blocker10(ui.spriteTopSpinBox);
		const QSignalBlocker blocker11(ui.spriteWidthSpinBox);
		const QSignalBlocker blocker12(ui.spriteHeightSpinBox);
		const QSignalBlocker blocker13(ui.spriteSourceComboBox);
		const QSignalBlocker blocker14(ui.spriteImageLineEdit);

		if (sprite)
		{
			ui.spriteEditPanel->setEnabled(true);

			ui.spriteIDLineEdit->setText(QString::number(sprite->index));
			ui.commentLineEdit->setText(sprite->comment);
			ui.rotationDoubleSpinBox->setValue(sprite->rotation);
			ui.rotationSlider->setValue(sprite->rotation);
			ui.xScaleSlider->setValue(int(sprite->xscale * 10));
			ui.yScaleSlider->setValue(int(sprite->yscale * 10));
			ui.xScaleDoubleSpinBox->setValue(sprite->xscale);
			ui.yScaleDoubleSpinBox->setValue(sprite->yscale);

			ui.colorEffectCheckBox->setChecked(sprite->colorEffectEnabled);
			ui.changeColorEffectButton->setEnabled(sprite->colorEffectEnabled);
			ui.changeColorEffectButton->setStyleSheet(QString("background-color: %1").arg(sprite->colorEffect.name()));
			ui.changeColorEffectButton->update();
			
			ui.spriteSourceComboBox->setCurrentText(sprite->type);
			ui.spriteImageLineEdit->setText(sprite->customImageName);

			ui.spriteImageLineEdit->setEnabled(sprite->type == "CUSTOM");

			ui.spriteLeftSpinBox->setValue(sprite->left);
			ui.spriteTopSpinBox->setValue(sprite->top);
			ui.spriteWidthSpinBox->setValue(sprite->width);
			ui.spriteHeightSpinBox->setValue(sprite->height);
			ui.graphicsViewSprite->centerOn(16, 16);
		}
		else {

			ui.spriteEditPanel->setEnabled(false);
			ui.spriteIDLineEdit->setText("");
			ui.commentLineEdit->setText("");
			ui.spriteImageLineEdit->setText("");
			ui.rotationDoubleSpinBox->setValue(0.0);
			ui.rotationSlider->setValue(0.0);
			ui.xScaleSlider->setValue(10);
			ui.yScaleSlider->setValue(10);
			ui.xScaleDoubleSpinBox->setValue(1.0);
			ui.yScaleDoubleSpinBox->setValue(1.0);
			ui.spriteLeftSpinBox->setValue(0);
			ui.spriteTopSpinBox->setValue(0);
			ui.spriteWidthSpinBox->setValue(0);
			ui.spriteHeightSpinBox->setValue(0);
			ui.colorEffectCheckBox->setChecked(false);
			ui.changeColorEffectButton->setEnabled(false);
			ui.changeColorEffectButton->setStyleSheet(QString("background-color: %1").arg("black"));
		}

		ui.graphicsViewSprite->redraw();
	}

	void AniEditor::spritePropertyChanged()
	{
		if (m_editingSprite)
		{
			if (sender() == ui.yScaleDoubleSpinBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "xscale", ui.yScaleDoubleSpinBox->value(), m_editingSprite->yscale, nullptr));
			}
			else if (sender() == ui.yScaleSlider)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "yscale", ui.yScaleSlider->value() / 10.0, m_editingSprite->yscale, nullptr));
			}
			else
			if (sender() == ui.xScaleDoubleSpinBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "xscale", ui.xScaleDoubleSpinBox->value(), m_editingSprite->xscale, nullptr));
			}
			else if (sender() == ui.xScaleSlider)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "xscale", ui.xScaleSlider->value() / 10.0, m_editingSprite->xscale, nullptr));
			}else if (sender() == ui.rotationDoubleSpinBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "rotation", ui.rotationDoubleSpinBox->value(), m_editingSprite->rotation, nullptr));
			}
			else if (sender() == ui.rotationSlider)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "rotation", ui.rotationSlider->value(), m_editingSprite->rotation, nullptr));
			} else if (sender() == ui.commentLineEdit)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "comment", ui.commentLineEdit->text(), m_editingSprite->comment, nullptr));
			}
			else if (sender() == ui.colorEffectCheckBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "colorEffect", ui.colorEffectCheckBox->isChecked(), m_editingSprite->colorEffectEnabled, nullptr));
			}
			else if (sender() == ui.spriteLeftSpinBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "left", ui.spriteLeftSpinBox->value(), m_editingSprite->left, nullptr));
			}
			else if (sender() == ui.spriteTopSpinBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "top", ui.spriteTopSpinBox->value(), m_editingSprite->top, nullptr));
			}
			else if (sender() == ui.spriteWidthSpinBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "width", ui.spriteWidthSpinBox->value(), m_editingSprite->width, nullptr));
			}
			else if (sender() == ui.spriteHeightSpinBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "height", ui.spriteHeightSpinBox->value(), m_editingSprite->height, nullptr));
			}
			else if (sender() == ui.spriteSourceComboBox)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "source", ui.spriteSourceComboBox->currentText(), m_editingSprite->type, nullptr));
			}
			else if (sender() == ui.spriteImageLineEdit)
			{
				addUndoCommand(new UndoCommandSetSpriteProperty(this, m_editingSprite, "image", ui.spriteImageLineEdit->text(), m_editingSprite->customImageName, nullptr));
			}
			this->setModified();

			m_editingSprite->updateBoundingBox();

			updateSpriteItemWidget(m_editingSprite, sender() == ui.commentLineEdit);

		}
	}

	void AniEditor::spriteColorEffectClicked(bool checked)
	{

	}



	void AniEditor::onionSkinClicked(bool checked)
	{
		ui.graphicsView->redraw();
	}

	void AniEditor::centerViewClicked(bool checked)
	{
		ui.graphicsView->centerOn(0.0, 0.0);
	}

	void AniEditor::undoClicked(bool checked)
	{
		stop();
		deselect();

		m_undoStack.undo();

		if (m_undoStack.isClean())
		{
			setUnmodified();

		}
		ui.graphicsView->redraw();
		ui.graphicsViewTimeline->redraw();
		ui.graphicsViewSprite->redraw();
		ui.undoButton->setEnabled(m_undoStack.canUndo());
		ui.redoButton->setEnabled(m_undoStack.canRedo());
		
	}

	void AniEditor::redoClicked(bool checked)
	{
		stop();
		deselect();

		m_undoStack.redo();
		if (m_undoStack.isClean())
		{
			setUnmodified();

		}
		ui.graphicsView->redraw();
		ui.graphicsViewTimeline->redraw();
		ui.graphicsViewSprite->redraw();
		ui.undoButton->setEnabled(m_undoStack.canUndo());
		ui.redoButton->setEnabled(m_undoStack.canRedo());
		
	}

	void AniEditor::setFrame(int frameIndex)
	{
		ui.copyFrameButton->setEnabled(true);
		deselect();

		m_frame = frameIndex;
		
		const QSignalBlocker blocker(ui.timelineSlider);
		ui.timelineSlider->setValue(m_frame);

		
		auto frame = m_ani.getFrame(m_frame);
		if (frame)
		{
			{
				const QSignalBlocker blocker(ui.durationWidget);

				if(ui.durationWidget->value() != frame->duration)
					ui.durationWidget->setValue(frame->duration);
			}
			updateSoundsWidget();

			ui.undoButton->setEnabled(m_undoStack.canUndo());
			ui.redoButton->setEnabled(m_undoStack.canRedo());
		}
		
		updateFrame();
		m_firstSetFrame = false;
		const QSignalBlocker blocker2(ui.frameCountLabel);

		ui.frameCountLabel->setText(QString("%1/%2").arg(getFrame()+1).arg(m_ani.getFrameCount()));
	}

	void AniEditor::setFrameAndDir(int frame, int dir)
	{
		if (getDir() != dir || getFrame() != frame)
		{
			const QSignalBlocker blocker(ui.directionComboBox);
			ui.directionComboBox->setCurrentIndex(dir);
			setFrame(frame);
		}


	}

	void AniEditor::setSpritePreviewZoomLevel(int level)
	{
		m_spritePreviewZoomLevel = level;
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
		 
		auto zoomLevel = zoomFactors[m_spritePreviewZoomLevel];

		auto scaleX = zoomLevel;
		auto scaleY = zoomLevel;

		ui.graphicsViewSprite->setAntiAlias(zoomLevel < 1);

		ui.graphicsViewSprite->resetTransform();
		ui.graphicsViewSprite->scale(scaleX, scaleY);

		ui.graphicsViewSprite->centerOn(16, 16);
	}

	void AniEditor::updateSoundsWidget(bool playSound)
	{
		const QSignalBlocker blocker(ui.soundsWidget);
		ui.soundsWidget->clear();
		auto frame = m_ani.getFrame(getFrame());
		if (frame)
		{
	
			for (auto piece : frame->sounds)
			{
				if (piece->type == Ani::Frame::PIECE_SOUND)
				{
					auto sound = static_cast<Ani::Frame::FramePieceSound*>(piece);
					if (playSound && !m_firstSetFrame)
					{
						
						if (!sound->soundEffect.isLoaded() && !sound->fullPath.isEmpty())
						{
							sound->soundEffect.setSource(QUrl::fromLocalFile(sound->fullPath));
							sound->soundEffect.setLoopCount(1);
							sound->soundEffect.setVolume(1.0f);
						}

						sound->soundEffect.play();
					}

					auto listItem = new QListWidgetItem(sound->fileName);
					listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
					listItem->setData(Qt::UserRole, QVariant(sound->id));
					ui.soundsWidget->addItem(listItem);
				}

			}
		}
	}


	void AniEditor::initNewAni()
	{
		m_ani.setDefaultImage("HEAD", "head1.png", m_resourceManager);
		m_ani.setDefaultImage("BODY", "body.png", m_resourceManager);
		m_ani.setDefaultImage("SWORD", "sword1.png", m_resourceManager);
		m_ani.setDefaultImage("SHIELD", "shield1.png", m_resourceManager);

		{
			const QSignalBlocker blocker(ui.defaultsTable);
			for (int i = 0; i < ui.defaultsTable->verticalHeader()->count(); ++i)
			{
				QString keyName = ui.defaultsTable->verticalHeaderItem(i)->text().toUpper();
				ui.defaultsTable->setItem(i, 0, new QTableWidgetItem(m_ani.getDefaultImageName(keyName)));
			}
		}
		auto shadow = new Ani::AniSprite();
		shadow->type = "SPRITES";
		shadow->left = shadow->top = 0;
		shadow->width = 24;
		shadow->height = 12;
		shadow->comment = "shadow";
		shadow->updateBoundingBox();
		shadow->index = m_ani.getNextSpriteIndex(false);
		m_ani.addSprite(shadow);
		
		auto pixmap = getSpritePixmap(shadow);
		auto item = new SpriteListWidgetItem(pixmap, QString::number(shadow->index));
		
		item->setData(Qt::UserRole, QVariant(shadow->index));
		item->setData(Qt::UserRole + 1, QVariant(shadow->type));
		ui.spritesListWidget->addItem(item);
		item->setToolTip(shadow->comment);

		m_ani.insertFrame(0, new Ani::Frame());

		for (int dir = 0; dir < 4; ++dir)
		{
			auto shadowPiece = new Ani::Frame::FramePieceSprite();
			shadowPiece->spriteIndex = shadow->index;
			shadowPiece->xoffset = 12;
			shadowPiece->yoffset = dir % 2 == 0 ? 34 : 36;

			auto frame = m_ani.getFrame(0);
			if (frame)
			{
				frame->pieces[dir].push_back(shadowPiece);
			}
		}
		populateItems();

		
		ui.frameCountLabel->setText(QString("%1/%2").arg(getFrame() + 1).arg(m_ani.getFrameCount()));
		ui.totalDurationLabel->setText(QString("%1s").arg(QString::number(m_ani.getTotalDuration() / 1000.0, 103, 3)));
	}


	void AniEditor::updateSpriteItemWidget(Ani::AniSprite* sprite, bool commentOnly)
	{

		QListWidgetItem* item = nullptr;
		for (qsizetype i = 0; i < ui.spritesListWidget->count(); ++i)
		{
			if (ui.spritesListWidget->item(i)->data(Qt::UserRole).toInt() == sprite->index)
			{
				item = ui.spritesListWidget->item(i);

				break;
			}
		}

		if (item)
		{
			item->setToolTip(ui.commentLineEdit->text());
			if (!commentOnly)
				item->setIcon(getSpritePixmap(sprite));
			
			ui.spritesListWidget->doItemsLayout();
		}

		if (!commentOnly)
		{
			ui.graphicsView->redraw();
			ui.graphicsViewSprite->redraw();
		}
	}

	void AniEditor::endMovePieces()
	{
		if (m_selectedPieces.size())
		{


		}
	}

	bool AniEditor::loadFile(const QString& fileName, const QString& fullPath)
	{
		m_ani.setFileName(fileName);
		m_ani.setFullPath(fullPath);

		QFile file(m_ani.getFullPath());
		if (file.open(QIODeviceBase::ReadOnly))
		{
			if (Ani::loadGraalAni(&m_ani, &file, m_resourceManager))
			{
				{
					const QSignalBlocker blocker(ui.defaultsTable);
					for (int i = 0; i < ui.defaultsTable->verticalHeader()->count(); ++i)
					{
						QString keyName = ui.defaultsTable->verticalHeaderItem(i)->text().toUpper();
						ui.defaultsTable->setItem(i, 0, new QTableWidgetItem(m_ani.getDefaultImageName(keyName)));
					}
				}
				{

					const QSignalBlocker blocker(ui.timelineSlider);
					ui.timelineSlider->setMaximum(qMax(0, m_ani.getFrameCount() - 1));
				}
				ui.singleDirCheckBox->setChecked(m_ani.isSingleDir());
				ui.continousCheckBox->setChecked(m_ani.isContinous());
				ui.loopedCheckBox->setChecked(m_ani.isLooped());
				ui.directionComboBox->setEnabled(!m_ani.isSingleDir());
				ui.nextAniLineEdit->setText(m_ani.getNextAni());
				for (auto sprite : m_ani.getSprites())
				{
		
					auto pixmap = getSpritePixmap(sprite);
					auto item = new SpriteListWidgetItem(pixmap, QString::number(sprite->index));
					
					item->setData(Qt::UserRole, QVariant(sprite->index));
					item->setData(Qt::UserRole + 1, QVariant(sprite->type));
					item->setToolTip(sprite->comment);
					ui.spritesListWidget->addItem(item);

				}

				ui.totalDurationLabel->setText(QString("%1s").arg(QString::number(m_ani.getTotalDuration() / 1000.0, 103, 3)));
				populateItems();
				setFrame(0);

			}
			return true;
		}
		return false;
	}

	void AniEditor::setModified()
	{
		if (!m_modified)
		{
			m_modified = true;
			emit changeTabText((m_ani.getFileName().isEmpty() ? QString("new") : m_ani.getFileName())+ "*");
		}
	}

	void AniEditor::setFramePieceProperty(Ani::Frame::FramePiece* piece, const QString& propName, const QVariant& value)
	{
		this->setModified();
		if (propName == "spriteIndex")
		{
			if (piece->type == Ani::Frame::PIECE_SPRITE) {
				auto spritePiece = static_cast<Ani::Frame::FramePieceSprite*>(piece);
				spritePiece->spriteName = value.toString();

				bool isValidInteger = false;
				spritePiece->spriteIndex = spritePiece->spriteName.toInt(&isValidInteger);
				if (!isValidInteger)
					spritePiece->spriteIndex = Ani::SPRITE_INDEX_STRING;

				if (m_selectedPieces.size())
				{
					auto selectedPiece = *m_selectedPieces.begin();
					if (selectedPiece == piece)
					{ 
						const QSignalBlocker blocker(ui.itemSpriteIDEdit);
						ui.itemSpriteIDEdit->setText(spritePiece->spriteIndex == Ani::SPRITE_INDEX_STRING ? spritePiece->spriteName : QString::number(spritePiece->spriteIndex));
					}
				}
			}
			ui.graphicsView->redraw();
		}
		else if (propName == "soundFile")
		{
			if (piece->type == Ani::Frame::PIECE_SOUND)
			{
				auto soundPiece = static_cast<Ani::Frame::FramePieceSound*>(piece);

				soundPiece->setSoundFile(value.toString(), m_resourceManager);

				const QSignalBlocker blocker(ui.soundsWidget);
				for (auto i = 0; i < ui.soundsWidget->count(); ++i)
				{
					auto item = ui.soundsWidget->item(i);
					if (item->data(Qt::UserRole).toULongLong() == soundPiece->id)
					{
						item->setText(soundPiece->fileName);
					}

				}
			}
		}
	}

	void AniEditor::setSpriteProperty(Ani::AniSprite* sprite, const QString& propName, const QVariant& value)
	{
		this->setModified();
		if (propName == "rotation")
		{
			sprite->rotation = value.toDouble();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.rotationDoubleSpinBox);
				const QSignalBlocker blocker2(ui.rotationSlider);
				ui.rotationDoubleSpinBox->setValue(sprite->rotation);
				ui.rotationSlider->setValue(sprite->rotation);
			}
		}
		else if (propName == "xscale")
		{
			sprite->xscale = value.toDouble();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.xScaleDoubleSpinBox);
				const QSignalBlocker blocker2(ui.xScaleSlider);
				ui.xScaleDoubleSpinBox->setValue(sprite->xscale);
				ui.xScaleSlider->setValue(sprite->xscale * 10);
			}
		}
		else if (propName == "yscale")
		{
			sprite->yscale = value.toDouble();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.yScaleDoubleSpinBox);
				const QSignalBlocker blocker2(ui.yScaleSlider);
				ui.yScaleDoubleSpinBox->setValue(sprite->yscale);
				ui.yScaleSlider->setValue(sprite->yscale * 10);
			}
		}
		else if (propName == "comment")
		{
			sprite->comment = value.toString();
			if (sprite == m_editingSprite)
				ui.commentLineEdit->setText(sprite->comment);
			populateItems();
		}
		else if (propName == "colorEffect")
		{
			sprite->colorEffectEnabled = value.toBool();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.colorEffectCheckBox);
				ui.colorEffectCheckBox->setChecked(sprite->colorEffectEnabled);
				ui.changeColorEffectButton->setEnabled(sprite->colorEffectEnabled);
			}
		}
		else if (propName == "color")
		{
			sprite->colorEffect = QColor::fromRgba(value.toUInt());
			if (sprite == m_editingSprite)
			{
				ui.changeColorEffectButton->setStyleSheet(QString("background-color: %1").arg(sprite->colorEffect.name()));

				ui.changeColorEffectButton->update();
			}
		} else if (propName == "left")
		{
			sprite->left = value.toInt();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.spriteLeftSpinBox);
				ui.spriteLeftSpinBox->setValue(sprite->left);
			}
		}
		else if (propName == "top")
		{
			sprite->top = value.toInt();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.spriteTopSpinBox);
				ui.spriteTopSpinBox->setValue(sprite->top);
			}
		}
		else if (propName == "width")
		{

			sprite->width = value.toInt();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.spriteWidthSpinBox);
				ui.spriteWidthSpinBox->setValue(sprite->width);
			}

		}
		else if (propName == "height")
		{
			sprite->height = value.toInt();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.spriteHeightSpinBox);
				ui.spriteHeightSpinBox->setValue(sprite->height);
			}
		}
		else if (propName == "source")
		{
			sprite->type = value.toString();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.spriteSourceComboBox);

				ui.spriteSourceComboBox->setCurrentText(sprite->type);

				ui.spriteImageLineEdit->setEnabled(sprite->type == "CUSTOM");
				const QSignalBlocker blocker2(ui.spriteImageLineEdit);

				if (sprite->type == "CUSTOM")
					ui.spriteImageLineEdit->setText(sprite->customImageName);
				else ui.spriteImageLineEdit->setText("");
			}
		}
		else if (propName == "image")
		{
			sprite->releaseCustomImage(m_resourceManager);
			sprite->customImageName = value.toString();
			if (sprite == m_editingSprite)
			{
				const QSignalBlocker blocker(ui.spriteImageLineEdit);
				ui.spriteImageLineEdit->setText(sprite->customImageName);

			}
		}
		else if (propName == "drawIndex")
		{
			auto newIndex = value.toInt();
			sprite->m_drawIndex = value.toInt();

			if (sprite == m_editingSprite)
				ui.graphicsViewSprite->redraw();
		}

		sprite->updateBoundingBox();
		updateSpriteItemWidget(sprite, propName == "comment");
		ui.graphicsViewSprite->redraw();
	}

	void AniEditor::setDefaultValue(const QString& name, const QString& value, bool setCell)
	{
		this->setModified();
		m_ani.setDefaultImage(name, value, m_resourceManager);


		if (setCell)
		{
			const QSignalBlocker blocker(ui.defaultsTable);
			for (int i = 0; i < ui.defaultsTable->verticalHeader()->count(); ++i)
			{
				QString keyName = ui.defaultsTable->verticalHeaderItem(i)->text().toUpper();

				if (keyName == name)
				{
					ui.defaultsTable->setItem(i, 0, new QTableWidgetItem(value));
					break;
				}
			}
		}

		bool iconChanged = false;
		for (qsizetype i = 0; i < ui.spritesListWidget->count(); ++i)
		{
			auto item = ui.spritesListWidget->item(i);


			//Update the image of any sprite in the sprite list (left panel)
			if (item->data(Qt::UserRole + 1).toString() == name)
			{

				auto spriteID = item->data(Qt::UserRole).toInt();

				auto sprite = m_ani.getAniSprite(spriteID, "");
				if (sprite)
				{

					auto pixmap = getSpritePixmap(sprite);
					if (!pixmap.isNull())
					{

						item->setIcon(pixmap);
						iconChanged = true;
					}

				}
			}
		}

		if (iconChanged)
		{
			ui.spritesListWidget->doItemsLayout();
		}
		ui.graphicsView->redraw();
		ui.graphicsViewTimeline->redraw();


	}


	void AniEditor::saveFile()
	{

	}

	void AniEditor::setAniProperty(const QString& propName, const QVariant& value)
	{
		this->setModified();
		if (propName == "loop")
		{
			m_ani.setLooped(value.toBool());
			ui.loopedCheckBox->setChecked(m_ani.isLooped());
		}
		else if (propName == "continous")
		{
			m_ani.setContinous(value.toBool());
			ui.continousCheckBox->setChecked(m_ani.isContinous());
		}
		else if (propName == "singledir")
		{
			m_ani.setSingleDir(value.toBool());
			ui.singleDirCheckBox->setChecked(m_ani.isSingleDir());
			ui.directionComboBox->setEnabled(!m_ani.isSingleDir());

		}
		else if (propName == "nextani")
		{
			m_ani.setNextAni(value.toString());
			ui.nextAniLineEdit->setText(m_ani.getNextAni());
		}
		else if (propName == "script")
		{
			m_ani.setScript(value.toString());
		}
	}

	void AniEditor::setFrameProperty(Ani::Frame* frame, int dir, const QString& propName, const QVariant& value)
	{
		this->setModified();
		if (propName == "duration")
		{
			frame->duration = value.toInt();

			if (frame == m_ani.getFrame(getFrame()))
			{
				const QSignalBlocker blocker(ui.durationWidget);
				ui.durationWidget->setValue(frame->duration);
			}
			ui.totalDurationLabel->setText(QString("%1s").arg(QString::number(m_ani.getTotalDuration() / 1000.0, 103, 3)));
		}
	}

	void AniEditor::setBackgroundColour(const QColor& color)
	{
		m_backgroundColor = color;
		ui.graphicsView->redraw();
		ui.graphicsViewSprite->redraw();
	}
}
