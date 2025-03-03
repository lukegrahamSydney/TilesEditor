#ifndef GRAPHICSVIEWH
#define GRAPHICSVIEWH

#include <QGraphicsView>
#include <QResizeEvent>
#include <QScrollBar>
#include <QDrag>
#include <QMimeData>

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
        void contentsScrolled();

    public slots:
        void redraw() {
            this->scene()->update();
        }

    private:
        bool m_antialias;

    public:
        GraphicsView(QWidget* parent = nullptr) :
            QGraphicsView(parent)
        {
            setFocusPolicy(Qt::StrongFocus);

            m_antialias = true;
            this->setScene(new QGraphicsScene());


            this->setMouseTracking(true);

            scale(1.0, 1.0f);
            setTransformationAnchor(QGraphicsView::NoAnchor);
        }


        void redrawRect(const QRectF& rect) {
            this->scene()->update(rect);

           // this->update()
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

        void scrollContentsBy(int dx, int dy) override;





    };
}
#endif
