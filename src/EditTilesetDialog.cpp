#include <QFileDialog>
#include "EditTilesetDialog.h"
#include "ResourceManagerFileSystem.h"

namespace TilesEditor
{
	QByteArray EditTilesetDialog::savedGeometry;
	int EditTilesetDialog::m_defaultOverlay = 50;
	EditTilesetDialog::EditTilesetDialog(const Tileset* tileset, AbstractResourceManager* resourceManager, QWidget* parent)
		: QDialog(parent), m_resourceManager(resourceManager)
	{
		ui.setupUi(this);
		this->setWindowFlag(Qt::Window);
		this->setWindowFlag(Qt::WindowMaximizeButtonHint);

		m_selectedType = 0;
		connect(ui.graphicsView, &GraphicsView::renderView, this, &EditTilesetDialog::renderScene);
		connect(ui.graphicsView, &GraphicsView::mouseWheelEvent, this, &EditTilesetDialog::graphicsMouseWheel);
		connect(ui.graphicsView, &GraphicsView::mouseMove, this, &EditTilesetDialog::graphicsMouseMove);
		connect(ui.graphicsView, &GraphicsView::mousePress, this, &EditTilesetDialog::graphicsMouseMove);

		connect(ui.overlaySlider, &QSlider::valueChanged, this, &EditTilesetDialog::overlayChanged);
		connect(ui.browseButton, &QAbstractButton::clicked, this, &EditTilesetDialog::browseButtonClicked);
		connect(ui.imageTextBox, &QLineEdit::editingFinished, this, &EditTilesetDialog::reloadImage);

		connect(ui.radioButtonType0, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType1, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType2, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType3, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType4, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType5, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType6, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType7, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType8, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType9, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType10, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);
		connect(ui.radioButtonType11, &QAbstractButton::clicked, this, &EditTilesetDialog::tileTypeClicked);

		ui.imageTextBox->setText(tileset->getImageName());

		m_tileset = *tileset;

		m_tilesetImage = static_cast<Image*>(resourceManager->loadResource(nullptr, m_tileset.getImageName(), ResourceType::RESOURCE_IMAGE));

		ui.overlaySlider->setValue(m_defaultOverlay);
		if (m_tilesetImage)
		{
			ui.graphicsView->setSceneRect(0, 0, m_tilesetImage->width(), m_tilesetImage->height());
		}

		ui.fileLineEdit->setText(m_tileset.getFileName());

		if (!savedGeometry.isNull())
			restoreGeometry(savedGeometry);
	}


	EditTilesetDialog::~EditTilesetDialog()
	{
		if (m_tilesetImage != nullptr)
			m_resourceManager->freeResource(m_tilesetImage);

		savedGeometry = saveGeometry();
	
	}


	QPixmap EditTilesetDialog::getLetterImage(int type)
	{
		auto& pixmap = m_letters[type];

		if (pixmap.isNull())
		{
			pixmap = QPixmap(16, 16);
			pixmap.fill(Qt::transparent);
			QPainter painter(&pixmap);
			painter.drawText(0, 0, 16, 16, Qt::AlignHCenter, QString::number(type));
		}
		return pixmap;
	}

	void EditTilesetDialog::renderScene(QPainter* painter, const QRectF& rect)
	{
		static QColor tileColours[] = {
			QColorConstants::Svg::lime,
			QColorConstants::Svg::gray,
			QColorConstants::Yellow,
			QColorConstants::Blue,
			QColor(128, 64, 0),
			QColorConstants::Svg::teal,
			QColorConstants::Green,
			QColor(128, 128, 255),
			QColorConstants::Red,
			QColor(255, 128, 128),
			QColor(192, 255, 255),
			QColor(255, 128, 255)
		};

		static auto colourCount = sizeof(tileColours) / sizeof(QColor);

		if (m_tilesetImage)
		{
			painter->setOpacity(1.0);
			painter->drawPixmap(0, 0, m_tilesetImage->pixmap());
			
			int opacity = int((ui.overlaySlider->value() / 100.0) * 255);

			int tileLeft = int(rect.left() / 16.0);
			int tileTop = int(rect.top() / 16.0);
			int right = tileLeft + int(qCeil(rect.width() / 16.0));
			int bottom = tileTop + int(qCeil(rect.height() / 16.0));

			right = qMin(right, m_tileset.getHCount() - 1);
			bottom = qMin(bottom, m_tileset.getVCount() - 1);


			for (auto top = tileTop; top <= bottom; ++top)
			{
				for (auto left = tileLeft; left <= right; ++left)
				{
					auto tileType = m_tileset.getTileType(left, top);

					if (tileType >= 0 && tileType < colourCount) {
						auto colour = tileColours[tileType];

						if(opacity > 0)
							painter->fillRect(left * 16, top * 16, 16, 16, QColor(colour.red(), colour.green(), colour.blue(), opacity));
						auto image = getLetterImage(tileType);
						painter->drawPixmap(left * 16, top * 16, image);
					}

				}
			}
				
		
		}
	}

	void EditTilesetDialog::graphicsMouseWheel(QWheelEvent* event)
	{
		event->ignore();
	}

	void EditTilesetDialog::graphicsMouseMove(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::MouseButton::LeftButton))
		{
			auto pos = ui.graphicsView->mapToScene(event->pos());

			if (pos.x() >= 0 && pos.x() < ui.graphicsView->sceneRect().width() && pos.y() >= 0 && pos.y() < ui.graphicsView->sceneRect().height())
			{

				auto tileXPos = int(pos.x() / 16.0);
				auto tileYPos = int(pos.y() / 16.0);

				m_tileset.setTileType(tileXPos, tileYPos, m_selectedType);

				ui.graphicsView->scene()->update(QRect(tileXPos * 16, tileYPos * 16, 16, 16));
			}
		}
	}

	void EditTilesetDialog::tileTypeClicked(bool checked)
	{
		if (checked)
		{
			auto widget = static_cast<QWidget*>(this->sender());

			m_selectedType = widget->toolTip().toInt();
			
		}
	}

	void EditTilesetDialog::browseButtonClicked(bool checked)
	{
		auto fileName = QFileDialog::getOpenFileName(nullptr, "Select Image", QString(), "Image Files (*.png *.gif)");
		if (!fileName.isEmpty())
		{
			QFileInfo fi(fileName);
			auto directory = fi.absolutePath() + "/";

			if(m_resourceManager->getType() == "FileSystem")
				static_cast<ResourceManagerFileSystem*>(m_resourceManager)->addSearchDir(directory);

			ui.imageTextBox->setText(fi.fileName());

			reloadImage();
		}
	}


	void EditTilesetDialog::reloadImage()
	{
		if (m_tilesetImage)
			m_resourceManager->freeResource(m_tilesetImage);

		m_tilesetImage = static_cast<Image*>(m_resourceManager->loadResource(nullptr, ui.imageTextBox->text(), ResourceType::RESOURCE_IMAGE));

		if (m_tilesetImage)
		{
			auto hcount = m_tilesetImage->width() / 16;
			auto vcount = m_tilesetImage->height() / 16;

			m_tileset.resize(hcount, vcount);
			ui.graphicsView->setSceneRect(0, 0, m_tilesetImage->width(), m_tilesetImage->height());
		}
	}

	void EditTilesetDialog::overlayChanged(int value)
	{
		m_defaultOverlay = value;
		ui.graphicsView->redraw();
	}

	void EditTilesetDialog::accept()
	{
		m_tileset.setImageName(ui.imageTextBox->text());

		m_tileset.saveToFile();
		QDialog::accept();
	}
};
