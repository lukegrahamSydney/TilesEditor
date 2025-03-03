#pragma once

#include <QDialog>
#include "ui_ScreenshotDialog.h"
#include "IWorld.h"

namespace TilesEditor
{
	class ScreenshotDialog : public QDialog
	{
		Q_OBJECT
	public slots:
		void renderScene(QPainter* painter, const QRectF& rect);
		void zoomLevelChanged(double d);
		void saveFileClicked(bool checked);
		void saveFolderClicked(bool checked);
		
		void graphicsMouseWheel(QWheelEvent* event);

	private:
		static QByteArray savedGeometry;

		IWorld* m_world;
		QPixmap m_output;
		double m_zoomLevel;

	public:
		ScreenshotDialog(IWorld* world, double centerX, double centerY, QWidget* parent = nullptr);
		~ScreenshotDialog();

	private:
		Ui::ScreenshotDialogClass ui;
	};
};
