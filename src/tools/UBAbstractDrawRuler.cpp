
#include "UBAbstractDrawRuler.h"
#include <QtSvg>
#include "core/UB.h"
#include "gui/UBResources.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#include "core/memcheck.h"


const QColor UBAbstractDrawRuler::sLightBackgroundMiddleFillColor = QColor(0x72, 0x72, 0x72, sFillTransparency);
const QColor UBAbstractDrawRuler::sLightBackgroundEdgeFillColor = QColor(0xc3, 0xc3, 0xc3, sFillTransparency);
const QColor UBAbstractDrawRuler::sLightBackgroundDrawColor = QColor(0x33, 0x33, 0x33, sDrawTransparency);
const QColor UBAbstractDrawRuler::sDarkBackgroundMiddleFillColor = QColor(0xb5, 0xb5, 0xb5, sFillTransparency);
const QColor UBAbstractDrawRuler::sDarkBackgroundEdgeFillColor = QColor(0xdd, 0xdd, 0xdd, sFillTransparency);
const QColor UBAbstractDrawRuler::sDarkBackgroundDrawColor = QColor(0xff, 0xff, 0xff, sDrawTransparency);

UBAbstractDrawRuler::UBAbstractDrawRuler()
	: mResizing(false)
    , mRotating(false)
    , mShowButtons(false)
    , mAntiScaleRatio(1.0)
{}

void UBAbstractDrawRuler::create(QGraphicsItem& item)
{
	item.setFlag(QGraphicsItem::ItemIsMovable, true);
    item.setFlag(QGraphicsItem::ItemIsSelectable, true);
    item.setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    item.setAcceptsHoverEvents(true);

    mCloseSvgItem = new QGraphicsSvgItem(":/images/closeTool.svg", &item);
    mCloseSvgItem->setVisible(false);
    mCloseSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

    mRotateSvgItem = new QGraphicsSvgItem(":/images/rotateTool.svg", &item);
    mRotateSvgItem->setVisible(false);
    mRotateSvgItem->setData(UBGraphicsItemData::ItemLayerType, QVariant(UBItemLayerType::Control));

	updateResizeCursor(item);
}


UBAbstractDrawRuler::~UBAbstractDrawRuler()
{
}

void UBAbstractDrawRuler::updateResizeCursor(QGraphicsItem &item)
{
    QPixmap pix(":/images/cursors/resize.png");
    QTransform itemTransform = item.sceneTransform();
    QRectF itemRect = item.boundingRect();
    QPointF topLeft = itemTransform.map(itemRect.topLeft());
    QPointF topRight = itemTransform.map(itemRect.topRight());
    QLineF topLine(topLeft, topRight);
    qreal angle = topLine.angle();
    QTransform tr;
    tr.rotate(- angle);
    QCursor resizeCursor  = QCursor(pix.transformed(tr, Qt::SmoothTransformation), pix.width() / 2,  pix.height() / 2);
    mResizeCursor = resizeCursor;
}


QCursor UBAbstractDrawRuler::moveCursor() const
{
    return Qt::SizeAllCursor;
}

QCursor UBAbstractDrawRuler::resizeCursor() const
{
    return mResizeCursor;
}

QCursor UBAbstractDrawRuler::rotateCursor() const
{
    return UBResources::resources()->rotateCursor;
}

QCursor UBAbstractDrawRuler::closeCursor() const
{
    return Qt::ArrowCursor;
}

QCursor UBAbstractDrawRuler::drawRulerLineCursor() const
{
	return UBResources::resources()->drawLineRulerCursor;
}

QColor UBAbstractDrawRuler::drawColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundDrawColor : sLightBackgroundDrawColor;
}

QColor UBAbstractDrawRuler::middleFillColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundMiddleFillColor : sLightBackgroundMiddleFillColor;
}

QColor UBAbstractDrawRuler::edgeFillColor() const
{
    return scene()->isDarkBackground() ? sDarkBackgroundEdgeFillColor : sLightBackgroundEdgeFillColor;
}

QFont UBAbstractDrawRuler::font() const
{
    QFont font("Arial");
    font.setPixelSize(16);
    return font;
}

void UBAbstractDrawRuler::StartLine(const QPointF& position, qreal width)
{}
void UBAbstractDrawRuler::DrawLine(const QPointF& position, qreal width)
{}
void UBAbstractDrawRuler::EndLine()
{}


void UBAbstractDrawRuler::paint()
{
    mAntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());
    QTransform antiScaleTransform;
    antiScaleTransform.scale(mAntiScaleRatio, mAntiScaleRatio);

    mCloseSvgItem->setTransform(antiScaleTransform);
    mCloseSvgItem->setPos(closeButtonRect().topLeft());

    mRotateSvgItem->setTransform(antiScaleTransform);
    mRotateSvgItem->setPos(rotateButtonRect().topLeft());

}

