
/***************************************************************************
**                                                                        **
**  QCustomPlot, an easy to use, modern plotting widget for Qt            **
**  Copyright (C) 2011, 2012, 2013 Emanuel Eichhammer                     **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Emanuel Eichhammer                                   **
**  Website/Contact: http://www.qcustomplot.com/                          **
**             Date: 04.11.13                                             **
**          Version: 1.1.0                                                **
****************************************************************************/

#include <qcustomplot.h>




////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPPainter
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPPainter
  \brief QPainter subclass used internally
  
  This internal class is used to provide some extended functionality e.g. for tweaking position
  consistency between antialiased and non-antialiased painting. Further it provides workarounds
  for QPainter quirks.
  
  \warning This class intentionally hides non-virtual functions of QPainter, e.g. setPen, save and
  restore. So while it is possible to pass a QCPPainter instance to a function that expects a
  QPainter pointer, some of the workarounds and tweaks will be unavailable to the function (because
  it will call the base class implementations of the functions actually hidden by QCPPainter).
*/

/*!
  Creates a new QCPPainter instance and sets default values
*/
QCPPainter::QCPPainter() :
  QPainter(),
  mModes(pmDefault),
  mIsAntialiasing(false)
{
  // don't setRenderHint(QPainter::NonCosmeticDefautPen) here, because painter isn't active yet and
  // a call to begin() will follow
}

/*!
  Creates a new QCPPainter instance on the specified paint \a device and sets default values. Just
  like the analogous QPainter constructor, begins painting on \a device immediately.
  
  Like \ref begin, this method sets QPainter::NonCosmeticDefaultPen in Qt versions before Qt5.
*/
QCPPainter::QCPPainter(QPaintDevice *device) :
  QPainter(device),
  mModes(pmDefault),
  mIsAntialiasing(false)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0) // before Qt5, default pens used to be cosmetic if NonCosmeticDefaultPen flag isn't set. So we set it to get consistency across Qt versions.
  if (isActive())
    setRenderHint(QPainter::NonCosmeticDefaultPen);
#endif
}

QCPPainter::~QCPPainter()
{
}

/*!
  Sets the pen of the painter and applies certain fixes to it, depending on the mode of this
  QCPPainter.
  
  \note this function hides the non-virtual base class implementation.
*/
void QCPPainter::setPen(const QPen &pen)
{
  QPainter::setPen(pen);
  if (mModes.testFlag(pmNonCosmetic))
    makeNonCosmetic();
}

/*! \overload
  
  Sets the pen (by color) of the painter and applies certain fixes to it, depending on the mode of
  this QCPPainter.
  
  \note this function hides the non-virtual base class implementation.
*/
void QCPPainter::setPen(const QColor &color)
{
  QPainter::setPen(color);
  if (mModes.testFlag(pmNonCosmetic))
    makeNonCosmetic();
}

/*! \overload
  
  Sets the pen (by style) of the painter and applies certain fixes to it, depending on the mode of
  this QCPPainter.
  
  \note this function hides the non-virtual base class implementation.
*/
void QCPPainter::setPen(Qt::PenStyle penStyle)
{
  QPainter::setPen(penStyle);
  if (mModes.testFlag(pmNonCosmetic))
    makeNonCosmetic();
}

/*! \overload
  
  Works around a Qt bug introduced with Qt 4.8 which makes drawing QLineF unpredictable when
  antialiasing is disabled. Thus when antialiasing is disabled, it rounds the \a line to
  integer coordinates and then passes it to the original drawLine.
  
  \note this function hides the non-virtual base class implementation.
*/
void QCPPainter::drawLine(const QLineF &line)
{
  if (mIsAntialiasing || mModes.testFlag(pmVectorized))
    QPainter::drawLine(line);
  else
    QPainter::drawLine(line.toLine());
}

/*! 
  Sets whether painting uses antialiasing or not. Use this method instead of using setRenderHint
  with QPainter::Antialiasing directly, as it allows QCPPainter to regain pixel exactness between
  antialiased and non-antialiased painting (Since Qt < 5.0 uses slightly different coordinate systems for
  AA/Non-AA painting).
*/
void QCPPainter::setAntialiasing(bool enabled)
{
  setRenderHint(QPainter::Antialiasing, enabled);
  if (mIsAntialiasing != enabled)
  {
    mIsAntialiasing = enabled;
    if (!mModes.testFlag(pmVectorized)) // antialiasing half-pixel shift only needed for rasterized outputs
    {
      if (mIsAntialiasing)
        translate(0.5, 0.5);
      else
        translate(-0.5, -0.5);
    }
  }
}

/*!
  Sets the mode of the painter. This controls whether the painter shall adjust its
  fixes/workarounds optimized for certain output devices.
*/
void QCPPainter::setModes(QCPPainter::PainterModes modes)
{
  mModes = modes;
}

/*!
  Sets the QPainter::NonCosmeticDefaultPen in Qt versions before Qt5 after beginning painting on \a
  device. This is necessary to get cosmetic pen consistency across Qt versions, because since Qt5,
  all pens are non-cosmetic by default, and in Qt4 this render hint must be set to get that
  behaviour.
  
  The Constructor \ref QCPPainter(QPaintDevice *device) which directly starts painting also sets
  the render hint as appropriate.
  
  \note this function hides the non-virtual base class implementation.
*/
bool QCPPainter::begin(QPaintDevice *device)
{
  bool result = QPainter::begin(device);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0) // before Qt5, default pens used to be cosmetic if NonCosmeticDefaultPen flag isn't set. So we set it to get consistency across Qt versions.
  if (result)
    setRenderHint(QPainter::NonCosmeticDefaultPen);
#endif
  return result;
}

/*! \overload
  
  Sets the mode of the painter. This controls whether the painter shall adjust its
  fixes/workarounds optimized for certain output devices.
*/
void QCPPainter::setMode(QCPPainter::PainterMode mode, bool enabled)
{
  if (!enabled && mModes.testFlag(mode))
    mModes &= ~mode;
  else if (enabled && !mModes.testFlag(mode))
    mModes |= mode;
}

/*!
  Saves the painter (see QPainter::save). Since QCPPainter adds some new internal state to
  QPainter, the save/restore functions are reimplemented to also save/restore those members.
  
  \note this function hides the non-virtual base class implementation.
  
  \see restore
*/
void QCPPainter::save()
{
  mAntialiasingStack.push(mIsAntialiasing);
  QPainter::save();
}

/*!
  Restores the painter (see QPainter::restore). Since QCPPainter adds some new internal state to
  QPainter, the save/restore functions are reimplemented to also save/restore those members.
  
  \note this function hides the non-virtual base class implementation.
  
  \see save
*/
void QCPPainter::restore()
{
  if (!mAntialiasingStack.isEmpty())
    mIsAntialiasing = mAntialiasingStack.pop();
  else
    qDebug() << Q_FUNC_INFO << "Unbalanced save/restore";
  QPainter::restore();
}

/*!
  Changes the pen width to 1 if it currently is 0. This function is called in the \ref setPen
  overrides when the \ref pmNonCosmetic mode is set.
*/
void QCPPainter::makeNonCosmetic()
{
  if (qFuzzyIsNull(pen().widthF()))
  {
    QPen p = pen();
    p.setWidth(1);
    QPainter::setPen(p);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPScatterStyle
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPScatterStyle
  \brief Represents the visual appearance of scatter points
  
  This class holds information about shape, color and size of scatter points. In plottables like
  QCPGraph it is used to store how scatter points shall be drawn. For example, \ref
  QCPGraph::setScatterStyle takes a QCPScatterStyle instance.
  
  A scatter style consists of a shape (\ref setShape), a line color (\ref setPen) and possibly a
  fill (\ref setBrush), if the shape provides a fillable area. Further, the size of the shape can
  be controlled with \ref setSize.

  \section QCPScatterStyle-defining Specifying a scatter style
  
  You can set all these configurations either by calling the respective functions on an instance:
  \code
  QCPScatterStyle myScatter;
  myScatter.setShape(QCPScatterStyle::ssCircle);
  myScatter.setPen(Qt::blue);
  myScatter.setBrush(Qt::white);
  myScatter.setSize(5);
  customPlot->graph(0)->setScatterStyle(myScatter);
  \endcode
  
  Or you can use one of the various constructors that take different parameter combinations, making
  it easy to specify a scatter style in a single call, like so:
  \code
  customPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::blue, Qt::white, 5));
  \endcode
  
  \section QCPScatterStyle-undefinedpen Leaving the color/pen up to the plottable
  
  There are two constructors which leave the pen undefined: \ref QCPScatterStyle() and \ref
  QCPScatterStyle(ScatterShape shape, double size). If those constructors are used, a call to \ref
  isPenDefined will return false. It leads to scatter points that inherit the pen from the
  plottable that uses the scatter style. Thus, if such a scatter style is passed to QCPGraph, the line
  color of the graph (\ref QCPGraph::setPen) will be used by the scatter points. This makes
  it very convenient to set up typical scatter settings:
  
  \code
  customPlot->graph(0)->setScatterStyle(QCPScatterStyle::ssPlus);
  \endcode

  Notice that it wasn't even necessary to explicitly call a QCPScatterStyle constructor. This works
  because QCPScatterStyle provides a constructor that can transform a \ref ScatterShape directly
  into a QCPScatterStyle instance (that's the \ref QCPScatterStyle(ScatterShape shape, double size)
  constructor with a default for \a size). In those cases, C++ allows directly supplying a \ref
  ScatterShape, where actually a QCPScatterStyle is expected.
  
  \section QCPScatterStyle-custompath-and-pixmap Custom shapes and pixmaps
  
  QCPScatterStyle supports drawing custom shapes and arbitrary pixmaps as scatter points.

  For custom shapes, you can provide a QPainterPath with the desired shape to the \ref
  setCustomPath function or call the constructor that takes a painter path. The scatter shape will
  automatically be set to \ref ssCustom.
  
  For pixmaps, you call \ref setPixmap with the desired QPixmap. Alternatively you can use the
  constructor that takes a QPixmap. The scatter shape will automatically be set to \ref ssPixmap.
  Note that \ref setSize does not influence the appearance of the pixmap.
*/

/* start documentation of inline functions */

/*! \fn bool QCPScatterStyle::isNone() const
  
  Returns whether the scatter shape is \ref ssNone.
  
  \see setShape
*/

/*! \fn bool QCPScatterStyle::isPenDefined() const
  
  Returns whether a pen has been defined for this scatter style.
  
  The pen is undefined if a constructor is called that does not carry \a pen as parameter. Those are
  \ref QCPScatterStyle() and \ref QCPScatterStyle(ScatterShape shape, double size). If the pen is
  left undefined, the scatter color will be inherited from the plottable that uses this scatter
  style.
  
  \see setPen
*/

/* end documentation of inline functions */

/*!
  Creates a new QCPScatterStyle instance with size set to 6. No shape, pen or brush is defined.
  
  Since the pen is undefined (\ref isPenDefined returns false), the scatter color will be inherited
  from the plottable that uses this scatter style.
*/
QCPScatterStyle::QCPScatterStyle() :
  mSize(6),
  mShape(ssNone),
  mPen(Qt::NoPen),
  mBrush(Qt::NoBrush),
  mPenDefined(false)
{
}

/*!
  Creates a new QCPScatterStyle instance with shape set to \a shape and size to \a size. No pen or
  brush is defined.
  
  Since the pen is undefined (\ref isPenDefined returns false), the scatter color will be inherited
  from the plottable that uses this scatter style.
*/
QCPScatterStyle::QCPScatterStyle(ScatterShape shape, double size) :
  mSize(size),
  mShape(shape),
  mPen(Qt::NoPen),
  mBrush(Qt::NoBrush),
  mPenDefined(false)
{
}

/*!
  Creates a new QCPScatterStyle instance with shape set to \a shape, the pen color set to \a color,
  and size to \a size. No brush is defined, i.e. the scatter point will not be filled.
*/
QCPScatterStyle::QCPScatterStyle(ScatterShape shape, const QColor &color, double size) :
  mSize(size),
  mShape(shape),
  mPen(QPen(color)),
  mBrush(Qt::NoBrush),
  mPenDefined(true)
{
}

/*!
  Creates a new QCPScatterStyle instance with shape set to \a shape, the pen color set to \a color,
  the brush color to \a fill (with a solid pattern), and size to \a size.
*/
QCPScatterStyle::QCPScatterStyle(ScatterShape shape, const QColor &color, const QColor &fill, double size) :
  mSize(size),
  mShape(shape),
  mPen(QPen(color)),
  mBrush(QBrush(fill)),
  mPenDefined(true)
{
}

/*!
  Creates a new QCPScatterStyle instance with shape set to \a shape, the pen set to \a pen, the
  brush to \a brush, and size to \a size.
  
  \warning In some cases it might be tempting to directly use a pen style like <tt>Qt::NoPen</tt> as \a pen
  and a color like <tt>Qt::blue</tt> as \a brush. Notice however, that the corresponding call\n
  <tt>QCPScatterStyle(QCPScatterShape::ssCircle, Qt::NoPen, Qt::blue, 5)</tt>\n
  doesn't necessarily lead C++ to use this constructor in some cases, but might mistake
  <tt>Qt::NoPen</tt> for a QColor and use the
  \ref QCPScatterStyle(ScatterShape shape, const QColor &color, const QColor &fill, double size)
  constructor instead (which will lead to an unexpected look of the scatter points). To prevent
  this, be more explicit with the parameter types. For example, use <tt>QBrush(Qt::blue)</tt>
  instead of just <tt>Qt::blue</tt>, to clearly point out to the compiler that this constructor is
  wanted.
*/
QCPScatterStyle::QCPScatterStyle(ScatterShape shape, const QPen &pen, const QBrush &brush, double size) :
  mSize(size),
  mShape(shape),
  mPen(pen),
  mBrush(brush),
  mPenDefined(pen.style() != Qt::NoPen)
{
}

/*!
  Creates a new QCPScatterStyle instance which will show the specified \a pixmap. The scatter shape
  is set to \ref ssPixmap.
*/
QCPScatterStyle::QCPScatterStyle(const QPixmap &pixmap) :
  mSize(5),
  mShape(ssPixmap),
  mPen(Qt::NoPen),
  mBrush(Qt::NoBrush),
  mPixmap(pixmap),
  mPenDefined(false)
{
}

/*!
  Creates a new QCPScatterStyle instance with a custom shape that is defined via \a customPath. The
  scatter shape is set to \ref ssCustom.
  
  The custom shape line will be drawn with \a pen and filled with \a brush. The size has a slightly
  different meaning than for built-in scatter points: The custom path will be drawn scaled by a
  factor of \a size/6.0. Since the default \a size is 6, the custom path will appear at a its
  natural size by default. To double the size of the path for example, set \a size to 12.
*/
QCPScatterStyle::QCPScatterStyle(const QPainterPath &customPath, const QPen &pen, const QBrush &brush, double size) :
  mSize(size),
  mShape(ssCustom),
  mPen(pen),
  mBrush(brush),
  mCustomPath(customPath),
  mPenDefined(false)
{
}

/*!
  Sets the size (pixel diameter) of the drawn scatter points to \a size.
  
  \see setShape
*/
void QCPScatterStyle::setSize(double size)
{
  mSize = size;
}

/*!
  Sets the shape to \a shape.
  
  Note that the calls \ref setPixmap and \ref setCustomPath automatically set the shape to \ref
  ssPixmap and \ref ssCustom, respectively.
  
  \see setSize
*/
void QCPScatterStyle::setShape(QCPScatterStyle::ScatterShape shape)
{
  mShape = shape;
}

/*!
  Sets the pen that will be used to draw scatter points to \a pen.
  
  If the pen was previously undefined (see \ref isPenDefined), the pen is considered defined after
  a call to this function, even if \a pen is <tt>Qt::NoPen</tt>.
  
  \see setBrush
*/
void QCPScatterStyle::setPen(const QPen &pen)
{
  mPenDefined = true;
  mPen = pen;
}

/*!
  Sets the brush that will be used to fill scatter points to \a brush. Note that not all scatter
  shapes have fillable areas. For example, \ref ssPlus does not while \ref ssCircle does.
  
  \see setPen
*/
void QCPScatterStyle::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  Sets the pixmap that will be drawn as scatter point to \a pixmap.
  
  Note that \ref setSize does not influence the appearance of the pixmap.
  
  The scatter shape is automatically set to \ref ssPixmap.
*/
void QCPScatterStyle::setPixmap(const QPixmap &pixmap)
{
  setShape(ssPixmap);
  mPixmap = pixmap;
}

/*!
  Sets the custom shape that will be drawn as scatter point to \a customPath.
  
  The scatter shape is automatically set to \ref ssCustom.
*/
void QCPScatterStyle::setCustomPath(const QPainterPath &customPath)
{
  setShape(ssCustom);
  mCustomPath = customPath;
}

/*!
  Applies the pen and the brush of this scatter style to \a painter. If this scatter style has an
  undefined pen (\ref isPenDefined), sets the pen of \a painter to \a defaultPen instead.
  
  This function is used by plottables (or any class that wants to draw scatters) just before a
  number of scatters with this style shall be drawn with the \a painter.
  
  \see drawShape
*/
void QCPScatterStyle::applyTo(QCPPainter *painter, const QPen &defaultPen) const
{
  painter->setPen(mPenDefined ? mPen : defaultPen);
  painter->setBrush(mBrush);
}

/*!
  Draws the scatter shape with \a painter at position \a pos.
  
  This function does not modify the pen or the brush on the painter, as \ref applyTo is meant to be
  called before scatter points are drawn with \ref drawShape.
  
  \see applyTo
*/
void QCPScatterStyle::drawShape(QCPPainter *painter, QPointF pos) const
{
  drawShape(painter, pos.x(), pos.y());
}

/*! \overload
  Draws the scatter shape with \a painter at position \a x and \a y.
*/
void QCPScatterStyle::drawShape(QCPPainter *painter, double x, double y) const
{
  double w = mSize/2.0;
  switch (mShape)
  {
    case ssNone: break;
    case ssDot:
    {
      painter->drawLine(QPointF(x, y), QPointF(x+0.0001, y));
      break;
    }
    case ssCross:
    {
      painter->drawLine(QLineF(x-w, y-w, x+w, y+w));
      painter->drawLine(QLineF(x-w, y+w, x+w, y-w));
      break;
    }
    case ssPlus:
    {
      painter->drawLine(QLineF(x-w,   y, x+w,   y));
      painter->drawLine(QLineF(  x, y+w,   x, y-w));
      break;
    }
    case ssCircle:
    {
      painter->drawEllipse(QPointF(x , y), w, w);
      break;
    }
    case ssDisc:
    {
      QBrush b = painter->brush();
      painter->setBrush(painter->pen().color());
      painter->drawEllipse(QPointF(x , y), w, w);
      painter->setBrush(b);
      break;
    }
    case ssSquare:
    {
      painter->drawRect(QRectF(x-w, y-w, mSize, mSize));
      break;
    }
    case ssDiamond:
    {
      painter->drawLine(QLineF(x-w,   y,   x, y-w));
      painter->drawLine(QLineF(  x, y-w, x+w,   y));
      painter->drawLine(QLineF(x+w,   y,   x, y+w));
      painter->drawLine(QLineF(  x, y+w, x-w,   y));
      break;
    }
    case ssStar:
    {
      painter->drawLine(QLineF(x-w,   y, x+w,   y));
      painter->drawLine(QLineF(  x, y+w,   x, y-w));
      painter->drawLine(QLineF(x-w*0.707, y-w*0.707, x+w*0.707, y+w*0.707));
      painter->drawLine(QLineF(x-w*0.707, y+w*0.707, x+w*0.707, y-w*0.707));
      break;
    }
    case ssTriangle:
    {
       painter->drawLine(QLineF(x-w, y+0.755*w, x+w, y+0.755*w));
       painter->drawLine(QLineF(x+w, y+0.755*w,   x, y-0.977*w));
       painter->drawLine(QLineF(  x, y-0.977*w, x-w, y+0.755*w));
      break;
    }
    case ssTriangleInverted:
    {
       painter->drawLine(QLineF(x-w, y-0.755*w, x+w, y-0.755*w));
       painter->drawLine(QLineF(x+w, y-0.755*w,   x, y+0.977*w));
       painter->drawLine(QLineF(  x, y+0.977*w, x-w, y-0.755*w));
      break;
    }
    case ssCrossSquare:
    {
       painter->drawLine(QLineF(x-w, y-w, x+w*0.95, y+w*0.95));
       painter->drawLine(QLineF(x-w, y+w*0.95, x+w*0.95, y-w));
       painter->drawRect(QRectF(x-w, y-w, mSize, mSize));
      break;
    }
    case ssPlusSquare:
    {
       painter->drawLine(QLineF(x-w,   y, x+w*0.95,   y));
       painter->drawLine(QLineF(  x, y+w,        x, y-w));
       painter->drawRect(QRectF(x-w, y-w, mSize, mSize));
      break;
    }
    case ssCrossCircle:
    {
       painter->drawLine(QLineF(x-w*0.707, y-w*0.707, x+w*0.670, y+w*0.670));
       painter->drawLine(QLineF(x-w*0.707, y+w*0.670, x+w*0.670, y-w*0.707));
       painter->drawEllipse(QPointF(x, y), w, w);
      break;
    }
    case ssPlusCircle:
    {
       painter->drawLine(QLineF(x-w,   y, x+w,   y));
       painter->drawLine(QLineF(  x, y+w,   x, y-w));
       painter->drawEllipse(QPointF(x, y), w, w);
      break;
    }
    case ssPeace:
    {
       painter->drawLine(QLineF(x, y-w,         x,       y+w));
       painter->drawLine(QLineF(x,   y, x-w*0.707, y+w*0.707));
       painter->drawLine(QLineF(x,   y, x+w*0.707, y+w*0.707));
       painter->drawEllipse(QPointF(x, y), w, w);
      break;
    }
    case ssPixmap:
    {
      painter->drawPixmap(x-mPixmap.width()*0.5, y-mPixmap.height()*0.5, mPixmap);
      break;
    }
    case ssCustom:
    {
      QTransform oldTransform = painter->transform();
      painter->translate(x, y);
      painter->scale(mSize/6.0, mSize/6.0);
      painter->drawPath(mCustomPath);
      painter->setTransform(oldTransform);
      break;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLayer
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLayer
  \brief A layer that may contain objects, to control the rendering order
  
  The Layering system of QCustomPlot is the mechanism to control the rendering order of the
  elements inside the plot.
  
  It is based on the two classes QCPLayer and QCPLayerable. QCustomPlot holds an ordered list of
  one or more instances of QCPLayer (see QCustomPlot::addLayer, QCustomPlot::layer,
  QCustomPlot::moveLayer, etc.). When replotting, QCustomPlot goes through the list of layers
  bottom to top and successively draws the layerables of the layers.
  
  A QCPLayer contains an ordered list of QCPLayerable instances. QCPLayerable is an abstract base
  class from which almost all visible objects derive, like axes, grids, graphs, items, etc.
  
  Initially, QCustomPlot has five layers: "background", "grid", "main", "axes" and "legend" (in
  that order). The top two layers "axes" and "legend" contain the default axes and legend, so they
  will be drawn on top. In the middle, there is the "main" layer. It is initially empty and set as
  the current layer (see QCustomPlot::setCurrentLayer). This means, all new plottables, items etc.
  are created on this layer by default. Then comes the "grid" layer which contains the QCPGrid
  instances (which belong tightly to QCPAxis, see \ref QCPAxis::grid). The Axis rect background
  shall be drawn behind everything else, thus the default QCPAxisRect instance is placed on the
  "background" layer. Of course, the layer affiliation of the individual objects can be changed as
  required (\ref QCPLayerable::setLayer).
  
  Controlling the ordering of objects is easy: Create a new layer in the position you want it to
  be, e.g. above "main", with QCustomPlot::addLayer. Then set the current layer with
  QCustomPlot::setCurrentLayer to that new layer and finally create the objects normally. They will
  be placed on the new layer automatically, due to the current layer setting. Alternatively you
  could have also ignored the current layer setting and just moved the objects with
  QCPLayerable::setLayer to the desired layer after creating them.
  
  It is also possible to move whole layers. For example, If you want the grid to be shown in front
  of all plottables/items on the "main" layer, just move it above "main" with
  QCustomPlot::moveLayer.
  
  The rendering order within one layer is simply by order of creation or insertion. The item
  created last (or added last to the layer), is drawn on top of all other objects on that layer.
  
  When a layer is deleted, the objects on it are not deleted with it, but fall on the layer below
  the deleted layer, see QCustomPlot::removeLayer.
*/

/* start documentation of inline functions */

/*! \fn QList<QCPLayerable*> QCPLayer::children() const
  
  Returns a list of all layerables on this layer. The order corresponds to the rendering order:
  layerables with higher indices are drawn above layerables with lower indices.
*/

/*! \fn int QCPLayer::index() const
  
  Returns the index this layer has in the QCustomPlot. The index is the integer number by which this layer can be
  accessed via \ref QCustomPlot::layer.
  
  Layers with higher indices will be drawn above layers with lower indices.
*/

/* end documentation of inline functions */

/*!
  Creates a new QCPLayer instance.
  
  Normally you shouldn't directly instantiate layers, use \ref QCustomPlot::addLayer instead.
  
  \warning It is not checked that \a layerName is actually a unique layer name in \a parentPlot.
  This check is only performed by \ref QCustomPlot::addLayer.
*/
QCPLayer::QCPLayer(QCustomPlot *parentPlot, const QString &layerName) :
  QObject(parentPlot),
  mParentPlot(parentPlot),
  mName(layerName),
  mIndex(-1) // will be set to a proper value by the QCustomPlot layer creation function
{
  // Note: no need to make sure layerName is unique, because layer
  // management is done with QCustomPlot functions.
}

QCPLayer::~QCPLayer()
{
  // If child layerables are still on this layer, detach them, so they don't try to reach back to this
  // then invalid layer once they get deleted/moved themselves. This only happens when layers are deleted
  // directly, like in the QCustomPlot destructor. (The regular layer removal procedure for the user is to
  // call QCustomPlot::removeLayer, which moves all layerables off this layer before deleting it.)
  
  while (!mChildren.isEmpty())
    mChildren.last()->setLayer(0); // removes itself from mChildren via removeChild()
  
  if (mParentPlot->currentLayer() == this)
    qDebug() << Q_FUNC_INFO << "The parent plot's mCurrentLayer will be a dangling pointer. Should have been set to a valid layer or 0 beforehand.";
}

/*! \internal
  
  Adds the \a layerable to the list of this layer. If \a prepend is set to true, the layerable will
  be prepended to the list, i.e. be drawn beneath the other layerables already in the list.
  
  This function does not change the \a mLayer member of \a layerable to this layer. (Use
  QCPLayerable::setLayer to change the layer of an object, not this function.)
  
  \see removeChild
*/
void QCPLayer::addChild(QCPLayerable *layerable, bool prepend)
{
  if (!mChildren.contains(layerable))
  {
    if (prepend)
      mChildren.prepend(layerable);
    else
      mChildren.append(layerable);
  } else
    qDebug() << Q_FUNC_INFO << "layerable is already child of this layer" << reinterpret_cast<quintptr>(layerable);
}

/*! \internal
  
  Removes the \a layerable from the list of this layer.
  
  This function does not change the \a mLayer member of \a layerable. (Use QCPLayerable::setLayer
  to change the layer of an object, not this function.)
  
  \see addChild
*/
void QCPLayer::removeChild(QCPLayerable *layerable)
{
  if (!mChildren.removeOne(layerable))
    qDebug() << Q_FUNC_INFO << "layerable is not child of this layer" << reinterpret_cast<quintptr>(layerable);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLayerable
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLayerable
  \brief Base class for all drawable objects
  
  This is the abstract base class most visible objects derive from, e.g. plottables, axes, grid
  etc.

  Every layerable is on a layer (QCPLayer) which allows controlling the rendering order by stacking
  the layers accordingly.
  
  For details about the layering mechanism, see the QCPLayer documentation.
*/

/* start documentation of inline functions */

/*! \fn QCPLayerable *QCPLayerable::parentLayerable() const
 
  Returns the parent layerable of this layerable. The parent layerable is used to provide
  visibility hierarchies in conjunction with the method \ref realVisibility. This way, layerables
  only get drawn if their parent layerables are visible, too.
  
  Note that a parent layerable is not necessarily also the QObject parent for memory management.
  Further, a layerable doesn't always have a parent layerable, so this function may return 0.
  
  A parent layerable is set implicitly with when placed inside layout elements and doesn't need to be
  set manually by the user.
*/

/* end documentation of inline functions */
/* start documentation of pure virtual functions */

/*! \fn virtual void QCPLayerable::applyDefaultAntialiasingHint(QCPPainter *painter) const = 0
  \internal
  
  This function applies the default antialiasing setting to the specified \a painter, using the
  function \ref applyAntialiasingHint. It is the antialiasing state the painter is put in, when
  \ref draw is called on the layerable. If the layerable has multiple entities whose antialiasing
  setting may be specified individually, this function should set the antialiasing state of the
  most prominent entity. In this case however, the \ref draw function usually calls the specialized
  versions of this function before drawing each entity, effectively overriding the setting of the
  default antialiasing hint.
  
  <b>First example:</b> QCPGraph has multiple entities that have an antialiasing setting: The graph
  line, fills, scatters and error bars. Those can be configured via QCPGraph::setAntialiased,
  QCPGraph::setAntialiasedFill, QCPGraph::setAntialiasedScatters etc. Consequently, there isn't
  only the QCPGraph::applyDefaultAntialiasingHint function (which corresponds to the graph line's
  antialiasing), but specialized ones like QCPGraph::applyFillAntialiasingHint and
  QCPGraph::applyScattersAntialiasingHint. So before drawing one of those entities, QCPGraph::draw
  calls the respective specialized applyAntialiasingHint function.
  
  <b>Second example:</b> QCPItemLine consists only of a line so there is only one antialiasing
  setting which can be controlled with QCPItemLine::setAntialiased. (This function is inherited by
  all layerables. The specialized functions, as seen on QCPGraph, must be added explicitly to the
  respective layerable subclass.) Consequently it only has the normal
  QCPItemLine::applyDefaultAntialiasingHint. The \ref QCPItemLine::draw function doesn't need to
  care about setting any antialiasing states, because the default antialiasing hint is already set
  on the painter when the \ref draw function is called, and that's the state it wants to draw the
  line with.
*/

/*! \fn virtual void QCPLayerable::draw(QCPPainter *painter) const = 0
  \internal
  
  This function draws the layerable with the specified \a painter. It is only called by
  QCustomPlot, if the layerable is visible (\ref setVisible).
  
  Before this function is called, the painter's antialiasing state is set via \ref
  applyDefaultAntialiasingHint, see the documentation there. Further, the clipping rectangle was
  set to \ref clipRect.
*/

/* end documentation of pure virtual functions */

/*!
  Creates a new QCPLayerable instance.
  
  Since QCPLayerable is an abstract base class, it can't be instantiated directly. Use one of the
  derived classes.
  
  If \a plot is provided, it automatically places itself on the layer named \a targetLayer. If \a
  targetLayer is an empty string, it places itself on the current layer of the plot (see \ref
  QCustomPlot::setCurrentLayer).
  
  It is possible to provide 0 as \a plot. In that case, you should assign a parent plot at a later
  time with \ref initializeParentPlot.
  
  The layerable's parent layerable is set to \a parentLayerable, if provided. Direct layerable parents
  are mainly used to control visibility in a hierarchy of layerables. This means a layerable is
  only drawn, if all its ancestor layerables are also visible. Note that \a parentLayerable does
  not become the QObject-parent (for memory management) of this layerable, \a plot does.
*/
QCPLayerable::QCPLayerable(QCustomPlot *plot, QString targetLayer, QCPLayerable *parentLayerable) :
  QObject(plot),
  mVisible(true),
  mParentPlot(plot),
  mParentLayerable(parentLayerable),
  mLayer(0),
  mAntialiased(true)
{
  if (mParentPlot)
  {
    if (targetLayer.isEmpty())
      setLayer(mParentPlot->currentLayer());
    else if (!setLayer(targetLayer))
      qDebug() << Q_FUNC_INFO << "setting QCPlayerable initial layer to" << targetLayer << "failed.";
  }
}

QCPLayerable::~QCPLayerable()
{
  if (mLayer)
  {
    mLayer->removeChild(this);
    mLayer = 0;
  }
}

/*!
  Sets the visibility of this layerable object. If an object is not visible, it will not be drawn
  on the QCustomPlot surface, and user interaction with it (e.g. click and selection) is not
  possible.
*/
void QCPLayerable::setVisible(bool on)
{
  mVisible = on;
}

/*!
  Sets the \a layer of this layerable object. The object will be placed on top of the other objects
  already on \a layer.
  
  Returns true on success, i.e. if \a layer is a valid layer.
*/
bool QCPLayerable::setLayer(QCPLayer *layer)
{
  return moveToLayer(layer, false);
}

/*! \overload
  Sets the layer of this layerable object by name
  
  Returns true on success, i.e. if \a layerName is a valid layer name.
*/
bool QCPLayerable::setLayer(const QString &layerName)
{
  if (!mParentPlot)
  {
    qDebug() << Q_FUNC_INFO << "no parent QCustomPlot set";
    return false;
  }
  if (QCPLayer *layer = mParentPlot->layer(layerName))
  {
    return setLayer(layer);
  } else
  {
    qDebug() << Q_FUNC_INFO << "there is no layer with name" << layerName;
    return false;
  }
}

/*!
  Sets whether this object will be drawn antialiased or not.
  
  Note that antialiasing settings may be overridden by QCustomPlot::setAntialiasedElements and
  QCustomPlot::setNotAntialiasedElements.
*/
void QCPLayerable::setAntialiased(bool enabled)
{
  mAntialiased = enabled;
}

/*!
  Returns whether this layerable is visible, taking possible direct layerable parent visibility
  into account. This is the method that is consulted to decide whether a layerable shall be drawn
  or not.
  
  If this layerable has a direct layerable parent (usually set via hierarchies implemented in
  subclasses, like in the case of QCPLayoutElement), this function returns true only if this
  layerable has its visibility set to true and the parent layerable's \ref realVisibility returns
  true.
  
  If this layerable doesn't have a direct layerable parent, returns the state of this layerable's
  visibility.
*/
bool QCPLayerable::realVisibility() const
{
  return mVisible && (!mParentLayerable || mParentLayerable.data()->realVisibility());
}

/*!
  This function is used to decide whether a click hits a layerable object or not.

  \a pos is a point in pixel coordinates on the QCustomPlot surface. This function returns the
  shortest pixel distance of this point to the object. If the object is either invisible or the
  distance couldn't be determined, -1.0 is returned. Further, if \a onlySelectable is true and the
  object is not selectable, -1.0 is returned, too.

  If the item is represented not by single lines but by an area like QCPItemRect or QCPItemText, a
  click inside the area returns a constant value greater zero (typically the selectionTolerance of
  the parent QCustomPlot multiplied by 0.99). If the click lies outside the area, this function
  returns -1.0.
  
  Providing a constant value for area objects allows selecting line objects even when they are
  obscured by such area objects, by clicking close to the lines (i.e. closer than
  0.99*selectionTolerance).
  
  The actual setting of the selection state is not done by this function. This is handled by the
  parent QCustomPlot when the mouseReleaseEvent occurs, and the finally selected object is notified
  via the selectEvent/deselectEvent methods.
  
  \a details is an optional output parameter. Every layerable subclass may place any information
  in \a details. This information will be passed to \ref selectEvent when the parent QCustomPlot
  decides on the basis of this selectTest call, that the object was successfully selected. The
  subsequent call to \ref selectEvent will carry the \a details. This is useful for multi-part
  objects (like QCPAxis). This way, a possibly complex calculation to decide which part was clicked
  is only done once in \ref selectTest. The result (i.e. the actually clicked part) can then be
  placed in \a details. So in the subsequent \ref selectEvent, the decision which part was
  selected doesn't have to be done a second time for a single selection operation.
  
  You may pass 0 as \a details to indicate that you are not interested in those selection details.
  
  \see selectEvent, deselectEvent, QCustomPlot::setInteractions
*/
double QCPLayerable::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(pos)
  Q_UNUSED(onlySelectable)
  Q_UNUSED(details)
  return -1.0;
}

/*! \internal
  
  Sets the parent plot of this layerable. Use this function once to set the parent plot if you have
  passed 0 in the constructor. It can not be used to move a layerable from one QCustomPlot to
  another one.
  
  Note that, unlike when passing a non-null parent plot in the constructor, this function does not
  make \a parentPlot the QObject-parent of this layerable. If you want this, call
  QObject::setParent(\a parentPlot) in addition to this function.
  
  Further, you will probably want to set a layer (\ref setLayer) after calling this function, to
  make the layerable appear on the QCustomPlot.
  
  The parent plot change will be propagated to subclasses via a call to \ref parentPlotInitialized
  so they can react accordingly (e.g. also initialize the parent plot of child layerables, like
  QCPLayout does).
*/
void QCPLayerable::initializeParentPlot(QCustomPlot *parentPlot)
{
  if (mParentPlot)
  {
    qDebug() << Q_FUNC_INFO << "called with mParentPlot already initialized";
    return;
  }
  
  if (!parentPlot)
    qDebug() << Q_FUNC_INFO << "called with parentPlot zero";
  
  mParentPlot = parentPlot;
  parentPlotInitialized(mParentPlot);
}

/*! \internal
  
  Sets the parent layerable of this layerable to \a parentLayerable. Note that \a parentLayerable does not
  become the QObject-parent (for memory management) of this layerable.
  
  The parent layerable has influence on the return value of the \ref realVisibility method. Only
  layerables with a fully visible parent tree will return true for \ref realVisibility, and thus be
  drawn.
  
  \see realVisibility
*/
void QCPLayerable::setParentLayerable(QCPLayerable *parentLayerable)
{
  mParentLayerable = parentLayerable;
}

/*! \internal
  
  Moves this layerable object to \a layer. If \a prepend is true, this object will be prepended to
  the new layer's list, i.e. it will be drawn below the objects already on the layer. If it is
  false, the object will be appended.
  
  Returns true on success, i.e. if \a layer is a valid layer.
*/
bool QCPLayerable::moveToLayer(QCPLayer *layer, bool prepend)
{
  if (layer && !mParentPlot)
  {
    qDebug() << Q_FUNC_INFO << "no parent QCustomPlot set";
    return false;
  }
  if (layer && layer->parentPlot() != mParentPlot)
  {
    qDebug() << Q_FUNC_INFO << "layer" << layer->name() << "is not in same QCustomPlot as this layerable";
    return false;
  }
  
  if (mLayer)
    mLayer->removeChild(this);
  mLayer = layer;
  if (mLayer)
    mLayer->addChild(this, prepend);
  return true;
}

/*! \internal

  Sets the QCPainter::setAntialiasing state on the provided \a painter, depending on the \a
  localAntialiased value as well as the overrides \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements. Which override enum this function takes into account is
  controlled via \a overrideElement.
*/
void QCPLayerable::applyAntialiasingHint(QCPPainter *painter, bool localAntialiased, QCP::AntialiasedElement overrideElement) const
{
  if (mParentPlot && mParentPlot->notAntialiasedElements().testFlag(overrideElement))
    painter->setAntialiasing(false);
  else if (mParentPlot && mParentPlot->antialiasedElements().testFlag(overrideElement))
    painter->setAntialiasing(true);
  else
    painter->setAntialiasing(localAntialiased);
}

/*! \internal

  This function is called by \ref initializeParentPlot, to allow subclasses to react on the setting
  of a parent plot. This is the case when 0 was passed as parent plot in the constructor, and the
  parent plot is set at a later time.
  
  For example, QCPLayoutElement/QCPLayout hierarchies may be created independently of any
  QCustomPlot at first. When they are then added to a layout inside the QCustomPlot, the top level
  element of the hierarchy gets its parent plot initialized with \ref initializeParentPlot. To
  propagate the parent plot to all the children of the hierarchy, the top level element then uses
  this function to pass the parent plot on to its child elements.
  
  The default implementation does nothing.
  
  \see initializeParentPlot
*/
void QCPLayerable::parentPlotInitialized(QCustomPlot *parentPlot)
{
   Q_UNUSED(parentPlot)
}

/*! \internal

  Returns the selection category this layerable shall belong to. The selection category is used in
  conjunction with \ref QCustomPlot::setInteractions to control which objects are selectable and
  which aren't.
  
  Subclasses that don't fit any of the normal \ref QCP::Interaction values can use \ref
  QCP::iSelectOther. This is what the default implementation returns.
  
  \see QCustomPlot::setInteractions
*/
QCP::Interaction QCPLayerable::selectionCategory() const
{
  return QCP::iSelectOther;
}

/*! \internal
  
  Returns the clipping rectangle of this layerable object. By default, this is the viewport of the
  parent QCustomPlot. Specific subclasses may reimplement this function to provide different
  clipping rects.
  
  The returned clipping rect is set on the painter before the draw function of the respective
  object is called.
*/
QRect QCPLayerable::clipRect() const
{
  if (mParentPlot)
    return mParentPlot->viewport();
  else
    return QRect();
}

/*! \internal
  
  This event is called when the layerable shall be selected, as a consequence of a click by the
  user. Subclasses should react to it by setting their selection state appropriately. The default
  implementation does nothing.
  
  \a event is the mouse event that caused the selection. \a additive indicates, whether the user
  was holding the multi-select-modifier while performing the selection (see \ref
  QCustomPlot::setMultiSelectModifier). if \a additive is true, the selection state must be toggled
  (i.e. become selected when unselected and unselected when selected).
  
  Every selectEvent is preceded by a call to \ref selectTest, which has returned positively (i.e.
  returned a value greater than 0 and less than the selection tolerance of the parent QCustomPlot).
  The \a details data you output from \ref selectTest is feeded back via \a details here. You may
  use it to transport any kind of information from the selectTest to the possibly subsequent
  selectEvent. Usually \a details is used to transfer which part was clicked, if it is a layerable
  that has multiple individually selectable parts (like QCPAxis). This way selectEvent doesn't need
  to do the calculation again to find out which part was actually clicked.
  
  \a selectionStateChanged is an output parameter. If the pointer is non-null, this function must
  set the value either to true or false, depending on whether the selection state of this layerable
  was actually changed. For layerables that only are selectable as a whole and not in parts, this
  is simple: if \a additive is true, \a selectionStateChanged must also be set to true, because the
  selection toggles. If \a additive is false, \a selectionStateChanged is only set to true, if the
  layerable was previously unselected and now is switched to the selected state.
  
  \see selectTest, deselectEvent
*/
void QCPLayerable::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  Q_UNUSED(additive)
  Q_UNUSED(details)
  Q_UNUSED(selectionStateChanged)
}

/*! \internal
  
  This event is called when the layerable shall be deselected, either as consequence of a user
  interaction or a call to \ref QCustomPlot::deselectAll. Subclasses should react to it by
  unsetting their selection appropriately.
  
  just as in \ref selectEvent, the output parameter \a selectionStateChanged (if non-null), must
  return true or false when the selection state of this layerable has changed or not changed,
  respectively.
  
  \see selectTest, selectEvent
*/
void QCPLayerable::deselectEvent(bool *selectionStateChanged)
{
  Q_UNUSED(selectionStateChanged)
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPRange
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPRange
  \brief Represents the range an axis is encompassing.
  
  contains a \a lower and \a upper double value and provides convenience input, output and
  modification functions.
  
  \see QCPAxis::setRange
*/

/*! 
  Minimum range size (\a upper - \a lower) the range changing functions will accept. Smaller
  intervals would cause errors due to the 11-bit exponent of double precision numbers,
  corresponding to a minimum magnitude of roughly 1e-308.
  \see validRange, maxRange
*/
const double QCPRange::minRange = 1e-280;

/*! 
  Maximum values (negative and positive) the range will accept in range-changing functions.
  Larger absolute values would cause errors due to the 11-bit exponent of double precision numbers,
  corresponding to a maximum magnitude of roughly 1e308.
  Since the number of planck-volumes in the entire visible universe is only ~1e183, this should
  be enough.
  \see validRange, minRange
*/
const double QCPRange::maxRange = 1e250;

/*! 
  Constructs a range with \a lower and \a upper set to zero.
*/
QCPRange::QCPRange() :
  lower(0),
  upper(0)
{
}

/*! \overload
  Constructs a range with the specified \a lower and \a upper values.
*/
QCPRange::QCPRange(double lower, double upper) :
  lower(lower),
  upper(upper)
{
  normalize();
}

/*! 
  Returns the size of the range, i.e. \a upper-\a lower
*/
double QCPRange::size() const
{
  return upper-lower;
}

/*! 
  Returns the center of the range, i.e. (\a upper+\a lower)*0.5
*/
double QCPRange::center() const
{
  return (upper+lower)*0.5;
}

/*! 
  Makes sure \a lower is numerically smaller than \a upper. If this is not the case, the values
  are swapped.
*/
void QCPRange::normalize()
{
  if (lower > upper)
    qSwap(lower, upper);
}

/*! 
  Expands this range such that \a otherRange is contained in the new range. It is assumed that both
  this range and \a otherRange are normalized (see \ref normalize).
  
  If \a otherRange is already inside the current range, this function does nothing.
  
  \see expanded
*/
void QCPRange::expand(const QCPRange &otherRange)
{
  if (lower > otherRange.lower)
    lower = otherRange.lower;
  if (upper < otherRange.upper)
    upper = otherRange.upper;
}


/*! 
  Returns an expanded range that contains this and \a otherRange. It is assumed that both this
  range and \a otherRange are normalized (see \ref normalize).
  
  \see expand
*/
QCPRange QCPRange::expanded(const QCPRange &otherRange) const
{
  QCPRange result = *this;
  result.expand(otherRange);
  return result;
}

/*! 
  Returns a sanitized version of the range. Sanitized means for logarithmic scales, that
  the range won't span the positive and negative sign domain, i.e. contain zero. Further
  \a lower will always be numerically smaller (or equal) to \a upper.
  
  If the original range does span positive and negative sign domains or contains zero,
  the returned range will try to approximate the original range as good as possible.
  If the positive interval of the original range is wider than the negative interval, the
  returned range will only contain the positive interval, with lower bound set to \a rangeFac or
  \a rangeFac *\a upper, whichever is closer to zero. Same procedure is used if the negative interval
  is wider than the positive interval, this time by changing the \a upper bound.
*/
QCPRange QCPRange::sanitizedForLogScale() const
{
  double rangeFac = 1e-3;
  QCPRange sanitizedRange(lower, upper);
  sanitizedRange.normalize();
  // can't have range spanning negative and positive values in log plot, so change range to fix it
  //if (qFuzzyCompare(sanitizedRange.lower+1, 1) && !qFuzzyCompare(sanitizedRange.upper+1, 1))
  if (sanitizedRange.lower == 0.0 && sanitizedRange.upper != 0.0)
  {
    // case lower is 0
    if (rangeFac < sanitizedRange.upper*rangeFac)
      sanitizedRange.lower = rangeFac;
    else
      sanitizedRange.lower = sanitizedRange.upper*rangeFac;
  } //else if (!qFuzzyCompare(lower+1, 1) && qFuzzyCompare(upper+1, 1))
  else if (sanitizedRange.lower != 0.0 && sanitizedRange.upper == 0.0)
  {
    // case upper is 0
    if (-rangeFac > sanitizedRange.lower*rangeFac)
      sanitizedRange.upper = -rangeFac;
    else
      sanitizedRange.upper = sanitizedRange.lower*rangeFac;
  } else if (sanitizedRange.lower < 0 && sanitizedRange.upper > 0)
  {
    // find out whether negative or positive interval is wider to decide which sign domain will be chosen
    if (-sanitizedRange.lower > sanitizedRange.upper)
    {
      // negative is wider, do same as in case upper is 0
      if (-rangeFac > sanitizedRange.lower*rangeFac)
        sanitizedRange.upper = -rangeFac;
      else
        sanitizedRange.upper = sanitizedRange.lower*rangeFac;
    } else
    {
      // positive is wider, do same as in case lower is 0
      if (rangeFac < sanitizedRange.upper*rangeFac)
        sanitizedRange.lower = rangeFac;
      else
        sanitizedRange.lower = sanitizedRange.upper*rangeFac;
    }
  }
  // due to normalization, case lower>0 && upper<0 should never occur, because that implies upper<lower
  return sanitizedRange;
}

/*! 
  Returns a sanitized version of the range. Sanitized means for linear scales, that
  \a lower will always be numerically smaller (or equal) to \a upper.
*/
QCPRange QCPRange::sanitizedForLinScale() const
{
  QCPRange sanitizedRange(lower, upper);
  sanitizedRange.normalize();
  return sanitizedRange;
}

/*! 
  Returns true when \a value lies within or exactly on the borders of the range.
*/
bool QCPRange::contains(double value) const
{
  return value >= lower && value <= upper;
}

/*! 
  Checks, whether the specified range is within valid bounds, which are defined
  as QCPRange::maxRange and QCPRange::minRange.
  A valid range means:
  \li range bounds within -maxRange and maxRange
  \li range size above minRange
  \li range size below maxRange
*/
bool QCPRange::validRange(double lower, double upper)
{
  /*
  return (lower > -maxRange &&
          upper < maxRange &&
          qAbs(lower-upper) > minRange &&
          (lower < -minRange || lower > minRange) &&
          (upper < -minRange || upper > minRange));
          */
  return (lower > -maxRange &&
          upper < maxRange &&
          qAbs(lower-upper) > minRange &&
          qAbs(lower-upper) < maxRange);
}

/*! 
  \overload
  Checks, whether the specified range is within valid bounds, which are defined
  as QCPRange::maxRange and QCPRange::minRange.
  A valid range means:
  \li range bounds within -maxRange and maxRange
  \li range size above minRange
  \li range size below maxRange
*/
bool QCPRange::validRange(const QCPRange &range)
{
  /*
  return (range.lower > -maxRange &&
          range.upper < maxRange &&
          qAbs(range.lower-range.upper) > minRange &&
          qAbs(range.lower-range.upper) < maxRange &&
          (range.lower < -minRange || range.lower > minRange) &&
          (range.upper < -minRange || range.upper > minRange));
          */
  return (range.lower > -maxRange &&
          range.upper < maxRange &&
          qAbs(range.lower-range.upper) > minRange &&
          qAbs(range.lower-range.upper) < maxRange);
}


/*! \page thelayoutsystem The Layout System
 
  The layout system is responsible for positioning and scaling layout elements such as axis rects,
  legends and plot titles in a QCustomPlot.

  \section layoutsystem-classesandmechanisms Classes and mechanisms
  
  The layout system is based on the abstract base class \ref QCPLayoutElement. All objects that
  take part in the layout system derive from this class, either directly or indirectly.
  
  Since QCPLayoutElement itself derives from \ref QCPLayerable, a layout element may draw its own
  content. However, it is perfectly possible for a layout element to only serve as a structuring
  and/or positioning element, not drawing anything on its own.
  
  \subsection layoutsystem-rects Rects of a layout element
  
  A layout element is a rectangular object described by two rects: the inner rect (\ref
  QCPLayoutElement::rect) and the outer rect (\ref QCPLayoutElement::setOuterRect). The inner rect
  is calculated automatically by applying the margin (\ref QCPLayoutElement::setMargins) inward
  from the outer rect. The inner rect is meant for main content while the margin area may either be
  left blank or serve for displaying peripheral graphics. For example, \ref QCPAxisRect positions
  the four main axes at the sides of the inner rect, so graphs end up inside it and the axis labels
  and tick labels are in the margin area.
  
  \subsection layoutsystem-margins Margins
  
  Each layout element may provide a mechanism to automatically determine its margins. Internally,
  this is realized with the \ref QCPLayoutElement::calculateAutoMargin function which takes a \ref
  QCP::MarginSide and returns an integer value which represents the ideal margin for the specified
  side. The automatic margin will be used on the sides specified in \ref
  QCPLayoutElement::setAutoMargins. By default, it is set to \ref QCP::msAll meaning automatic
  margin calculation is enabled for all four sides. In this case, a minimum margin may be set with
  \ref QCPLayoutElement::setMinimumMargins, to prevent the automatic margin mechanism from setting
  margins smaller than desired for a specific situation. If automatic margin calculation is unset
  for a specific side, the margin of that side can be controlled directy via \ref
  QCPLayoutElement::setMargins.
  
  If multiple layout ements are arranged next to or beneath each other, it may be desirable to
  align their inner rects on certain sides. Since they all might have different automatic margins,
  this usually isn't the case. The class \ref QCPMarginGroup and \ref
  QCPLayoutElement::setMarginGroup fix this by allowing to synchronize multiple margins. See the
  documentation there for details.
  
  \subsection layoutsystem-layout Layouts
  
  As mentioned, a QCPLayoutElement may have an arbitrary number of child layout elements and in
  princple can have the only purpose to manage/arrange those child elements. This is what the
  subclass \ref QCPLayout specializes on. It is a QCPLayoutElement itself but has no visual
  representation. It defines an interface to add, remove and manage child layout elements.
  QCPLayout isn't a usable layout though, it's an abstract base class that concrete layouts derive
  from, like \ref QCPLayoutGrid which arranges its child elements in a grid and \ref QCPLayoutInset
  which allows placing child elements freely inside its rect.
  
  Since a QCPLayout is a layout element itself, it may be placed inside other layouts. This way,
  complex hierarchies may be created, offering very flexible arrangements.
  
  <div style="text-align:center">
  <div style="display:inline-block; margin-left:auto; margin-right:auto">\image html LayoutsystemSketch0.png ""</div>
  <div style="display:inline-block; margin-left:auto; margin-right:auto">\image html LayoutsystemSketch1.png ""</div>
  <div style="clear:both"></div>
  <div style="display:inline-block; max-width:1000px; text-align:justify">
  Sketch of the default QCPLayoutGrid accessible via \ref QCustomPlot::plotLayout. The left image
  shows the outer and inner rect of the grid layout itself while the right image shows how two
  child layout elements are placed inside the grid layout next to each other in cells (0, 0) and
  (0, 1).
  </div>
  </div>
  
  \subsection layoutsystem-plotlayout The top level plot layout
  
  Every QCustomPlot has one top level layout of type \ref QCPLayoutGrid. It is accessible via \ref
  QCustomPlot::plotLayout and contains (directly or indirectly via other sub-layouts) all layout
  elements in the QCustomPlot. By default, this top level grid layout contains a single cell which
  holds the main axis rect.
 
  \subsection layoutsystem-examples Examples
  
  <b>Adding a plot title</b> is a typical and simple case to demonstrate basic workings of the layout system.
  \code
  // first we create and prepare a plot title layout element:
  QCPPlotTitle *title = new QCPPlotTitle(customPlot);
  title->setText("Plot Title Example");
  title->setFont(QFont("sans", 12, QFont::Bold));
  // then we add it to the main plot layout:
  customPlot->plotLayout()->insertRow(0); // insert an empty row above the axis rect
  customPlot->plotLayout()->addElement(0, 0, title); // insert the title in the empty cell we just created
  \endcode
  \image html layoutsystem-addingplottitle.png

  <b>Arranging multiple axis rects</b> actually is the central purpose of the layout system.
  \code
  customPlot->plotLayout()->clear(); // let's start from scratch and remove the default axis rect
  // add the first axis rect in second row (row index 1):
  customPlot->plotLayout()->addElement(1, 0, new QCPAxisRect(customPlot));
  // create a sub layout that we'll place in first row:
  QCPLayoutGrid *subLayout = new QCPLayoutGrid;
  customPlot->plotLayout()->addElement(0, 0, subLayout);
  // add two axis rects in the sub layout next to eachother:
  subLayout->addElement(0, 0, new QCPAxisRect(customPlot));
  subLayout->addElement(0, 1, new QCPAxisRect(customPlot));
  subLayout->setColumnStretchFactor(0, 3); // left axis rect shall have 60% of width
  subLayout->setColumnStretchFactor(1, 2); // right one only 40% (3:2 = 60:40)
  \endcode
  \image html layoutsystem-multipleaxisrects.png
  
*/


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPMarginGroup
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPMarginGroup
  \brief A margin group allows synchronization of margin sides if working with multiple layout elements.
  
  QCPMarginGroup allows you to tie a margin side of two or more layout elements together, such that
  they will all have the same size, based on the largest required margin in the group.
  
  \n
  \image html QCPMarginGroup.png "Demonstration of QCPMarginGroup"
  \n
  
  In certain situations it is desirable that margins at specific sides are synchronized across
  layout elements. For example, if one QCPAxisRect is below another one in a grid layout, it will
  provide a cleaner look to the user if the left and right margins of the two axis rects are of the
  same size. The left axis of the top axis rect will then be at the same horizontal position as the
  left axis of the lower axis rect, making them appear aligned. The same applies for the right
  axes. This is what QCPMarginGroup makes possible.
  
  To add/remove a specific side of a layout element to/from a margin group, use the \ref
  QCPLayoutElement::setMarginGroup method. To completely break apart the margin group, either call
  \ref clear, or just delete the margin group.
  
  \section QCPMarginGroup-example Example
  
  First create a margin group:
  \code 
  QCPMarginGroup *group = new QCPMarginGroup(customPlot);
  \endcode
  Then set this group on the layout element sides:
  \code
  customPlot->axisRect(0)->setMarginGroup(QCP::msLeft|QCP::msRight, group);
  customPlot->axisRect(1)->setMarginGroup(QCP::msLeft|QCP::msRight, group);
  \endcode
  Here, we've used the first two axis rects of the plot and synchronized their left margins with
  each other and their right margins with each other.
*/

/* start documentation of inline functions */

/*! \fn QList<QCPLayoutElement*> QCPMarginGroup::elements(QCP::MarginSide side) const
  
  Returns a list of all layout elements that have their margin \a side associated with this margin
  group.
*/

/* end documentation of inline functions */

/*!
  Creates a new QCPMarginGroup instance in \a parentPlot.
*/
QCPMarginGroup::QCPMarginGroup(QCustomPlot *parentPlot) :
  QObject(parentPlot),
  mParentPlot(parentPlot)
{
  mChildren.insert(QCP::msLeft, QList<QCPLayoutElement*>());
  mChildren.insert(QCP::msRight, QList<QCPLayoutElement*>());
  mChildren.insert(QCP::msTop, QList<QCPLayoutElement*>());
  mChildren.insert(QCP::msBottom, QList<QCPLayoutElement*>());
}

QCPMarginGroup::~QCPMarginGroup()
{
  clear();
}

/*!
  Returns whether this margin group is empty. If this function returns true, no layout elements use
  this margin group to synchronize margin sides.
*/
bool QCPMarginGroup::isEmpty() const
{
  QHashIterator<QCP::MarginSide, QList<QCPLayoutElement*> > it(mChildren);
  while (it.hasNext())
  {
    it.next();
    if (!it.value().isEmpty())
      return false;
  }
  return true;
}

/*!
  Clears this margin group. The synchronization of the margin sides that use this margin group is
  lifted and they will use their individual margin sizes again.
*/
void QCPMarginGroup::clear()
{
  // make all children remove themselves from this margin group:
  QHashIterator<QCP::MarginSide, QList<QCPLayoutElement*> > it(mChildren);
  while (it.hasNext())
  {
    it.next();
    const QList<QCPLayoutElement*> elements = it.value();
    for (int i=elements.size()-1; i>=0; --i)
      elements.at(i)->setMarginGroup(it.key(), 0); // removes itself from mChildren via removeChild
  }
}

/*! \internal
  
  Returns the synchronized common margin for \a side. This is the margin value that will be used by
  the layout element on the respective side, if it is part of this margin group.
  
  The common margin is calculated by requesting the automatic margin (\ref
  QCPLayoutElement::calculateAutoMargin) of each element associated with \a side in this margin
  group, and choosing the largest returned value. (QCPLayoutElement::minimumMargins is taken into
  account, too.)
*/
int QCPMarginGroup::commonMargin(QCP::MarginSide side) const
{
  // query all automatic margins of the layout elements in this margin group side and find maximum:
  int result = 0;
  const QList<QCPLayoutElement*> elements = mChildren.value(side);
  for (int i=0; i<elements.size(); ++i)
  {
    if (!elements.at(i)->autoMargins().testFlag(side))
      continue;
    int m = qMax(elements.at(i)->calculateAutoMargin(side), QCP::getMarginValue(elements.at(i)->minimumMargins(), side));
    if (m > result)
      result = m;
  }
  return result;
}

/*! \internal
  
  Adds \a element to the internal list of child elements, for the margin \a side.
  
  This function does not modify the margin group property of \a element.
*/
void QCPMarginGroup::addChild(QCP::MarginSide side, QCPLayoutElement *element)
{
  if (!mChildren[side].contains(element))
    mChildren[side].append(element);
  else
    qDebug() << Q_FUNC_INFO << "element is already child of this margin group side" << reinterpret_cast<quintptr>(element);
}

/*! \internal
  
  Removes \a element from the internal list of child elements, for the margin \a side.
  
  This function does not modify the margin group property of \a element.
*/
void QCPMarginGroup::removeChild(QCP::MarginSide side, QCPLayoutElement *element)
{
  if (!mChildren[side].removeOne(element))
    qDebug() << Q_FUNC_INFO << "element is not child of this margin group side" << reinterpret_cast<quintptr>(element);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLayoutElement
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLayoutElement
  \brief The abstract base class for all objects that form \ref thelayoutsystem "the layout system".
  
  This is an abstract base class. As such, it can't be instantiated directly, rather use one of its subclasses.
  
  A Layout element is a rectangular object which can be placed in layouts. It has an outer rect
  (QCPLayoutElement::outerRect) and an inner rect (\ref QCPLayoutElement::rect). The difference
  between outer and inner rect is called its margin. The margin can either be set to automatic or
  manual (\ref setAutoMargins) on a per-side basis. If a side is set to manual, that margin can be
  set explicitly with \ref setMargins and will stay fixed at that value. If it's set to automatic,
  the layout element subclass will control the value itself (via \ref calculateAutoMargin).
  
  Layout elements can be placed in layouts (base class QCPLayout) like QCPLayoutGrid. The top level
  layout is reachable via \ref QCustomPlot::plotLayout, and is a \ref QCPLayoutGrid. Since \ref
  QCPLayout itself derives from \ref QCPLayoutElement, layouts can be nested.
  
  Thus in QCustomPlot one can divide layout elements into two categories: The ones that are
  invisible by themselves, because they don't draw anything. Their only purpose is to manage the
  position and size of other layout elements. This category of layout elements usually use
  QCPLayout as base class. Then there is the category of layout elements which actually draw
  something. For example, QCPAxisRect, QCPLegend and QCPPlotTitle are of this category. This does
  not necessarily mean that the latter category can't have child layout elements. QCPLegend for
  instance, actually derives from QCPLayoutGrid and the individual legend items are child layout
  elements in the grid layout.
*/

/* start documentation of inline functions */

/*! \fn QCPLayout *QCPLayoutElement::layout() const
  
  Returns the parent layout of this layout element.
*/

/*! \fn QRect QCPLayoutElement::rect() const
  
  Returns the inner rect of this layout element. The inner rect is the outer rect (\ref
  setOuterRect) shrinked by the margins (\ref setMargins, \ref setAutoMargins).
  
  In some cases, the area between outer and inner rect is left blank. In other cases the margin
  area is used to display peripheral graphics while the main content is in the inner rect. This is
  where automatic margin calculation becomes interesting because it allows the layout element to
  adapt the margins to the peripheral graphics it wants to draw. For example, \ref QCPAxisRect
  draws the axis labels and tick labels in the margin area, thus needs to adjust the margins (if
  \ref setAutoMargins is enabled) according to the space required by the labels of the axes.
*/

/*! \fn virtual void QCPLayoutElement::mousePressEvent(QMouseEvent *event)
  
  This event is called, if the mouse was pressed while being inside the outer rect of this layout
  element.
*/

/*! \fn virtual void QCPLayoutElement::mouseMoveEvent(QMouseEvent *event)
  
  This event is called, if the mouse is moved inside the outer rect of this layout element.
*/

/*! \fn virtual void QCPLayoutElement::mouseReleaseEvent(QMouseEvent *event)
  
  This event is called, if the mouse was previously pressed inside the outer rect of this layout
  element and is now released.
*/

/*! \fn virtual void QCPLayoutElement::mouseDoubleClickEvent(QMouseEvent *event)
  
  This event is called, if the mouse is double-clicked inside the outer rect of this layout
  element.
*/

/*! \fn virtual void QCPLayoutElement::wheelEvent(QWheelEvent *event)
  
  This event is called, if the mouse wheel is scrolled while the cursor is inside the rect of this
  layout element.
*/

/* end documentation of inline functions */

/*!
  Creates an instance of QCPLayoutElement and sets default values.
*/
QCPLayoutElement::QCPLayoutElement(QCustomPlot *parentPlot) :
  QCPLayerable(parentPlot), // parenthood is changed as soon as layout element gets inserted into a layout (except for top level layout)
  mParentLayout(0),
  mMinimumSize(),
  mMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX),
  mRect(0, 0, 0, 0),
  mOuterRect(0, 0, 0, 0),
  mMargins(0, 0, 0, 0),
  mMinimumMargins(0, 0, 0, 0),
  mAutoMargins(QCP::msAll)
{
}

QCPLayoutElement::~QCPLayoutElement()
{
  setMarginGroup(QCP::msAll, 0); // unregister at margin groups, if there are any
  // unregister at layout:
  if (qobject_cast<QCPLayout*>(mParentLayout)) // the qobject_cast is just a safeguard in case the layout forgets to call clear() in its dtor and this dtor is called by QObject dtor
    mParentLayout->take(this);
}

/*!
  Sets the outer rect of this layout element. If the layout element is inside a layout, the layout
  sets the position and size of this layout element using this function.
  
  Calling this function externally has no effect, since the layout will overwrite any changes to
  the outer rect upon the next replot.
  
  The layout element will adapt its inner \ref rect by applying the margins inward to the outer rect.
  
  \see rect
*/
void QCPLayoutElement::setOuterRect(const QRect &rect)
{
  if (mOuterRect != rect)
  {
    mOuterRect = rect;
    mRect = mOuterRect.adjusted(mMargins.left(), mMargins.top(), -mMargins.right(), -mMargins.bottom());
  }
}

/*!
  Sets the margins of this layout element. If \ref setAutoMargins is disabled for some or all
  sides, this function is used to manually set the margin on those sides. Sides that are still set
  to be handled automatically are ignored and may have any value in \a margins.
  
  The margin is the distance between the outer rect (controlled by the parent layout via \ref
  setOuterRect) and the inner \ref rect (which usually contains the main content of this layout
  element).
  
  \see setAutoMargins
*/
void QCPLayoutElement::setMargins(const QMargins &margins)
{
  if (mMargins != margins)
  {
    mMargins = margins;
    mRect = mOuterRect.adjusted(mMargins.left(), mMargins.top(), -mMargins.right(), -mMargins.bottom());
  }
}

/*!
  If \ref setAutoMargins is enabled on some or all margins, this function is used to provide
  minimum values for those margins.
  
  The minimum values are not enforced on margin sides that were set to be under manual control via
  \ref setAutoMargins.
  
  \see setAutoMargins
*/
void QCPLayoutElement::setMinimumMargins(const QMargins &margins)
{
  if (mMinimumMargins != margins)
  {
    mMinimumMargins = margins;
  }
}

/*!
  Sets on which sides the margin shall be calculated automatically. If a side is calculated
  automatically, a minimum margin value may be provided with \ref setMinimumMargins. If a side is
  set to be controlled manually, the value may be specified with \ref setMargins.
  
  Margin sides that are under automatic control may participate in a \ref QCPMarginGroup (see \ref
  setMarginGroup), to synchronize (align) it with other layout elements in the plot.
  
  \see setMinimumMargins, setMargins
*/
void QCPLayoutElement::setAutoMargins(QCP::MarginSides sides)
{
  mAutoMargins = sides;
}

/*!
  Sets the minimum size for the inner \ref rect of this layout element. A parent layout tries to
  respect the \a size here by changing row/column sizes in the layout accordingly.
  
  If the parent layout size is not sufficient to satisfy all minimum size constraints of its child
  layout elements, the layout may set a size that is actually smaller than \a size. QCustomPlot
  propagates the layout's size constraints to the outside by setting its own minimum QWidget size
  accordingly, so violations of \a size should be exceptions.
*/
void QCPLayoutElement::setMinimumSize(const QSize &size)
{
  if (mMinimumSize != size)
  {
    mMinimumSize = size;
    if (mParentLayout)
      mParentLayout->sizeConstraintsChanged();
  }
}

/*! \overload
  
  Sets the minimum size for the inner \ref rect of this layout element.
*/
void QCPLayoutElement::setMinimumSize(int width, int height)
{
  setMinimumSize(QSize(width, height));
}

/*!
  Sets the maximum size for the inner \ref rect of this layout element. A parent layout tries to
  respect the \a size here by changing row/column sizes in the layout accordingly.
*/
void QCPLayoutElement::setMaximumSize(const QSize &size)
{
  if (mMaximumSize != size)
  {
    mMaximumSize = size;
    if (mParentLayout)
      mParentLayout->sizeConstraintsChanged();
  }
}

/*! \overload
  
  Sets the maximum size for the inner \ref rect of this layout element.
*/
void QCPLayoutElement::setMaximumSize(int width, int height)
{
  setMaximumSize(QSize(width, height));
}

/*!
  Sets the margin \a group of the specified margin \a sides.
  
  Margin groups allow synchronizing specified margins across layout elements, see the documentation
  of \ref QCPMarginGroup.
  
  To unset the margin group of \a sides, set \a group to 0.
  
  Note that margin groups only work for margin sides that are set to automatic (\ref
  setAutoMargins).
*/
void QCPLayoutElement::setMarginGroup(QCP::MarginSides sides, QCPMarginGroup *group)
{
  QVector<QCP::MarginSide> sideVector;
  if (sides.testFlag(QCP::msLeft)) sideVector.append(QCP::msLeft);
  if (sides.testFlag(QCP::msRight)) sideVector.append(QCP::msRight);
  if (sides.testFlag(QCP::msTop)) sideVector.append(QCP::msTop);
  if (sides.testFlag(QCP::msBottom)) sideVector.append(QCP::msBottom);
  
  for (int i=0; i<sideVector.size(); ++i)
  {
    QCP::MarginSide side = sideVector.at(i);
    if (marginGroup(side) != group)
    {
      QCPMarginGroup *oldGroup = marginGroup(side);
      if (oldGroup) // unregister at old group
        oldGroup->removeChild(side, this);
      
      if (!group) // if setting to 0, remove hash entry. Else set hash entry to new group and register there
      {
        mMarginGroups.remove(side);
      } else // setting to a new group
      {
        mMarginGroups[side] = group;
        group->addChild(side, this);
      }
    }
  }
}

/*!
  Updates the layout element and sub-elements. This function is automatically called upon replot by
  the parent layout element.
  
  Layout elements that have child elements should call the \ref update method of their child
  elements.
  
  The default implementation executes the automatic margin mechanism, so subclasses should make
  sure to call the base class implementation.
*/
void QCPLayoutElement::update()
{
  if (mAutoMargins != QCP::msNone)
  {
    // set the margins of this layout element according to automatic margin calculation, either directly or via a margin group:
    QMargins newMargins = mMargins;
    QVector<QCP::MarginSide> marginSides = QVector<QCP::MarginSide>() << QCP::msLeft << QCP::msRight << QCP::msTop << QCP::msBottom;
    for (int i=0; i<marginSides.size(); ++i)
    {
      QCP::MarginSide side = marginSides.at(i);
      if (mAutoMargins.testFlag(side)) // this side's margin shall be calculated automatically
      {
        if (mMarginGroups.contains(side)) 
          QCP::setMarginValue(newMargins, side, mMarginGroups[side]->commonMargin(side)); // this side is part of a margin group, so get the margin value from that group
        else 
          QCP::setMarginValue(newMargins, side, calculateAutoMargin(side)); // this side is not part of a group, so calculate the value directly
        // apply minimum margin restrictions:
        if (QCP::getMarginValue(newMargins, side) < QCP::getMarginValue(mMinimumMargins, side))
          QCP::setMarginValue(newMargins, side, QCP::getMarginValue(mMinimumMargins, side));
      }
    }
    setMargins(newMargins);
  }
}

/*!
  Returns the minimum size this layout element (the inner \ref rect) may be compressed to.
  
  if a minimum size (\ref setMinimumSize) was not set manually, parent layouts consult this
  function to determine the minimum allowed size of this layout element. (A manual minimum size is
  considered set if it is non-zero.)
*/
QSize QCPLayoutElement::minimumSizeHint() const
{
  return mMinimumSize;
}

/*!
  Returns the maximum size this layout element (the inner \ref rect) may be expanded to.
  
  if a maximum size (\ref setMaximumSize) was not set manually, parent layouts consult this
  function to determine the maximum allowed size of this layout element. (A manual maximum size is
  considered set if it is smaller than Qt's QWIDGETSIZE_MAX.)
*/
QSize QCPLayoutElement::maximumSizeHint() const
{
  return mMaximumSize;
}

/*!
  Returns a list of all child elements in this layout element. If \a recursive is true, all
  sub-child elements are included in the list, too.
  
  Note that there may be entries with value 0 in the returned list. (For example, QCPLayoutGrid may have
  empty cells which yield 0 at the respective index.)
*/
QList<QCPLayoutElement*> QCPLayoutElement::elements(bool recursive) const
{
  Q_UNUSED(recursive)
  return QList<QCPLayoutElement*>();
}

/*!
  Layout elements are sensitive to events inside their outer rect. If \a pos is within the outer
  rect, this method returns a value corresponding to 0.99 times the parent plot's selection
  tolerance. However, layout elements are not selectable by default. So if \a onlySelectable is
  true, -1.0 is returned.
  
  See \ref QCPLayerable::selectTest for a general explanation of this virtual method.
  
  QCPLayoutElement subclasses may reimplement this method to provide more specific selection test
  behaviour.
*/
double QCPLayoutElement::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  
  if (onlySelectable)
    return -1;
  
  if (QRectF(mOuterRect).contains(pos))
  {
    if (mParentPlot)
      return mParentPlot->selectionTolerance()*0.99;
    else
    {
      qDebug() << Q_FUNC_INFO << "parent plot not defined";
      return -1;
    }
  } else
    return -1;
}

/*! \internal 
  
  propagates the parent plot initialization to all child elements, by calling \ref
  QCPLayerable::initializeParentPlot on them.
*/
void QCPLayoutElement::parentPlotInitialized(QCustomPlot *parentPlot)
{
  QList<QCPLayoutElement*> els = elements(false);
  for (int i=0; i<els.size(); ++i)
  {
    if (!els.at(i)->parentPlot())
      els.at(i)->initializeParentPlot(parentPlot);
  }
}

/*! \internal 
  
  Returns the margin size for this \a side. It is used if automatic margins is enabled for this \a
  side (see \ref setAutoMargins). If a minimum margin was set with \ref setMinimumMargins, the
  returned value will not be smaller than the specified minimum margin.
  
  The default implementation just returns the respective manual margin (\ref setMargins) or the
  minimum margin, whichever is larger.
*/
int QCPLayoutElement::calculateAutoMargin(QCP::MarginSide side)
{
  return qMax(QCP::getMarginValue(mMargins, side), QCP::getMarginValue(mMinimumMargins, side));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLayout
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLayout
  \brief The abstract base class for layouts
  
  This is an abstract base class for layout elements whose main purpose is to define the position
  and size of other child layout elements. In most cases, layouts don't draw anything themselves
  (but there are exceptions to this, e.g. QCPLegend).
  
  QCPLayout derives from QCPLayoutElement, and thus can itself be nested in other layouts.
  
  QCPLayout introduces a common interface for accessing and manipulating the child elements. Those
  functions are most notably \ref elementCount, \ref elementAt, \ref takeAt, \ref take, \ref
  simplify, \ref removeAt, \ref remove and \ref clear. Individual subclasses may add more functions
  to this interface which are more specialized to the form of the layout. For example, \ref
  QCPLayoutGrid adds functions that take row and column indices to access cells of the layout grid
  more conveniently.
  
  Since this is an abstract base class, you can't instantiate it directly. Rather use one of its
  subclasses like QCPLayoutGrid or QCPLayoutInset.
  
  For a general introduction to the layout system, see the dedicated documentation page \ref
  thelayoutsystem "The Layout System".
*/

/* start documentation of pure virtual functions */

/*! \fn virtual int QCPLayout::elementCount() const = 0
  
  Returns the number of elements/cells in the layout.
  
  \see elements, elementAt
*/

/*! \fn virtual QCPLayoutElement* QCPLayout::elementAt(int index) const = 0
  
  Returns the element in the cell with the given \a index. If \a index is invalid, returns 0.
  
  Note that even if \a index is valid, the respective cell may be empty in some layouts (e.g.
  QCPLayoutGrid), so this function may return 0 in those cases. You may use this function to check
  whether a cell is empty or not.
  
  \see elements, elementCount, takeAt
*/

/*! \fn virtual QCPLayoutElement* QCPLayout::takeAt(int index) = 0
  
  Removes the element with the given \a index from the layout and returns it.
  
  If the \a index is invalid or the cell with that index is empty, returns 0.
  
  Note that some layouts don't remove the respective cell right away but leave an empty cell after
  successful removal of the layout element. To collapse empty cells, use \ref simplify.
  
  \see elementAt, take
*/

/*! \fn virtual bool QCPLayout::take(QCPLayoutElement* element) = 0
  
  Removes the specified \a element from the layout and returns true on success.
  
  If the \a element isn't in this layout, returns false.
  
  Note that some layouts don't remove the respective cell right away but leave an empty cell after
  successful removal of the layout element. To collapse empty cells, use \ref simplify.
  
  \see takeAt
*/

/* end documentation of pure virtual functions */

/*!
  Creates an instance of QCPLayoutElement and sets default values. Note that since QCPLayoutElement
  is an abstract base class, it can't be instantiated directly.
*/
QCPLayout::QCPLayout()
{
}

/*!
  First calls the QCPLayoutElement::update base class implementation to update the margins on this
  layout.
  
  Then calls \ref updateLayout which subclasses reimplement to reposition and resize their cells.
  
  Finally, \ref update is called on all child elements.
*/
void QCPLayout::update()
{
  QCPLayoutElement::update(); // recalculates (auto-)margins
  
  // set child element rects according to layout:
  updateLayout();
  
  // propagate update call to child elements:
  for (int i=0; i<elementCount(); ++i)
  {
    if (QCPLayoutElement *el = elementAt(i))
      el->update();
  }
}

/* inherits documentation from base class */
QList<QCPLayoutElement*> QCPLayout::elements(bool recursive) const
{
  int c = elementCount();
  QList<QCPLayoutElement*> result;
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
  result.reserve(c);
#endif
  for (int i=0; i<c; ++i)
    result.append(elementAt(i));
  if (recursive)
  {
    for (int i=0; i<c; ++i)
    {
      if (result.at(i))
        result << result.at(i)->elements(recursive);
    }
  }
  return result;
}

/*!
  Simplifies the layout by collapsing empty cells. The exact behavior depends on subclasses, the
  default implementation does nothing.
  
  Not all layouts need simplification. For example, QCPLayoutInset doesn't use explicit
  simplification while QCPLayoutGrid does.
*/
void QCPLayout::simplify()
{
}

/*!
  Removes and deletes the element at the provided \a index. Returns true on success. If \a index is
  invalid or points to an empty cell, returns false.
  
  This function internally uses \ref takeAt to remove the element from the layout and then deletes
  the returned element.
  
  \see remove, takeAt
*/
bool QCPLayout::removeAt(int index)
{
  if (QCPLayoutElement *el = takeAt(index))
  {
    delete el;
    return true;
  } else
    return false;
}

/*!
  Removes and deletes the provided \a element. Returns true on success. If \a element is not in the
  layout, returns false.
  
  This function internally uses \ref takeAt to remove the element from the layout and then deletes
  the element.
  
  \see removeAt, take
*/
bool QCPLayout::remove(QCPLayoutElement *element)
{
  if (take(element))
  {
    delete element;
    return true;
  } else
    return false;
}

/*!
  Removes and deletes all layout elements in this layout.
  
  \see remove, removeAt
*/
void QCPLayout::clear()
{
  for (int i=elementCount()-1; i>=0; --i)
  {
    if (elementAt(i))
      removeAt(i);
  }
  simplify();
}

/*!
  Subclasses call this method to report changed (minimum/maximum) size constraints.
  
  If the parent of this layout is again a QCPLayout, forwards the call to the parent's \ref
  sizeConstraintsChanged. If the parent is a QWidget (i.e. is the \ref QCustomPlot::plotLayout of
  QCustomPlot), calls QWidget::updateGeometry, so if the QCustomPlot widget is inside a Qt QLayout,
  it may update itself and resize cells accordingly.
*/
void QCPLayout::sizeConstraintsChanged() const
{
  if (QWidget *w = qobject_cast<QWidget*>(parent()))
    w->updateGeometry();
  else if (QCPLayout *l = qobject_cast<QCPLayout*>(parent()))
    l->sizeConstraintsChanged();
}

/*! \internal
  
  Subclasses reimplement this method to update the position and sizes of the child elements/cells
  via calling their \ref QCPLayoutElement::setOuterRect. The default implementation does nothing.
  
  The geometry used as a reference is the inner \ref rect of this layout. Child elements should stay
  within that rect.
  
  \ref getSectionSizes may help with the reimplementation of this function.
  
  \see update
*/
void QCPLayout::updateLayout()
{
}


/*! \internal
  
  Associates \a el with this layout. This is done by setting the \ref QCPLayoutElement::layout, the
  \ref QCPLayerable::parentLayerable and the QObject parent to this layout.
  
  Further, if \a el didn't previously have a parent plot, calls \ref
  QCPLayerable::initializeParentPlot on \a el to set the paret plot.
  
  This method is used by subclass specific methods that add elements to the layout. Note that this
  method only changes properties in \a el. The removal from the old layout and the insertion into
  the new layout must be done additionally.
*/
void QCPLayout::adoptElement(QCPLayoutElement *el)
{
  if (el)
  {
    el->mParentLayout = this;
    el->setParentLayerable(this);
    el->setParent(this);
    if (!el->parentPlot())
      el->initializeParentPlot(mParentPlot);
  } else
    qDebug() << Q_FUNC_INFO << "Null element passed";
}

/*! \internal
  
  Disassociates \a el from this layout. This is done by setting the \ref QCPLayoutElement::layout
  and the \ref QCPLayerable::parentLayerable to zero. The QObject parent is set to the parent
  QCustomPlot.
  
  This method is used by subclass specific methods that remove elements from the layout (e.g. \ref
  take or \ref takeAt). Note that this method only changes properties in \a el. The removal from
  the old layout must be done additionally.
*/
void QCPLayout::releaseElement(QCPLayoutElement *el)
{
  if (el)
  {
    el->mParentLayout = 0;
    el->setParentLayerable(0);
    el->setParent(mParentPlot);
    // Note: Don't initializeParentPlot(0) here, because layout element will stay in same parent plot
  } else
    qDebug() << Q_FUNC_INFO << "Null element passed";
}

/*! \internal
  
  This is a helper function for the implementation of \ref updateLayout in subclasses.
  
  It calculates the sizes of one-dimensional sections with provided constraints on maximum section
  sizes, minimum section sizes, relative stretch factors and the final total size of all sections.
  
  The QVector entries refer to the sections. Thus all QVectors must have the same size.
  
  \a maxSizes gives the maximum allowed size of each section. If there shall be no maximum size
  imposed, set all vector values to Qt's QWIDGETSIZE_MAX.
  
  \a minSizes gives the minimum allowed size of each section. If there shall be no minimum size
  imposed, set all vector values to zero. If the \a minSizes entries add up to a value greater than
  \a totalSize, sections will be scaled smaller than the proposed minimum sizes. (In other words,
  not exceeding the allowed total size is taken to be more important than not going below minimum
  section sizes.)
  
  \a stretchFactors give the relative proportions of the sections to each other. If all sections
  shall be scaled equally, set all values equal. If the first section shall be double the size of
  each individual other section, set the first number of \a stretchFactors to double the value of
  the other individual values (e.g. {2, 1, 1, 1}).
  
  \a totalSize is the value that the final section sizes will add up to. Due to rounding, the
  actual sum may differ slightly. If you want the section sizes to sum up to exactly that value,
  you could distribute the remaining difference on the sections.
  
  The return value is a QVector containing the section sizes.
*/
QVector<int> QCPLayout::getSectionSizes(QVector<int> maxSizes, QVector<int> minSizes, QVector<double> stretchFactors, int totalSize) const
{
  if (maxSizes.size() != minSizes.size() || minSizes.size() != stretchFactors.size())
  {
    qDebug() << Q_FUNC_INFO << "Passed vector sizes aren't equal:" << maxSizes << minSizes << stretchFactors;
    return QVector<int>();
  }
  if (stretchFactors.isEmpty())
    return QVector<int>();
  int sectionCount = stretchFactors.size();
  QVector<double> sectionSizes(sectionCount);
  // if provided total size is forced smaller than total minimum size, ignore minimum sizes (squeeze sections):
  int minSizeSum = 0;
  for (int i=0; i<sectionCount; ++i)
    minSizeSum += minSizes.at(i);
  if (totalSize < minSizeSum)
  {
    // new stretch factors are minimum sizes and minimum sizes are set to zero:
    for (int i=0; i<sectionCount; ++i)
    {
      stretchFactors[i] = minSizes.at(i);
      minSizes[i] = 0;
    }
  }
  
  QList<int> minimumLockedSections;
  QList<int> unfinishedSections;
  for (int i=0; i<sectionCount; ++i)
    unfinishedSections.append(i);
  double freeSize = totalSize;
  
  int outerIterations = 0;
  while (!unfinishedSections.isEmpty() && outerIterations < sectionCount*2) // the iteration check ist just a failsafe in case something really strange happens
  {
    ++outerIterations;
    int innerIterations = 0;
    while (!unfinishedSections.isEmpty() && innerIterations < sectionCount*2) // the iteration check ist just a failsafe in case something really strange happens
    {
      ++innerIterations;
      // find section that hits its maximum next:
      int nextId = -1;
      double nextMax = 1e12;
      for (int i=0; i<unfinishedSections.size(); ++i)
      {
        int secId = unfinishedSections.at(i);
        double hitsMaxAt = (maxSizes.at(secId)-sectionSizes.at(secId))/stretchFactors.at(secId);
        if (hitsMaxAt < nextMax)
        {
          nextMax = hitsMaxAt;
          nextId = secId;
        }
      }
      // check if that maximum is actually within the bounds of the total size (i.e. can we stretch all remaining sections so far that the found section
      // actually hits its maximum, without exceeding the total size when we add up all sections)
      double stretchFactorSum = 0;
      for (int i=0; i<unfinishedSections.size(); ++i)
        stretchFactorSum += stretchFactors.at(unfinishedSections.at(i));
      double nextMaxLimit = freeSize/stretchFactorSum;
      if (nextMax < nextMaxLimit) // next maximum is actually hit, move forward to that point and fix the size of that section
      {
        for (int i=0; i<unfinishedSections.size(); ++i)
        {
          sectionSizes[unfinishedSections.at(i)] += nextMax*stretchFactors.at(unfinishedSections.at(i)); // increment all sections
          freeSize -= nextMax*stretchFactors.at(unfinishedSections.at(i));
        }
        unfinishedSections.removeOne(nextId); // exclude the section that is now at maximum from further changes
      } else // next maximum isn't hit, just distribute rest of free space on remaining sections
      {
        for (int i=0; i<unfinishedSections.size(); ++i)
          sectionSizes[unfinishedSections.at(i)] += nextMaxLimit*stretchFactors.at(unfinishedSections.at(i)); // increment all sections
        unfinishedSections.clear();
      }
    }
    if (innerIterations == sectionCount*2)
      qDebug() << Q_FUNC_INFO << "Exceeded maximum expected inner iteration count, layouting aborted. Input was:" << maxSizes << minSizes << stretchFactors << totalSize;
    
    // now check whether the resulting section sizes violate minimum restrictions:
    bool foundMinimumViolation = false;
    for (int i=0; i<sectionSizes.size(); ++i)
    {
      if (minimumLockedSections.contains(i))
        continue;
      if (sectionSizes.at(i) < minSizes.at(i)) // section violates minimum
      {
        sectionSizes[i] = minSizes.at(i); // set it to minimum
        foundMinimumViolation = true; // make sure we repeat the whole optimization process
        minimumLockedSections.append(i);
      }
    }
    if (foundMinimumViolation)
    {
      freeSize = totalSize;
      for (int i=0; i<sectionCount; ++i)
      {
        if (!minimumLockedSections.contains(i)) // only put sections that haven't hit their minimum back into the pool
          unfinishedSections.append(i);
        else
          freeSize -= sectionSizes.at(i); // remove size of minimum locked sections from available space in next round
      }
      // reset all section sizes to zero that are in unfinished sections (all others have been set to their minimum):
      for (int i=0; i<unfinishedSections.size(); ++i)
        sectionSizes[unfinishedSections.at(i)] = 0;
    }
  }
  if (outerIterations == sectionCount*2)
    qDebug() << Q_FUNC_INFO << "Exceeded maximum expected outer iteration count, layouting aborted. Input was:" << maxSizes << minSizes << stretchFactors << totalSize;
  
  QVector<int> result(sectionCount);
  for (int i=0; i<sectionCount; ++i)
    result[i] = qRound(sectionSizes.at(i));
  return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLayoutGrid
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLayoutGrid
  \brief A layout that arranges child elements in a grid
  
  Elements are laid out in a grid with configurable stretch factors (\ref setColumnStretchFactor,
  \ref setRowStretchFactor) and spacing (\ref setColumnSpacing, \ref setRowSpacing).
  
  Elements can be added to cells via \ref addElement. The grid is expanded if the specified row or
  column doesn't exist yet. Whether a cell contains a valid layout element can be checked with \ref
  hasElement, that element can be retrieved with \ref element. If rows and columns that only have
  empty cells shall be removed, call \ref simplify. Removal of elements is either done by just
  adding the element to a different layout or by using the QCPLayout interface \ref take or \ref
  remove.
  
  Row and column insertion can be performed with \ref insertRow and \ref insertColumn.
*/

/*!
  Creates an instance of QCPLayoutGrid and sets default values.
*/
QCPLayoutGrid::QCPLayoutGrid() :
  mColumnSpacing(5),
  mRowSpacing(5)
{
}

QCPLayoutGrid::~QCPLayoutGrid()
{
  // clear all child layout elements. This is important because only the specific layouts know how
  // to handle removing elements (clear calls virtual removeAt method to do that).
  clear();
}

/*!
  Returns the element in the cell in \a row and \a column.
  
  Returns 0 if either the row/column is invalid or if the cell is empty. In those cases, a qDebug
  message is printed. To check whether a cell exists and isn't empty, use \ref hasElement.
  
  \see addElement, hasElement
*/
QCPLayoutElement *QCPLayoutGrid::element(int row, int column) const
{
  if (row >= 0 && row < mElements.size())
  {
    if (column >= 0 && column < mElements.first().size())
    {
      if (QCPLayoutElement *result = mElements.at(row).at(column))
        return result;
      else
        qDebug() << Q_FUNC_INFO << "Requested cell is empty. Row:" << row << "Column:" << column;
    } else
      qDebug() << Q_FUNC_INFO << "Invalid column. Row:" << row << "Column:" << column;
  } else
    qDebug() << Q_FUNC_INFO << "Invalid row. Row:" << row << "Column:" << column;
  return 0;
}

/*!
  Returns the number of rows in the layout.
  
  \see columnCount
*/
int QCPLayoutGrid::rowCount() const
{
  return mElements.size();
}

/*!
  Returns the number of columns in the layout.
  
  \see rowCount
*/
int QCPLayoutGrid::columnCount() const
{
  if (mElements.size() > 0)
    return mElements.first().size();
  else
    return 0;
}

/*!
  Adds the \a element to cell with \a row and \a column. If \a element is already in a layout, it
  is first removed from there. If \a row or \a column don't exist yet, the layout is expanded
  accordingly.
  
  Returns true if the element was added successfully, i.e. if the cell at \a row and \a column
  didn't already have an element.
  
  \see element, hasElement, take, remove
*/
bool QCPLayoutGrid::addElement(int row, int column, QCPLayoutElement *element)
{
  if (element)
  {
    if (!hasElement(row, column))
    {
      if (element->layout()) // remove from old layout first
        element->layout()->take(element);
      expandTo(row+1, column+1);
      mElements[row][column] = element;
      adoptElement(element);
      return true;
    } else
      qDebug() << Q_FUNC_INFO << "There is already an element in the specified row/column:" << row << column; 
  } else
    qDebug() << Q_FUNC_INFO << "Can't add null element to row/column:" << row << column;
  return false;
}

/*!
  Returns whether the cell at \a row and \a column exists and contains a valid element, i.e. isn't
  empty.
  
  \see element
*/
bool QCPLayoutGrid::hasElement(int row, int column)
{
  if (row >= 0 && row < rowCount() && column >= 0 && column < columnCount())
    return mElements.at(row).at(column);
  else
    return false;
}

/*!
  Sets the stretch \a factor of \a column.
  
  Stretch factors control the relative sizes of rows and columns. Cells will not be resized beyond
  their minimum and maximum widths/heights (\ref QCPLayoutElement::setMinimumSize, \ref
  QCPLayoutElement::setMaximumSize), regardless of the stretch factor.
  
  The default stretch factor of newly created rows/columns is 1.
  
  \see setColumnStretchFactors, setRowStretchFactor
*/
void QCPLayoutGrid::setColumnStretchFactor(int column, double factor)
{
  if (column >= 0 && column < columnCount())
  {
    if (factor > 0)
      mColumnStretchFactors[column] = factor;
    else
      qDebug() << Q_FUNC_INFO << "Invalid stretch factor, must be positive:" << factor;
  } else
    qDebug() << Q_FUNC_INFO << "Invalid column:" << column;
}

/*!
  Sets the stretch \a factors of all columns. \a factors must have the size \ref columnCount.
  
  Stretch factors control the relative sizes of rows and columns. Cells will not be resized beyond
  their minimum and maximum widths/heights (\ref QCPLayoutElement::setMinimumSize, \ref
  QCPLayoutElement::setMaximumSize), regardless of the stretch factor.
  
  The default stretch factor of newly created rows/columns is 1.
  
  \see setColumnStretchFactor, setRowStretchFactors
*/
void QCPLayoutGrid::setColumnStretchFactors(const QList<double> &factors)
{
  if (factors.size() == mColumnStretchFactors.size())
  {
    mColumnStretchFactors = factors;
    for (int i=0; i<mColumnStretchFactors.size(); ++i)
    {
      if (mColumnStretchFactors.at(i) <= 0)
      {
        qDebug() << Q_FUNC_INFO << "Invalid stretch factor, must be positive:" << mColumnStretchFactors.at(i);
        mColumnStretchFactors[i] = 1;
      }
    }
  } else
    qDebug() << Q_FUNC_INFO << "Column count not equal to passed stretch factor count:" << factors;
}

/*!
  Sets the stretch \a factor of \a row.
  
  Stretch factors control the relative sizes of rows and columns. Cells will not be resized beyond
  their minimum and maximum widths/heights (\ref QCPLayoutElement::setMinimumSize, \ref
  QCPLayoutElement::setMaximumSize), regardless of the stretch factor.
  
  The default stretch factor of newly created rows/columns is 1.
  
  \see setColumnStretchFactors, setRowStretchFactor
*/
void QCPLayoutGrid::setRowStretchFactor(int row, double factor)
{
  if (row >= 0 && row < rowCount())
  {
    if (factor > 0)
      mRowStretchFactors[row] = factor;
    else
      qDebug() << Q_FUNC_INFO << "Invalid stretch factor, must be positive:" << factor;
  } else
    qDebug() << Q_FUNC_INFO << "Invalid row:" << row;
}

/*!
  Sets the stretch \a factors of all rows. \a factors must have the size \ref rowCount.
  
  Stretch factors control the relative sizes of rows and columns. Cells will not be resized beyond
  their minimum and maximum widths/heights (\ref QCPLayoutElement::setMinimumSize, \ref
  QCPLayoutElement::setMaximumSize), regardless of the stretch factor.
  
  The default stretch factor of newly created rows/columns is 1.
  
  \see setRowStretchFactor, setColumnStretchFactors
*/
void QCPLayoutGrid::setRowStretchFactors(const QList<double> &factors)
{
  if (factors.size() == mRowStretchFactors.size())
  {
    mRowStretchFactors = factors;
    for (int i=0; i<mRowStretchFactors.size(); ++i)
    {
      if (mRowStretchFactors.at(i) <= 0)
      {
        qDebug() << Q_FUNC_INFO << "Invalid stretch factor, must be positive:" << mRowStretchFactors.at(i);
        mRowStretchFactors[i] = 1;
      }
    }
  } else
    qDebug() << Q_FUNC_INFO << "Row count not equal to passed stretch factor count:" << factors;
}

/*!
  Sets the gap that is left blank between columns to \a pixels.
  
  \see setRowSpacing
*/
void QCPLayoutGrid::setColumnSpacing(int pixels)
{
  mColumnSpacing = pixels;
}

/*!
  Sets the gap that is left blank between rows to \a pixels.
  
  \see setColumnSpacing
*/
void QCPLayoutGrid::setRowSpacing(int pixels)
{
  mRowSpacing = pixels;
}

/*!
  Expands the layout to have \a newRowCount rows and \a newColumnCount columns. So the last valid
  row index will be \a newRowCount-1, the last valid column index will be \a newColumnCount-1.
  
  If the current column/row count is already larger or equal to \a newColumnCount/\a newRowCount,
  this function does nothing in that dimension.
  
  Newly created cells are empty, new rows and columns have the stretch factor 1.
  
  Note that upon a call to \ref addElement, the layout is expanded automatically to contain the
  specified row and column, using this function.
  
  \see simplify
*/
void QCPLayoutGrid::expandTo(int newRowCount, int newColumnCount)
{
  // add rows as necessary:
  while (rowCount() < newRowCount)
  {
    mElements.append(QList<QCPLayoutElement*>());
    mRowStretchFactors.append(1);
  }
  // go through rows and expand columns as necessary:
  int newColCount = qMax(columnCount(), newColumnCount);
  for (int i=0; i<rowCount(); ++i)
  {
    while (mElements.at(i).size() < newColCount)
      mElements[i].append(0);
  }
  while (mColumnStretchFactors.size() < newColCount)
    mColumnStretchFactors.append(1);
}

/*!
  Inserts a new row with empty cells at the row index \a newIndex. Valid values for \a newIndex
  range from 0 (inserts a row at the top) to \a rowCount (appends a row at the bottom).
  
  \see insertColumn
*/
void QCPLayoutGrid::insertRow(int newIndex)
{
  if (mElements.isEmpty() || mElements.first().isEmpty()) // if grid is completely empty, add first cell
  {
    expandTo(1, 1);
    return;
  }
  
  if (newIndex < 0)
    newIndex = 0;
  if (newIndex > rowCount())
    newIndex = rowCount();
  
  mRowStretchFactors.insert(newIndex, 1);
  QList<QCPLayoutElement*> newRow;
  for (int col=0; col<columnCount(); ++col)
    newRow.append((QCPLayoutElement*)0);
  mElements.insert(newIndex, newRow);
}

/*!
  Inserts a new column with empty cells at the column index \a newIndex. Valid values for \a
  newIndex range from 0 (inserts a row at the left) to \a rowCount (appends a row at the right).
  
  \see insertRow
*/
void QCPLayoutGrid::insertColumn(int newIndex)
{
  if (mElements.isEmpty() || mElements.first().isEmpty()) // if grid is completely empty, add first cell
  {
    expandTo(1, 1);
    return;
  }
  
  if (newIndex < 0)
    newIndex = 0;
  if (newIndex > columnCount())
    newIndex = columnCount();
  
  mColumnStretchFactors.insert(newIndex, 1);
  for (int row=0; row<rowCount(); ++row)
    mElements[row].insert(newIndex, (QCPLayoutElement*)0);
}

/* inherits documentation from base class */
void QCPLayoutGrid::updateLayout()
{
  QVector<int> minColWidths, minRowHeights, maxColWidths, maxRowHeights;
  getMinimumRowColSizes(&minColWidths, &minRowHeights);
  getMaximumRowColSizes(&maxColWidths, &maxRowHeights);
  
  int totalRowSpacing = (rowCount()-1) * mRowSpacing;
  int totalColSpacing = (columnCount()-1) * mColumnSpacing;
  QVector<int> colWidths = getSectionSizes(maxColWidths, minColWidths, mColumnStretchFactors.toVector(), mRect.width()-totalColSpacing);
  QVector<int> rowHeights = getSectionSizes(maxRowHeights, minRowHeights, mRowStretchFactors.toVector(), mRect.height()-totalRowSpacing);
  
  // go through cells and set rects accordingly:
  int yOffset = mRect.top();
  for (int row=0; row<rowCount(); ++row)
  {
    if (row > 0)
      yOffset += rowHeights.at(row-1)+mRowSpacing;
    int xOffset = mRect.left();
    for (int col=0; col<columnCount(); ++col)
    {
      if (col > 0)
        xOffset += colWidths.at(col-1)+mColumnSpacing;
      if (mElements.at(row).at(col))
        mElements.at(row).at(col)->setOuterRect(QRect(xOffset, yOffset, colWidths.at(col), rowHeights.at(row)));
    }
  }
}

/* inherits documentation from base class */
int QCPLayoutGrid::elementCount() const
{
  return rowCount()*columnCount();
}

/* inherits documentation from base class */
QCPLayoutElement *QCPLayoutGrid::elementAt(int index) const
{
  if (index >= 0 && index < elementCount())
    return mElements.at(index / columnCount()).at(index % columnCount());
  else
    return 0;
}

/* inherits documentation from base class */
QCPLayoutElement *QCPLayoutGrid::takeAt(int index)
{
  if (QCPLayoutElement *el = elementAt(index))
  {
    releaseElement(el);
    mElements[index / columnCount()][index % columnCount()] = 0;
    return el;
  } else
  {
    qDebug() << Q_FUNC_INFO << "Attempt to take invalid index:" << index;
    return 0;
  }
}

/* inherits documentation from base class */
bool QCPLayoutGrid::take(QCPLayoutElement *element)
{
  if (element)
  {
    for (int i=0; i<elementCount(); ++i)
    {
      if (elementAt(i) == element)
      {
        takeAt(i);
        return true;
      }
    }
    qDebug() << Q_FUNC_INFO << "Element not in this layout, couldn't take";
  } else
    qDebug() << Q_FUNC_INFO << "Can't take null element";
  return false;
}

/* inherits documentation from base class */
QList<QCPLayoutElement*> QCPLayoutGrid::elements(bool recursive) const
{
  QList<QCPLayoutElement*> result;
  int colC = columnCount();
  int rowC = rowCount();
#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
  result.reserve(colC*rowC);
#endif
  for (int row=0; row<rowC; ++row)
  {
    for (int col=0; col<colC; ++col)
    {
      result.append(mElements.at(row).at(col));
    }
  }
  if (recursive)
  {
    int c = result.size();
    for (int i=0; i<c; ++i)
    {
      if (result.at(i))
        result << result.at(i)->elements(recursive);
    }
  }
  return result;
}

/*!
  Simplifies the layout by collapsing rows and columns which only contain empty cells.
*/
void QCPLayoutGrid::simplify()
{
  // remove rows with only empty cells:
  for (int row=rowCount()-1; row>=0; --row)
  {
    bool hasElements = false;
    for (int col=0; col<columnCount(); ++col)
    {
      if (mElements.at(row).at(col))
      {
        hasElements = true;
        break;
      }
    }
    if (!hasElements)
    {
      mRowStretchFactors.removeAt(row);
      mElements.removeAt(row);
      if (mElements.isEmpty()) // removed last element, also remove stretch factor (wouldn't happen below because also columnCount changed to 0 now)
        mColumnStretchFactors.clear();
    }
  }
  
  // remove columns with only empty cells:
  for (int col=columnCount()-1; col>=0; --col)
  {
    bool hasElements = false;
    for (int row=0; row<rowCount(); ++row)
    {
      if (mElements.at(row).at(col))
      {
        hasElements = true;
        break;
      }
    }
    if (!hasElements)
    {
      mColumnStretchFactors.removeAt(col);
      for (int row=0; row<rowCount(); ++row)
        mElements[row].removeAt(col);
    }
  }
}

/* inherits documentation from base class */
QSize QCPLayoutGrid::minimumSizeHint() const
{
  QVector<int> minColWidths, minRowHeights;
  getMinimumRowColSizes(&minColWidths, &minRowHeights);
  QSize result(0, 0);
  for (int i=0; i<minColWidths.size(); ++i)
    result.rwidth() += minColWidths.at(i);
  for (int i=0; i<minRowHeights.size(); ++i)
    result.rheight() += minRowHeights.at(i);
  result.rwidth() += qMax(0, columnCount()-1) * mColumnSpacing + mMargins.left() + mMargins.right();
  result.rheight() += qMax(0, rowCount()-1) * mRowSpacing + mMargins.top() + mMargins.bottom();
  return result;
}

/* inherits documentation from base class */
QSize QCPLayoutGrid::maximumSizeHint() const
{
  QVector<int> maxColWidths, maxRowHeights;
  getMaximumRowColSizes(&maxColWidths, &maxRowHeights);
  
  QSize result(0, 0);
  for (int i=0; i<maxColWidths.size(); ++i)
    result.setWidth(qMin(result.width()+maxColWidths.at(i), QWIDGETSIZE_MAX));
  for (int i=0; i<maxRowHeights.size(); ++i)
    result.setHeight(qMin(result.height()+maxRowHeights.at(i), QWIDGETSIZE_MAX));
  result.rwidth() += qMax(0, columnCount()-1) * mColumnSpacing + mMargins.left() + mMargins.right();
  result.rheight() += qMax(0, rowCount()-1) * mRowSpacing + mMargins.top() + mMargins.bottom();
  return result;
}

/*! \internal
  
  Places the minimum column widths and row heights into \a minColWidths and \a minRowHeights
  respectively.
  
  The minimum height of a row is the largest minimum height of any element in that row. The minimum
  width of a column is the largest minimum width of any element in that column.
  
  This is a helper function for \ref updateLayout.
  
  \see getMaximumRowColSizes
*/
void QCPLayoutGrid::getMinimumRowColSizes(QVector<int> *minColWidths, QVector<int> *minRowHeights) const
{
  *minColWidths = QVector<int>(columnCount(), 0);
  *minRowHeights = QVector<int>(rowCount(), 0);
  for (int row=0; row<rowCount(); ++row)
  {
    for (int col=0; col<columnCount(); ++col)
    {
      if (mElements.at(row).at(col))
      {
        QSize minHint = mElements.at(row).at(col)->minimumSizeHint();
        QSize min = mElements.at(row).at(col)->minimumSize();
        QSize final(min.width() > 0 ? min.width() : minHint.width(), min.height() > 0 ? min.height() : minHint.height());
        if (minColWidths->at(col) < final.width())
          (*minColWidths)[col] = final.width();
        if (minRowHeights->at(row) < final.height())
          (*minRowHeights)[row] = final.height();
      }
    }
  }
}

/*! \internal
  
  Places the maximum column widths and row heights into \a maxColWidths and \a maxRowHeights
  respectively.
  
  The maximum height of a row is the smallest maximum height of any element in that row. The
  maximum width of a column is the smallest maximum width of any element in that column.
  
  This is a helper function for \ref updateLayout.
  
  \see getMinimumRowColSizes
*/
void QCPLayoutGrid::getMaximumRowColSizes(QVector<int> *maxColWidths, QVector<int> *maxRowHeights) const
{
  *maxColWidths = QVector<int>(columnCount(), QWIDGETSIZE_MAX);
  *maxRowHeights = QVector<int>(rowCount(), QWIDGETSIZE_MAX);
  for (int row=0; row<rowCount(); ++row)
  {
    for (int col=0; col<columnCount(); ++col)
    {
      if (mElements.at(row).at(col))
      {
        QSize maxHint = mElements.at(row).at(col)->maximumSizeHint();
        QSize max = mElements.at(row).at(col)->maximumSize();
        QSize final(max.width() < QWIDGETSIZE_MAX ? max.width() : maxHint.width(), max.height() < QWIDGETSIZE_MAX ? max.height() : maxHint.height());
        if (maxColWidths->at(col) > final.width())
          (*maxColWidths)[col] = final.width();
        if (maxRowHeights->at(row) > final.height())
          (*maxRowHeights)[row] = final.height();
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLayoutInset
////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \class QCPLayoutInset
  \brief A layout that places child elements aligned to the border or arbitrarily positioned
  
  Elements are placed either aligned to the border or at arbitrary position in the area of the
  layout. Which placement applies is controlled with the \ref InsetPlacement (\ref
  setInsetPlacement).

  Elements are added via \ref addElement(QCPLayoutElement *element, Qt::Alignment alignment) or
  addElement(QCPLayoutElement *element, const QRectF &rect). If the first method is used, the inset
  placement will default to \ref ipBorderAligned and the element will be aligned according to the
  \a alignment parameter. The second method defaults to \ref ipFree and allows placing elements at
  arbitrary position and size, defined by \a rect.
  
  The alignment or rect can be set via \ref setInsetAlignment or \ref setInsetRect, respectively.
  
  This is the layout that every QCPAxisRect has as \ref QCPAxisRect::insetLayout.
*/

/* start documentation of inline functions */

/*! \fn virtual void QCPLayoutInset::simplify()
  
  The QCPInsetLayout does not need simplification since it can never have empty cells due to its
  linear index structure. This method does nothing.
*/

/* end documentation of inline functions */

/*!
  Creates an instance of QCPLayoutInset and sets default values.
*/
QCPLayoutInset::QCPLayoutInset()
{
}

QCPLayoutInset::~QCPLayoutInset()
{
  // clear all child layout elements. This is important because only the specific layouts know how
  // to handle removing elements (clear calls virtual removeAt method to do that).
  clear();
}

/*!
  Returns the placement type of the element with the specified \a index.
*/
QCPLayoutInset::InsetPlacement QCPLayoutInset::insetPlacement(int index) const
{
  if (elementAt(index))
    return mInsetPlacement.at(index);
  else
  {
    qDebug() << Q_FUNC_INFO << "Invalid element index:" << index;
    return ipFree;
  }
}

/*!
  Returns the alignment of the element with the specified \a index. The alignment only has a
  meaning, if the inset placement (\ref setInsetPlacement) is \ref ipBorderAligned.
*/
Qt::Alignment QCPLayoutInset::insetAlignment(int index) const
{
  if (elementAt(index))
    return mInsetAlignment.at(index);
  else
  {
    qDebug() << Q_FUNC_INFO << "Invalid element index:" << index;
    return 0;
  }
}

/*!
  Returns the rect of the element with the specified \a index. The rect only has a
  meaning, if the inset placement (\ref setInsetPlacement) is \ref ipFree.
*/
QRectF QCPLayoutInset::insetRect(int index) const
{
  if (elementAt(index))
    return mInsetRect.at(index);
  else
  {
    qDebug() << Q_FUNC_INFO << "Invalid element index:" << index;
    return QRectF();
  }
}

/*!
  Sets the inset placement type of the element with the specified \a index to \a placement.
  
  \see InsetPlacement
*/
void QCPLayoutInset::setInsetPlacement(int index, QCPLayoutInset::InsetPlacement placement)
{
  if (elementAt(index))
    mInsetPlacement[index] = placement;
  else
    qDebug() << Q_FUNC_INFO << "Invalid element index:" << index;
}

/*!
  If the inset placement (\ref setInsetPlacement) is \ref ipBorderAligned, this function
  is used to set the alignment of the element with the specified \a index to \a alignment.
  
  \a alignment is an or combination of the following alignment flags: Qt::AlignLeft,
  Qt::AlignHCenter, Qt::AlighRight, Qt::AlignTop, Qt::AlignVCenter, Qt::AlignBottom. Any other
  alignment flags will be ignored.
*/
void QCPLayoutInset::setInsetAlignment(int index, Qt::Alignment alignment)
{
  if (elementAt(index))
    mInsetAlignment[index] = alignment;
  else
    qDebug() << Q_FUNC_INFO << "Invalid element index:" << index;
}

/*!
  If the inset placement (\ref setInsetPlacement) is \ref ipFree, this function is used to set the
  position and size of the element with the specified \a index to \a rect.
  
  \a rect is given in fractions of the whole inset layout rect. So an inset with rect (0, 0, 1, 1)
  will span the entire layout. An inset with rect (0.6, 0.1, 0.35, 0.35) will be in the top right
  corner of the layout, with 35% width and height of the parent layout.
  
  Note that the minimum and maximum sizes of the embedded element (\ref
  QCPLayoutElement::setMinimumSize, \ref QCPLayoutElement::setMaximumSize) are enforced.
*/
void QCPLayoutInset::setInsetRect(int index, const QRectF &rect)
{
  if (elementAt(index))
    mInsetRect[index] = rect;
  else
    qDebug() << Q_FUNC_INFO << "Invalid element index:" << index;
}

/* inherits documentation from base class */
void QCPLayoutInset::updateLayout()
{
  for (int i=0; i<mElements.size(); ++i)
  {
    QRect insetRect;
    QSize finalMinSize, finalMaxSize;
    QSize minSizeHint = mElements.at(i)->minimumSizeHint();
    QSize maxSizeHint = mElements.at(i)->maximumSizeHint();
    finalMinSize.setWidth(mElements.at(i)->minimumSize().width() > 0 ? mElements.at(i)->minimumSize().width() : minSizeHint.width());
    finalMinSize.setHeight(mElements.at(i)->minimumSize().height() > 0 ? mElements.at(i)->minimumSize().height() : minSizeHint.height());
    finalMaxSize.setWidth(mElements.at(i)->maximumSize().width() < QWIDGETSIZE_MAX ? mElements.at(i)->maximumSize().width() : maxSizeHint.width());
    finalMaxSize.setHeight(mElements.at(i)->maximumSize().height() < QWIDGETSIZE_MAX ? mElements.at(i)->maximumSize().height() : maxSizeHint.height());
    if (mInsetPlacement.at(i) == ipFree)
    {
      insetRect = QRect(rect().x()+rect().width()*mInsetRect.at(i).x(),
                        rect().y()+rect().height()*mInsetRect.at(i).y(),
                        rect().width()*mInsetRect.at(i).width(),
                        rect().height()*mInsetRect.at(i).height());
      if (insetRect.size().width() < finalMinSize.width())
        insetRect.setWidth(finalMinSize.width());
      if (insetRect.size().height() < finalMinSize.height())
        insetRect.setHeight(finalMinSize.height());
      if (insetRect.size().width() > finalMaxSize.width())
        insetRect.setWidth(finalMaxSize.width());
      if (insetRect.size().height() > finalMaxSize.height())
        insetRect.setHeight(finalMaxSize.height());
    } else if (mInsetPlacement.at(i) == ipBorderAligned)
    {
      insetRect.setSize(finalMinSize);
      Qt::Alignment al = mInsetAlignment.at(i);
      if (al.testFlag(Qt::AlignLeft)) insetRect.moveLeft(rect().x());
      else if (al.testFlag(Qt::AlignRight)) insetRect.moveRight(rect().x()+rect().width());
      else insetRect.moveLeft(rect().x()+rect().width()*0.5-finalMinSize.width()*0.5); // default to Qt::AlignHCenter
      if (al.testFlag(Qt::AlignTop)) insetRect.moveTop(rect().y());
      else if (al.testFlag(Qt::AlignBottom)) insetRect.moveBottom(rect().y()+rect().height());
      else insetRect.moveTop(rect().y()+rect().height()*0.5-finalMinSize.height()*0.5); // default to Qt::AlignVCenter
    }
    mElements.at(i)->setOuterRect(insetRect);
  }
}

/* inherits documentation from base class */
int QCPLayoutInset::elementCount() const
{
  return mElements.size();
}

/* inherits documentation from base class */
QCPLayoutElement *QCPLayoutInset::elementAt(int index) const
{
  if (index >= 0 && index < mElements.size())
    return mElements.at(index);
  else
    return 0;
}

/* inherits documentation from base class */
QCPLayoutElement *QCPLayoutInset::takeAt(int index)
{
  if (QCPLayoutElement *el = elementAt(index))
  {
    releaseElement(el);
    mElements.removeAt(index);
    mInsetPlacement.removeAt(index);
    mInsetAlignment.removeAt(index);
    mInsetRect.removeAt(index);
    return el;
  } else
  {
    qDebug() << Q_FUNC_INFO << "Attempt to take invalid index:" << index;
    return 0;
  }
}

/* inherits documentation from base class */
bool QCPLayoutInset::take(QCPLayoutElement *element)
{
  if (element)
  {
    for (int i=0; i<elementCount(); ++i)
    {
      if (elementAt(i) == element)
      {
        takeAt(i);
        return true;
      }
    }
    qDebug() << Q_FUNC_INFO << "Element not in this layout, couldn't take";
  } else
    qDebug() << Q_FUNC_INFO << "Can't take null element";
  return false;
}

/*!
  The inset layout is sensitive to events only at areas where its child elements are sensitive. If
  the selectTest method of any of the child elements returns a positive number for \a pos, this
  method returns a value corresponding to 0.99 times the parent plot's selection tolerance. The
  inset layout is not selectable itself by default. So if \a onlySelectable is true, -1.0 is
  returned.
  
  See \ref QCPLayerable::selectTest for a general explanation of this virtual method.
*/
double QCPLayoutInset::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  Q_UNUSED(details)
  if (onlySelectable)
    return -1;
  
  for (int i=0; i<mElements.size(); ++i)
  {
    // inset layout shall only return positive selectTest, if actually an inset object is at pos
    // else it would block the entire underlying QCPAxisRect with its surface.
    if (mElements.at(i)->selectTest(pos, onlySelectable) >= 0)
      return mParentPlot->selectionTolerance()*0.99;
  }
  return -1;
}

/*!
  Adds the specified \a element to the layout as an inset aligned at the border (\ref
  setInsetAlignment is initialized with \ref ipBorderAligned). The alignment is set to \a
  alignment.
  
  \a alignment is an or combination of the following alignment flags: Qt::AlignLeft,
  Qt::AlignHCenter, Qt::AlighRight, Qt::AlignTop, Qt::AlignVCenter, Qt::AlignBottom. Any other
  alignment flags will be ignored.
  
  \see addElement(QCPLayoutElement *element, const QRectF &rect)
*/
void QCPLayoutInset::addElement(QCPLayoutElement *element, Qt::Alignment alignment)
{
  if (element)
  {
    if (element->layout()) // remove from old layout first
      element->layout()->take(element);
    mElements.append(element);
    mInsetPlacement.append(ipBorderAligned);
    mInsetAlignment.append(alignment);
    mInsetRect.append(QRectF(0.6, 0.6, 0.4, 0.4));
    adoptElement(element);
  } else
    qDebug() << Q_FUNC_INFO << "Can't add null element";
}

/*!
  Adds the specified \a element to the layout as an inset with free positioning/sizing (\ref
  setInsetAlignment is initialized with \ref ipFree). The position and size is set to \a
  rect.
  
  \a rect is given in fractions of the whole inset layout rect. So an inset with rect (0, 0, 1, 1)
  will span the entire layout. An inset with rect (0.6, 0.1, 0.35, 0.35) will be in the top right
  corner of the layout, with 35% width and height of the parent layout.
  
  \see addElement(QCPLayoutElement *element, Qt::Alignment alignment)
*/
void QCPLayoutInset::addElement(QCPLayoutElement *element, const QRectF &rect)
{
  if (element)
  {
    if (element->layout()) // remove from old layout first
      element->layout()->take(element);
    mElements.append(element);
    mInsetPlacement.append(ipFree);
    mInsetAlignment.append(Qt::AlignRight|Qt::AlignTop);
    mInsetRect.append(rect);
    adoptElement(element);
  } else
    qDebug() << Q_FUNC_INFO << "Can't add null element";
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPLineEnding
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPLineEnding
  \brief Handles the different ending decorations for line-like items
  
  \image html QCPLineEnding.png "The various ending styles currently supported"
  
  For every ending a line-like item has, an instance of this class exists. For example, QCPItemLine
  has two endings which can be set with QCPItemLine::setHead and QCPItemLine::setTail.
 
  The styles themselves are defined via the enum QCPLineEnding::EndingStyle. Most decorations can
  be modified regarding width and length, see \ref setWidth and \ref setLength. The direction of
  the ending decoration (e.g. direction an arrow is pointing) is controlled by the line-like item.
  For example, when both endings of a QCPItemLine are set to be arrows, they will point to opposite
  directions, e.g. "outward". This can be changed by \ref setInverted, which would make the
  respective arrow point inward.
  
  Note that due to the overloaded QCPLineEnding constructor, you may directly specify a
  QCPLineEnding::EndingStyle where actually a QCPLineEnding is expected, e.g. \code
  myItemLine->setHead(QCPLineEnding::esSpikeArrow) \endcode
*/

/*!
  Creates a QCPLineEnding instance with default values (style \ref esNone).
*/
QCPLineEnding::QCPLineEnding() :
  mStyle(esNone),
  mWidth(8),
  mLength(10),
  mInverted(false)
{
}

/*!
  Creates a QCPLineEnding instance with the specified values.
*/
QCPLineEnding::QCPLineEnding(QCPLineEnding::EndingStyle style, double width, double length, bool inverted) :
  mStyle(style),
  mWidth(width),
  mLength(length),
  mInverted(inverted)
{
}

/*!
  Sets the style of the ending decoration.
*/
void QCPLineEnding::setStyle(QCPLineEnding::EndingStyle style)
{
  mStyle = style;
}

/*!
  Sets the width of the ending decoration, if the style supports it. On arrows, for example, the
  width defines the size perpendicular to the arrow's pointing direction.
  
  \see setLength
*/
void QCPLineEnding::setWidth(double width)
{
  mWidth = width;
}

/*!
  Sets the length of the ending decoration, if the style supports it. On arrows, for example, the
  length defines the size in pointing direction.
  
  \see setWidth
*/
void QCPLineEnding::setLength(double length)
{
  mLength = length;
}

/*!
  Sets whether the ending decoration shall be inverted. For example, an arrow decoration will point
  inward when \a inverted is set to true.

  Note that also the \a width direction is inverted. For symmetrical ending styles like arrows or
  discs, this doesn't make a difference. However, asymmetric styles like \ref esHalfBar are
  affected by it, which can be used to control to which side the half bar points to.
*/
void QCPLineEnding::setInverted(bool inverted)
{
  mInverted = inverted;
}

/*! \internal
  
  Returns the maximum pixel radius the ending decoration might cover, starting from the position
  the decoration is drawn at (typically a line ending/\ref QCPItemPosition of an item).
  
  This is relevant for clipping. Only omit painting of the decoration when the position where the
  decoration is supposed to be drawn is farther away from the clipping rect than the returned
  distance.
*/
double QCPLineEnding::boundingDistance() const
{
  switch (mStyle)
  {
    case esNone:
      return 0;
      
    case esFlatArrow:
    case esSpikeArrow:
    case esLineArrow:
    case esSkewedBar:
      return qSqrt(mWidth*mWidth+mLength*mLength); // items that have width and length
      
    case esDisc:
    case esSquare:
    case esDiamond:
    case esBar:
    case esHalfBar:
      return mWidth*1.42; // items that only have a width -> width*sqrt(2)

  }
  return 0;
}

/*!
  Starting from the origin of this line ending (which is style specific), returns the length
  covered by the line ending symbol, in backward direction.
  
  For example, the \ref esSpikeArrow has a shorter real length than a \ref esFlatArrow, even if
  both have the same \ref setLength value, because the spike arrow has an inward curved back, which
  reduces the length along its center axis (the drawing origin for arrows is at the tip).
  
  This function is used for precise, style specific placement of line endings, for example in
  QCPAxes.
*/
double QCPLineEnding::realLength() const
{
  switch (mStyle)
  {
    case esNone:
    case esLineArrow:
    case esSkewedBar:
    case esBar:
    case esHalfBar:
      return 0;
      
    case esFlatArrow:
      return mLength;
      
    case esDisc:
    case esSquare:
    case esDiamond:
      return mWidth*0.5;
      
    case esSpikeArrow:
      return mLength*0.8;
  }
  return 0;
}

/*! \internal
  
  Draws the line ending with the specified \a painter at the position \a pos. The direction of the
  line ending is controlled with \a dir.
*/
void QCPLineEnding::draw(QCPPainter *painter, const QVector2D &pos, const QVector2D &dir) const
{
  if (mStyle == esNone)
    return;
  
  QVector2D lengthVec(dir.normalized());
  if (lengthVec.isNull())
    lengthVec = QVector2D(1, 0);
  QVector2D widthVec(-lengthVec.y(), lengthVec.x());
  lengthVec *= mLength*(mInverted ? -1 : 1);
  widthVec *= mWidth*0.5*(mInverted ? -1 : 1);
  
  QPen penBackup = painter->pen();
  QBrush brushBackup = painter->brush();
  QPen miterPen = penBackup;
  miterPen.setJoinStyle(Qt::MiterJoin); // to make arrow heads spikey
  QBrush brush(painter->pen().color(), Qt::SolidPattern);
  switch (mStyle)
  {
    case esNone: break;
    case esFlatArrow:
    {
      QPointF points[3] = {pos.toPointF(),
                           (pos-lengthVec+widthVec).toPointF(),
                           (pos-lengthVec-widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->setBrush(brush);
      painter->drawConvexPolygon(points, 3);
      painter->setBrush(brushBackup);
      painter->setPen(penBackup);
      break;
    }
    case esSpikeArrow:
    {
      QPointF points[4] = {pos.toPointF(),
                           (pos-lengthVec+widthVec).toPointF(),
                           (pos-lengthVec*0.8).toPointF(),
                           (pos-lengthVec-widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->setBrush(brush);
      painter->drawConvexPolygon(points, 4);
      painter->setBrush(brushBackup);
      painter->setPen(penBackup);
      break;
    }
    case esLineArrow:
    {
      QPointF points[3] = {(pos-lengthVec+widthVec).toPointF(),
                           pos.toPointF(),
                           (pos-lengthVec-widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->drawPolyline(points, 3);
      painter->setPen(penBackup);
      break;
    }
    case esDisc:
    {
      painter->setBrush(brush);
      painter->drawEllipse(pos.toPointF(),  mWidth*0.5, mWidth*0.5);
      painter->setBrush(brushBackup);
      break;
    }
    case esSquare:
    {
      QVector2D widthVecPerp(-widthVec.y(), widthVec.x());
      QPointF points[4] = {(pos-widthVecPerp+widthVec).toPointF(),
                           (pos-widthVecPerp-widthVec).toPointF(),
                           (pos+widthVecPerp-widthVec).toPointF(),
                           (pos+widthVecPerp+widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->setBrush(brush);
      painter->drawConvexPolygon(points, 4);
      painter->setBrush(brushBackup);
      painter->setPen(penBackup);
      break;
    }
    case esDiamond:
    {
      QVector2D widthVecPerp(-widthVec.y(), widthVec.x());
      QPointF points[4] = {(pos-widthVecPerp).toPointF(),
                           (pos-widthVec).toPointF(),
                           (pos+widthVecPerp).toPointF(),
                           (pos+widthVec).toPointF()
                          };
      painter->setPen(miterPen);
      painter->setBrush(brush);
      painter->drawConvexPolygon(points, 4);
      painter->setBrush(brushBackup);
      painter->setPen(penBackup);
      break;
    }
    case esBar:
    {
      painter->drawLine((pos+widthVec).toPointF(), (pos-widthVec).toPointF());
      break;
    }
    case esHalfBar:
    {
      painter->drawLine((pos+widthVec).toPointF(), pos.toPointF());
      break;
    }
    case esSkewedBar:
    {
      if (qFuzzyIsNull(painter->pen().widthF()) && !painter->modes().testFlag(QCPPainter::pmNonCosmetic))
      {
        // if drawing with cosmetic pen (perfectly thin stroke, happens only in vector exports), draw bar exactly on tip of line
        painter->drawLine((pos+widthVec+lengthVec*0.2*(mInverted?-1:1)).toPointF(),
                          (pos-widthVec-lengthVec*0.2*(mInverted?-1:1)).toPointF());
      } else
      {
        // if drawing with thick (non-cosmetic) pen, shift bar a little in line direction to prevent line from sticking through bar slightly
        painter->drawLine((pos+widthVec+lengthVec*0.2*(mInverted?-1:1)+dir.normalized()*qMax(1.0, (double)painter->pen().widthF())*0.5).toPointF(),
                          (pos-widthVec-lengthVec*0.2*(mInverted?-1:1)+dir.normalized()*qMax(1.0, (double)painter->pen().widthF())*0.5).toPointF());
      }
      break;
    }
  }
}

/*! \internal
  \overload
  
  Draws the line ending. The direction is controlled with the \a angle parameter in radians.
*/
void QCPLineEnding::draw(QCPPainter *painter, const QVector2D &pos, double angle) const
{
  draw(painter, pos, QVector2D(qCos(angle), qSin(angle)));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPGrid
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPGrid
  \brief Responsible for drawing the grid of a QCPAxis.
  
  This class is tightly bound to QCPAxis. Every axis owns a grid instance and uses it to draw the
  grid lines, sub grid lines and zero-line. You can interact with the grid of an axis via \ref
  QCPAxis::grid. Normally, you don't need to create an instance of QCPGrid yourself.
  
  The axis and grid drawing was split into two classes to allow them to be placed on different
  layers (both QCPAxis and QCPGrid inherit from QCPLayerable). Thus it is possible to have the grid
  in the background and the axes in the foreground, and any plottables/items in between. This
  described situation is the default setup, see the QCPLayer documentation.
*/

/*!
  Creates a QCPGrid instance and sets default values.
  
  You shouldn't instantiate grids on their own, since every QCPAxis brings its own QCPGrid.
*/
QCPGrid::QCPGrid(QCPAxis *parentAxis) :
  QCPLayerable(parentAxis->parentPlot(), "", parentAxis),
  mParentAxis(parentAxis)
{
  // warning: this is called in QCPAxis constructor, so parentAxis members should not be accessed/called
  setParent(parentAxis);
  setPen(QPen(QColor(200,200,200), 0, Qt::DotLine));
  setSubGridPen(QPen(QColor(220,220,220), 0, Qt::DotLine));
  setZeroLinePen(QPen(QColor(200,200,200), 0, Qt::SolidLine));
  setSubGridVisible(false);
  setAntialiased(false);
  setAntialiasedSubGrid(false);
  setAntialiasedZeroLine(false);
}

/*!
  Sets whether grid lines at sub tick marks are drawn.
  
  \see setSubGridPen
*/
void QCPGrid::setSubGridVisible(bool visible)
{
  mSubGridVisible = visible;
}

/*!
  Sets whether sub grid lines are drawn antialiased.
*/
void QCPGrid::setAntialiasedSubGrid(bool enabled)
{
  mAntialiasedSubGrid = enabled;
}

/*!
  Sets whether zero lines are drawn antialiased.
*/
void QCPGrid::setAntialiasedZeroLine(bool enabled)
{
  mAntialiasedZeroLine = enabled;
}

/*!
  Sets the pen with which (major) grid lines are drawn.
*/
void QCPGrid::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  Sets the pen with which sub grid lines are drawn.
*/
void QCPGrid::setSubGridPen(const QPen &pen)
{
  mSubGridPen = pen;
}

/*!
  Sets the pen with which zero lines are drawn.
  
  Zero lines are lines at value coordinate 0 which may be drawn with a different pen than other grid
  lines. To disable zero lines and just draw normal grid lines at zero, set \a pen to Qt::NoPen.
*/
void QCPGrid::setZeroLinePen(const QPen &pen)
{
  mZeroLinePen = pen;
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing the major grid lines.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased
*/
void QCPGrid::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeGrid);
}

/*! \internal
  
  Draws grid lines and sub grid lines at the positions of (sub) ticks of the parent axis, spanning
  over the complete axis rect. Also draws the zero line, if appropriate (\ref setZeroLinePen).
*/
void QCPGrid::draw(QCPPainter *painter)
{
  if (!mParentAxis) { qDebug() << Q_FUNC_INFO << "invalid parent axis"; return; }
  
  if (mSubGridVisible)
    drawSubGridLines(painter);
  drawGridLines(painter);
}

/*! \internal
  
  Draws the main grid lines and possibly a zero line with the specified painter.
  
  This is a helper function called by \ref draw.
*/
void QCPGrid::drawGridLines(QCPPainter *painter) const
{
  if (!mParentAxis) { qDebug() << Q_FUNC_INFO << "invalid parent axis"; return; }
  
  int lowTick = mParentAxis->mLowestVisibleTick;
  int highTick = mParentAxis->mHighestVisibleTick;
  double t; // helper variable, result of coordinate-to-pixel transforms
  if (mParentAxis->orientation() == Qt::Horizontal)
  {
    // draw zeroline:
    int zeroLineIndex = -1;
    if (mZeroLinePen.style() != Qt::NoPen && mParentAxis->mRange.lower < 0 && mParentAxis->mRange.upper > 0)
    {
      applyAntialiasingHint(painter, mAntialiasedZeroLine, QCP::aeZeroLine);
      painter->setPen(mZeroLinePen);
      double epsilon = mParentAxis->range().size()*1E-6; // for comparing double to zero
      for (int i=lowTick; i <= highTick; ++i)
      {
        if (qAbs(mParentAxis->mTickVector.at(i)) < epsilon)
        {
          zeroLineIndex = i;
          t = mParentAxis->coordToPixel(mParentAxis->mTickVector.at(i)); // x
          painter->drawLine(QLineF(t, mParentAxis->mAxisRect->bottom(), t, mParentAxis->mAxisRect->top()));
          break;
        }
      }
    }
    // draw grid lines:
    applyDefaultAntialiasingHint(painter);
    painter->setPen(mPen);
    for (int i=lowTick; i <= highTick; ++i)
    {
      if (i == zeroLineIndex) continue; // don't draw a gridline on top of the zeroline
      t = mParentAxis->coordToPixel(mParentAxis->mTickVector.at(i)); // x
      painter->drawLine(QLineF(t, mParentAxis->mAxisRect->bottom(), t, mParentAxis->mAxisRect->top()));
    }
  } else
  {
    // draw zeroline:
    int zeroLineIndex = -1;
    if (mZeroLinePen.style() != Qt::NoPen && mParentAxis->mRange.lower < 0 && mParentAxis->mRange.upper > 0)
    {
      applyAntialiasingHint(painter, mAntialiasedZeroLine, QCP::aeZeroLine);
      painter->setPen(mZeroLinePen);
      double epsilon = mParentAxis->mRange.size()*1E-6; // for comparing double to zero
      for (int i=lowTick; i <= highTick; ++i)
      {
        if (qAbs(mParentAxis->mTickVector.at(i)) < epsilon)
        {
          zeroLineIndex = i;
          t = mParentAxis->coordToPixel(mParentAxis->mTickVector.at(i)); // y
          painter->drawLine(QLineF(mParentAxis->mAxisRect->left(), t, mParentAxis->mAxisRect->right(), t));
          break;
        }
      }
    }
    // draw grid lines:
    applyDefaultAntialiasingHint(painter);
    painter->setPen(mPen);
    for (int i=lowTick; i <= highTick; ++i)
    {
      if (i == zeroLineIndex) continue; // don't draw a gridline on top of the zeroline
      t = mParentAxis->coordToPixel(mParentAxis->mTickVector.at(i)); // y
      painter->drawLine(QLineF(mParentAxis->mAxisRect->left(), t, mParentAxis->mAxisRect->right(), t));
    }
  }
}

/*! \internal
  
  Draws the sub grid lines with the specified painter.
  
  This is a helper function called by \ref draw.
*/
void QCPGrid::drawSubGridLines(QCPPainter *painter) const
{
  if (!mParentAxis) { qDebug() << Q_FUNC_INFO << "invalid parent axis"; return; }
  
  applyAntialiasingHint(painter, mAntialiasedSubGrid, QCP::aeSubGrid);
  double t; // helper variable, result of coordinate-to-pixel transforms
  painter->setPen(mSubGridPen);
  if (mParentAxis->orientation() == Qt::Horizontal)
  {
    for (int i=0; i<mParentAxis->mSubTickVector.size(); ++i)
    {
      t = mParentAxis->coordToPixel(mParentAxis->mSubTickVector.at(i)); // x
      painter->drawLine(QLineF(t, mParentAxis->mAxisRect->bottom(), t, mParentAxis->mAxisRect->top()));
    }
  } else
  {
    for (int i=0; i<mParentAxis->mSubTickVector.size(); ++i)
    {
      t = mParentAxis->coordToPixel(mParentAxis->mSubTickVector.at(i)); // y
      painter->drawLine(QLineF(mParentAxis->mAxisRect->left(), t, mParentAxis->mAxisRect->right(), t));
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAxis
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAxis
  \brief Manages a single axis inside a QCustomPlot.

  Usually doesn't need to be instantiated externally. Access %QCustomPlot's default four axes via
  QCustomPlot::xAxis (bottom), QCustomPlot::yAxis (left), QCustomPlot::xAxis2 (top) and
  QCustomPlot::yAxis2 (right).
  
  Axes are always part of an axis rect, see QCPAxisRect.
  \image html AxisNamesOverview.png
  <center>Naming convention of axis parts</center>
  \n
    
  \image html AxisRectSpacingOverview.png
  <center>Overview of the spacings and paddings that define the geometry of an axis. The dashed gray line
  on the left represents the QCustomPlot widget border.</center>

*/

/* start of documentation of inline functions */

/*! \fn Qt::Orientation QCPAxis::orientation() const
  
  Returns the orientation of the axis. The axis orientation (horizontal or vertical) is deduced
  from the axis type (left, top, right or bottom).
*/

/*! \fn QCPGrid *QCPAxis::grid() const
  
  Returns the \ref QCPGrid instance belonging to this axis. Access it to set details about the way the
  grid is displayed.
*/

/* end of documentation of inline functions */
/* start of documentation of signals */

/*! \fn void QCPAxis::ticksRequest()
  
  This signal is emitted when \ref setAutoTicks is false and the axis is about to generate tick
  labels for a replot.
  
  Modifying the tick positions can be done with \ref setTickVector. If you also want to control the
  tick labels, set \ref setAutoTickLabels to false and also provide the labels with \ref
  setTickVectorLabels.
  
  If you only want static ticks you probably don't need this signal, since you can just set the
  tick vector (and possibly tick label vector) once. However, if you want to provide ticks (and
  maybe labels) dynamically, e.g. depending on the current axis range, connect a slot to this
  signal and set the vector/vectors there.
*/

/*! \fn void QCPAxis::rangeChanged(const QCPRange &newRange)

  This signal is emitted when the range of this axis has changed. You can connect it to the \ref
  setRange slot of another axis to communicate the new range to the other axis, in order for it to
  be synchronized.
*/

/*! \fn void QCPAxis::rangeChanged(const QCPRange &newRange, const QCPRange &oldRange)
  \overload
  
  Additionally to the new range, this signal also provides the previous range held by the axis as
  \a oldRange.
*/

/*! \fn void QCPAxis::selectionChanged(QCPAxis::SelectableParts selection)
  
  This signal is emitted when the selection state of this axis has changed, either by user interaction
  or by a direct call to \ref setSelectedParts.
*/

/* end of documentation of signals */

/*!
  Constructs an Axis instance of Type \a type for the axis rect \a parent.
  You shouldn't instantiate axes directly, rather use \ref QCPAxisRect::addAxis.
*/
QCPAxis::QCPAxis(QCPAxisRect *parent, AxisType type) :
  QCPLayerable(parent->parentPlot(), "", parent),
  // axis base:
  mAxisType(type),
  mAxisRect(parent),
  mOffset(0),
  mPadding(5),
  mOrientation((type == atBottom || type == atTop) ? Qt::Horizontal : Qt::Vertical),
  mSelectableParts(spAxis | spTickLabels | spAxisLabel),
  mSelectedParts(spNone),
  mBasePen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedBasePen(QPen(Qt::blue, 2)),
  mLowerEnding(QCPLineEnding::esNone),
  mUpperEnding(QCPLineEnding::esNone),
  // axis label:
  mLabelPadding(0),
  mLabel(""),
  mLabelFont(mParentPlot->font()),
  mSelectedLabelFont(QFont(mLabelFont.family(), mLabelFont.pointSize(), QFont::Bold)),
  mLabelColor(Qt::black),
  mSelectedLabelColor(Qt::blue),
  // tick labels:
  mTickLabelPadding(0),
  mTickLabels(true),
  mAutoTickLabels(true),
  mTickLabelRotation(0),
  mTickLabelType(ltNumber),
  mTickLabelFont(mParentPlot->font()),
  mSelectedTickLabelFont(QFont(mTickLabelFont.family(), mTickLabelFont.pointSize(), QFont::Bold)),
  mTickLabelColor(Qt::black),
  mSelectedTickLabelColor(Qt::blue),
  mDateTimeFormat("hh:mm:ss\ndd.MM.yy"),
  mDateTimeSpec(Qt::LocalTime),
  mNumberPrecision(6),
  mNumberFormatChar('g'),
  mNumberBeautifulPowers(true),
  mNumberMultiplyCross(false),
  // ticks and subticks:
  mTicks(true),
  mTickStep(1),
  mSubTickCount(4),
  mAutoTickCount(6),
  mAutoTicks(true),
  mAutoTickStep(true),
  mAutoSubTicks(true),
  mTickLengthIn(5),
  mTickLengthOut(0),
  mSubTickLengthIn(2),
  mSubTickLengthOut(0),
  mTickPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedTickPen(QPen(Qt::blue, 2)),
  mSubTickPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::SquareCap)),
  mSelectedSubTickPen(QPen(Qt::blue, 2)),
  // scale and range:
  mRange(0, 5),
  mRangeReversed(false),
  mScaleType(stLinear),
  mScaleLogBase(10),
  mScaleLogBaseLogInv(1.0/qLn(mScaleLogBase)),
  // internal members:
  mGrid(new QCPGrid(this)),
  mLabelCache(16), // cache at most 16 (tick) labels
  mLowestVisibleTick(0),
  mHighestVisibleTick(-1),
  mExponentialChar('e'), // will be updated with locale sensitive values in setupTickVector
  mPositiveSignChar('+'), // will be updated with locale sensitive values in setupTickVector
  mCachedMarginValid(false),
  mCachedMargin(0)
{
  mGrid->setVisible(false);
  setAntialiased(false);
  setLayer(mParentPlot->currentLayer()); // it's actually on that layer already, but we want it in front of the grid, so we place it on there again
  
  if (type == atTop)
  {
    setTickLabelPadding(3);
    setLabelPadding(6);
  } else if (type == atRight)
  {
    setTickLabelPadding(7);
    setLabelPadding(12);
  } else if (type == atBottom)
  {
    setTickLabelPadding(3);
    setLabelPadding(3);
  } else if (type == atLeft)
  {
    setTickLabelPadding(5);
    setLabelPadding(10);
  }
}

/* No documentation as it is a property getter */
QString QCPAxis::numberFormat() const
{
  QString result;
  result.append(mNumberFormatChar);
  if (mNumberBeautifulPowers)
  {
    result.append("b");
    if (mNumberMultiplyCross)
      result.append("c");
  }
  return result;
}

/*!
  Sets whether the axis uses a linear scale or a logarithmic scale. If \a type is set to \ref
  stLogarithmic, the logarithm base can be set with \ref setScaleLogBase. In logarithmic axis
  scaling, major tick marks appear at all powers of the logarithm base. Properties like tick step
  (\ref setTickStep) don't apply in logarithmic scaling. If you wish a decimal base but less major
  ticks, consider choosing a logarithm base of 100, 1000 or even higher.
  
  If \a type is \ref stLogarithmic and the number format (\ref setNumberFormat) uses the 'b' option
  (beautifully typeset decimal powers), the display usually is "1 [multiplication sign] 10
  [superscript] n", which looks unnatural for logarithmic scaling (the "1 [multiplication sign]"
  part). To only display the decimal power, set the number precision to zero with
  \ref setNumberPrecision.
*/
void QCPAxis::setScaleType(ScaleType type)
{
  if (mScaleType != type)
  {
    mScaleType = type;
    if (mScaleType == stLogarithmic)
      mRange = mRange.sanitizedForLogScale();
    mCachedMarginValid = false;
  }
}

/*!
  If \ref setScaleType is set to \ref stLogarithmic, \a base will be the logarithm base of the
  scaling. In logarithmic axis scaling, major tick marks appear at all powers of \a base.
  
  Properties like tick step (\ref setTickStep) don't apply in logarithmic scaling. If you wish a decimal base but
  less major ticks, consider choosing \a base 100, 1000 or even higher.
*/
void QCPAxis::setScaleLogBase(double base)
{
  if (base > 1)
  {
    mScaleLogBase = base;
    mScaleLogBaseLogInv = 1.0/qLn(mScaleLogBase); // buffer for faster baseLog() calculation
    mCachedMarginValid = false;
  } else
    qDebug() << Q_FUNC_INFO << "Invalid logarithmic scale base (must be greater 1):" << base;
}

/*!
  Sets the range of the axis.
  
  This slot may be connected with the \ref rangeChanged signal of another axis so this axis
  is always synchronized with the other axis range, when it changes.
  
  To invert the direction of an axis, use \ref setRangeReversed.
*/
void QCPAxis::setRange(const QCPRange &range)
{
  if (range.lower == mRange.lower && range.upper == mRange.upper)
    return;
  
  if (!QCPRange::validRange(range)) return;
  QCPRange oldRange = mRange;
  if (mScaleType == stLogarithmic)
  {
    mRange = range.sanitizedForLogScale();
  } else
  {
    mRange = range.sanitizedForLinScale();
  }
  mCachedMarginValid = false;
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets whether the user can (de-)select the parts in \a selectable by clicking on the QCustomPlot surface.
  (When \ref QCustomPlot::setInteractions contains iSelectAxes.)
  
  However, even when \a selectable is set to a value not allowing the selection of a specific part,
  it is still possible to set the selection of this part manually, by calling \ref setSelectedParts
  directly.
  
  \see SelectablePart, setSelectedParts
*/
void QCPAxis::setSelectableParts(const SelectableParts &selectable)
{
  mSelectableParts = selectable;
}

/*!
  Sets the selected state of the respective axis parts described by \ref SelectablePart. When a part
  is selected, it uses a different pen/font.
  
  The entire selection mechanism for axes is handled automatically when \ref
  QCustomPlot::setInteractions contains iSelectAxes. You only need to call this function when you
  wish to change the selection state manually.
  
  This function can change the selection state of a part, independent of the \ref setSelectableParts setting.
  
  emits the \ref selectionChanged signal when \a selected is different from the previous selection state.
  
  \see SelectablePart, setSelectableParts, selectTest, setSelectedBasePen, setSelectedTickPen, setSelectedSubTickPen,
  setSelectedTickLabelFont, setSelectedLabelFont, setSelectedTickLabelColor, setSelectedLabelColor
*/
void QCPAxis::setSelectedParts(const SelectableParts &selected)
{
  if (mSelectedParts != selected)
  {
    if (mSelectedParts.testFlag(spTickLabels) != selected.testFlag(spTickLabels))
      mLabelCache.clear();
    mSelectedParts = selected;
    emit selectionChanged(mSelectedParts);
  }
}

/*!
  \overload
  
  Sets the lower and upper bound of the axis range.
  
  To invert the direction of an axis, use \ref setRangeReversed.
  
  There is also a slot to set a range, see \ref setRange(const QCPRange &range).
*/
void QCPAxis::setRange(double lower, double upper)
{
  if (lower == mRange.lower && upper == mRange.upper)
    return;
  
  if (!QCPRange::validRange(lower, upper)) return;
  QCPRange oldRange = mRange;
  mRange.lower = lower;
  mRange.upper = upper;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  mCachedMarginValid = false;
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  \overload
  
  Sets the range of the axis.
  
  The \a position coordinate indicates together with the \a alignment parameter, where the new
  range will be positioned. \a size defines the size of the new axis range. \a alignment may be
  Qt::AlignLeft, Qt::AlignRight or Qt::AlignCenter. This will cause the left border, right border,
  or center of the range to be aligned with \a position. Any other values of \a alignment will
  default to Qt::AlignCenter.
*/
void QCPAxis::setRange(double position, double size, Qt::AlignmentFlag alignment)
{
  if (alignment == Qt::AlignLeft)
    setRange(position, position+size);
  else if (alignment == Qt::AlignRight)
    setRange(position-size, position);
  else // alignment == Qt::AlignCenter
    setRange(position-size/2.0, position+size/2.0);
}

/*!
  Sets the lower bound of the axis range. The upper bound is not changed.
  \see setRange
*/
void QCPAxis::setRangeLower(double lower)
{
  if (mRange.lower == lower)
    return;
  
  QCPRange oldRange = mRange;
  mRange.lower = lower;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  mCachedMarginValid = false;
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets the upper bound of the axis range. The lower bound is not changed.
  \see setRange
*/
void QCPAxis::setRangeUpper(double upper)
{
  if (mRange.upper == upper)
    return;
  
  QCPRange oldRange = mRange;
  mRange.upper = upper;
  if (mScaleType == stLogarithmic)
  {
    mRange = mRange.sanitizedForLogScale();
  } else
  {
    mRange = mRange.sanitizedForLinScale();
  }
  mCachedMarginValid = false;
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Sets whether the axis range (direction) is displayed reversed. Normally, the values on horizontal
  axes increase left to right, on vertical axes bottom to top. When \a reversed is set to true, the
  direction of increasing values is inverted.

  Note that the range and data interface stays the same for reversed axes, e.g. the \a lower part
  of the \ref setRange interface will still reference the mathematically smaller number than the \a
  upper part.
*/
void QCPAxis::setRangeReversed(bool reversed)
{
  if (mRangeReversed != reversed)
  {
    mRangeReversed = reversed;
    mCachedMarginValid = false;
  }
}

/*!
  Sets whether the tick positions should be calculated automatically (either from an automatically
  generated tick step or a tick step provided manually via \ref setTickStep, see \ref setAutoTickStep).
  
  If \a on is set to false, you must provide the tick positions manually via \ref setTickVector.
  For these manual ticks you may let QCPAxis generate the appropriate labels automatically by
  leaving \ref setAutoTickLabels set to true. If you also wish to control the displayed labels
  manually, set \ref setAutoTickLabels to false and provide the label strings with \ref
  setTickVectorLabels.
  
  If you need dynamically calculated tick vectors (and possibly tick label vectors), set the
  vectors in a slot connected to the \ref ticksRequest signal.
*/
void QCPAxis::setAutoTicks(bool on)
{
  if (mAutoTicks != on)
  {
    mAutoTicks = on;
    mCachedMarginValid = false;
  }
}

/*!
  When \ref setAutoTickStep is true, \a approximateCount determines how many ticks should be
  generated in the visible range, approximately.
  
  It's not guaranteed that this number of ticks is met exactly, but approximately within a
  tolerance of about two.
  
  Only values greater than zero are accepted as \a approximateCount.
*/
void QCPAxis::setAutoTickCount(int approximateCount)
{
  if (mAutoTickCount != approximateCount)
  {
    if (approximateCount > 0)
    {
      mAutoTickCount = approximateCount;
      mCachedMarginValid = false;
    } else
      qDebug() << Q_FUNC_INFO << "approximateCount must be greater than zero:" << approximateCount;
  }
}

/*!
  Sets whether the tick labels are generated automatically. Depending on the tick label type (\ref
  ltNumber or \ref ltDateTime), the labels will either show the coordinate as floating point
  number (\ref setNumberFormat), or a date/time formatted according to \ref setDateTimeFormat.
  
  If \a on is set to false, you should provide the tick labels via \ref setTickVectorLabels. This
  is usually used in a combination with \ref setAutoTicks set to false for complete control over
  tick positions and labels, e.g. when the ticks should be at multiples of pi and show "2pi", "3pi"
  etc. as tick labels.
  
  If you need dynamically calculated tick vectors (and possibly tick label vectors), set the
  vectors in a slot connected to the \ref ticksRequest signal.
*/
void QCPAxis::setAutoTickLabels(bool on)
{
  if (mAutoTickLabels != on)
  {
    mAutoTickLabels = on;
    mCachedMarginValid = false;
  }
}

/*!
  Sets whether the tick step, i.e. the interval between two (major) ticks, is calculated
  automatically. If \a on is set to true, the axis finds a tick step that is reasonable for human
  readable plots. 

  The number of ticks the algorithm aims for within the visible range can be set with \ref
  setAutoTickCount.
  
  If \a on is set to false, you may set the tick step manually with \ref setTickStep.
*/
void QCPAxis::setAutoTickStep(bool on)
{
  if (mAutoTickStep != on)
  {
    mAutoTickStep = on;
    mCachedMarginValid = false;
  }
}

/*!
  Sets whether the number of sub ticks in one tick interval is determined automatically. This
  works, as long as the tick step mantissa is a multiple of 0.5. When \ref setAutoTickStep is
  enabled, this is always the case.
  
  When \a on is set to false, you may set the sub tick count with \ref setSubTickCount manually.
*/
void QCPAxis::setAutoSubTicks(bool on)
{
  if (mAutoSubTicks != on)
  {
    mAutoSubTicks = on;
    mCachedMarginValid = false;
  }
}

/*!
  Sets whether tick marks are displayed.

  Note that setting \a show to false does not imply that tick labels are invisible, too. To achieve
  that, see \ref setTickLabels.
*/
void QCPAxis::setTicks(bool show)
{
  if (mTicks != show)
  {
    mTicks = show;
    mCachedMarginValid = false;
  }
}

/*!
  Sets whether tick labels are displayed. Tick labels are the numbers drawn next to tick marks.
*/
void QCPAxis::setTickLabels(bool show)
{
  if (mTickLabels != show)
  {
    mTickLabels = show;
    mCachedMarginValid = false;
  }
}

/*!
  Sets the distance between the axis base line (including any outward ticks) and the tick labels.
  \see setLabelPadding, setPadding
*/
void QCPAxis::setTickLabelPadding(int padding)
{
  if (mTickLabelPadding != padding)
  {
    mTickLabelPadding = padding;
    mCachedMarginValid = false;
  }
}

/*!
  Sets whether the tick labels display numbers or dates/times.
  
  If \a type is set to \ref ltNumber, the format specifications of \ref setNumberFormat apply.
  
  If \a type is set to \ref ltDateTime, the format specifications of \ref setDateTimeFormat apply.
  
  In QCustomPlot, date/time coordinates are <tt>double</tt> numbers representing the seconds since
  1970-01-01T00:00:00 UTC. This format can be retrieved from QDateTime objects with the
  QDateTime::toTime_t() function. Since this only gives a resolution of one second, there is also
  the QDateTime::toMSecsSinceEpoch() function which returns the timespan described above in
  milliseconds. Divide its return value by 1000.0 to get a value with the format needed for
  date/time plotting, with a resolution of one millisecond.
  
  Using the toMSecsSinceEpoch function allows dates that go back to 2nd January 4713 B.C.
  (represented by a negative number), unlike the toTime_t function, which works with unsigned
  integers and thus only goes back to 1st January 1970. So both for range and accuracy, use of
  toMSecsSinceEpoch()/1000.0 should be preferred as key coordinate for date/time axes.
  
  \see setTickLabels
*/
void QCPAxis::setTickLabelType(LabelType type)
{
  if (mTickLabelType != type)
  {
    mTickLabelType = type;
    mCachedMarginValid = false;
  }
}

/*!
  Sets the font of the tick labels.
  
  \see setTickLabels, setTickLabelColor
*/
void QCPAxis::setTickLabelFont(const QFont &font)
{
  if (font != mTickLabelFont)
  {
    mTickLabelFont = font;
    mCachedMarginValid = false;
    mLabelCache.clear();
  }
}

/*!
  Sets the color of the tick labels.
  
  \see setTickLabels, setTickLabelFont
*/
void QCPAxis::setTickLabelColor(const QColor &color)
{
  if (color != mTickLabelColor)
  {
    mTickLabelColor = color;
    mCachedMarginValid = false;
    mLabelCache.clear();
  }
}

/*!
  Sets the rotation of the tick labels. If \a degrees is zero, the labels are drawn normally. Else,
  the tick labels are drawn rotated by \a degrees clockwise. The specified angle is bound to values
  from -90 to 90 degrees.
  
  If \a degrees is exactly -90, 0 or 90, the tick labels are centered on the tick coordinate. For
  other angles, the label is drawn with an offset such that it seems to point toward or away from
  the tick mark.
*/
void QCPAxis::setTickLabelRotation(double degrees)
{
  if (!qFuzzyIsNull(degrees-mTickLabelRotation))
  {
    mTickLabelRotation = qBound(-90.0, degrees, 90.0);
    mCachedMarginValid = false;
    mLabelCache.clear();
  }
}

/*!
  Sets the format in which dates and times are displayed as tick labels, if \ref setTickLabelType is \ref ltDateTime.
  for details about the \a format string, see the documentation of QDateTime::toString().
  
  Newlines can be inserted with "\n".
  
  \see setDateTimeSpec
*/
void QCPAxis::setDateTimeFormat(const QString &format)
{
  if (mDateTimeFormat != format)
  {
    mDateTimeFormat = format;
    mCachedMarginValid = false;
    mLabelCache.clear();
  }
}

/*!
  Sets the time spec that is used for the date time values when \ref setTickLabelType is \ref
  ltDateTime.

  The default value of QDateTime objects (and also QCustomPlot) is <tt>Qt::LocalTime</tt>. However,
  if the date time values passed to QCustomPlot are given in the UTC spec, set \a
  timeSpec to <tt>Qt::UTC</tt> to get the correct axis labels.
  
  \see setDateTimeFormat
*/
void QCPAxis::setDateTimeSpec(const Qt::TimeSpec &timeSpec)
{
  mDateTimeSpec = timeSpec;
}

/*!
  Sets the number format for the numbers drawn as tick labels (if tick label type is \ref
  ltNumber). This \a formatCode is an extended version of the format code used e.g. by
  QString::number() and QLocale::toString(). For reference about that, see the "Argument Formats"
  section in the detailed description of the QString class. \a formatCode is a string of one, two
  or three characters. The first character is identical to the normal format code used by Qt. In
  short, this means: 'e'/'E' scientific format, 'f' fixed format, 'g'/'G' scientific or fixed,
  whichever is shorter.
  
  The second and third characters are optional and specific to QCustomPlot:\n
  If the first char was 'e' or 'g', numbers are/might be displayed in the scientific format, e.g.
  "5.5e9", which is ugly in a plot. So when the second char of \a formatCode is set to 'b' (for
  "beautiful"), those exponential numbers are formatted in a more natural way, i.e. "5.5
  [multiplication sign] 10 [superscript] 9". By default, the multiplication sign is a centered dot.
  If instead a cross should be shown (as is usual in the USA), the third char of \a formatCode can
  be set to 'c'. The inserted multiplication signs are the UTF-8 characters 215 (0xD7) for the
  cross and 183 (0xB7) for the dot.
  
  If the scale type (\ref setScaleType) is \ref stLogarithmic and the \a formatCode uses the 'b'
  option (beautifully typeset decimal powers), the display usually is "1 [multiplication sign] 10
  [superscript] n", which looks unnatural for logarithmic scaling (the "1 [multiplication sign]"
  part). To only display the decimal power, set the number precision to zero with \ref
  setNumberPrecision.
  
  Examples for \a formatCode:
  \li \c g normal format code behaviour. If number is small, fixed format is used, if number is large,
  normal scientific format is used
  \li \c gb If number is small, fixed format is used, if number is large, scientific format is used with
  beautifully typeset decimal powers and a dot as multiplication sign
  \li \c ebc All numbers are in scientific format with beautifully typeset decimal power and a cross as
  multiplication sign
  \li \c fb illegal format code, since fixed format doesn't support (or need) beautifully typeset decimal
  powers. Format code will be reduced to 'f'.
  \li \c hello illegal format code, since first char is not 'e', 'E', 'f', 'g' or 'G'. Current format
  code will not be changed.
*/
void QCPAxis::setNumberFormat(const QString &formatCode)
{
  if (formatCode.isEmpty())
  {
    qDebug() << Q_FUNC_INFO << "Passed formatCode is empty";
    return;
  }
  mLabelCache.clear();
  mCachedMarginValid = false;
  
  // interpret first char as number format char:
  QString allowedFormatChars = "eEfgG";
  if (allowedFormatChars.contains(formatCode.at(0)))
  {
    mNumberFormatChar = formatCode.at(0).toLatin1();
  } else
  {
    qDebug() << Q_FUNC_INFO << "Invalid number format code (first char not in 'eEfgG'):" << formatCode;
    return;
  }
  if (formatCode.length() < 2)
  {
    mNumberBeautifulPowers = false;
    mNumberMultiplyCross = false;
    return;
  }
  
  // interpret second char as indicator for beautiful decimal powers:
  if (formatCode.at(1) == 'b' && (mNumberFormatChar == 'e' || mNumberFormatChar == 'g'))
  {
    mNumberBeautifulPowers = true;
  } else
  {
    qDebug() << Q_FUNC_INFO << "Invalid number format code (second char not 'b' or first char neither 'e' nor 'g'):" << formatCode;
    return;
  }
  if (formatCode.length() < 3)
  {
    mNumberMultiplyCross = false;
    return;
  }
  
  // interpret third char as indicator for dot or cross multiplication symbol:
  if (formatCode.at(2) == 'c')
  {
    mNumberMultiplyCross = true;
  } else if (formatCode.at(2) == 'd')
  {
    mNumberMultiplyCross = false;
  } else
  {
    qDebug() << Q_FUNC_INFO << "Invalid number format code (third char neither 'c' nor 'd'):" << formatCode;
    return;
  }
}

/*!
  Sets the precision of the tick label numbers. See QLocale::toString(double i, char f, int prec)
  for details. The effect of precisions are most notably for number Formats starting with 'e', see
  \ref setNumberFormat

  If the scale type (\ref setScaleType) is \ref stLogarithmic and the number format (\ref
  setNumberFormat) uses the 'b' format code (beautifully typeset decimal powers), the display
  usually is "1 [multiplication sign] 10 [superscript] n", which looks unnatural for logarithmic
  scaling (the redundant "1 [multiplication sign]" part). To only display the decimal power "10
  [superscript] n", set \a precision to zero.
*/
void QCPAxis::setNumberPrecision(int precision)
{
  if (mNumberPrecision != precision)
  {
    mNumberPrecision = precision;
    mCachedMarginValid = false;
  }
}

/*!
  If \ref setAutoTickStep is set to false, use this function to set the tick step manually.
  The tick step is the interval between (major) ticks, in plot coordinates.
  \see setSubTickCount
*/
void QCPAxis::setTickStep(double step)
{
  if (mTickStep != step)
  {
    mTickStep = step;
    mCachedMarginValid = false;
  }
}

/*!
  If you want full control over what ticks (and possibly labels) the axes show, this function is
  used to set the coordinates at which ticks will appear.\ref setAutoTicks must be disabled, else
  the provided tick vector will be overwritten with automatically generated tick coordinates upon
  replot. The labels of the ticks can be generated automatically when \ref setAutoTickLabels is
  left enabled. If it is disabled, you can set the labels manually with \ref setTickVectorLabels.
  
  \a vec is a vector containing the positions of the ticks, in plot coordinates.
  
  \warning \a vec must be sorted in ascending order, no additional checks are made to ensure this.

  \see setTickVectorLabels
*/
void QCPAxis::setTickVector(const QVector<double> &vec)
{
  // don't check whether mTickVector != vec here, because it takes longer than we would save
  mTickVector = vec;
  mCachedMarginValid = false;
}

/*!
  If you want full control over what ticks and labels the axes show, this function is used to set a
  number of QStrings that will be displayed at the tick positions which you need to provide with
  \ref setTickVector. These two vectors should have the same size. (Note that you need to disable
  \ref setAutoTicks and \ref setAutoTickLabels first.)
  
  \a vec is a vector containing the labels of the ticks. The entries correspond to the respective
  indices in the tick vector, passed via \ref setTickVector.
  
  \see setTickVector
*/
void QCPAxis::setTickVectorLabels(const QVector<QString> &vec)
{
  // don't check whether mTickVectorLabels != vec here, because it takes longer than we would save
  mTickVectorLabels = vec;
  mCachedMarginValid = false;
}

/*!
  Sets the length of the ticks in pixels. \a inside is the length the ticks will reach inside the
  plot and \a outside is the length they will reach outside the plot. If \a outside is greater than
  zero, the tick labels and axis label will increase their distance to the axis accordingly, so
  they won't collide with the ticks.
  
  \see setSubTickLength
*/
void QCPAxis::setTickLength(int inside, int outside)
{
  if (mTickLengthIn != inside)
  {
    mTickLengthIn = inside;
  }
  if (mTickLengthOut != outside)
  {
    mTickLengthOut = outside;
    mCachedMarginValid = false; // only outside tick length can change margin
  }
}

/*!
  Sets the length of the inward ticks in pixels. \a inside is the length the ticks will reach
  inside the plot.
  
  \see setTickLengthOut, setSubTickLength
*/
void QCPAxis::setTickLengthIn(int inside)
{
  if (mTickLengthIn != inside)
  {
    mTickLengthIn = inside;
  }
}

/*!
  Sets the length of the outward ticks in pixels. \a outside is the length the ticks will reach
  outside the plot. If \a outside is greater than zero, the tick labels and axis label will
  increase their distance to the axis accordingly, so they won't collide with the ticks.
  
  \see setTickLengthIn, setSubTickLength
*/
void QCPAxis::setTickLengthOut(int outside)
{
  if (mTickLengthOut != outside)
  {
    mTickLengthOut = outside;
    mCachedMarginValid = false; // only outside tick length can change margin
  }
}

/*!
  Sets the number of sub ticks in one (major) tick step. A sub tick count of three for example,
  divides the tick intervals in four sub intervals.
  
  By default, the number of sub ticks is chosen automatically in a reasonable manner as long as the
  mantissa of the tick step is a multiple of 0.5. When \ref setAutoTickStep is enabled, this is
  always the case.

  If you want to disable automatic sub tick count and use this function to set the count manually,
  see \ref setAutoSubTicks.
*/
void QCPAxis::setSubTickCount(int count)
{
  mSubTickCount = count;
}

/*!
  Sets the length of the subticks in pixels. \a inside is the length the subticks will reach inside
  the plot and \a outside is the length they will reach outside the plot. If \a outside is greater
  than zero, the tick labels and axis label will increase their distance to the axis accordingly,
  so they won't collide with the ticks.
*/
void QCPAxis::setSubTickLength(int inside, int outside)
{
  if (mSubTickLengthIn != inside)
  {
    mSubTickLengthIn = inside;
  }
  if (mSubTickLengthOut != outside)
  {
    mSubTickLengthOut = outside;
    mCachedMarginValid = false; // only outside tick length can change margin
  }
}

/*!
  Sets the length of the inward subticks in pixels. \a inside is the length the subticks will reach inside
  the plot.
  
  \see setSubTickLengthOut, setTickLength
*/
void QCPAxis::setSubTickLengthIn(int inside)
{
  if (mSubTickLengthIn != inside)
  {
    mSubTickLengthIn = inside;
  }
}

/*!
  Sets the length of the outward subticks in pixels. \a outside is the length the subticks will reach
  outside the plot. If \a outside is greater than zero, the tick labels will increase their
  distance to the axis accordingly, so they won't collide with the ticks.
  
  \see setSubTickLengthIn, setTickLength
*/
void QCPAxis::setSubTickLengthOut(int outside)
{
  if (mSubTickLengthOut != outside)
  {
    mSubTickLengthOut = outside;
    mCachedMarginValid = false; // only outside tick length can change margin
  }
}

/*!
  Sets the pen, the axis base line is drawn with.
  
  \see setTickPen, setSubTickPen
*/
void QCPAxis::setBasePen(const QPen &pen)
{
  mBasePen = pen;
}

/*!
  Sets the pen, tick marks will be drawn with.
  
  \see setTickLength, setBasePen
*/
void QCPAxis::setTickPen(const QPen &pen)
{
  mTickPen = pen;
}

/*!
  Sets the pen, subtick marks will be drawn with.
  
  \see setSubTickCount, setSubTickLength, setBasePen
*/
void QCPAxis::setSubTickPen(const QPen &pen)
{
  mSubTickPen = pen;
}

/*!
  Sets the font of the axis label.
  
  \see setLabelColor
*/
void QCPAxis::setLabelFont(const QFont &font)
{
  if (mLabelFont != font)
  {
    mLabelFont = font;
    mCachedMarginValid = false;
  }
}

/*!
  Sets the color of the axis label.
  
  \see setLabelFont
*/
void QCPAxis::setLabelColor(const QColor &color)
{
  mLabelColor = color;
}

/*!
  Sets the text of the axis label that will be shown below/above or next to the axis, depending on
  its orientation. To disable axis labels, pass an empty string as \a str.
*/
void QCPAxis::setLabel(const QString &str)
{
  if (mLabel != str)
  {
    mLabel = str;
    mCachedMarginValid = false;
  }
}

/*!
  Sets the distance between the tick labels and the axis label.
  
  \see setTickLabelPadding, setPadding
*/
void QCPAxis::setLabelPadding(int padding)
{
  if (mLabelPadding != padding)
  {
    mLabelPadding = padding;
    mCachedMarginValid = false;
  }
}

/*!
  Sets the padding of the axis.

  When \ref QCPAxisRect::setAutoMargins is enabled, the padding is the additional outer most space,
  that is left blank.
  
  The axis padding has no meaning if \ref QCPAxisRect::setAutoMargins is disabled.
  
  \see setLabelPadding, setTickLabelPadding
*/
void QCPAxis::setPadding(int padding)
{
  if (mPadding != padding)
  {
    mPadding = padding;
    mCachedMarginValid = false;
  }
}

/*!
  Sets the offset the axis has to its axis rect side.
  
  If an axis rect side has multiple axes, only the offset of the inner most axis has meaning. The offset of the other axes
  is controlled automatically, to place the axes at appropriate positions to prevent them from overlapping.
*/
void QCPAxis::setOffset(int offset)
{
  mOffset = offset;
}

/*!
  Sets the font that is used for tick labels when they are selected.
  
  \see setTickLabelFont, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPAxis::setSelectedTickLabelFont(const QFont &font)
{
  if (font != mSelectedTickLabelFont)
  {
    mSelectedTickLabelFont = font;
    mLabelCache.clear();
    // don't set mCachedMarginValid to false here because margin calculation is always done with non-selected fonts
  }
}

/*!
  Sets the font that is used for the axis label when it is selected.
  
  \see setLabelFont, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPAxis::setSelectedLabelFont(const QFont &font)
{
  mSelectedLabelFont = font;
  // don't set mCachedMarginValid to false here because margin calculation is always done with non-selected fonts
}

/*!
  Sets the color that is used for tick labels when they are selected.
  
  \see setTickLabelColor, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPAxis::setSelectedTickLabelColor(const QColor &color)
{
  if (color != mSelectedTickLabelColor)
  {
    mSelectedTickLabelColor = color;
    mLabelCache.clear();
  }
}

/*!
  Sets the color that is used for the axis label when it is selected.
  
  \see setLabelColor, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPAxis::setSelectedLabelColor(const QColor &color)
{
  mSelectedLabelColor = color;
}

/*!
  Sets the pen that is used to draw the axis base line when selected.
  
  \see setBasePen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPAxis::setSelectedBasePen(const QPen &pen)
{
  mSelectedBasePen = pen;
}

/*!
  Sets the pen that is used to draw the (major) ticks when selected.
  
  \see setTickPen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPAxis::setSelectedTickPen(const QPen &pen)
{
  mSelectedTickPen = pen;
}

/*!
  Sets the pen that is used to draw the subticks when selected.
  
  \see setSubTickPen, setSelectableParts, setSelectedParts, QCustomPlot::setInteractions
*/
void QCPAxis::setSelectedSubTickPen(const QPen &pen)
{
  mSelectedSubTickPen = pen;
}

/*!
  Sets the style for the lower axis ending. See the documentation of QCPLineEnding for available
  styles.
  
  For horizontal axes, this method refers to the left ending, for vertical axes the bottom ending.
  Note that this meaning does not change when the axis range is reversed with \ref
  setRangeReversed.
  
  \see setUpperEnding
*/
void QCPAxis::setLowerEnding(const QCPLineEnding &ending)
{
  mLowerEnding = ending;
}

/*!
  Sets the style for the upper axis ending. See the documentation of QCPLineEnding for available
  styles.
  
  For horizontal axes, this method refers to the right ending, for vertical axes the top ending.
  Note that this meaning does not change when the axis range is reversed with \ref
  setRangeReversed.
  
  \see setLowerEnding
*/
void QCPAxis::setUpperEnding(const QCPLineEnding &ending)
{
  mUpperEnding = ending;
}

/*!
  If the scale type (\ref setScaleType) is \ref stLinear, \a diff is added to the lower and upper
  bounds of the range. The range is simply moved by \a diff.
  
  If the scale type is \ref stLogarithmic, the range bounds are multiplied by \a diff. This
  corresponds to an apparent "linear" move in logarithmic scaling by a distance of log(diff).
*/
void QCPAxis::moveRange(double diff)
{
  QCPRange oldRange = mRange;
  if (mScaleType == stLinear)
  {
    mRange.lower += diff;
    mRange.upper += diff;
  } else // mScaleType == stLogarithmic
  {
    mRange.lower *= diff;
    mRange.upper *= diff;
  }
  mCachedMarginValid = false;
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Scales the range of this axis by \a factor around the coordinate \a center. For example, if \a
  factor is 2.0, \a center is 1.0, then the axis range will double its size, and the point at
  coordinate 1.0 won't have changed its position in the QCustomPlot widget (i.e. coordinates
  around 1.0 will have moved symmetrically closer to 1.0).
*/
void QCPAxis::scaleRange(double factor, double center)
{
  QCPRange oldRange = mRange;
  if (mScaleType == stLinear)
  {
    QCPRange newRange;
    newRange.lower = (mRange.lower-center)*factor + center;
    newRange.upper = (mRange.upper-center)*factor + center;
    if (QCPRange::validRange(newRange))
      mRange = newRange.sanitizedForLinScale();
  } else // mScaleType == stLogarithmic
  {
    if ((mRange.upper < 0 && center < 0) || (mRange.upper > 0 && center > 0)) // make sure center has same sign as range
    {
      QCPRange newRange;
      newRange.lower = pow(mRange.lower/center, factor)*center;
      newRange.upper = pow(mRange.upper/center, factor)*center;
      if (QCPRange::validRange(newRange))
        mRange = newRange.sanitizedForLogScale();
    } else
      qDebug() << Q_FUNC_INFO << "Center of scaling operation doesn't lie in same logarithmic sign domain as range:" << center;
  }
  mCachedMarginValid = false;
  emit rangeChanged(mRange);
  emit rangeChanged(mRange, oldRange);
}

/*!
  Scales the range of this axis to have a certain scale \a ratio to \a otherAxis. The scaling will
  be done around the center of the current axis range.

  For example, if \a ratio is 1, this axis is the \a yAxis and \a otherAxis is \a xAxis, graphs
  plotted with those axes will appear in a 1:1 aspect ratio, independent of the aspect ratio the
  axis rect has.

  This is an operation that changes the range of this axis once, it doesn't fix the scale ratio
  indefinitely. Note that calling this function in the constructor of the QCustomPlot's parent
  won't have the desired effect, since the widget dimensions aren't defined yet, and a resizeEvent
  will follow.
*/
void QCPAxis::setScaleRatio(const QCPAxis *otherAxis, double ratio)
{
  int otherPixelSize, ownPixelSize;
  
  if (otherAxis->orientation() == Qt::Horizontal)
    otherPixelSize = otherAxis->axisRect()->width();
  else
    otherPixelSize = otherAxis->axisRect()->height();
  
  if (orientation() == Qt::Horizontal)
    ownPixelSize = axisRect()->width();
  else
    ownPixelSize = axisRect()->height();
  
  double newRangeSize = ratio*otherAxis->range().size()*ownPixelSize/(double)otherPixelSize;
  setRange(range().center(), newRangeSize, Qt::AlignCenter);
}

/*!
  Changes the axis range such that all plottables associated with this axis are fully visible in
  that dimension.
  
  \see QCPAbstractPlottable::rescaleAxes, QCustomPlot::rescaleAxes
*/
void QCPAxis::rescale(bool onlyVisiblePlottables)
{
  QList<QCPAbstractPlottable*> p = plottables();
  QCPRange newRange;
  bool haveRange = false;
  for (int i=0; i<p.size(); ++i)
  {
    if (!p.at(i)->realVisibility() && onlyVisiblePlottables)
      continue;
    QCPRange plottableRange;
    bool validRange;
    QCPAbstractPlottable::SignDomain signDomain = QCPAbstractPlottable::sdBoth;
    if (mScaleType == stLogarithmic)
      signDomain = (mRange.upper < 0 ? QCPAbstractPlottable::sdNegative : QCPAbstractPlottable::sdPositive);
    if (p.at(i)->keyAxis() == this)
      plottableRange = p.at(i)->getKeyRange(validRange, signDomain);
    else
      plottableRange = p.at(i)->getValueRange(validRange, signDomain);
    if (validRange)
    {
      if (!haveRange)
        newRange = plottableRange;
      else
        newRange.expand(plottableRange);
      haveRange = true;
    }
  }
  if (haveRange)
    setRange(newRange);
}

/*!
  Transforms \a value, in pixel coordinates of the QCustomPlot widget, to axis coordinates.
*/
double QCPAxis::pixelToCoord(double value) const
{
  if (orientation() == Qt::Horizontal)
  {
    if (mScaleType == stLinear)
    {
      if (!mRangeReversed)
        return (value-mAxisRect->left())/(double)mAxisRect->width()*mRange.size()+mRange.lower;
      else
        return -(value-mAxisRect->left())/(double)mAxisRect->width()*mRange.size()+mRange.upper;
    } else // mScaleType == stLogarithmic
    {
      if (!mRangeReversed)
        return pow(mRange.upper/mRange.lower, (value-mAxisRect->left())/(double)mAxisRect->width())*mRange.lower;
      else
        return pow(mRange.upper/mRange.lower, (mAxisRect->left()-value)/(double)mAxisRect->width())*mRange.upper;
    }
  } else // orientation() == Qt::Vertical
  {
    if (mScaleType == stLinear)
    {
      if (!mRangeReversed)
        return (mAxisRect->bottom()-value)/(double)mAxisRect->height()*mRange.size()+mRange.lower;
      else
        return -(mAxisRect->bottom()-value)/(double)mAxisRect->height()*mRange.size()+mRange.upper;
    } else // mScaleType == stLogarithmic
    {
      if (!mRangeReversed)
        return pow(mRange.upper/mRange.lower, (mAxisRect->bottom()-value)/(double)mAxisRect->height())*mRange.lower;
      else
        return pow(mRange.upper/mRange.lower, (value-mAxisRect->bottom())/(double)mAxisRect->height())*mRange.upper;
    }
  }
}

/*!
  Transforms \a value, in coordinates of the axis, to pixel coordinates of the QCustomPlot widget.
*/
double QCPAxis::coordToPixel(double value) const
{
  if (orientation() == Qt::Horizontal)
  {
    if (mScaleType == stLinear)
    {
      if (!mRangeReversed)
        return (value-mRange.lower)/mRange.size()*mAxisRect->width()+mAxisRect->left();
      else
        return (mRange.upper-value)/mRange.size()*mAxisRect->width()+mAxisRect->left();
    } else // mScaleType == stLogarithmic
    {
      if (value >= 0 && mRange.upper < 0) // invalid value for logarithmic scale, just draw it outside visible range
        return !mRangeReversed ? mAxisRect->right()+200 : mAxisRect->left()-200;
      else if (value <= 0 && mRange.upper > 0) // invalid value for logarithmic scale, just draw it outside visible range
        return !mRangeReversed ? mAxisRect->left()-200 : mAxisRect->right()+200;
      else
      {
        if (!mRangeReversed)
          return baseLog(value/mRange.lower)/baseLog(mRange.upper/mRange.lower)*mAxisRect->width()+mAxisRect->left();
        else
          return baseLog(mRange.upper/value)/baseLog(mRange.upper/mRange.lower)*mAxisRect->width()+mAxisRect->left();
      }
    }
  } else // orientation() == Qt::Vertical
  {
    if (mScaleType == stLinear)
    {
      if (!mRangeReversed)
        return mAxisRect->bottom()-(value-mRange.lower)/mRange.size()*mAxisRect->height();
      else
        return mAxisRect->bottom()-(mRange.upper-value)/mRange.size()*mAxisRect->height();
    } else // mScaleType == stLogarithmic
    {     
      if (value >= 0 && mRange.upper < 0) // invalid value for logarithmic scale, just draw it outside visible range
        return !mRangeReversed ? mAxisRect->top()-200 : mAxisRect->bottom()+200;
      else if (value <= 0 && mRange.upper > 0) // invalid value for logarithmic scale, just draw it outside visible range
        return !mRangeReversed ? mAxisRect->bottom()+200 : mAxisRect->top()-200;
      else
      {
        if (!mRangeReversed)
          return mAxisRect->bottom()-baseLog(value/mRange.lower)/baseLog(mRange.upper/mRange.lower)*mAxisRect->height();
        else
          return mAxisRect->bottom()-baseLog(mRange.upper/value)/baseLog(mRange.upper/mRange.lower)*mAxisRect->height();
      }
    }
  }
}

/*!
  Returns the part of the axis that is hit by \a pos (in pixels). The return value of this function
  is independent of the user-selectable parts defined with \ref setSelectableParts. Further, this
  function does not change the current selection state of the axis.
  
  If the axis is not visible (\ref setVisible), this function always returns \ref spNone.
  
  \see setSelectedParts, setSelectableParts, QCustomPlot::setInteractions
*/
QCPAxis::SelectablePart QCPAxis::getPartAt(const QPointF &pos) const
{
  if (!mVisible)
    return spNone;
  
  if (mAxisSelectionBox.contains(pos.toPoint()))
    return spAxis;
  else if (mTickLabelsSelectionBox.contains(pos.toPoint()))
    return spTickLabels;
  else if (mLabelSelectionBox.contains(pos.toPoint()))
    return spAxisLabel;
  else
    return spNone;
}

/* inherits documentation from base class */
double QCPAxis::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
  if (!mParentPlot) return -1;
  SelectablePart part = getPartAt(pos);
  if ((onlySelectable && !mSelectableParts.testFlag(part)) || part == spNone)
    return -1;
  
  if (details)
    details->setValue(part);
  return mParentPlot->selectionTolerance()*0.99;
}

/*!
  Returns a list of all the plottables that have this axis as key or value axis.
  
  If you are only interested in plottables of type QCPGraph, see \ref graphs.
  
  \see graphs, items
*/
QList<QCPAbstractPlottable*> QCPAxis::plottables() const
{
  QList<QCPAbstractPlottable*> result;
  if (!mParentPlot) return result;
  
  for (int i=0; i<mParentPlot->mPlottables.size(); ++i)
  {
    if (mParentPlot->mPlottables.at(i)->keyAxis() == this ||mParentPlot->mPlottables.at(i)->valueAxis() == this)
      result.append(mParentPlot->mPlottables.at(i));
  }
  return result;
}

/*!
  Returns a list of all the graphs that have this axis as key or value axis.
  
  \see plottables, items
*/
QList<QCPGraph*> QCPAxis::graphs() const
{
  QList<QCPGraph*> result;
  if (!mParentPlot) return result;
  
  for (int i=0; i<mParentPlot->mGraphs.size(); ++i)
  {
    if (mParentPlot->mGraphs.at(i)->keyAxis() == this || mParentPlot->mGraphs.at(i)->valueAxis() == this)
      result.append(mParentPlot->mGraphs.at(i));
  }
  return result;
}

/*!
  Returns a list of all the items that are associated with this axis. An item is considered
  associated with an axis if at least one of its positions uses the axis as key or value axis.
  
  \see plottables, graphs
*/
QList<QCPAbstractItem*> QCPAxis::items() const
{
  QList<QCPAbstractItem*> result;
  if (!mParentPlot) return result;
  
  for (int itemId=0; itemId<mParentPlot->mItems.size(); ++itemId)
  {
    QList<QCPItemPosition*> positions = mParentPlot->mItems.at(itemId)->positions();
    for (int posId=0; posId<positions.size(); ++posId)
    {
      if (positions.at(posId)->keyAxis() == this || positions.at(posId)->valueAxis() == this)
      {
        result.append(mParentPlot->mItems.at(itemId));
        break;
      }
    }
  }
  return result;
}

/*!
  Transforms a margin side to the logically corresponding axis type. (QCP::msLeft to
  QCPAxis::atLeft, QCP::msRight to QCPAxis::atRight, etc.)
*/
QCPAxis::AxisType QCPAxis::marginSideToAxisType(QCP::MarginSide side)
{
  switch (side)
  {
    case QCP::msLeft: return atLeft;
    case QCP::msRight: return atRight;
    case QCP::msTop: return atTop;
    case QCP::msBottom: return atBottom;
    default: break;
  }
  qDebug() << Q_FUNC_INFO << "Invalid margin side passed:" << (int)side;
  return atLeft;
}

/*! \internal
  
  This function is called to prepare the tick vector, sub tick vector and tick label vector. If
  \ref setAutoTicks is set to true, appropriate tick values are determined automatically via \ref
  generateAutoTicks. If it's set to false, the signal ticksRequest is emitted, which can be used to
  provide external tick positions. Then the sub tick vectors and tick label vectors are created.
*/
void QCPAxis::setupTickVectors()
{
  if (!mParentPlot) return;
  if ((!mTicks && !mTickLabels && !mGrid->visible()) || mRange.size() <= 0) return;
  
  // fill tick vectors, either by auto generating or by notifying user to fill the vectors himself
  if (mAutoTicks)
  {
    generateAutoTicks();
  } else
  {
    emit ticksRequest();
  }
  
  visibleTickBounds(mLowestVisibleTick, mHighestVisibleTick);
  if (mTickVector.isEmpty())
  {
    mSubTickVector.clear();
    return;
  }
  
  // generate subticks between ticks:
  mSubTickVector.resize((mTickVector.size()-1)*mSubTickCount);
  if (mSubTickCount > 0)
  {
    double subTickStep = 0;
    double subTickPosition = 0;
    int subTickIndex = 0;
    bool done = false;
    int lowTick = mLowestVisibleTick > 0 ? mLowestVisibleTick-1 : mLowestVisibleTick;
    int highTick = mHighestVisibleTick < mTickVector.size()-1 ? mHighestVisibleTick+1 : mHighestVisibleTick;
    for (int i=lowTick+1; i<=highTick; ++i)
    {
      subTickStep = (mTickVector.at(i)-mTickVector.at(i-1))/(double)(mSubTickCount+1);
      for (int k=1; k<=mSubTickCount; ++k)
      {
        subTickPosition = mTickVector.at(i-1) + k*subTickStep;
        if (subTickPosition < mRange.lower)
          continue;
        if (subTickPosition > mRange.upper)
        {
          done = true;
          break;
        }
        mSubTickVector[subTickIndex] = subTickPosition;
        subTickIndex++;
      }
      if (done) break;
    }
    mSubTickVector.resize(subTickIndex);
  }

  // generate tick labels according to tick positions:
  mExponentialChar = mParentPlot->locale().exponential();   // will be needed when drawing the numbers generated here, in getTickLabelData()
  mPositiveSignChar = mParentPlot->locale().positiveSign(); // will be needed when drawing the numbers generated here, in getTickLabelData()
  if (mAutoTickLabels)
  {
    int vecsize = mTickVector.size();
    mTickVectorLabels.resize(vecsize);
    if (mTickLabelType == ltNumber)
    {
      for (int i=mLowestVisibleTick; i<=mHighestVisibleTick; ++i)
        mTickVectorLabels[i] = mParentPlot->locale().toString(mTickVector.at(i), mNumberFormatChar, mNumberPrecision);
    } else if (mTickLabelType == ltDateTime)
    {
      for (int i=mLowestVisibleTick; i<=mHighestVisibleTick; ++i)
      {
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0) // use fromMSecsSinceEpoch function if available, to gain sub-second accuracy on tick labels (e.g. for format "hh:mm:ss:zzz")
        mTickVectorLabels[i] = mParentPlot->locale().toString(QDateTime::fromTime_t(mTickVector.at(i)).toTimeSpec(mDateTimeSpec), mDateTimeFormat);
#else
        mTickVectorLabels[i] = mParentPlot->locale().toString(QDateTime::fromMSecsSinceEpoch(mTickVector.at(i)*1000).toTimeSpec(mDateTimeSpec), mDateTimeFormat);
#endif
      }
    }
  } else // mAutoTickLabels == false
  {
    if (mAutoTicks) // ticks generated automatically, but not ticklabels, so emit ticksRequest here for labels
    {
      emit ticksRequest();
    }
    // make sure provided tick label vector has correct (minimal) length:
    if (mTickVectorLabels.size() < mTickVector.size())
      mTickVectorLabels.resize(mTickVector.size());
  }
}

/*! \internal
  
  If \ref setAutoTicks is set to true, this function is called by \ref setupTickVectors to
  generate reasonable tick positions (and subtick count). The algorithm tries to create
  approximately <tt>mAutoTickCount</tt> ticks (set via \ref setAutoTickCount).
 
  If the scale is logarithmic, \ref setAutoTickCount is ignored, and one tick is generated at every
  power of the current logarithm base, set via \ref setScaleLogBase.
*/
void QCPAxis::generateAutoTicks()
{
  if (mScaleType == stLinear)
  {
    if (mAutoTickStep)
    {
      // Generate tick positions according to linear scaling:
      mTickStep = mRange.size()/(double)(mAutoTickCount+1e-10); // mAutoTickCount ticks on average, the small addition is to prevent jitter on exact integers
      double magnitudeFactor = qPow(10.0, qFloor(qLn(mTickStep)/qLn(10.0))); // get magnitude factor e.g. 0.01, 1, 10, 1000 etc.
      double tickStepMantissa = mTickStep/magnitudeFactor;
      if (tickStepMantissa < 5)
      {
        // round digit after decimal point to 0.5
        mTickStep = (int)(tickStepMantissa*2)/2.0*magnitudeFactor;
      } else
      {
        // round to first digit in multiples of 2
        mTickStep = (int)(tickStepMantissa/2.0)*2.0*magnitudeFactor;
      }
    }
    if (mAutoSubTicks)
      mSubTickCount = calculateAutoSubTickCount(mTickStep);
    // Generate tick positions according to mTickStep:
    qint64 firstStep = floor(mRange.lower/mTickStep);
    qint64 lastStep = ceil(mRange.upper/mTickStep);
    int tickcount = lastStep-firstStep+1;
    if (tickcount < 0) tickcount = 0;
    mTickVector.resize(tickcount);
    for (int i=0; i<tickcount; ++i)
      mTickVector[i] = (firstStep+i)*mTickStep;
  } else // mScaleType == stLogarithmic
  {
    // Generate tick positions according to logbase scaling:
    if (mRange.lower > 0 && mRange.upper > 0) // positive range
    {
      double lowerMag = basePow((int)floor(baseLog(mRange.lower)));
      double currentMag = lowerMag;
      mTickVector.clear();
      mTickVector.append(currentMag);
      while (currentMag < mRange.upper && currentMag > 0) // currentMag might be zero for ranges ~1e-300, just cancel in that case
      {
        currentMag *= mScaleLogBase;
        mTickVector.append(currentMag);
      }
    } else if (mRange.lower < 0 && mRange.upper < 0) // negative range
    {
      double lowerMag = -basePow((int)ceil(baseLog(-mRange.lower)));
      double currentMag = lowerMag;
      mTickVector.clear();
      mTickVector.append(currentMag);
      while (currentMag < mRange.upper && currentMag < 0) // currentMag might be zero for ranges ~1e-300, just cancel in that case
      {
        currentMag /= mScaleLogBase;
        mTickVector.append(currentMag);
      }
    } else // invalid range for logarithmic scale, because lower and upper have different sign
    {
      mTickVector.clear();
      qDebug() << Q_FUNC_INFO << "Invalid range for logarithmic plot: " << mRange.lower << "-" << mRange.upper;
    }
  }
}

/*! \internal
  
  Called by generateAutoTicks when \ref setAutoSubTicks is set to true. Depending on the \a
  tickStep between two major ticks on the axis, a different number of sub ticks is appropriate. For
  Example taking 4 sub ticks for a \a tickStep of 1 makes more sense than taking 5 sub ticks,
  because this corresponds to a sub tick step of 0.2, instead of the less intuitive 0.16667. Note
  that a subtick count of 4 means dividing the major tick step into 5 sections.
  
  This is implemented by a hand made lookup for integer tick steps as well as fractional tick steps
  with a fractional part of (approximately) 0.5. If a tick step is different (i.e. has no
  fractional part close to 0.5), the currently set sub tick count (\ref setSubTickCount) is
  returned.
*/
int QCPAxis::calculateAutoSubTickCount(double tickStep) const
{
  int result = mSubTickCount; // default to current setting, if no proper value can be found
  
  // get mantissa of tickstep:
  double magnitudeFactor = qPow(10.0, qFloor(qLn(tickStep)/qLn(10.0))); // get magnitude factor e.g. 0.01, 1, 10, 1000 etc.
  double tickStepMantissa = tickStep/magnitudeFactor;
  
  // separate integer and fractional part of mantissa:
  double epsilon = 0.01;
  double intPartf;
  int intPart;
  double fracPart = modf(tickStepMantissa, &intPartf);
  intPart = intPartf;
  
  // handle cases with (almost) integer mantissa:
  if (fracPart < epsilon || 1.0-fracPart < epsilon)
  {
    if (1.0-fracPart < epsilon)
      ++intPart;
    switch (intPart)
    {
      case 1: result = 4; break; // 1.0 -> 0.2 substep
      case 2: result = 3; break; // 2.0 -> 0.5 substep
      case 3: result = 2; break; // 3.0 -> 1.0 substep
      case 4: result = 3; break; // 4.0 -> 1.0 substep
      case 5: result = 4; break; // 5.0 -> 1.0 substep
      case 6: result = 2; break; // 6.0 -> 2.0 substep
      case 7: result = 6; break; // 7.0 -> 1.0 substep
      case 8: result = 3; break; // 8.0 -> 2.0 substep
      case 9: result = 2; break; // 9.0 -> 3.0 substep
    }
  } else
  {
    // handle cases with significantly fractional mantissa:
    if (qAbs(fracPart-0.5) < epsilon) // *.5 mantissa
    {
      switch (intPart)
      {
        case 1: result = 2; break; // 1.5 -> 0.5 substep
        case 2: result = 4; break; // 2.5 -> 0.5 substep
        case 3: result = 4; break; // 3.5 -> 0.7 substep
        case 4: result = 2; break; // 4.5 -> 1.5 substep
        case 5: result = 4; break; // 5.5 -> 1.1 substep (won't occur with autoTickStep from here on)
        case 6: result = 4; break; // 6.5 -> 1.3 substep
        case 7: result = 2; break; // 7.5 -> 2.5 substep
        case 8: result = 4; break; // 8.5 -> 1.7 substep
        case 9: result = 4; break; // 9.5 -> 1.9 substep
      }
    }
    // if mantissa fraction isnt 0.0 or 0.5, don't bother finding good sub tick marks, leave default
  }
  
  return result;
}

/*! \internal
  
  Draws the axis with the specified \a painter.
  
  The selection boxes (mAxisSelectionBox, mTickLabelsSelectionBox, mLabelSelectionBox) are set
  here, too.
*/
void QCPAxis::draw(QCPPainter *painter)
{
  if (!mParentPlot) return;
  QPoint origin;
  if (mAxisType == atLeft)
    origin = mAxisRect->bottomLeft()+QPoint(-mOffset, 0);
  else if (mAxisType == atRight)
    origin = mAxisRect->bottomRight()+QPoint(+mOffset, 0);
  else if (mAxisType == atTop)
    origin = mAxisRect->topLeft()+QPoint(0, -mOffset);
  else if (mAxisType == atBottom)
    origin = mAxisRect->bottomLeft()+QPoint(0, +mOffset);
  
  double xCor = 0, yCor = 0; // paint system correction, for pixel exact matches (affects baselines and ticks of top/right axes)
  switch (mAxisType)
  {
    case atTop: yCor = -1; break;
    case atRight: xCor = 1; break;
    default: break;
  }
  
  int margin = 0;
  int lowTick = mLowestVisibleTick;
  int highTick = mHighestVisibleTick;
  double t; // helper variable, result of coordinate-to-pixel transforms

  // draw baseline:
  QLineF baseLine;
  painter->setPen(getBasePen());
  if (orientation() == Qt::Horizontal)
    baseLine.setPoints(origin+QPointF(xCor, yCor), origin+QPointF(mAxisRect->width()+xCor, yCor));
  else
    baseLine.setPoints(origin+QPointF(xCor, yCor), origin+QPointF(xCor, -mAxisRect->height()+yCor));
  if (mRangeReversed)
    baseLine = QLineF(baseLine.p2(), baseLine.p1()); // won't make a difference for line itself, but for line endings later
  painter->drawLine(baseLine);
  
  // draw ticks:
  if (mTicks)
  {
    painter->setPen(getTickPen());
    // direction of ticks ("inward" is right for left axis and left for right axis)
    int tickDir = (mAxisType == atBottom || mAxisType == atRight) ? -1 : 1;
    if (orientation() == Qt::Horizontal)
    {
      for (int i=lowTick; i <= highTick; ++i)
      {
        t = coordToPixel(mTickVector.at(i)); // x
        painter->drawLine(QLineF(t+xCor, origin.y()-mTickLengthOut*tickDir+yCor, t+xCor, origin.y()+mTickLengthIn*tickDir+yCor));
      }
    } else
    {
      for (int i=lowTick; i <= highTick; ++i)
      {
        t = coordToPixel(mTickVector.at(i)); // y
        painter->drawLine(QLineF(origin.x()-mTickLengthOut*tickDir+xCor, t+yCor, origin.x()+mTickLengthIn*tickDir+xCor, t+yCor));
      }
    }
  }
  
  // draw subticks:
  if (mTicks && mSubTickCount > 0)
  {
    painter->setPen(getSubTickPen());
    // direction of ticks ("inward" is right for left axis and left for right axis)
    int tickDir = (mAxisType == atBottom || mAxisType == atRight) ? -1 : 1;
    if (orientation() == Qt::Horizontal)
    {
      for (int i=0; i<mSubTickVector.size(); ++i) // no need to check bounds because subticks are always only created inside current mRange
      {
        t = coordToPixel(mSubTickVector.at(i));
        painter->drawLine(QLineF(t+xCor, origin.y()-mSubTickLengthOut*tickDir+yCor, t+xCor, origin.y()+mSubTickLengthIn*tickDir+yCor));
      }
    } else
    {
      for (int i=0; i<mSubTickVector.size(); ++i)
      {
        t = coordToPixel(mSubTickVector.at(i));
        painter->drawLine(QLineF(origin.x()-mSubTickLengthOut*tickDir+xCor, t+yCor, origin.x()+mSubTickLengthIn*tickDir+xCor, t+yCor));
      }
    }
  }
  margin += qMax(0, qMax(mTickLengthOut, mSubTickLengthOut));
  
  // draw axis base endings:
  bool antialiasingBackup = painter->antialiasing();
  painter->setAntialiasing(true); // always want endings to be antialiased, even if base and ticks themselves aren't
  painter->setBrush(QBrush(basePen().color()));
  QVector2D baseLineVector(baseLine.dx(), baseLine.dy());
  if (mLowerEnding.style() != QCPLineEnding::esNone)
    mLowerEnding.draw(painter, QVector2D(baseLine.p1())-baseLineVector.normalized()*mLowerEnding.realLength()*(mLowerEnding.inverted()?-1:1), -baseLineVector);
  if (mUpperEnding.style() != QCPLineEnding::esNone)
    mUpperEnding.draw(painter, QVector2D(baseLine.p2())+baseLineVector.normalized()*mUpperEnding.realLength()*(mUpperEnding.inverted()?-1:1), baseLineVector);
  painter->setAntialiasing(antialiasingBackup);
  
  // tick labels:
  QSize tickLabelsSize(0, 0); // size of largest tick label, for offset calculation of axis label
  if (mTickLabels)
  {
    margin += mTickLabelPadding;
    painter->setFont(getTickLabelFont());
    painter->setPen(QPen(getTickLabelColor()));
    for (int i=lowTick; i <= highTick; ++i)
    {
      t = coordToPixel(mTickVector.at(i));
      placeTickLabel(painter, t, margin, mTickVectorLabels.at(i), &tickLabelsSize);
    }
  }
  if (orientation() == Qt::Horizontal)
    margin += tickLabelsSize.height();
  else
    margin += tickLabelsSize.width();

  // axis label:
  QRect labelBounds;
  if (!mLabel.isEmpty())
  {
    margin += mLabelPadding;
    painter->setFont(getLabelFont());
    painter->setPen(QPen(getLabelColor()));
    labelBounds = painter->fontMetrics().boundingRect(0, 0, 0, 0, Qt::TextDontClip, mLabel);
    if (mAxisType == atLeft)
    {
      QTransform oldTransform = painter->transform();
      painter->translate((origin.x()-margin-labelBounds.height()), origin.y());
      painter->rotate(-90);
      painter->drawText(0, 0, mAxisRect->height(), labelBounds.height(), Qt::TextDontClip | Qt::AlignCenter, mLabel);
      painter->setTransform(oldTransform);
    }
    else if (mAxisType == atRight)
    {
      QTransform oldTransform = painter->transform();
      painter->translate((origin.x()+margin+labelBounds.height()), origin.y()-mAxisRect->height());
      painter->rotate(90);
      painter->drawText(0, 0, mAxisRect->height(), labelBounds.height(), Qt::TextDontClip | Qt::AlignCenter, mLabel);
      painter->setTransform(oldTransform);
    }
    else if (mAxisType == atTop)
      painter->drawText(origin.x(), origin.y()-margin-labelBounds.height(), mAxisRect->width(), labelBounds.height(), Qt::TextDontClip | Qt::AlignCenter, mLabel);
    else if (mAxisType == atBottom)
      painter->drawText(origin.x(), origin.y()+margin, mAxisRect->width(), labelBounds.height(), Qt::TextDontClip | Qt::AlignCenter, mLabel);
  }
  
  // set selection boxes:
  int selAxisOutSize = qMax(qMax(mTickLengthOut, mSubTickLengthOut), mParentPlot->selectionTolerance());
  int selAxisInSize = mParentPlot->selectionTolerance();
  int selTickLabelSize = (orientation()==Qt::Horizontal ? tickLabelsSize.height() : tickLabelsSize.width());
  int selTickLabelOffset = qMax(mTickLengthOut, mSubTickLengthOut)+mTickLabelPadding;
  int selLabelSize = labelBounds.height();
  int selLabelOffset = selTickLabelOffset+selTickLabelSize+mLabelPadding;
  if (mAxisType == atLeft)
  {
    mAxisSelectionBox.setCoords(origin.x()-selAxisOutSize, mAxisRect->top(), origin.x()+selAxisInSize, mAxisRect->bottom());
    mTickLabelsSelectionBox.setCoords(origin.x()-selTickLabelOffset-selTickLabelSize, mAxisRect->top(), origin.x()-selTickLabelOffset, mAxisRect->bottom());
    mLabelSelectionBox.setCoords(origin.x()-selLabelOffset-selLabelSize, mAxisRect->top(), origin.x()-selLabelOffset, mAxisRect->bottom());
  } else if (mAxisType == atRight)
  {
    mAxisSelectionBox.setCoords(origin.x()-selAxisInSize, mAxisRect->top(), origin.x()+selAxisOutSize, mAxisRect->bottom());
    mTickLabelsSelectionBox.setCoords(origin.x()+selTickLabelOffset+selTickLabelSize, mAxisRect->top(), origin.x()+selTickLabelOffset, mAxisRect->bottom());
    mLabelSelectionBox.setCoords(origin.x()+selLabelOffset+selLabelSize, mAxisRect->top(), origin.x()+selLabelOffset, mAxisRect->bottom());
  } else if (mAxisType == atTop)
  {
    mAxisSelectionBox.setCoords(mAxisRect->left(), origin.y()-selAxisOutSize, mAxisRect->right(), origin.y()+selAxisInSize);
    mTickLabelsSelectionBox.setCoords(mAxisRect->left(), origin.y()-selTickLabelOffset-selTickLabelSize, mAxisRect->right(), origin.y()-selTickLabelOffset);
    mLabelSelectionBox.setCoords(mAxisRect->left(), origin.y()-selLabelOffset-selLabelSize, mAxisRect->right(), origin.y()-selLabelOffset);
  } else if (mAxisType == atBottom)
  {
    mAxisSelectionBox.setCoords(mAxisRect->left(), origin.y()-selAxisInSize, mAxisRect->right(), origin.y()+selAxisOutSize);
    mTickLabelsSelectionBox.setCoords(mAxisRect->left(), origin.y()+selTickLabelOffset+selTickLabelSize, mAxisRect->right(), origin.y()+selTickLabelOffset);
    mLabelSelectionBox.setCoords(mAxisRect->left(), origin.y()+selLabelOffset+selLabelSize, mAxisRect->right(), origin.y()+selLabelOffset);
  }
  // draw hitboxes for debug purposes:
  //painter->setBrush(Qt::NoBrush);
  //painter->drawRects(QVector<QRect>() << mAxisSelectionBox << mTickLabelsSelectionBox << mLabelSelectionBox);
}

/*! \internal
  
  Draws a single tick label with the provided \a painter, utilizing the internal label cache to
  significantly speed up drawing of labels that were drawn in previous calls. The tick label is
  always bound to an axis, the distance to the axis is controllable via \a distanceToAxis in
  pixels. The pixel position in the axis direction is passed in the \a position parameter. Hence
  for the bottom axis, \a position would indicate the horizontal pixel position (not coordinate),
  at which the label should be drawn.
  
  In order to later draw the axis label in a place that doesn't overlap with the tick labels, the
  largest tick label size is needed. This is acquired by passing a \a tickLabelsSize to the \ref
  drawTickLabel calls during the process of drawing all tick labels of one axis. In every call, \a
  tickLabelsSize is expanded, if the drawn label exceeds the value \a tickLabelsSize currently
  holds.
  
  The label is drawn with the font and pen that are currently set on the \a painter. To draw
  superscripted powers, the font is temporarily made smaller by a fixed factor (see \ref
  getTickLabelData).
*/
void QCPAxis::placeTickLabel(QCPPainter *painter, double position, int distanceToAxis, const QString &text, QSize *tickLabelsSize)
{
  // warning: if you change anything here, also adapt getMaxTickLabelSize() accordingly!
  if (!mParentPlot) return;
  if (text.isEmpty()) return;
  QSize finalSize;
  QPointF labelAnchor;
  switch (mAxisType)
  {
    case atLeft:   labelAnchor = QPointF(mAxisRect->left()-distanceToAxis-mOffset, position); break;
    case atRight:  labelAnchor = QPointF(mAxisRect->right()+distanceToAxis+mOffset, position); break;
    case atTop:    labelAnchor = QPointF(position, mAxisRect->top()-distanceToAxis-mOffset); break;
    case atBottom: labelAnchor = QPointF(position, mAxisRect->bottom()+distanceToAxis+mOffset); break;
  }
  if (parentPlot()->plottingHints().testFlag(QCP::phCacheLabels) && !painter->modes().testFlag(QCPPainter::pmNoCaching)) // label caching enabled
  {
    if (!mLabelCache.contains(text))  // no cached label exists, create it
    {
      CachedLabel *newCachedLabel = new CachedLabel;
      TickLabelData labelData = getTickLabelData(painter->font(), text);
      QPointF drawOffset = getTickLabelDrawOffset(labelData);
      newCachedLabel->offset = drawOffset+labelData.rotatedTotalBounds.topLeft();
      newCachedLabel->pixmap = QPixmap(labelData.rotatedTotalBounds.size());
      newCachedLabel->pixmap.fill(Qt::transparent);
      QCPPainter cachePainter(&newCachedLabel->pixmap);
      cachePainter.setPen(painter->pen());
      drawTickLabel(&cachePainter, -labelData.rotatedTotalBounds.topLeft().x(), -labelData.rotatedTotalBounds.topLeft().y(), labelData);
      mLabelCache.insert(text, newCachedLabel, 1);
    }
    // draw cached label:
    const CachedLabel *cachedLabel = mLabelCache.object(text);
    // if label would be partly clipped by widget border on sides, don't draw it:
    if (orientation() == Qt::Horizontal)
    {
      if (labelAnchor.x()+cachedLabel->offset.x()+cachedLabel->pixmap.width() > mParentPlot->viewport().right() ||
          labelAnchor.x()+cachedLabel->offset.x() < mParentPlot->viewport().left())
        return;
    } else
    {
      if (labelAnchor.y()+cachedLabel->offset.y()+cachedLabel->pixmap.height() > mParentPlot->viewport().bottom() ||
          labelAnchor.y()+cachedLabel->offset.y() < mParentPlot->viewport().top())
        return;
    }
    painter->drawPixmap(labelAnchor+cachedLabel->offset, cachedLabel->pixmap);
    finalSize = cachedLabel->pixmap.size();
  } else // label caching disabled, draw text directly on surface:
  {
    TickLabelData labelData = getTickLabelData(painter->font(), text);
    QPointF finalPosition = labelAnchor + getTickLabelDrawOffset(labelData);
    // if label would be partly clipped by widget border on sides, don't draw it:
    if (orientation() == Qt::Horizontal)
    {
      if (finalPosition.x()+(labelData.rotatedTotalBounds.width()+labelData.rotatedTotalBounds.left()) > mParentPlot->viewport().right() ||
          finalPosition.x()+labelData.rotatedTotalBounds.left() < mParentPlot->viewport().left())
        return;
    } else
    {
      if (finalPosition.y()+(labelData.rotatedTotalBounds.height()+labelData.rotatedTotalBounds.top()) > mParentPlot->viewport().bottom() ||
          finalPosition.y()+labelData.rotatedTotalBounds.top() < mParentPlot->viewport().top())
        return;
    }
    drawTickLabel(painter, finalPosition.x(), finalPosition.y(), labelData);
    finalSize = labelData.rotatedTotalBounds.size();
  }
  
  // expand passed tickLabelsSize if current tick label is larger:
  if (finalSize.width() > tickLabelsSize->width()) 
    tickLabelsSize->setWidth(finalSize.width());
  if (finalSize.height() > tickLabelsSize->height())
    tickLabelsSize->setHeight(finalSize.height());
}

/*! \internal
  
  This is a \ref placeTickLabel helper function.
  
  Draws the tick label specified in \a labelData with \a painter at the pixel positions \a x and \a
  y. This function is used by \ref placeTickLabel to create new tick labels for the cache, or to
  directly draw the labels on the QCustomPlot surface when label caching is disabled, i.e. when
  QCP::phCacheLabels plotting hint is not set.
*/
void QCPAxis::drawTickLabel(QCPPainter *painter, double x, double y, const QCPAxis::TickLabelData &labelData) const
{
  // backup painter settings that we're about to change:
  QTransform oldTransform = painter->transform();
  QFont oldFont = painter->font();
  
  // transform painter to position/rotation:
  painter->translate(x, y);
  if (!qFuzzyIsNull(mTickLabelRotation))
    painter->rotate(mTickLabelRotation);
  
  // draw text:
  if (!labelData.expPart.isEmpty()) // indicator that beautiful powers must be used
  {
    painter->setFont(labelData.baseFont);
    painter->drawText(0, 0, 0, 0, Qt::TextDontClip, labelData.basePart);
    painter->setFont(labelData.expFont);
    painter->drawText(labelData.baseBounds.width()+1, 0, labelData.expBounds.width(), labelData.expBounds.height(), Qt::TextDontClip,  labelData.expPart);
  } else
  {
    painter->setFont(labelData.baseFont);
    painter->drawText(0, 0, labelData.totalBounds.width(), labelData.totalBounds.height(), Qt::TextDontClip | Qt::AlignHCenter, labelData.basePart);
  }
  
  // reset painter settings to what it was before:
  painter->setTransform(oldTransform);
  painter->setFont(oldFont);
}

/*! \internal
  
  This is a \ref placeTickLabel helper function.
  
  Transforms the passed \a text and \a font to a tickLabelData structure that can then be further
  processed by \ref getTickLabelDrawOffset and \ref drawTickLabel. It splits the text into base and
  exponent if necessary (see \ref setNumberFormat) and calculates appropriate bounding boxes.
*/
QCPAxis::TickLabelData QCPAxis::getTickLabelData(const QFont &font, const QString &text) const
{
  TickLabelData result;
  
  // determine whether beautiful decimal powers should be used
  bool useBeautifulPowers = false;
  int ePos = -1;
  if (mAutoTickLabels && mNumberBeautifulPowers && mTickLabelType == ltNumber)
  {
    ePos = text.indexOf('e');
    if (ePos > -1)
      useBeautifulPowers = true;
  }
  
  // calculate text bounding rects and do string preparation for beautiful decimal powers:
  result.baseFont = font;
  result.baseFont.setPointSizeF(result.baseFont.pointSizeF()+0.05); // QFontMetrics.boundingRect has a bug for exact point sizes that make the results oscillate due to internal rounding 
  if (useBeautifulPowers)
  {
    // split text into parts of number/symbol that will be drawn normally and part that will be drawn as exponent:
    result.basePart = text.left(ePos);
    // in log scaling, we want to turn "1*10^n" into "10^n", else add multiplication sign and decimal base:
    if (mScaleType == stLogarithmic && result.basePart == "1")
      result.basePart = "10";
    else
      result.basePart += (mNumberMultiplyCross ? QString(QChar(215)) : QString(QChar(183))) + "10";
    result.expPart = text.mid(ePos+1);
    // clip "+" and leading zeros off expPart:
    while (result.expPart.at(1) == '0' && result.expPart.length() > 2) // length > 2 so we leave one zero when numberFormatChar is 'e'
      result.expPart.remove(1, 1);
    if (result.expPart.at(0) == mPositiveSignChar)
      result.expPart.remove(0, 1);
    // prepare smaller font for exponent:
    result.expFont = font;
    result.expFont.setPointSize(result.expFont.pointSize()*0.75);
    // calculate bounding rects of base part, exponent part and total one:
    result.baseBounds = QFontMetrics(result.baseFont).boundingRect(0, 0, 0, 0, Qt::TextDontClip, result.basePart);
    result.expBounds = QFontMetrics(result.expFont).boundingRect(0, 0, 0, 0, Qt::TextDontClip, result.expPart);
    result.totalBounds = result.baseBounds.adjusted(0, 0, result.expBounds.width()+2, 0); // +2 consists of the 1 pixel spacing between base and exponent (see drawTickLabel) and an extra pixel to include AA
  } else // useBeautifulPowers == false
  {
    result.basePart = text;
    result.totalBounds = QFontMetrics(result.baseFont).boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter, result.basePart);
  }
  result.totalBounds.moveTopLeft(QPoint(0, 0)); // want bounding box aligned top left at origin, independent of how it was created, to make further processing simpler
  
  // calculate possibly different bounding rect after rotation:
  result.rotatedTotalBounds = result.totalBounds;
  if (!qFuzzyIsNull(mTickLabelRotation))
  {
    QTransform transform;
    transform.rotate(mTickLabelRotation);
    result.rotatedTotalBounds = transform.mapRect(result.rotatedTotalBounds);
  }
  
  return result;
}

/*! \internal
  
  This is a \ref placeTickLabel helper function.
  
  Calculates the offset at which the top left corner of the specified tick label shall be drawn.
  The offset is relative to a point right next to the tick the label belongs to.
  
  This function is thus responsible for e.g. centering tick labels under ticks and positioning them
  appropriately when they are rotated.
*/
QPointF QCPAxis::getTickLabelDrawOffset(const QCPAxis::TickLabelData &labelData) const
{
  /*
    calculate label offset from base point at tick (non-trivial, for best visual appearance): short
    explanation for bottom axis: The anchor, i.e. the point in the label that is placed
    horizontally under the corresponding tick is always on the label side that is closer to the
    axis (e.g. the left side of the text when we're rotating clockwise). On that side, the height
    is halved and the resulting point is defined the anchor. This way, a 90 degree rotated text
    will be centered under the tick (i.e. displaced horizontally by half its height). At the same
    time, a 45 degree rotated text will "point toward" its tick, as is typical for rotated tick
    labels.
  */
  bool doRotation = !qFuzzyIsNull(mTickLabelRotation);
  bool flip = qFuzzyCompare(qAbs(mTickLabelRotation), 90.0); // perfect +/-90 degree flip. Indicates vertical label centering on vertical axes.
  double radians = mTickLabelRotation/180.0*M_PI;
  int x=0, y=0;
  if (mAxisType == atLeft)
  {
    if (doRotation)
    {
      if (mTickLabelRotation > 0)
      {
        x = -qCos(radians)*labelData.totalBounds.width();
        y = flip ? -labelData.totalBounds.width()/2.0 : -qSin(radians)*labelData.totalBounds.width()-qCos(radians)*labelData.totalBounds.height()/2.0;
      } else
      {
        x = -qCos(-radians)*labelData.totalBounds.width()-qSin(-radians)*labelData.totalBounds.height();
        y = flip ? +labelData.totalBounds.width()/2.0 : +qSin(-radians)*labelData.totalBounds.width()-qCos(-radians)*labelData.totalBounds.height()/2.0;
      }
    } else
    {
      x = -labelData.totalBounds.width();
      y = -labelData.totalBounds.height()/2.0;
    }
  } else if (mAxisType == atRight)
  {
    if (doRotation)
    {
      if (mTickLabelRotation > 0)
      {
        x = +qSin(radians)*labelData.totalBounds.height();
        y = flip ? -labelData.totalBounds.width()/2.0 : -qCos(radians)*labelData.totalBounds.height()/2.0;
      } else
      {
        x = 0;
        y = flip ? +labelData.totalBounds.width()/2.0 : -qCos(-radians)*labelData.totalBounds.height()/2.0;
      }
    } else
    {
      x = 0;
      y = -labelData.totalBounds.height()/2.0;
    }
  } else if (mAxisType == atTop)
  {
    if (doRotation)
    {
      if (mTickLabelRotation > 0)
      {
        x = -qCos(radians)*labelData.totalBounds.width()+qSin(radians)*labelData.totalBounds.height()/2.0;
        y = -qSin(radians)*labelData.totalBounds.width()-qCos(radians)*labelData.totalBounds.height();
      } else
      {
        x = -qSin(-radians)*labelData.totalBounds.height()/2.0;
        y = -qCos(-radians)*labelData.totalBounds.height();
      }
    } else
    {
      x = -labelData.totalBounds.width()/2.0;
      y = -labelData.totalBounds.height();
    }
  } else if (mAxisType == atBottom)
  {
    if (doRotation)
    {
      if (mTickLabelRotation > 0)
      {
        x = +qSin(radians)*labelData.totalBounds.height()/2.0;
        y = 0;
      } else
      {
        x = -qCos(-radians)*labelData.totalBounds.width()-qSin(-radians)*labelData.totalBounds.height()/2.0;
        y = +qSin(-radians)*labelData.totalBounds.width();
      }
    } else
    {
      x = -labelData.totalBounds.width()/2.0;
      y = 0;
    }
  }
  
  return QPointF(x, y);
}

/*! \internal
  
  Simulates the steps done by \ref placeTickLabel by calculating bounding boxes of the text label
  to be drawn, depending on number format etc. Since only the largest tick label is wanted for the
  margin calculation, the passed \a tickLabelsSize is only expanded, if it's currently set to a
  smaller width/height.
*/
void QCPAxis::getMaxTickLabelSize(const QFont &font, const QString &text,  QSize *tickLabelsSize) const
{
  // note: this function must return the same tick label sizes as the placeTickLabel function.
  QSize finalSize;
  if (parentPlot()->plottingHints().testFlag(QCP::phCacheLabels) && mLabelCache.contains(text)) // label caching enabled and have cached label
  {
    const CachedLabel *cachedLabel = mLabelCache.object(text);
    finalSize = cachedLabel->pixmap.size();
  } else // label caching disabled or no label with this text cached:
  {
    TickLabelData labelData = getTickLabelData(font, text);
    finalSize = labelData.rotatedTotalBounds.size();
  }
  
  // expand passed tickLabelsSize if current tick label is larger:
  if (finalSize.width() > tickLabelsSize->width()) 
    tickLabelsSize->setWidth(finalSize.width());
  if (finalSize.height() > tickLabelsSize->height())
    tickLabelsSize->setHeight(finalSize.height());
}

/* inherits documentation from base class */
void QCPAxis::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  SelectablePart part = details.value<SelectablePart>();
  if (mSelectableParts.testFlag(part))
  {
    SelectableParts selBefore = mSelectedParts;
    setSelectedParts(additive ? mSelectedParts^part : part);
    if (selectionStateChanged)
      *selectionStateChanged = mSelectedParts != selBefore;
  }
}

/* inherits documentation from base class */
void QCPAxis::deselectEvent(bool *selectionStateChanged)
{
  SelectableParts selBefore = mSelectedParts;
  setSelectedParts(mSelectedParts & ~mSelectableParts);
  if (selectionStateChanged)
    *selectionStateChanged = mSelectedParts != selBefore;
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing axis lines.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased
*/
void QCPAxis::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeAxes);
}

/*! \internal
  
  Returns via \a lowIndex and \a highIndex, which ticks in the current tick vector are visible in
  the current range. The return values are indices of the tick vector, not the positions of the
  ticks themselves.
  
  The actual use of this function is when an external tick vector is provided, since it might
  exceed far beyond the currently displayed range, and would cause unnecessary calculations e.g. of
  subticks.
  
  If all ticks are outside the axis range, an inverted range is returned, i.e. highIndex will be
  smaller than lowIndex. There is one case, where this function returns indices that are not really
  visible in the current axis range: When the tick spacing is larger than the axis range size and
  one tick is below the axis range and the next tick is already above the axis range. Because in
  such cases it is usually desirable to know the tick pair, to draw proper subticks.
*/
void QCPAxis::visibleTickBounds(int &lowIndex, int &highIndex) const
{
  bool lowFound = false;
  bool highFound = false;
  lowIndex = 0;
  highIndex = -1;
  
  for (int i=0; i < mTickVector.size(); ++i)
  {
    if (mTickVector.at(i) >= mRange.lower)
    {
      lowFound = true;
      lowIndex = i;
      break;
    }
  }
  for (int i=mTickVector.size()-1; i >= 0; --i)
  {
    if (mTickVector.at(i) <= mRange.upper)
    {
      highFound = true;
      highIndex = i;
      break;
    }
  }
  
  if (!lowFound && highFound)
    lowIndex = highIndex+1;
  else if (lowFound && !highFound)
    highIndex = lowIndex-1;
}

/*! \internal
  
  A log function with the base mScaleLogBase, used mostly for coordinate transforms in logarithmic
  scales with arbitrary log base. Uses the buffered mScaleLogBaseLogInv for faster calculation.
  This is set to <tt>1.0/qLn(mScaleLogBase)</tt> in \ref setScaleLogBase.
  
  \see basePow, setScaleLogBase, setScaleType
*/
double QCPAxis::baseLog(double value) const
{
  return qLn(value)*mScaleLogBaseLogInv;
}

/*! \internal
  
  A power function with the base mScaleLogBase, used mostly for coordinate transforms in
  logarithmic scales with arbitrary log base.
  
  \see baseLog, setScaleLogBase, setScaleType
*/
double QCPAxis::basePow(double value) const
{
  return qPow(mScaleLogBase, value);
}

/*! \internal
  
  Returns the pen that is used to draw the axis base line. Depending on the selection state, this
  is either mSelectedBasePen or mBasePen.
*/
QPen QCPAxis::getBasePen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedBasePen : mBasePen;
}

/*! \internal
  
  Returns the pen that is used to draw the (major) ticks. Depending on the selection state, this
  is either mSelectedTickPen or mTickPen.
*/
QPen QCPAxis::getTickPen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedTickPen : mTickPen;
}

/*! \internal
  
  Returns the pen that is used to draw the subticks. Depending on the selection state, this
  is either mSelectedSubTickPen or mSubTickPen.
*/
QPen QCPAxis::getSubTickPen() const
{
  return mSelectedParts.testFlag(spAxis) ? mSelectedSubTickPen : mSubTickPen;
}

/*! \internal
  
  Returns the font that is used to draw the tick labels. Depending on the selection state, this
  is either mSelectedTickLabelFont or mTickLabelFont.
*/
QFont QCPAxis::getTickLabelFont() const
{
  return mSelectedParts.testFlag(spTickLabels) ? mSelectedTickLabelFont : mTickLabelFont;
}

/*! \internal
  
  Returns the font that is used to draw the axis label. Depending on the selection state, this
  is either mSelectedLabelFont or mLabelFont.
*/
QFont QCPAxis::getLabelFont() const
{
  return mSelectedParts.testFlag(spAxisLabel) ? mSelectedLabelFont : mLabelFont;
}

/*! \internal
  
  Returns the color that is used to draw the tick labels. Depending on the selection state, this
  is either mSelectedTickLabelColor or mTickLabelColor.
*/
QColor QCPAxis::getTickLabelColor() const
{
  return mSelectedParts.testFlag(spTickLabels) ? mSelectedTickLabelColor : mTickLabelColor;
}

/*! \internal
  
  Returns the color that is used to draw the axis label. Depending on the selection state, this
  is either mSelectedLabelColor or mLabelColor.
*/
QColor QCPAxis::getLabelColor() const
{
  return mSelectedParts.testFlag(spAxisLabel) ? mSelectedLabelColor : mLabelColor;
}

/*! \internal
  
  Returns the appropriate outward margin for this axis. It is needed if \ref
  QCPAxisRect::setAutoMargins is set to true on the parent axis rect. An axis with axis type \ref
  atLeft will return an appropriate left margin, \ref atBottom will return an appropriate bottom
  margin and so forth. For the calculation, this function goes through similar steps as \ref draw,
  so changing one function likely requires the modification of the other one as well.
  
  The margin consists of the outward tick length, tick label padding, tick label size, label
  padding, label size, and padding.
  
  The margin is cached internally, so repeated calls while leaving the axis range, fonts, etc.
  unchanged are very fast.
*/
int QCPAxis::calculateMargin()
{
  if (mCachedMarginValid)
    return mCachedMargin;
  
  // run through similar steps as QCPAxis::draw, and caluclate margin needed to fit axis and its labels
  int margin = 0;
  
  if (mVisible)
  {
    int lowTick, highTick;
    visibleTickBounds(lowTick, highTick);
    // get length of tick marks pointing outwards:
    if (mTicks)
      margin += qMax(0, qMax(mTickLengthOut, mSubTickLengthOut));
    // calculate size of tick labels:
    QSize tickLabelsSize(0, 0);
    if (mTickLabels)
    {
      for (int i=lowTick; i<=highTick; ++i)
        getMaxTickLabelSize(mTickLabelFont, mTickVectorLabels.at(i), &tickLabelsSize); // don't use getTickLabelFont() because we don't want margin to possibly change on selection
      margin += orientation() == Qt::Horizontal ? tickLabelsSize.height() : tickLabelsSize.width();
      margin += mTickLabelPadding;
    }
    // calculate size of axis label (only height needed, because left/right labels are rotated by 90 degrees):
    if (!mLabel.isEmpty())
    {
      QFontMetrics fontMetrics(mLabelFont); // don't use getLabelFont() because we don't want margin to possibly change on selection
      QRect bounds;
      bounds = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | Qt::AlignHCenter | Qt::AlignVCenter, mLabel);
      margin += bounds.height() + mLabelPadding;
    }
  }
  margin += mPadding;

  mCachedMargin = margin;
  mCachedMarginValid = true;
  return margin;
}

/* inherits documentation from base class */
QCP::Interaction QCPAxis::selectionCategory() const
{
  return QCP::iSelectAxes;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAbstractPlottable
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAbstractPlottable
  \brief The abstract base class for all data representing objects in a plot.

  It defines a very basic interface like name, pen, brush, visibility etc. Since this class is
  abstract, it can't be instantiated. Use one of the subclasses or create a subclass yourself to
  create new ways of displaying data (see "Creating own plottables" below).
  
  All further specifics are in the subclasses, for example:
  \li A normal graph with possibly a line, scatter points and error bars is displayed by \ref QCPGraph
  (typically created with \ref QCustomPlot::addGraph).
  \li A parametric curve can be displayed with \ref QCPCurve.
  \li A stackable bar chart can be achieved with \ref QCPBars.
  \li A box of a statistical box plot is created with \ref QCPStatisticalBox.
  
  \section plottables-subclassing Creating own plottables
  
  To create an own plottable, you implement a subclass of QCPAbstractPlottable. These are the pure
  virtual functions, you must implement:
  \li \ref clearData
  \li \ref selectTest
  \li \ref draw
  \li \ref drawLegendIcon
  \li \ref getKeyRange
  \li \ref getValueRange
  
  See the documentation of those functions for what they need to do.
  
  For drawing your plot, you can use the \ref coordsToPixels functions to translate a point in plot
  coordinates to pixel coordinates. This function is quite convenient, because it takes the
  orientation of the key and value axes into account for you (x and y are swapped when the key axis
  is vertical and the value axis horizontal). If you are worried about performance (i.e. you need
  to translate many points in a loop like QCPGraph), you can directly use \ref
  QCPAxis::coordToPixel. However, you must then take care about the orientation of the axis
  yourself.
  
  Here are some important members you inherit from QCPAbstractPlottable:
  <table>
  <tr>
    <td>QCustomPlot *\b mParentPlot</td>
    <td>A pointer to the parent QCustomPlot instance. The parent plot is inferred from the axes that are passed in the constructor.</td>
  </tr><tr>
    <td>QString \b mName</td>
    <td>The name of the plottable.</td>
  </tr><tr>
    <td>QPen \b mPen</td>
    <td>The generic pen of the plottable. You should use this pen for the most prominent data representing lines in the plottable (e.g QCPGraph uses this pen for its graph lines and scatters)</td>
  </tr><tr>
    <td>QPen \b mSelectedPen</td>
    <td>The generic pen that should be used when the plottable is selected (hint: \ref mainPen gives you the right pen, depending on selection state).</td>
  </tr><tr>
    <td>QBrush \b mBrush</td>
    <td>The generic brush of the plottable. You should use this brush for the most prominent fillable structures in the plottable (e.g. QCPGraph uses this brush to control filling under the graph)</td>
  </tr><tr>
    <td>QBrush \b mSelectedBrush</td>
    <td>The generic brush that should be used when the plottable is selected (hint: \ref mainBrush gives you the right brush, depending on selection state).</td>
  </tr><tr>
    <td>QPointer<QCPAxis>\b mKeyAxis, \b mValueAxis</td>
    <td>The key and value axes this plottable is attached to. Call their QCPAxis::coordToPixel functions to translate coordinates to pixels in either the key or value dimension.
        Make sure to check whether the weak pointer is null before using it. If one of the axes is null, don't draw the plottable.</td>
  </tr><tr>
    <td>bool \b mSelected</td>
    <td>indicates whether the plottable is selected or not.</td>
  </tr>
  </table>
*/

/* start of documentation of pure virtual functions */

/*! \fn void QCPAbstractPlottable::clearData() = 0
  Clears all data in the plottable.
*/

/*! \fn void QCPAbstractPlottable::drawLegendIcon(QCPPainter *painter, const QRect &rect) const = 0
  \internal
  
  called by QCPLegend::draw (via QCPPlottableLegendItem::draw) to create a graphical representation
  of this plottable inside \a rect, next to the plottable name.
*/

/*! \fn QCPRange QCPAbstractPlottable::getKeyRange(bool &validRange, SignDomain inSignDomain) const = 0
  \internal
  
  called by rescaleAxes functions to get the full data key bounds. For logarithmic plots, one can
  set \a inSignDomain to either \ref sdNegative or \ref sdPositive in order to restrict the
  returned range to that sign domain. E.g. when only negative range is wanted, set \a inSignDomain
  to \ref sdNegative and all positive points will be ignored for range calculation. For no
  restriction, just set \a inSignDomain to \ref sdBoth (default). \a validRange is an output
  parameter that indicates whether a proper range could be found or not. If this is false, you
  shouldn't use the returned range (e.g. no points in data).
  
  \see rescaleAxes, getValueRange
*/

/*! \fn QCPRange QCPAbstractPlottable::getValueRange(bool &validRange, SignDomain inSignDomain) const = 0
  \internal
  
  called by rescaleAxes functions to get the full data value bounds. For logarithmic plots, one can
  set \a inSignDomain to either \ref sdNegative or \ref sdPositive in order to restrict the
  returned range to that sign domain. E.g. when only negative range is wanted, set \a inSignDomain
  to \ref sdNegative and all positive points will be ignored for range calculation. For no
  restriction, just set \a inSignDomain to \ref sdBoth (default). \a validRange is an output
  parameter that indicates whether a proper range could be found or not. If this is false, you
  shouldn't use the returned range (e.g. no points in data).
  
  \see rescaleAxes, getKeyRange
*/

/* end of documentation of pure virtual functions */
/* start of documentation of signals */

/*! \fn void QCPAbstractPlottable::selectionChanged(bool selected)
  
  This signal is emitted when the selection state of this plottable has changed to \a selected,
  either by user interaction or by a direct call to \ref setSelected.
*/

/* end of documentation of signals */

/*!
  Constructs an abstract plottable which uses \a keyAxis as its key axis ("x") and \a valueAxis as
  its value axis ("y"). \a keyAxis and \a valueAxis must reside in the same QCustomPlot instance
  and have perpendicular orientations. If either of these restrictions is violated, a corresponding
  message is printed to the debug output (qDebug), the construction is not aborted, though.
  
  Since QCPAbstractPlottable is an abstract class that defines the basic interface to plottables,
  it can't be directly instantiated.
  
  You probably want one of the subclasses like \ref QCPGraph or \ref QCPCurve instead.
*/
QCPAbstractPlottable::QCPAbstractPlottable(QCPAxis *keyAxis, QCPAxis *valueAxis) :
  QCPLayerable(keyAxis->parentPlot(), "", keyAxis->axisRect()),
  mName(""),
  mAntialiasedFill(true),
  mAntialiasedScatters(true),
  mAntialiasedErrorBars(false),
  mPen(Qt::black),
  mSelectedPen(Qt::black),
  mBrush(Qt::NoBrush),
  mSelectedBrush(Qt::NoBrush),
  mKeyAxis(keyAxis),
  mValueAxis(valueAxis),
  mSelectable(true),
  mSelected(false)
{
  if (keyAxis->parentPlot() != valueAxis->parentPlot())
    qDebug() << Q_FUNC_INFO << "Parent plot of keyAxis is not the same as that of valueAxis.";
  if (keyAxis->orientation() == valueAxis->orientation())
    qDebug() << Q_FUNC_INFO << "keyAxis and valueAxis must be orthogonal to each other.";
}

/*!
   The name is the textual representation of this plottable as it is displayed in the legend
   (\ref QCPLegend). It may contain any UTF-8 characters, including newlines.
*/
void QCPAbstractPlottable::setName(const QString &name)
{
  mName = name;
}

/*!
  Sets whether fills of this plottable is drawn antialiased or not.
  
  Note that this setting may be overridden by \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
*/
void QCPAbstractPlottable::setAntialiasedFill(bool enabled)
{
  mAntialiasedFill = enabled;
}

/*!
  Sets whether the scatter symbols of this plottable are drawn antialiased or not.
  
  Note that this setting may be overridden by \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
*/
void QCPAbstractPlottable::setAntialiasedScatters(bool enabled)
{
  mAntialiasedScatters = enabled;
}

/*!
  Sets whether the error bars of this plottable are drawn antialiased or not.
  
  Note that this setting may be overridden by \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
*/
void QCPAbstractPlottable::setAntialiasedErrorBars(bool enabled)
{
  mAntialiasedErrorBars = enabled;
}


/*!
  The pen is used to draw basic lines that make up the plottable representation in the
  plot.
  
  For example, the \ref QCPGraph subclass draws its graph lines and scatter points
  with this pen.

  \see setBrush
*/
void QCPAbstractPlottable::setPen(const QPen &pen)
{
  mPen = pen;
}

/*!
  When the plottable is selected, this pen is used to draw basic lines instead of the normal
  pen set via \ref setPen.

  \see setSelected, setSelectable, setSelectedBrush, selectTest
*/
void QCPAbstractPlottable::setSelectedPen(const QPen &pen)
{
  mSelectedPen = pen;
}

/*!
  The brush is used to draw basic fills of the plottable representation in the
  plot. The Fill can be a color, gradient or texture, see the usage of QBrush.
  
  For example, the \ref QCPGraph subclass draws the fill under the graph with this brush, when
  it's not set to Qt::NoBrush.

  \see setPen
*/
void QCPAbstractPlottable::setBrush(const QBrush &brush)
{
  mBrush = brush;
}

/*!
  When the plottable is selected, this brush is used to draw fills instead of the normal
  brush set via \ref setBrush.

  \see setSelected, setSelectable, setSelectedPen, selectTest
*/
void QCPAbstractPlottable::setSelectedBrush(const QBrush &brush)
{
  mSelectedBrush = brush;
}

/*!
  The key axis of a plottable can be set to any axis of a QCustomPlot, as long as it is orthogonal
  to the plottable's value axis. This function performs no checks to make sure this is the case.
  The typical mathematical choice is to use the x-axis (QCustomPlot::xAxis) as key axis and the
  y-axis (QCustomPlot::yAxis) as value axis.
  
  Normally, the key and value axes are set in the constructor of the plottable (or \ref
  QCustomPlot::addGraph when working with QCPGraphs through the dedicated graph interface).

  \see setValueAxis
*/
void QCPAbstractPlottable::setKeyAxis(QCPAxis *axis)
{
  mKeyAxis = axis;
}

/*!
  The value axis of a plottable can be set to any axis of a QCustomPlot, as long as it is
  orthogonal to the plottable's key axis. This function performs no checks to make sure this is the
  case. The typical mathematical choice is to use the x-axis (QCustomPlot::xAxis) as key axis and
  the y-axis (QCustomPlot::yAxis) as value axis.

  Normally, the key and value axes are set in the constructor of the plottable (or \ref
  QCustomPlot::addGraph when working with QCPGraphs through the dedicated graph interface).
  
  \see setKeyAxis
*/
void QCPAbstractPlottable::setValueAxis(QCPAxis *axis)
{
  mValueAxis = axis;
}

/*!
  Sets whether the user can (de-)select this plottable by clicking on the QCustomPlot surface.
  (When \ref QCustomPlot::setInteractions contains iSelectPlottables.)
  
  However, even when \a selectable was set to false, it is possible to set the selection manually,
  by calling \ref setSelected directly.
  
  \see setSelected
*/
void QCPAbstractPlottable::setSelectable(bool selectable)
{
  mSelectable = selectable;
}

/*!
  Sets whether this plottable is selected or not. When selected, it uses a different pen and brush
  to draw its lines and fills, see \ref setSelectedPen and \ref setSelectedBrush.

  The entire selection mechanism for plottables is handled automatically when \ref
  QCustomPlot::setInteractions contains iSelectPlottables. You only need to call this function when
  you wish to change the selection state manually.
  
  This function can change the selection state even when \ref setSelectable was set to false.
  
  emits the \ref selectionChanged signal when \a selected is different from the previous selection state.
  
  \see setSelectable, selectTest
*/
void QCPAbstractPlottable::setSelected(bool selected)
{
  if (mSelected != selected)
  {
    mSelected = selected;
    emit selectionChanged(mSelected);
  }
}

/*!
  Rescales the key and value axes associated with this plottable to contain all displayed data, so
  the whole plottable is visible. If the scaling of an axis is logarithmic, rescaleAxes will make
  sure not to rescale to an illegal range i.e. a range containing different signs and/or zero.
  Instead it will stay in the current sign domain and ignore all parts of the plottable that lie
  outside of that domain.
  
  \a onlyEnlarge makes sure the ranges are only expanded, never reduced. So it's possible to show
  multiple plottables in their entirety by multiple calls to rescaleAxes where the first call has
  \a onlyEnlarge set to false (the default), and all subsequent set to true.
  
  \see rescaleKeyAxis, rescaleValueAxis, QCustomPlot::rescaleAxes, QCPAxis::rescale
*/
void QCPAbstractPlottable::rescaleAxes(bool onlyEnlarge) const
{
  rescaleKeyAxis(onlyEnlarge);
  rescaleValueAxis(onlyEnlarge);
}

/*!
  Rescales the key axis of the plottable so the whole plottable is visible.
  
  See \ref rescaleAxes for detailed behaviour.
*/
void QCPAbstractPlottable::rescaleKeyAxis(bool onlyEnlarge) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  if (!keyAxis) { qDebug() << Q_FUNC_INFO << "invalid key axis"; return; }
  
  SignDomain signDomain = sdBoth;
  if (keyAxis->scaleType() == QCPAxis::stLogarithmic)
    signDomain = (keyAxis->range().upper < 0 ? sdNegative : sdPositive);
  
  bool rangeValid;
  QCPRange newRange = getKeyRange(rangeValid, signDomain);
  if (rangeValid)
  {
    if (onlyEnlarge)
      newRange.expand(keyAxis->range());
    keyAxis->setRange(newRange);
  }
}

/*!
  Rescales the value axis of the plottable so the whole plottable is visible.
  
  Returns true if the axis was actually scaled. This might not be the case if this plottable has an
  invalid range, e.g. because it has no data points.
  
  See \ref rescaleAxes for detailed behaviour.
*/
void QCPAbstractPlottable::rescaleValueAxis(bool onlyEnlarge) const
{
  QCPAxis *valueAxis = mValueAxis.data();
  if (!valueAxis) { qDebug() << Q_FUNC_INFO << "invalid value axis"; return; }
  
  SignDomain signDomain = sdBoth;
  if (valueAxis->scaleType() == QCPAxis::stLogarithmic)
    signDomain = (valueAxis->range().upper < 0 ? sdNegative : sdPositive);
  
  bool rangeValid;
  QCPRange newRange = getValueRange(rangeValid, signDomain);
  if (rangeValid)
  {
    if (onlyEnlarge)
      newRange.expand(valueAxis->range());
    valueAxis->setRange(newRange);
  }
}

/*!
  Adds this plottable to the legend of the parent QCustomPlot (QCustomPlot::legend).
    
  Normally, a QCPPlottableLegendItem is created and inserted into the legend. If the plottable
  needs a more specialized representation in the legend, this function will take this into account
  and instead create the specialized subclass of QCPAbstractLegendItem.
    
  Returns true on success, i.e. when the legend exists and a legend item associated with this plottable isn't already in
  the legend.
    
  \see removeFromLegend, QCPLegend::addItem
*/
bool QCPAbstractPlottable::addToLegend()
{
  if (!mParentPlot || !mParentPlot->legend)
    return false;
  
  if (!mParentPlot->legend->hasItemWithPlottable(this))
  {
    mParentPlot->legend->addItem(new QCPPlottableLegendItem(mParentPlot->legend, this));
    return true;
  } else
    return false;
}

/*! 
  Removes the plottable from the legend of the parent QCustomPlot. This means the
  QCPAbstractLegendItem (usually a QCPPlottableLegendItem) that is associated with this plottable
  is removed.
    
  Returns true on success, i.e. if the legend exists and a legend item associated with this
  plottable was found and removed.
    
  \see addToLegend, QCPLegend::removeItem
*/
bool QCPAbstractPlottable::removeFromLegend() const
{
  if (!mParentPlot->legend)
    return false;
  
  if (QCPPlottableLegendItem *lip = mParentPlot->legend->itemWithPlottable(this))
    return mParentPlot->legend->removeItem(lip);
  else
    return false;
}

/* inherits documentation from base class */
QRect QCPAbstractPlottable::clipRect() const
{
  if (mKeyAxis && mValueAxis)
    return mKeyAxis.data()->axisRect()->rect() & mValueAxis.data()->axisRect()->rect();
  else
    return QRect();
}

/* inherits documentation from base class */
QCP::Interaction QCPAbstractPlottable::selectionCategory() const
{
  return QCP::iSelectPlottables;
}

/*! \internal
  
  Convenience function for transforming a key/value pair to pixels on the QCustomPlot surface,
  taking the orientations of the axes associated with this plottable into account (e.g. whether key
  represents x or y).
  
  \a key and \a value are transformed to the coodinates in pixels and are written to \a x and \a y.
    
  \see pixelsToCoords, QCPAxis::coordToPixel
*/
void QCPAbstractPlottable::coordsToPixels(double key, double value, double &x, double &y) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  if (keyAxis->orientation() == Qt::Horizontal)
  {
    x = keyAxis->coordToPixel(key);
    y = valueAxis->coordToPixel(value);
  } else
  {
    y = keyAxis->coordToPixel(key);
    x = valueAxis->coordToPixel(value);
  }
}

/*! \internal 
  \overload
  
  Returns the input as pixel coordinates in a QPointF.
*/
const QPointF QCPAbstractPlottable::coordsToPixels(double key, double value) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return QPointF(); }
  
  if (keyAxis->orientation() == Qt::Horizontal)
    return QPointF(keyAxis->coordToPixel(key), valueAxis->coordToPixel(value));
  else
    return QPointF(valueAxis->coordToPixel(value), keyAxis->coordToPixel(key));
}

/*! \internal
  
  Convenience function for transforming a x/y pixel pair on the QCustomPlot surface to plot coordinates,
  taking the orientations of the axes associated with this plottable into account (e.g. whether key
  represents x or y).
  
  \a x and \a y are transformed to the plot coodinates and are written to \a key and \a value.
    
  \see coordsToPixels, QCPAxis::coordToPixel
*/
void QCPAbstractPlottable::pixelsToCoords(double x, double y, double &key, double &value) const
{
  QCPAxis *keyAxis = mKeyAxis.data();
  QCPAxis *valueAxis = mValueAxis.data();
  if (!keyAxis || !valueAxis) { qDebug() << Q_FUNC_INFO << "invalid key or value axis"; return; }
  
  if (keyAxis->orientation() == Qt::Horizontal)
  {
    key = keyAxis->pixelToCoord(x);
    value = valueAxis->pixelToCoord(y);
  } else
  {
    key = keyAxis->pixelToCoord(y);
    value = valueAxis->pixelToCoord(x);
  }
}

/*! \internal
  \overload

  Returns the pixel input \a pixelPos as plot coordinates \a key and \a value.
*/
void QCPAbstractPlottable::pixelsToCoords(const QPointF &pixelPos, double &key, double &value) const
{
  pixelsToCoords(pixelPos.x(), pixelPos.y(), key, value);
}

/*! \internal

  Returns the pen that should be used for drawing lines of the plottable. Returns mPen when the
  graph is not selected and mSelectedPen when it is.
*/
QPen QCPAbstractPlottable::mainPen() const
{
  return mSelected ? mSelectedPen : mPen;
}

/*! \internal

  Returns the brush that should be used for drawing fills of the plottable. Returns mBrush when the
  graph is not selected and mSelectedBrush when it is.
*/
QBrush QCPAbstractPlottable::mainBrush() const
{
  return mSelected ? mSelectedBrush : mBrush;
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing plottable lines.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased, applyFillAntialiasingHint, applyScattersAntialiasingHint, applyErrorBarsAntialiasingHint
*/
void QCPAbstractPlottable::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aePlottables);
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing plottable fills.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased, applyDefaultAntialiasingHint, applyScattersAntialiasingHint, applyErrorBarsAntialiasingHint
*/
void QCPAbstractPlottable::applyFillAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiasedFill, QCP::aeFills);
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing plottable scatter points.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased, applyFillAntialiasingHint, applyDefaultAntialiasingHint, applyErrorBarsAntialiasingHint
*/
void QCPAbstractPlottable::applyScattersAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiasedScatters, QCP::aeScatters);
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing plottable error bars.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased, applyFillAntialiasingHint, applyScattersAntialiasingHint, applyDefaultAntialiasingHint
*/
void QCPAbstractPlottable::applyErrorBarsAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiasedErrorBars, QCP::aeErrorBars);
}

/*! \internal

  Finds the shortest squared distance of \a point to the line segment defined by \a start and \a
  end.
  
  This function may be used to help with the implementation of the \ref selectTest function for
  specific plottables.
  
  \note This function is identical to QCPAbstractItem::distSqrToLine
*/
double QCPAbstractPlottable::distSqrToLine(const QPointF &start, const QPointF &end, const QPointF &point) const
{
  QVector2D a(start);
  QVector2D b(end);
  QVector2D p(point);
  QVector2D v(b-a);
  
  double vLengthSqr = v.lengthSquared();
  if (!qFuzzyIsNull(vLengthSqr))
  {
    double mu = QVector2D::dotProduct(p-a, v)/vLengthSqr;
    if (mu < 0)
      return (a-p).lengthSquared();
    else if (mu > 1)
      return (b-p).lengthSquared();
    else
      return ((a + mu*v)-p).lengthSquared();
  } else
    return (a-p).lengthSquared();
}

/* inherits documentation from base class */
void QCPAbstractPlottable::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  Q_UNUSED(details)
  if (mSelectable)
  {
    bool selBefore = mSelected;
    setSelected(additive ? !mSelected : true);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

/* inherits documentation from base class */
void QCPAbstractPlottable::deselectEvent(bool *selectionStateChanged)
{
  if (mSelectable)
  {
    bool selBefore = mSelected;
    setSelected(false);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPItemAnchor
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPItemAnchor
  \brief An anchor of an item to which positions can be attached to.
  
  An item (QCPAbstractItem) may have one or more anchors. Unlike QCPItemPosition, an anchor doesn't
  control anything on its item, but provides a way to tie other items via their positions to the
  anchor.

  For example, a QCPItemRect is defined by its positions \a topLeft and \a bottomRight.
  Additionally it has various anchors like \a top, \a topRight or \a bottomLeft etc. So you can
  attach the \a start (which is a QCPItemPosition) of a QCPItemLine to one of the anchors by
  calling QCPItemPosition::setParentAnchor on \a start, passing the wanted anchor of the
  QCPItemRect. This way the start of the line will now always follow the respective anchor location
  on the rect item.
  
  Note that QCPItemPosition derives from QCPItemAnchor, so every position can also serve as an
  anchor to other positions.
  
  To learn how to provide anchors in your own item subclasses, see the subclassing section of the
  QCPAbstractItem documentation.
*/

/* start documentation of inline functions */

/*! \fn virtual QCPItemPosition *QCPItemAnchor::toQCPItemPosition()
  
  Returns 0 if this instance is merely a QCPItemAnchor, and a valid pointer of type QCPItemPosition* if
  it actually is a QCPItemPosition (which is a subclass of QCPItemAnchor).
  
  This safe downcast functionality could also be achieved with a dynamic_cast. However, QCustomPlot avoids
  dynamic_cast to work with projects that don't have RTTI support enabled (e.g. -fno-rtti flag with
  gcc compiler).
*/

/* end documentation of inline functions */

/*!
  Creates a new QCPItemAnchor. You shouldn't create QCPItemAnchor instances directly, even if
  you want to make a new item subclass. Use \ref QCPAbstractItem::createAnchor instead, as
  explained in the subclassing section of the QCPAbstractItem documentation.
*/
QCPItemAnchor::QCPItemAnchor(QCustomPlot *parentPlot, QCPAbstractItem *parentItem, const QString name, int anchorId) :
  mName(name),
  mParentPlot(parentPlot),
  mParentItem(parentItem),
  mAnchorId(anchorId)
{
}

QCPItemAnchor::~QCPItemAnchor()
{
  // unregister as parent at children:
  QList<QCPItemPosition*> currentChildren(mChildren.toList());
  for (int i=0; i<currentChildren.size(); ++i)
    currentChildren.at(i)->setParentAnchor(0); // this acts back on this anchor and child removes itself from mChildren
}

/*!
  Returns the final absolute pixel position of the QCPItemAnchor on the QCustomPlot surface.
  
  The pixel information is internally retrieved via QCPAbstractItem::anchorPixelPosition of the
  parent item, QCPItemAnchor is just an intermediary.
*/
QPointF QCPItemAnchor::pixelPoint() const
{
  if (mParentItem)
  {
    if (mAnchorId > -1)
    {
      return mParentItem->anchorPixelPoint(mAnchorId);
    } else
    {
      qDebug() << Q_FUNC_INFO << "no valid anchor id set:" << mAnchorId;
      return QPointF();
    }
  } else
  {
    qDebug() << Q_FUNC_INFO << "no parent item set";
    return QPointF();
  }
}

/*! \internal

  Adds \a pos to the child list of this anchor. This is necessary to notify the children prior to
  destruction of the anchor.
  
  Note that this function does not change the parent setting in \a pos.
*/
void QCPItemAnchor::addChild(QCPItemPosition *pos)
{
  if (!mChildren.contains(pos))
    mChildren.insert(pos);
  else
    qDebug() << Q_FUNC_INFO << "provided pos is child already" << reinterpret_cast<quintptr>(pos);
}

/*! \internal

  Removes \a pos from the child list of this anchor.
  
  Note that this function does not change the parent setting in \a pos.
*/
void QCPItemAnchor::removeChild(QCPItemPosition *pos)
{
  if (!mChildren.remove(pos))
    qDebug() << Q_FUNC_INFO << "provided pos isn't child" << reinterpret_cast<quintptr>(pos);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPItemPosition
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPItemPosition
  \brief Manages the position of an item.
  
  Every item has at least one public QCPItemPosition member pointer which provides ways to position the
  item on the QCustomPlot surface. Some items have multiple positions, for example QCPItemRect has two:
  \a topLeft and \a bottomRight.

  QCPItemPosition has a type (\ref PositionType) that can be set with \ref setType. This type defines
  how coordinates passed to \ref setCoords are to be interpreted, e.g. as absolute pixel coordinates, as
  plot coordinates of certain axes, etc.

  Further, QCPItemPosition may have a parent QCPItemAnchor, see \ref setParentAnchor. (Note that every
  QCPItemPosition inherits from QCPItemAnchor and thus can itself be used as parent anchor for other
  positions.) This way you can tie multiple items together. If the QCPItemPosition has a parent, the
  coordinates set with \ref setCoords are considered to be absolute values in the reference frame of the
  parent anchor, where (0, 0) means directly ontop of the parent anchor. For example, You could attach
  the \a start position of a QCPItemLine to the \a bottom anchor of a QCPItemText to make the starting
  point of the line always be centered under the text label, no matter where the text is moved to, or is
  itself tied to.

  To set the apparent pixel position on the QCustomPlot surface directly, use \ref setPixelPoint. This
  works no matter what type this QCPItemPosition is or what parent-child situation it is in, as \ref
  setPixelPoint transforms the coordinates appropriately, to make the position appear at the specified
  pixel values.
*/

/*!
  Creates a new QCPItemPosition. You shouldn't create QCPItemPosition instances directly, even if
  you want to make a new item subclass. Use \ref QCPAbstractItem::createPosition instead, as
  explained in the subclassing section of the QCPAbstractItem documentation.
*/
QCPItemPosition::QCPItemPosition(QCustomPlot *parentPlot, QCPAbstractItem *parentItem, const QString name) :
  QCPItemAnchor(parentPlot, parentItem, name),
  mPositionType(ptAbsolute),
  mKey(0),
  mValue(0),
  mParentAnchor(0)
{
}

QCPItemPosition::~QCPItemPosition()
{
  // unregister as parent at children:
  // Note: this is done in ~QCPItemAnchor again, but it's important QCPItemPosition does it itself, because only then
  //       the setParentAnchor(0) call the correct QCPItemPosition::pixelPoint function instead of QCPItemAnchor::pixelPoint
  QList<QCPItemPosition*> currentChildren(mChildren.toList());
  for (int i=0; i<currentChildren.size(); ++i)
    currentChildren.at(i)->setParentAnchor(0); // this acts back on this anchor and child removes itself from mChildren
  // unregister as child in parent:
  if (mParentAnchor)
    mParentAnchor->removeChild(this);
}

/* can't make this a header inline function, because QPointer breaks with forward declared types, see QTBUG-29588 */
QCPAxisRect *QCPItemPosition::axisRect() const
{
  return mAxisRect.data();
}

/*!
  Sets the type of the position. The type defines how the coordinates passed to \ref setCoords
  should be handled and how the QCPItemPosition should behave in the plot.
  
  The possible values for \a type can be separated in two main categories:

  \li The position is regarded as a point in plot coordinates. This corresponds to \ref ptPlotCoords
  and requires two axes that define the plot coordinate system. They can be specified with \ref setAxes.
  By default, the QCustomPlot's x- and yAxis are used.
  
  \li The position is fixed on the QCustomPlot surface, i.e. independent of axis ranges. This
  corresponds to all other types, i.e. \ref ptAbsolute, \ref ptViewportRatio and \ref
  ptAxisRectRatio. They differ only in the way the absolute position is described, see the
  documentation of PositionType for details. For \ref ptAxisRectRatio, note that you can specify
  the axis rect with \ref setAxisRect. By default this is set to the main axis rect.
  
  Note that the position type \ref ptPlotCoords is only available (and sensible) when the position
  has no parent anchor (\ref setParentAnchor).
  
  If the type is changed, the apparent pixel position on the plot is preserved. This means
  the coordinates as retrieved with coords() and set with \ref setCoords may change in the process.
*/
void QCPItemPosition::setType(QCPItemPosition::PositionType type)
{
  if (mPositionType != type)
  {
    // if switching from or to coordinate type that isn't valid (e.g. because axes or axis rect
    // were deleted), don't try to recover the pixelPoint() because it would output a qDebug warning.
    bool recoverPixelPosition = true;
    if ((mPositionType == ptPlotCoords || type == ptPlotCoords) && (!mKeyAxis || !mValueAxis))
      recoverPixelPosition = false;
    if ((mPositionType == ptAxisRectRatio || type == ptAxisRectRatio) && (!mAxisRect))
      recoverPixelPosition = false;
      
    QPointF pixelP;
    if (recoverPixelPosition)
      pixelP = pixelPoint();
    
    mPositionType = type;
    
    if (recoverPixelPosition)
      setPixelPoint(pixelP);
  }
}

/*!
  Sets the parent of this QCPItemPosition to \a parentAnchor. This means the position will now
  follow any position changes of the anchor. The local coordinate system of positions with a parent
  anchor always is absolute with (0, 0) being exactly on top of the parent anchor. (Hence the type
  shouldn't be \ref ptPlotCoords for positions with parent anchors.)
  
  if \a keepPixelPosition is true, the current pixel position of the QCPItemPosition is preserved
  during reparenting. If it's set to false, the coordinates are set to (0, 0), i.e. the position
  will be exactly on top of the parent anchor.
  
  To remove this QCPItemPosition from any parent anchor, set \a parentAnchor to 0.
  
  If the QCPItemPosition previously had no parent and the type is \ref ptPlotCoords, the type is
  set to \ref ptAbsolute, to keep the position in a valid state.
*/
bool QCPItemPosition::setParentAnchor(QCPItemAnchor *parentAnchor, bool keepPixelPosition)
{
  // make sure self is not assigned as parent:
  if (parentAnchor == this)
  {
    qDebug() << Q_FUNC_INFO << "can't set self as parent anchor" << reinterpret_cast<quintptr>(parentAnchor);
    return false;
  }
  // make sure no recursive parent-child-relationships are created:
  QCPItemAnchor *currentParent = parentAnchor;
  while (currentParent)
  {
    if (QCPItemPosition *currentParentPos = currentParent->toQCPItemPosition())
    {
      // is a QCPItemPosition, might have further parent, so keep iterating
      if (currentParentPos == this)
      {
        qDebug() << Q_FUNC_INFO << "can't create recursive parent-child-relationship" << reinterpret_cast<quintptr>(parentAnchor);
        return false;
      }
      currentParent = currentParentPos->mParentAnchor;
    } else
    {
      // is a QCPItemAnchor, can't have further parent. Now make sure the parent items aren't the
      // same, to prevent a position being child of an anchor which itself depends on the position,
      // because they're both on the same item:
      if (currentParent->mParentItem == mParentItem)
      {
        qDebug() << Q_FUNC_INFO << "can't set parent to be an anchor which itself depends on this position" << reinterpret_cast<quintptr>(parentAnchor);
        return false;
      }
      break;
    }
  }
  
  // if previously no parent set and PosType is still ptPlotCoords, set to ptAbsolute:
  if (!mParentAnchor && mPositionType == ptPlotCoords)
    setType(ptAbsolute);
  
  // save pixel position:
  QPointF pixelP;
  if (keepPixelPosition)
    pixelP = pixelPoint();
  // unregister at current parent anchor:
  if (mParentAnchor)
    mParentAnchor->removeChild(this);
  // register at new parent anchor:
  if (parentAnchor)
    parentAnchor->addChild(this);
  mParentAnchor = parentAnchor;
  // restore pixel position under new parent:
  if (keepPixelPosition)
    setPixelPoint(pixelP);
  else
    setCoords(0, 0);
  return true;
}

/*!
  Sets the coordinates of this QCPItemPosition. What the coordinates mean, is defined by the type
  (\ref setType).
  
  For example, if the type is \ref ptAbsolute, \a key and \a value mean the x and y pixel position
  on the QCustomPlot surface. In that case the origin (0, 0) is in the top left corner of the
  QCustomPlot viewport. If the type is \ref ptPlotCoords, \a key and \a value mean a point in the
  plot coordinate system defined by the axes set by \ref setAxes. By default those are the
  QCustomPlot's xAxis and yAxis. See the documentation of \ref setType for other available
  coordinate types and their meaning.

  \see setPixelPoint
*/
void QCPItemPosition::setCoords(double key, double value)
{
  mKey = key;
  mValue = value;
}

/*! \overload

  Sets the coordinates as a QPointF \a pos where pos.x has the meaning of \a key and pos.y the
  meaning of \a value of the \ref setCoords(double key, double value) method.
*/
void QCPItemPosition::setCoords(const QPointF &pos)
{
  setCoords(pos.x(), pos.y());
}

/*!
  Returns the final absolute pixel position of the QCPItemPosition on the QCustomPlot surface. It
  includes all effects of type (\ref setType) and possible parent anchors (\ref setParentAnchor).

  \see setPixelPoint
*/
QPointF QCPItemPosition::pixelPoint() const
{
  switch (mPositionType)
  {
    case ptAbsolute:
    {
      if (mParentAnchor)
        return QPointF(mKey, mValue) + mParentAnchor->pixelPoint();
      else
        return QPointF(mKey, mValue);
    }
    
    case ptViewportRatio:
    {
      if (mParentAnchor)
      {
        return QPointF(mKey*mParentPlot->viewport().width(),
                       mValue*mParentPlot->viewport().height()) + mParentAnchor->pixelPoint();
      } else
      {
        return QPointF(mKey*mParentPlot->viewport().width(),
                       mValue*mParentPlot->viewport().height()) + mParentPlot->viewport().topLeft();
      }
    }
      
    case ptAxisRectRatio:
    {
      if (mAxisRect)
      {
        if (mParentAnchor)
        {
          return QPointF(mKey*mAxisRect.data()->width(),
                         mValue*mAxisRect.data()->height()) + mParentAnchor->pixelPoint();
        } else
        {
          return QPointF(mKey*mAxisRect.data()->width(),
                       mValue*mAxisRect.data()->height()) + mAxisRect.data()->topLeft();
        }
      } else
      {
        qDebug() << Q_FUNC_INFO << "No axis rect defined";
        return QPointF(mKey, mValue);
      }
    }
    
    case ptPlotCoords:
    {
      double x, y;
      if (mKeyAxis && mValueAxis)
      {
        // both key and value axis are given, translate key/value to x/y coordinates:
        if (mKeyAxis.data()->orientation() == Qt::Horizontal)
        {
          x = mKeyAxis.data()->coordToPixel(mKey);
          y = mValueAxis.data()->coordToPixel(mValue);
        } else
        {
          y = mKeyAxis.data()->coordToPixel(mKey);
          x = mValueAxis.data()->coordToPixel(mValue);
        }
      } else if (mKeyAxis)
      {
        // only key axis is given, depending on orientation only transform x or y to key coordinate, other stays pixel:
        if (mKeyAxis.data()->orientation() == Qt::Horizontal)
        {
          x = mKeyAxis.data()->coordToPixel(mKey);
          y = mValue;
        } else
        {
          y = mKeyAxis.data()->coordToPixel(mKey);
          x = mValue;
        }
      } else if (mValueAxis)
      {
        // only value axis is given, depending on orientation only transform x or y to value coordinate, other stays pixel:
        if (mValueAxis.data()->orientation() == Qt::Horizontal)
        {
          x = mValueAxis.data()->coordToPixel(mValue);
          y = mKey;
        } else
        {
          y = mValueAxis.data()->coordToPixel(mValue);
          x = mKey;
        }
      } else
      {
        // no axis given, basically the same as if mPositionType were ptAbsolute
        qDebug() << Q_FUNC_INFO << "No axes defined";
        x = mKey;
        y = mValue;
      }
      return QPointF(x, y);
    }
  }
  return QPointF();
}

/*!
  When \ref setType is \ref ptPlotCoords, this function may be used to specify the axes the
  coordinates set with \ref setCoords relate to. By default they are set to the initial xAxis and
  yAxis of the QCustomPlot.
*/
void QCPItemPosition::setAxes(QCPAxis *keyAxis, QCPAxis *valueAxis)
{
  mKeyAxis = keyAxis;
  mValueAxis = valueAxis;
}

/*!
  When \ref setType is \ref ptAxisRectRatio, this function may be used to specify the axis rect the
  coordinates set with \ref setCoords relate to. By default this is set to the main axis rect of
  the QCustomPlot.
*/
void QCPItemPosition::setAxisRect(QCPAxisRect *axisRect)
{
  mAxisRect = axisRect;
}

/*!
  Sets the apparent pixel position. This works no matter what type (\ref setType) this
  QCPItemPosition is or what parent-child situation it is in, as coordinates are transformed
  appropriately, to make the position finally appear at the specified pixel values.

  Only if the type is \ref ptAbsolute and no parent anchor is set, this function's effect is
  identical to that of \ref setCoords.

  \see pixelPoint, setCoords
*/
void QCPItemPosition::setPixelPoint(const QPointF &pixelPoint)
{
  switch (mPositionType)
  {
    case ptAbsolute:
    {
      if (mParentAnchor)
        setCoords(pixelPoint-mParentAnchor->pixelPoint());
      else
        setCoords(pixelPoint);
      break;
    }
      
    case ptViewportRatio:
    {
      if (mParentAnchor)
      {
        QPointF p(pixelPoint-mParentAnchor->pixelPoint());
        p.rx() /= (double)mParentPlot->viewport().width();
        p.ry() /= (double)mParentPlot->viewport().height();
        setCoords(p);
      } else
      {
        QPointF p(pixelPoint-mParentPlot->viewport().topLeft());
        p.rx() /= (double)mParentPlot->viewport().width();
        p.ry() /= (double)mParentPlot->viewport().height();
        setCoords(p);
      }
      break;
    }
      
    case ptAxisRectRatio:
    {
      if (mAxisRect)
      {
        if (mParentAnchor)
        {
          QPointF p(pixelPoint-mParentAnchor->pixelPoint());
          p.rx() /= (double)mAxisRect.data()->width();
          p.ry() /= (double)mAxisRect.data()->height();
          setCoords(p);
        } else
        {
          QPointF p(pixelPoint-mAxisRect.data()->topLeft());
          p.rx() /= (double)mAxisRect.data()->width();
          p.ry() /= (double)mAxisRect.data()->height();
          setCoords(p);
        }
      } else
      {
        qDebug() << Q_FUNC_INFO << "No axis rect defined";
        setCoords(pixelPoint);
      }
      break;
    }
      
    case ptPlotCoords:
    {
      double newKey, newValue;
      if (mKeyAxis && mValueAxis)
      {
        // both key and value axis are given, translate point to key/value coordinates:
        if (mKeyAxis.data()->orientation() == Qt::Horizontal)
        {
          newKey = mKeyAxis.data()->pixelToCoord(pixelPoint.x());
          newValue = mValueAxis.data()->pixelToCoord(pixelPoint.y());
        } else
        {
          newKey = mKeyAxis.data()->pixelToCoord(pixelPoint.y());
          newValue = mValueAxis.data()->pixelToCoord(pixelPoint.x());
        }
      } else if (mKeyAxis)
      {
        // only key axis is given, depending on orientation only transform x or y to key coordinate, other stays pixel:
        if (mKeyAxis.data()->orientation() == Qt::Horizontal)
        {
          newKey = mKeyAxis.data()->pixelToCoord(pixelPoint.x());
          newValue = pixelPoint.y();
        } else
        {
          newKey = mKeyAxis.data()->pixelToCoord(pixelPoint.y());
          newValue = pixelPoint.x();
        }
      } else if (mValueAxis)
      {
        // only value axis is given, depending on orientation only transform x or y to value coordinate, other stays pixel:
        if (mValueAxis.data()->orientation() == Qt::Horizontal)
        {
          newKey = pixelPoint.y();
          newValue = mValueAxis.data()->pixelToCoord(pixelPoint.x());
        } else
        {
          newKey = pixelPoint.x();
          newValue = mValueAxis.data()->pixelToCoord(pixelPoint.y());
        }
      } else
      {
        // no axis given, basically the same as if mPositionType were ptAbsolute
        qDebug() << Q_FUNC_INFO << "No axes defined";
        newKey = pixelPoint.x();
        newValue = pixelPoint.y();
      }
      setCoords(newKey, newValue);
      break;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCPAbstractItem
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCPAbstractItem
  \brief The abstract base class for all items in a plot.
  
  In QCustomPlot, items are supplemental graphical elements that are neither plottables
  (QCPAbstractPlottable) nor axes (QCPAxis). While plottables are always tied to two axes and thus
  plot coordinates, items can also be placed in absolute coordinates independent of any axes. Each
  specific item has at least one QCPItemPosition member which controls the positioning. Some items
  are defined by more than one coordinate and thus have two or more QCPItemPosition members (For
  example, QCPItemRect has \a topLeft and \a bottomRight).
  
  This abstract base class defines a very basic interface like visibility and clipping. Since this
  class is abstract, it can't be instantiated. Use one of the subclasses or create a subclass
  yourself to create new items.
  
  The built-in items are:
  <table>
  <tr><td>QCPItemLine</td><td>A line defined by a start and an end point. May have different ending styles on each side (e.g. arrows).</td></tr>
  <tr><td>QCPItemStraightLine</td><td>A straight line defined by a start and a direction point. Unlike QCPItemLine, the straight line is infinitely long and has no endings.</td></tr>
  <tr><td>QCPItemCurve</td><td>A curve defined by start, end and two intermediate control points. May have different ending styles on each side (e.g. arrows).</td></tr>
  <tr><td>QCPItemRect</td><td>A rectangle</td></tr>
  <tr><td>QCPItemEllipse</td><td>An ellipse</td></tr>
  <tr><td>QCPItemPixmap</td><td>An arbitrary pixmap</td></tr>
  <tr><td>QCPItemText</td><td>A text label</td></tr>
  <tr><td>QCPItemBracket</td><td>A bracket which may be used to reference/highlight certain parts in the plot.</td></tr>
  <tr><td>QCPItemTracer</td><td>An item that can be attached to a QCPGraph and sticks to its data points, given a key coordinate.</td></tr>
  </table>
  
  Items are by default clipped to the main axis rect. To make an item visible outside that axis
  rect, disable clipping via \ref setClipToAxisRect.
  
  \section items-using Using items
  
  First you instantiate the item you want to use and add it to the plot:
  \code
  QCPItemLine *line = new QCPItemLine(customPlot);
  customPlot->addItem(line);
  \endcode
  by default, the positions of the item are bound to the x- and y-Axis of the plot. So we can just
  set the plot coordinates where the line should start/end:
  \code
  line->start->setCoords(-0.1, 0.8);
  line->end->setCoords(1.1, 0.2);
  \endcode
  If we don't want the line to be positioned in plot coordinates but a different coordinate system,
  e.g. absolute pixel positions on the QCustomPlot surface, we need to change the position type like this:
  \code
  line->start->setType(QCPItemPosition::ptAbsolute);
  line->end->setType(QCPItemPosition::ptAbsolute);
  \endcode
  Then we can set the coordinates, this time in pixels:
  \code
  line->start->setCoords(100, 200);
  line->end->setCoords(450, 320);
  \endcode
  
  \section items-subclassing Creating own items
  
  To create an own item, you implement a subclass of QCPAbstractItem. These are the pure
  virtual functions, you must implement:
  \li \ref selectTest
  \li \ref draw
  
  See the documentation of those functions for what they need to do.
  
  \subsection items-positioning Allowing the item to be positioned
  
  As mentioned, item positions are represented by QCPItemPosition members. Let's assume the new item shall
  have only one point as its position (as opposed to two like a rect or multiple like a polygon). You then add
  a public member of type QCPItemPosition like so:
  
  \code QCPItemPosition * const myPosition;\endcode
  
  the const makes sure the pointer itself can't be modified from the user of your new item (the QCPItemPosition
  instance it points to, can be modified, of course).
  The initialization of this pointer is made easy with the \ref createPosition function. Just assign
  the return value of this function to each QCPItemPosition in the constructor of your item. \ref createPosition
  takes a string which is the name of the position, typically this is identical to the variable name.
  For example, the constructor of QCPItemExample could look like this:
  
  \code
  QCPItemExample::QCPItemExample(QCustomPlot *parentPlot) :
    QCPAbstractItem(parentPlot),
    myPosition(createPosition("myPosition"))
  {
    // other constructor code
  }
  \endcode
  
  \subsection items-drawing The draw function
  
  To give your item a visual representation, reimplement the \ref draw function and use the passed
  QCPPainter to draw the item. You can retrieve the item position in pixel coordinates from the
  position member(s) via \ref QCPItemPosition::pixelPoint.

  To optimize performance you should calculate a bounding rect first (don't forget to take the pen
  width into account), check whether it intersects the \ref clipRect, and only draw the item at all
  if this is the case.
  
  \subsection items-selection The selectTest function
  
  Your implementation of the \ref selectTest function may use the helpers \ref distSqrToLine and
  \ref rectSelectTest. With these, the implementation of the selection test becomes significantly
  simpler for most items. See the documentation of \ref selectTest for what the function parameters
  mean and what the function should return.
  
  \subsection anchors Providing anchors
  
  Providing anchors (QCPItemAnchor) starts off like adding a position. First you create a public
  member, e.g.
  
  \code QCPItemAnchor * const bottom;\endcode

  and create it in the constructor with the \ref createAnchor function, assigning it a name and an
  anchor id (an integer enumerating all anchors on the item, you may create an own enum for this).
  Since anchors can be placed anywhere, relative to the item's position(s), your item needs to
  provide the position of every anchor with the reimplementation of the \ref anchorPixelPoint(int
  anchorId) function.
  
  In essence the QCPItemAnchor is merely an intermediary that itself asks your item for the pixel
  position when anything attached to the anchor needs to know the coordinates.
*/

/* start of documentation of inline functions */

/*! \fn QList<QCPItemPosition*> QCPAbstractItem::positions() const
  
  Returns all positions of the item in a list.
  
  \see anchors, position
*/

/*! \fn QList<QCPItemAnchor*> QCPAbstractItem::anchors() const
  
  Returns all anchors of the item in a list. Note that since a position (QCPItemPosition) is always
  also an anchor, the list will also contain the positions of this item.
  
  \see positions, anchor
*/

/* end of documentation of inline functions */
/* start documentation of pure virtual functions */

/*! \fn void QCPAbstractItem::draw(QCPPainter *painter) = 0
  \internal
  
  Draws this item with the provided \a painter.
  
  The cliprect of the provided painter is set to the rect returned by \ref clipRect before this
  function is called. The clipRect depends on the clipping settings defined by \ref
  setClipToAxisRect and \ref setClipAxisRect.
*/

/* end documentation of pure virtual functions */
/* start documentation of signals */

/*! \fn void QCPAbstractItem::selectionChanged(bool selected)
  This signal is emitted when the selection state of this item has changed, either by user interaction
  or by a direct call to \ref setSelected.
*/

/* end documentation of signals */

/*!
  Base class constructor which initializes base class members.
*/
QCPAbstractItem::QCPAbstractItem(QCustomPlot *parentPlot) :
  QCPLayerable(parentPlot),
  mClipToAxisRect(false),
  mSelectable(true),
  mSelected(false)
{
  QList<QCPAxisRect*> rects = parentPlot->axisRects();
  if (rects.size() > 0)
  {
    setClipToAxisRect(true);
    setClipAxisRect(rects.first());
  }
}

QCPAbstractItem::~QCPAbstractItem()
{
  // don't delete mPositions because every position is also an anchor and thus in mAnchors
  qDeleteAll(mAnchors);
}

/* can't make this a header inline function, because QPointer breaks with forward declared types, see QTBUG-29588 */
QCPAxisRect *QCPAbstractItem::clipAxisRect() const
{
  return mClipAxisRect.data();
}

/*!
  Sets whether the item shall be clipped to an axis rect or whether it shall be visible on the
  entire QCustomPlot. The axis rect can be set with \ref setClipAxisRect.
  
  \see setClipAxisRect
*/
void QCPAbstractItem::setClipToAxisRect(bool clip)
{
  mClipToAxisRect = clip;
  if (mClipToAxisRect)
    setParentLayerable(mClipAxisRect.data());
}

/*!
  Sets the clip axis rect. It defines the rect that will be used to clip the item when \ref
  setClipToAxisRect is set to true.
  
  \see setClipToAxisRect
*/
void QCPAbstractItem::setClipAxisRect(QCPAxisRect *rect)
{
  mClipAxisRect = rect;
  if (mClipToAxisRect)
    setParentLayerable(mClipAxisRect.data());
}

/*!
  Sets whether the user can (de-)select this item by clicking on the QCustomPlot surface.
  (When \ref QCustomPlot::setInteractions contains QCustomPlot::iSelectItems.)
  
  However, even when \a selectable was set to false, it is possible to set the selection manually,
  by calling \ref setSelected.
  
  \see QCustomPlot::setInteractions, setSelected
*/
void QCPAbstractItem::setSelectable(bool selectable)
{
  mSelectable = selectable;
}

/*!
  Sets whether this item is selected or not. When selected, it might use a different visual
  appearance (e.g. pen and brush), this depends on the specific item though.

  The entire selection mechanism for items is handled automatically when \ref
  QCustomPlot::setInteractions contains QCustomPlot::iSelectItems. You only need to call this
  function when you wish to change the selection state manually.
  
  This function can change the selection state even when \ref setSelectable was set to false.
  
  emits the \ref selectionChanged signal when \a selected is different from the previous selection state.
  
  \see setSelectable, selectTest
*/
void QCPAbstractItem::setSelected(bool selected)
{
  if (mSelected != selected)
  {
    mSelected = selected;
    emit selectionChanged(mSelected);
  }
}

/*!
  Returns the QCPItemPosition with the specified \a name. If this item doesn't have a position by
  that name, returns 0.
  
  This function provides an alternative way to access item positions. Normally, you access
  positions direcly by their member pointers (which typically have the same variable name as \a
  name).
  
  \see positions, anchor 
*/
QCPItemPosition *QCPAbstractItem::position(const QString &name) const
{
  for (int i=0; i<mPositions.size(); ++i)
  {
    if (mPositions.at(i)->name() == name)
      return mPositions.at(i);
  }
  qDebug() << Q_FUNC_INFO << "position with name not found:" << name;
  return 0;
}

/*!
  Returns the QCPItemAnchor with the specified \a name. If this item doesn't have an anchor by
  that name, returns 0.
  
  This function provides an alternative way to access item anchors. Normally, you access
  anchors direcly by their member pointers (which typically have the same variable name as \a
  name).
  
  \see anchors, position 
*/
QCPItemAnchor *QCPAbstractItem::anchor(const QString &name) const
{
  for (int i=0; i<mAnchors.size(); ++i)
  {
    if (mAnchors.at(i)->name() == name)
      return mAnchors.at(i);
  }
  qDebug() << Q_FUNC_INFO << "anchor with name not found:" << name;
  return 0;
}

/*!
  Returns whether this item has an anchor with the specified \a name.
  
  Note that you can check for positions with this function, too. This is because every position is
  also an anchor (QCPItemPosition inherits from QCPItemAnchor).
  
  \see anchor, position 
*/
bool QCPAbstractItem::hasAnchor(const QString &name) const
{
  for (int i=0; i<mAnchors.size(); ++i)
  {
    if (mAnchors.at(i)->name() == name)
      return true;
  }
  return false;
}

/*! \internal
  
  Returns the rect the visual representation of this item is clipped to. This depends on the
  current setting of \ref setClipToAxisRect as well as the axis rect set with \ref setClipAxisRect.
  
  If the item is not clipped to an axis rect, the \ref QCustomPlot::viewport rect is returned.
  
  \see draw
*/
QRect QCPAbstractItem::clipRect() const
{
  if (mClipToAxisRect && mClipAxisRect)
    return mClipAxisRect.data()->rect();
  else
    return mParentPlot->viewport();
}

/*! \internal

  A convenience function to easily set the QPainter::Antialiased hint on the provided \a painter
  before drawing item lines.

  This is the antialiasing state the painter passed to the \ref draw method is in by default.
  
  This function takes into account the local setting of the antialiasing flag as well as the
  overrides set with \ref QCustomPlot::setAntialiasedElements and \ref
  QCustomPlot::setNotAntialiasedElements.
  
  \see setAntialiased
*/
void QCPAbstractItem::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
  applyAntialiasingHint(painter, mAntialiased, QCP::aeItems);
}

/*! \internal

  Finds the shortest squared distance of \a point to the line segment defined by \a start and \a
  end.
  
  This function may be used to help with the implementation of the \ref selectTest function for
  specific items.
  
  \note This function is identical to QCPAbstractPlottable::distSqrToLine
  
  \see rectSelectTest
*/
double QCPAbstractItem::distSqrToLine(const QPointF &start, const QPointF &end, const QPointF &point) const
{
  QVector2D a(start);
  QVector2D b(end);
  QVector2D p(point);
  QVector2D v(b-a);
  
  double vLengthSqr = v.lengthSquared();
  if (!qFuzzyIsNull(vLengthSqr))
  {
    double mu = QVector2D::dotProduct(p-a, v)/vLengthSqr;
    if (mu < 0)
      return (a-p).lengthSquared();
    else if (mu > 1)
      return (b-p).lengthSquared();
    else
      return ((a + mu*v)-p).lengthSquared();
  } else
    return (a-p).lengthSquared();
}

/*! \internal

  A convenience function which returns the selectTest value for a specified \a rect and a specified
  click position \a pos. \a filledRect defines whether a click inside the rect should also be
  considered a hit or whether only the rect border is sensitive to hits.
  
  This function may be used to help with the implementation of the \ref selectTest function for
  specific items.
  
  For example, if your item consists of four rects, call this function four times, once for each
  rect, in your \ref selectTest reimplementation. Finally, return the minimum of all four returned
  values which were greater or equal to zero. (Because this function may return -1.0 when \a pos
  doesn't hit \a rect at all). If all calls returned -1.0, return -1.0, too, because your item
  wasn't hit.
  
  \see distSqrToLine
*/
double QCPAbstractItem::rectSelectTest(const QRectF &rect, const QPointF &pos, bool filledRect) const
{
  double result = -1;

  // distance to border:
  QList<QLineF> lines;
  lines << QLineF(rect.topLeft(), rect.topRight()) << QLineF(rect.bottomLeft(), rect.bottomRight())
        << QLineF(rect.topLeft(), rect.bottomLeft()) << QLineF(rect.topRight(), rect.bottomRight());
  double minDistSqr = std::numeric_limits<double>::max();
  for (int i=0; i<lines.size(); ++i)
  {
    double distSqr = distSqrToLine(lines.at(i).p1(), lines.at(i).p2(), pos);
    if (distSqr < minDistSqr)
      minDistSqr = distSqr;
  }
  result = qSqrt(minDistSqr);
  
  // filled rect, allow click inside to count as hit:
  if (filledRect && result > mParentPlot->selectionTolerance()*0.99)
  {
    if (rect.contains(pos))
      result = mParentPlot->selectionTolerance()*0.99;
  }
  return result;
}

/*! \internal

  Returns the pixel position of the anchor with Id \a anchorId. This function must be reimplemented in
  item subclasses if they want to provide anchors (QCPItemAnchor).
  
  For example, if the item has two anchors with id 0 and 1, this function takes one of these anchor
  ids and returns the respective pixel points of the specified anchor.
  
  \see createAnchor
*/
QPointF QCPAbstractItem::anchorPixelPoint(int anchorId) const
{
  qDebug() << Q_FUNC_INFO << "called on item which shouldn't have any anchors (this method not reimplemented). anchorId" << anchorId;
  return QPointF();
}

/*! \internal

  Creates a QCPItemPosition, registers it with this item and returns a pointer to it. The specified
  \a name must be a unique string that is usually identical to the variable name of the position
  member (This is needed to provide the name-based \ref position access to positions).
  
  Don't delete positions created by this function manually, as the item will take care of it.
  
  Use this function in the constructor (initialization list) of the specific item subclass to
  create each position member. Don't create QCPItemPositions with \b new yourself, because they
  won't be registered with the item properly.
  
  \see createAnchor
*/
QCPItemPosition *QCPAbstractItem::createPosition(const QString &name)
{
  if (hasAnchor(name))
    qDebug() << Q_FUNC_INFO << "anchor/position with name exists already:" << name;
  QCPItemPosition *newPosition = new QCPItemPosition(mParentPlot, this, name);
  mPositions.append(newPosition);
  mAnchors.append(newPosition); // every position is also an anchor
  newPosition->setAxes(mParentPlot->xAxis, mParentPlot->yAxis);
  newPosition->setType(QCPItemPosition::ptPlotCoords);
  if (mParentPlot->axisRect())
    newPosition->setAxisRect(mParentPlot->axisRect());
  newPosition->setCoords(0, 0);
  return newPosition;
}

/*! \internal

  Creates a QCPItemAnchor, registers it with this item and returns a pointer to it. The specified
  \a name must be a unique string that is usually identical to the variable name of the anchor
  member (This is needed to provide the name based \ref anchor access to anchors).
  
  The \a anchorId must be a number identifying the created anchor. It is recommended to create an
  enum (e.g. "AnchorIndex") for this on each item that uses anchors. This id is used by the anchor
  to identify itself when it calls QCPAbstractItem::anchorPixelPoint. That function then returns
  the correct pixel coordinates for the passed anchor id.
  
  Don't delete anchors created by this function manually, as the item will take care of it.
  
  Use this function in the constructor (initialization list) of the specific item subclass to
  create each anchor member. Don't create QCPItemAnchors with \b new yourself, because then they
  won't be registered with the item properly.
  
  \see createPosition
*/
QCPItemAnchor *QCPAbstractItem::createAnchor(const QString &name, int anchorId)
{
  if (hasAnchor(name))
    qDebug() << Q_FUNC_INFO << "anchor/position with name exists already:" << name;
  QCPItemAnchor *newAnchor = new QCPItemAnchor(mParentPlot, this, name, anchorId);
  mAnchors.append(newAnchor);
  return newAnchor;
}

/* inherits documentation from base class */
void QCPAbstractItem::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  Q_UNUSED(event)
  Q_UNUSED(details)
  if (mSelectable)
  {
    bool selBefore = mSelected;
    setSelected(additive ? !mSelected : true);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

/* inherits documentation from base class */
void QCPAbstractItem::deselectEvent(bool *selectionStateChanged)
{
  if (mSelectable)
  {
    bool selBefore = mSelected;
    setSelected(false);
    if (selectionStateChanged)
      *selectionStateChanged = mSelected != selBefore;
  }
}

/* inherits documentation from base class */
QCP::Interaction QCPAbstractItem::selectionCategory() const
{
  return QCP::iSelectItems;
}


/*! \file */



/*! \mainpage %QCustomPlot 1.1.0 Documentation

  \image html qcp-doc-logo.png
  
  Below is a brief overview of and guide to the classes and their relations. If you are new to
  QCustomPlot and just want to start using it, it's recommended to look at the tutorials and
  examples at
 
  http://www.qcustomplot.com/
 
  This documentation is especially helpful as a reference, when you're familiar with the basic
  concept of how to use %QCustomPlot and you wish to learn more about specific functionality.
  See the \ref classoverview "class overview" for diagrams explaining the relationships between
  the most important classes of the QCustomPlot library.
  
  The central widget which displays the plottables and axes on its surface is QCustomPlot. Every
  QCustomPlot contains four axes by default. They can be accessed via the members xAxis, yAxis,
  xAxis2 and yAxis2, and are of type QCPAxis. QCustomPlot supports an arbitrary number of axes and
  axis rects, see the documentation of QCPAxisRect for details.

  \section mainpage-plottables Plottables
  
  \a Plottables are classes that display any kind of data inside the QCustomPlot. They all derive
  from QCPAbstractPlottable. For example, the QCPGraph class is a plottable that displays a graph
  inside the plot with different line styles, scatter styles, filling etc.
  
  Since plotting graphs is such a dominant use case, QCustomPlot has a special interface for working
  with QCPGraph plottables, that makes it very easy to handle them:\n
  You create a new graph with QCustomPlot::addGraph and access them with QCustomPlot::graph.
  
  For all other plottables, you need to use the normal plottable interface:\n
  First, you create an instance of the plottable you want, e.g.
  \code
  QCPCurve *newCurve = new QCPCurve(customPlot->xAxis, customPlot->yAxis);\endcode
  add it to the customPlot:
  \code
  customPlot->addPlottable(newCurve);\endcode
  and then modify the properties of the newly created plottable via the <tt>newCurve</tt> pointer.
  
  Plottables (including graphs) can be retrieved via QCustomPlot::plottable. Since the return type
  of that function is the abstract base class of all plottables, QCPAbstractPlottable, you will
  probably want to qobject_cast the returned pointer to the respective plottable subclass. (As
  usual, if the cast returns zero, the plottable wasn't of that specific subclass.)
  
  All further interfacing with plottables (e.g how to set data) is specific to the plottable type.
  See the documentations of the subclasses: QCPGraph, QCPCurve, QCPBars, QCPStatisticalBox.

  \section mainpage-axes Controlling the Axes
  
  As mentioned, QCustomPlot has four axes by default: \a xAxis (bottom), \a yAxis (left), \a xAxis2
  (top), \a yAxis2 (right).
  
  Their range is handled by the simple QCPRange class. You can set the range with the
  QCPAxis::setRange function. By default, the axes represent a linear scale. To set a logarithmic
  scale, set \ref QCPAxis::setScaleType to \ref QCPAxis::stLogarithmic. The logarithm base can be set freely
  with \ref QCPAxis::setScaleLogBase.
  
  By default, an axis automatically creates and labels ticks in a sensible manner. See the
  following functions for tick manipulation:\n QCPAxis::setTicks, QCPAxis::setAutoTicks,
  QCPAxis::setAutoTickCount, QCPAxis::setAutoTickStep, QCPAxis::setTickLabels,
  QCPAxis::setTickLabelType, QCPAxis::setTickLabelRotation, QCPAxis::setTickStep,
  QCPAxis::setTickLength,...
  
  Each axis can be given an axis label (e.g. "Voltage (mV)") with QCPAxis::setLabel.
  
  The distance of an axis backbone to the respective viewport border is called its margin.
  Normally, the margins are calculated automatically. To change this, set
  \ref QCPAxisRect::setAutoMargins to exclude the respective margin sides, set the margins manually with
  \ref QCPAxisRect::setMargins. The main axis rect can be reached with \ref QCustomPlot::axisRect().
  
  \section mainpage-legend Plot Legend
  
  Every QCustomPlot owns one QCPLegend (as \a legend) by default. A legend is a small layout
  element inside the plot which lists the plottables with an icon of the plottable line/symbol and
  a description. The Description is retrieved from the plottable name
  (QCPAbstractPlottable::setName). Plottables can be added and removed from the legend via \ref
  QCPAbstractPlottable::addToLegend and \ref QCPAbstractPlottable::removeFromLegend. By default,
  adding a plottable to QCustomPlot automatically adds it to the legend, too. This behaviour can be
  modified with the QCustomPlot::setAutoAddPlottableToLegend property.
  
  The QCPLegend provides an interface to access, add and remove legend items directly, too. See
  QCPLegend::item, QCPLegend::itemWithPlottable, QCPLegend::addItem, QCPLegend::removeItem for
  example.
  
  Multiple legends are supported via the layout system (as a QCPLegend simply is a normal layout
  element).
  
  \section mainpage-userinteraction User Interactions
  
  QCustomPlot supports dragging axis ranges with the mouse (\ref
  QCPAxisRect::setRangeDrag), zooming axis ranges with the mouse wheel (\ref
  QCPAxisRect::setRangeZoom) and a complete selection mechanism.
  
  The availability of these interactions is controlled with \ref QCustomPlot::setInteractions. For
  details about the interaction system, see the documentation there.
  
  Further, QCustomPlot always emits corresponding signals, when objects are clicked or
  doubleClicked. See \ref QCustomPlot::plottableClick, \ref QCustomPlot::plottableDoubleClick
  and \ref QCustomPlot::axisClick for example.
  
  \section mainpage-items Items
  
  Apart from plottables there is another category of plot objects that are important: Items. The
  base class of all items is QCPAbstractItem. An item sets itself apart from plottables in that
  it's not necessarily bound to any axes. This means it may also be positioned in absolute pixel
  coordinates or placed at a relative position on an axis rect. Further, it usually doesn't
  represent data directly, but acts as decoration, emphasis, description etc.
  
  Multiple items can be arranged in a parent-child-hierarchy allowing for dynamical behaviour. For
  example, you could place the head of an arrow at a fixed plot coordinate, so it always points to
  some important area in the plot. The tail of the arrow can be anchored to a text item which
  always resides in the top center of the axis rect, independent of where the user drags the axis
  ranges. This way the arrow stretches and turns so it always points from the label to the
  specified plot coordinate, without any further code necessary.
  
  For a more detailed introduction, see the QCPAbstractItem documentation, and from there the
  documentations of the individual built-in items, to find out how to use them.
  
  \section mainpage-layoutelements Layout elements and layouts
  
  QCustomPlot uses an internal layout system to provide dynamic sizing and positioning of objects like
  the axis rect(s), legends and the plot title. They are all based on \ref QCPLayoutElement and are arranged by
  placing them inside a \ref QCPLayout.
  
  Details on this topic are given on the dedicated page about \ref thelayoutsystem "the layout system".
  
  \section mainpage-performancetweaks Performance Tweaks
  
  Although QCustomPlot is quite fast, some features like translucent fills, antialiasing and thick
  lines can cause a significant slow down. If you notice this in your application, here are some
  thoughts on how to increase performance. By far the most time is spent in the drawing functions,
  specifically the drawing of graphs. For maximum performance, consider the following (most
  recommended/effective measures first):
  
  \li use Qt 4.8.0 and up. Performance has doubled or tripled with respect to Qt 4.7.4. However
  QPainter was broken and drawing pixel precise things, e.g. scatters, isn't possible with Qt >=
  4.8.0. So it's a performance vs. plot quality tradeoff when switching to Qt 4.8.
  \li To increase responsiveness during dragging, consider setting \ref QCustomPlot::setNoAntialiasingOnDrag to true.
  \li On X11 (GNU/Linux), avoid the slow native drawing system, use raster by supplying
  "-graphicssystem raster" as command line argument or calling QApplication::setGraphicsSystem("raster")
  before creating the QApplication object. (Only available for Qt versions before 5.0)
  \li On all operating systems, use OpenGL hardware acceleration by supplying "-graphicssystem
  opengl" as command line argument or calling QApplication::setGraphicsSystem("opengl") (Only
  available for Qt versions before 5.0). If OpenGL is available, this will slightly decrease the
  quality of antialiasing, but extremely increase performance especially with alpha
  (semi-transparent) fills, much antialiasing and a large QCustomPlot drawing surface. Note
  however, that the maximum frame rate might be constrained by the vertical sync frequency of your
  monitor (VSync can be disabled in the graphics card driver configuration). So for simple plots
  (where the potential framerate is far above 60 frames per second), OpenGL acceleration might
  achieve numerically lower frame rates than the other graphics systems, because they are not
  capped at the VSync frequency.
  \li Avoid any kind of alpha (transparency), especially in fills
  \li Avoid lines with a pen width greater than one
  \li Avoid any kind of antialiasing, especially in graph lines (see \ref QCustomPlot::setNotAntialiasedElements)
  \li Avoid repeatedly setting the complete data set with \ref QCPGraph::setData. Use \ref QCPGraph::addData instead, if most
  data points stay unchanged, e.g. in a running measurement.
  \li Set the \a copy parameter of the setData functions to false, so only pointers get
  transferred. (Relevant only if preparing data maps with a large number of points, i.e. over 10000)
  
  \section mainpage-flags Preprocessor Define Flags
  
  QCustomPlot understands some preprocessor defines that are useful for debugging and compilation:
  <dl>
  <dt>\c QCUSTOMPLOT_COMPILE_LIBRARY
  <dd>Define this flag when you compile QCustomPlot as a shared library (.so/.dll)
  <dt>\c QCUSTOMPLOT_USE_LIBRARY
  <dd>Define this flag before including the header, when using QCustomPlot as a shared library
  <dt>\c QCUSTOMPLOT_CHECK_DATA
  <dd>If this flag is defined, the QCustomPlot plottables will perform data validity checks on every redraw.
      This means they will give qDebug output when you plot \e inf or \e nan values, they will not
      fix your data.
  </dl>

*/

/*! \page classoverview Class Overview
  
  The following diagrams may help to gain a deeper understanding of the relationships between classes that make up
  the QCustomPlot library. The diagrams are not exhaustive, so only the classes deemed most relevant are shown.
  
  \section classoverview-relations Class Relationship Diagram
  \image html RelationOverview.png "Overview of most important classes and their relations"
  \section classoverview-inheritance Class Inheritance Tree
  \image html InheritanceOverview.png "Inheritance tree of most important classes"
  
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// QCustomPlot
////////////////////////////////////////////////////////////////////////////////////////////////////

/*! \class QCustomPlot
  
  \brief The central class of the library. This is the QWidget which displays the plot and
  interacts with the user.
  
  For tutorials on how to use QCustomPlot, see the website\n
  http://www.qcustomplot.com/
*/

/* start of documentation of inline functions */

/*! \fn QRect QCustomPlot::viewport() const
  
  Returns the viewport rect of this QCustomPlot instance. The viewport is the area the plot is
  drawn in, all mechanisms, e.g. margin caluclation take the viewport to be the outer border of the
  plot. The viewport normally is the rect() of the QCustomPlot widget, i.e. a rect with top left
  (0, 0) and size of the QCustomPlot widget.
  
  Don't confuse the viewport with the axis rect (QCustomPlot::axisRect). An axis rect is typically
  an area enclosed by four axes, where the graphs/plottables are drawn in. The viewport is larger
  and contains also the axes themselves, their tick numbers, their labels, the plot title etc.
  
  Only when saving to a file (see \ref savePng, savePdf etc.) the viewport is temporarily modified
  to allow saving plots with sizes independent of the current widget size.
*/

/*! \fn QCPLayoutGrid *QCustomPlot::plotLayout() const
  
  Returns the top level layout of this QCustomPlot instance. It is a \ref QCPLayoutGrid, initially containing just
  one cell with the main QCPAxisRect inside.
*/

/* end of documentation of inline functions */
/* start of documentation of signals */

/*! \fn void QCustomPlot::mouseDoubleClick(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse double click event.
*/

/*! \fn void QCustomPlot::mousePress(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse press event.
  
  It is emitted before QCustomPlot handles any other mechanism like range dragging. So a slot
  connected to this signal can still influence the behaviour e.g. with \ref QCPAxisRect::setRangeDrag or \ref
  QCPAxisRect::setRangeDragAxes.
*/

/*! \fn void QCustomPlot::mouseMove(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse move event.
  
  It is emitted before QCustomPlot handles any other mechanism like range dragging. So a slot
  connected to this signal can still influence the behaviour e.g. with \ref QCPAxisRect::setRangeDrag or \ref
  QCPAxisRect::setRangeDragAxes.
  
  \warning It is discouraged to change the drag-axes with \ref QCPAxisRect::setRangeDragAxes here,
  because the dragging starting point was saved the moment the mouse was pressed. Thus it only has
  a meaning for the range drag axes that were set at that moment. If you want to change the drag
  axes, consider doing this in the \ref mousePress signal instead.
*/

/*! \fn void QCustomPlot::mouseRelease(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse release event.
  
  It is emitted before QCustomPlot handles any other mechanisms like object selection. So a
  slot connected to this signal can still influence the behaviour e.g. with \ref setInteractions or
  \ref QCPAbstractPlottable::setSelectable.
*/

/*! \fn void QCustomPlot::mouseWheel(QMouseEvent *event)

  This signal is emitted when the QCustomPlot receives a mouse wheel event.
  
  It is emitted before QCustomPlot handles any other mechanisms like range zooming. So a slot
  connected to this signal can still influence the behaviour e.g. with \ref QCPAxisRect::setRangeZoom, \ref
  QCPAxisRect::setRangeZoomAxes or \ref QCPAxisRect::setRangeZoomFactor.
*/

/*! \fn void QCustomPlot::plottableClick(QCPAbstractPlottable *plottable, QMouseEvent *event)
  
  This signal is emitted when a plottable is clicked.

  \a event is the mouse event that caused the click and \a plottable is the plottable that received
  the click.
  
  \see plottableDoubleClick
*/

/*! \fn void QCustomPlot::plottableDoubleClick(QCPAbstractPlottable *plottable, QMouseEvent *event)
  
  This signal is emitted when a plottable is double clicked.
  
  \a event is the mouse event that caused the click and \a plottable is the plottable that received
  the click.
  
  \see plottableClick
*/

/*! \fn void QCustomPlot::itemClick(QCPAbstractItem *item, QMouseEvent *event)
  
  This signal is emitted when an item is clicked.

  \a event is the mouse event that caused the click and \a item is the item that received the
  click.
  
  \see itemDoubleClick
*/

/*! \fn void QCustomPlot::itemDoubleClick(QCPAbstractItem *item, QMouseEvent *event)
  
  This signal is emitted when an item is double clicked.
  
  \a event is the mouse event that caused the click and \a item is the item that received the
  click.
  
  \see itemClick
*/

/*! \fn void QCustomPlot::axisClick(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event)
  
  This signal is emitted when an axis is clicked.
  
  \a event is the mouse event that caused the click, \a axis is the axis that received the click and
  \a part indicates the part of the axis that was clicked.
  
  \see axisDoubleClick
*/

/*! \fn void QCustomPlot::axisDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part, QMouseEvent *event)

  This signal is emitted when an axis is double clicked.
  
  \a event is the mouse event that caused the click, \a axis is the axis that received the click and
  \a part indicates the part of the axis that was clicked.
  
  \see axisClick
*/

/*! \fn void QCustomPlot::legendClick(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)

  This signal is emitted when a legend (item) is clicked.
  
  \a event is the mouse event that caused the click, \a legend is the legend that received the
  click and \a item is the legend item that received the click. If only the legend and no item is
  clicked, \a item is 0. This happens for a click inside the legend padding or the space between
  two items.
  
  \see legendDoubleClick
*/

/*! \fn void QCustomPlot::legendDoubleClick(QCPLegend *legend,  QCPAbstractLegendItem *item, QMouseEvent *event)

  This signal is emitted when a legend (item) is double clicked.
  
  \a event is the mouse event that caused the click, \a legend is the legend that received the
  click and \a item is the legend item that received the click. If only the legend and no item is
  clicked, \a item is 0. This happens for a click inside the legend padding or the space between
  two items.
  
  \see legendClick
*/

/*! \fn void QCustomPlot:: titleClick(QMouseEvent *event, QCPPlotTitle *title)

  This signal is emitted when a plot title is clicked.
  
  \a event is the mouse event that caused the click and \a title is the plot title that received
  the click.
  
  \see titleDoubleClick
*/