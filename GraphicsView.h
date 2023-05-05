#ifndef GRAPHICSVIEWH
#define GRAPHICSVIEWH

#include <QGraphicsView>
#include <QResizeEvent>
#include <QScrollBar>
#include <QDrag>

namespace TilesEditor
{
    class GraphicsView : public QGraphicsView
    {
        Q_OBJECT

    signals:
        void renderView(QPainter* painter, const QRectF& rect);
        void mousePress(QMouseEvent* event);
        void mouseRelease(QMouseEvent* event);
        void mouseMove(QMouseEvent* event);
        void mouseDoubleClick(QMouseEvent* event);
        void mouseWheelEvent(QWheelEvent* event);
        void keyPress(QKeyEvent* event);

    private:
        bool m_antialias;

    public:
        GraphicsView(QWidget* parent = nullptr) :
            QGraphicsView(parent)
        {
            setFocusPolicy(Qt::StrongFocus);
            m_antialias = false;
            this->setScene(new QGraphicsScene());


            this->setMouseTracking(true);

            scale(1.0, 1.0f);
            setResizeAnchor(QGraphicsView::AnchorUnderMouse);
        }

        void redraw() {
            this->scene()->update();
        }

        void setAntiAlias(bool value) { m_antialias = value; }

    protected:
        void mouseMoveEvent(QMouseEvent* event) override;
        void mousePressEvent(QMouseEvent* event)override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        void drawBackground(QPainter* painter, const QRectF& rect) override;
        void keyPressEvent(QKeyEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;

        void dragEnterEvent(QDragEnterEvent* event) override
        {

            QDrag::cancel();
        }



    };
}
#endif
