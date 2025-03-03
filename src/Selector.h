#ifndef SELECTORH
#define SELECTORH

#include <qmath.h>
#include <QRect>
#include <qpainter.h>
#include "AbstractSelection.h"

namespace TilesEditor
{
    class Selector
    {

    private:
        qreal   m_x1 = 0.0,
            m_y1 = 0.0,
            m_x2 = 0.0,
            m_y2 = 0.0;

        qreal   m_snapX = 0.0,
            m_snapY = 0.0;

        bool    m_selecting = false;
        bool    m_visible = false;


    public:
        bool selecting() const {
            return m_selecting;
        }

        bool visible() const {
            return m_visible;
        }

        void setVisible(bool val) {
            m_visible = val;
        }


        void beginSelection(qreal x, qreal y, int snapX, int snapY)
        {
            m_snapX = snapX;
            m_snapY = snapY;


            m_x1 = qFloor(x / snapX) * snapX;
            m_y1 = qFloor(y / snapY) * snapY;

            m_x2 = m_x1 + snapX;
            m_y2 = m_y1 + snapY;
            m_selecting = true;


        }

        void updateSelection(float x, float y)
        {
            if (x > m_x1)
                m_x2 = qCeil(x / m_snapX) * m_snapX;
            else m_x2 = qFloor(x / m_snapX) * m_snapX;

            if (y > m_y1)
                m_y2 = qCeil(y / m_snapY) * m_snapY;
            else m_y2 = qFloor(y / m_snapY) * m_snapY;
        }

        void endSelection(float x, float y)
        {
            updateSelection(x, y);
            m_selecting = false;
            auto rect = getSelection();
            m_x1 = rect.x();
            m_y1 = rect.y();
            m_x2 = rect.x() + qMax(1, (int)rect.width());
            m_y2 = rect.y() + qMax(1, (int)rect.height());

        }

        QRectF getSelection() const
        {
            float x1 = qMin(m_x1, m_x2);
            float y1 = qMin(m_y1, m_y2);
            float x2 = qMax(m_x1, m_x2);
            float y2 = qMax(m_y1, m_y2);

            return QRectF(x1, y1, x2 - x1, y2 - y1);
        }

        void draw(QPainter* painter, const QRectF& viewRect, const QColor& lineColour, const QColor& fillColour)
        {

            if (this->selecting())
            {

                auto  rect = getSelection();
                draw(painter, viewRect, rect.x(), rect.y(), (int)rect.width(), (int)rect.height(), lineColour, fillColour, false, true);

            }
            else
            {
                draw(painter, viewRect, m_x1, m_y1, (int)(m_x2 - m_x1), (int)(m_y2 - m_y1), lineColour, fillColour, false, true);
            }

        }

        int getResizeEdge(int mouseX, int mouseY)
        {
            auto rect = getSelection();

            if (mouseX > rect.x() - 4 && mouseX < rect.right() + 4 && mouseY > rect.y() - 4 && mouseY < rect.bottom() + 4)
            {
                int retval = 0;

                if (mouseX <= rect.x() + 2)
                {
                    retval |= AbstractSelection::Edges::EDGE_LEFT;
                }
                else if (mouseX >= rect.right() - 2)
                {
                    retval |= AbstractSelection::Edges::EDGE_RIGHT;
                }

                if (mouseY <= rect.y() + 2)
                {
                    retval |= AbstractSelection::Edges::EDGE_TOP;
                }
                else if (mouseY >= rect.bottom() - 2)
                {
                    retval |= AbstractSelection::Edges::EDGE_BOTTOM;
                }
                return retval;

            }
            

            return 0;

        }

        static void draw(QPainter* painter, const QRectF& viewRect, qreal x, qreal y, int width, int height, const QColor& lineColour, const QColor& fillColour, bool doubleOutline = false, bool marchingAnts = false)
        {
            auto compositionMode = painter->compositionMode();
            painter->setCompositionMode(QPainter::CompositionMode_Difference);

            if(fillColour.alpha() > 0)
                painter->fillRect((int)x, (int)y, width, height, fillColour);

            QPen oldPen = painter->pen();

            QPen newPen(lineColour, 2);
            newPen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
            painter->setPen(newPen);
            painter->drawRect(x - 1, y - 1, width + 2, height + 2);
            painter->setPen(oldPen);
            painter->setCompositionMode(compositionMode);

        }

    };
};

#endif
