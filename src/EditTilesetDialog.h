#ifndef EDITTILESETDIALOGH
#define EDITTILESETDIALOGH

#include <QDialog>
#include <QString>
#include <QSet>
#include <QPixmap>
#include "ui_EditTilesetDialog.h"
#include "Tileset.h"
#include "Image.h"
#include "AbstractResourceManager.h"

namespace TilesEditor
{
	class EditTilesetDialog : public QDialog
	{
		Q_OBJECT

	private slots:


		void renderScene(QPainter* painter, const QRectF& rect);
		void graphicsMouseWheel(QWheelEvent* event);
		void graphicsMouseMove(QMouseEvent* event);
		void tileTypeClicked(bool checked);
		void browseButtonClicked(bool checked);
		void reloadImage();
		void overlayChanged(int);

	private:
		static QByteArray savedGeometry;
		static int m_defaultOverlay;
		AbstractResourceManager* m_resourceManager;
		Tileset m_tileset;
		Image* m_tilesetImage;

		int m_selectedType;
		QMap<int, QPixmap> m_letters;

		QPixmap getLetterImage(int type);

	public:
		EditTilesetDialog(const Tileset* tileset, AbstractResourceManager* resourceManager, QWidget* parent = nullptr);
		~EditTilesetDialog();

		const Tileset& getTileset() const { return m_tileset; }

	private:
		Ui::EditTilesetDialogClass ui;

	protected:
		void accept() override;
	};
};
#endif
