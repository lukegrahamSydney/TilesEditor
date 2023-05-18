#ifndef EDITTILESETDIALOGH
#define EDITTILESETDIALOGH

#include <QDialog>
#include <QString>
#include "ui_EditTilesetDialog.h"
#include "Tileset.h"
#include "Image.h"
#include "ResourceManager.h"

namespace TilesEditor
{
	class EditTilesetDialog : public QDialog
	{
		Q_OBJECT

	public slots:
		void renderScene(QPainter* painter, const QRectF& rect);
		void graphicsMouseWheel(QWheelEvent* event);
		void graphicsMouseMove(QMouseEvent* event);
		void tileTypeClicked(bool checked);
		void browseButtonClicked(bool checked);

	private:
		ResourceManager& m_resourceManager;
		Tileset m_tileset;
		Image* m_tilesetImage;

		int m_selectedType;

	public:
		EditTilesetDialog(const Tileset* tileset, ResourceManager& resourceManager, QWidget* parent = nullptr);
		~EditTilesetDialog();

		const Tileset& getTileset() const { return m_tileset; }

	private:
		Ui::EditTilesetDialogClass ui;

	protected:
		void accept() override;
	};
};
#endif
