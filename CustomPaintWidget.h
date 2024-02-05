#ifndef CUSTOMPAINTWIDGETH
#define CUSTOMPAINTWIDGETH

#include <QtWidgets>

//Create this widget and connect to the paint signal to custom draw.
//Used to display the default tile

namespace TilesEditor
{ 

    class CustomPaintWidget : public QWidget
    {
        Q_OBJECT

    signals:
        void paint(QPainter* painter, const QRectF& rect);
        void mouseDoubleClick(QMouseEvent* event);

    public:
        CustomPaintWidget(QWidget* parent) :
            QWidget(parent) {}


    protected:
        void paintEvent(QPaintEvent* p) override
        { 
            QPainter painter(this);
            QRectF rect(0, 0, this->width(), this->height());

            emit paint(&painter, rect);
        }

        void mouseDoubleClickEvent(QMouseEvent* event) override;
    };
};
#endif
