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

#ifndef QCUSTOMPLOT_H
#define QCUSTOMPLOT_H

#include <QObject>
#include <QPointer>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QVector>
#include <QString>
#include <QDateTime>
#include <QMultiMap>
#include <QFlags>
#include <QDebug>
#include <QVector2D>
#include <QStack>
#include <QCache>
#include <QMargins>
#include <qmath.h>
#include <limits>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#  include <qnumeric.h>
#  include <QPrinter>
#else
#  include <QtNumeric>
#  include <QtPrintSupport>
#endif

class QCPPainter;
class QCustomPlot;
class QCPLayerable;
class QCPLayoutElement;
class QCPLayout;
class QCPAxis;
class QCPAxisRect;
class QCPAbstractPlottable;
class QCPGraph;
class QCPAbstractItem;
class QCPItemPosition;
class QCPLayer;
class QCPPlotTitle;
class QCPLegend;
class QCPAbstractLegendItem;


/*! \file */


// decl definitions for shared library compilation/usage:
#if defined(QCUSTOMPLOT_COMPILE_LIBRARY)
#  define QCP_LIB_DECL Q_DECL_EXPORT
#elif defined(QCUSTOMPLOT_USE_LIBRARY)
#  define QCP_LIB_DECL Q_DECL_IMPORT
#else
#  define QCP_LIB_DECL
#endif

/*!
  The QCP Namespace contains general enums and QFlags used throughout the QCustomPlot library
*/
namespace QCP
{
/*!
  Defines the sides of a rectangular entity to which margins can be applied.
  
  \see QCPLayoutElement::setAutoMargins, QCPAxisRect::setAutoMargins
*/
enum MarginSide { msLeft     = 0x01 ///< <tt>0x01</tt> left margin
                  ,msRight   = 0x02 ///< <tt>0x02</tt> right margin
                  ,msTop     = 0x04 ///< <tt>0x04</tt> top margin
                  ,msBottom  = 0x08 ///< <tt>0x08</tt> bottom margin
                  ,msAll     = 0xFF ///< <tt>0xFF</tt> all margins
                  ,msNone    = 0x00 ///< <tt>0x00</tt> no margin
                };
Q_DECLARE_FLAGS(MarginSides, MarginSide)

/*!
  Defines what objects of a plot can be forcibly drawn antialiased/not antialiased. If an object is
  neither forcibly drawn antialiased nor forcibly drawn not antialiased, it is up to the respective
  element how it is drawn. Typically it provides a \a setAntialiased function for this.
  
  \c AntialiasedElements is a flag of or-combined elements of this enum type.
  
  \see QCustomPlot::setAntialiasedElements, QCustomPlot::setNotAntialiasedElements
*/
enum AntialiasedElement { aeAxes           = 0x0001 ///< <tt>0x0001</tt> Axis base line and tick marks
                          ,aeGrid          = 0x0002 ///< <tt>0x0002</tt> Grid lines
                          ,aeSubGrid       = 0x0004 ///< <tt>0x0004</tt> Sub grid lines
                          ,aeLegend        = 0x0008 ///< <tt>0x0008</tt> Legend box
                          ,aeLegendItems   = 0x0010 ///< <tt>0x0010</tt> Legend items
                          ,aePlottables    = 0x0020 ///< <tt>0x0020</tt> Main lines of plottables (excluding error bars, see element \ref aeErrorBars)
                          ,aeItems         = 0x0040 ///< <tt>0x0040</tt> Main lines of items
                          ,aeScatters      = 0x0080 ///< <tt>0x0080</tt> Scatter symbols of plottables (excluding scatter symbols of type ssPixmap)
                          ,aeErrorBars     = 0x0100 ///< <tt>0x0100</tt> Error bars
                          ,aeFills         = 0x0200 ///< <tt>0x0200</tt> Borders of fills (e.g. under or between graphs)
                          ,aeZeroLine      = 0x0400 ///< <tt>0x0400</tt> Zero-lines, see \ref QCPGrid::setZeroLinePen
                          ,aeAll           = 0xFFFF ///< <tt>0xFFFF</tt> All elements
                          ,aeNone          = 0x0000 ///< <tt>0x0000</tt> No elements
                        };
Q_DECLARE_FLAGS(AntialiasedElements, AntialiasedElement)

/*!
  Defines plotting hints that control various aspects of the quality and speed of plotting.
  
  \see QCustomPlot::setPlottingHints
*/
enum PlottingHint { phNone            = 0x000 ///< <tt>0x000</tt> No hints are set
                    ,phFastPolylines  = 0x001 ///< <tt>0x001</tt> Graph/Curve lines are drawn with a faster method. This reduces the quality
                                              ///<                especially of the line segment joins. (Only relevant for solid line pens.)
                    ,phForceRepaint   = 0x002 ///< <tt>0x002</tt> causes an immediate repaint() instead of a soft update() when QCustomPlot::replot() is called. This is set by default
                                              ///<                on Windows-Systems to prevent the plot from freezing on fast consecutive replots (e.g. user drags ranges with mouse).
                    ,phCacheLabels    = 0x004 ///< <tt>0x004</tt> axis (tick) labels will be cached as pixmaps, increasing replot performance.
                  };
Q_DECLARE_FLAGS(PlottingHints, PlottingHint)

/*!
  Defines the mouse interactions possible with QCustomPlot.
  
  \c Interactions is a flag of or-combined elements of this enum type.
  
  \see QCustomPlot::setInteractions
*/
enum Interaction { iRangeDrag         = 0x001 ///< <tt>0x001</tt> Axis ranges are draggable (see \ref QCPAxisRect::setRangeDrag, \ref QCPAxisRect::setRangeDragAxes)
                   ,iRangeZoom        = 0x002 ///< <tt>0x002</tt> Axis ranges are zoomable with the mouse wheel (see \ref QCPAxisRect::setRangeZoom, \ref QCPAxisRect::setRangeZoomAxes)
                   ,iMultiSelect      = 0x004 ///< <tt>0x004</tt> The user can select multiple objects by holding the modifier set by \ref QCustomPlot::setMultiSelectModifier while clicking
                   ,iSelectPlottables = 0x008 ///< <tt>0x008</tt> Plottables are selectable (e.g. graphs, curves, bars,... see QCPAbstractPlottable)
                   ,iSelectAxes       = 0x010 ///< <tt>0x010</tt> Axes are selectable (or parts of them, see QCPAxis::setSelectableParts)
                   ,iSelectLegend     = 0x020 ///< <tt>0x020</tt> Legends are selectable (or their child items, see QCPLegend::setSelectableParts)
                   ,iSelectItems      = 0x040 ///< <tt>0x040</tt> Items are selectable (Rectangles, Arrows, Textitems, etc. see \ref QCPAbstractItem)
                   ,iSelectOther      = 0x080 ///< <tt>0x080</tt> All other objects are selectable (e.g. your own derived layerables, the plot title,...)
                 };
Q_DECLARE_FLAGS(Interactions, Interaction)

/*! \internal
  
  Returns whether the specified \a value is considered an invalid data value for plottables (i.e.
  is \e nan or \e +/-inf). This function is used to check data validity upon replots, when the
  compiler flag \c QCUSTOMPLOT_CHECK_DATA is set.
*/
inline bool isInvalidData(double value)
{
  return qIsNaN(value) || qIsInf(value);
}

/*! \internal
  \overload
  
  Checks two arguments instead of one.
*/
inline bool isInvalidData(double value1, double value2)
{
  return isInvalidData(value1) || isInvalidData(value2);
}

/*! \internal
  
  Sets the specified \a side of \a margins to \a value
  
  \see getMarginValue
*/
inline void setMarginValue(QMargins &margins, QCP::MarginSide side, int value)
{
  switch (side)
  {
    case QCP::msLeft: margins.setLeft(value); break;
    case QCP::msRight: margins.setRight(value); break;
    case QCP::msTop: margins.setTop(value); break;
    case QCP::msBottom: margins.setBottom(value); break;
    case QCP::msAll: margins = QMargins(value, value, value, value); break;
    default: break;
  }
}

/*! \internal
  
  Returns the value of the specified \a side of \a margins. If \a side is \ref QCP::msNone or
  \ref QCP::msAll, returns 0.
  
  \see setMarginValue
*/
inline int getMarginValue(const QMargins &margins, QCP::MarginSide side)
{
  switch (side)
  {
    case QCP::msLeft: return margins.left();
    case QCP::msRight: return margins.right();
    case QCP::msTop: return margins.top();
    case QCP::msBottom: return margins.bottom();
    default: break;
  }
  return 0;
}

} // end of namespace QCP

Q_DECLARE_OPERATORS_FOR_FLAGS(QCP::AntialiasedElements)
Q_DECLARE_OPERATORS_FOR_FLAGS(QCP::PlottingHints)
Q_DECLARE_OPERATORS_FOR_FLAGS(QCP::MarginSides)
Q_DECLARE_OPERATORS_FOR_FLAGS(QCP::Interactions)


class QCP_LIB_DECL QCPScatterStyle
{
  Q_GADGET
public:
  /*!
    Defines the shape used for scatter points.

    On plottables/items that draw scatters, the sizes of these visualizations (with exception of
    \ref ssDot and \ref ssPixmap) can be controlled with the \ref setSize function. Scatters are
    drawn with the pen and brush specified with \ref setPen and \ref setBrush.
  */
  Q_ENUMS(ScatterShape)
  enum ScatterShape { ssNone       ///< no scatter symbols are drawn (e.g. in QCPGraph, data only represented with lines)
                      ,ssDot       ///< \enumimage{ssDot.png} a single pixel (use \ref ssDisc or \ref ssCircle if you want a round shape with a certain radius)
                      ,ssCross     ///< \enumimage{ssCross.png} a cross
                      ,ssPlus      ///< \enumimage{ssPlus.png} a plus
                      ,ssCircle    ///< \enumimage{ssCircle.png} a circle
                      ,ssDisc      ///< \enumimage{ssDisc.png} a circle which is filled with the pen's color (not the brush as with ssCircle)
                      ,ssSquare    ///< \enumimage{ssSquare.png} a square
                      ,ssDiamond   ///< \enumimage{ssDiamond.png} a diamond
                      ,ssStar      ///< \enumimage{ssStar.png} a star with eight arms, i.e. a combination of cross and plus
                      ,ssTriangle  ///< \enumimage{ssTriangle.png} an equilateral triangle, standing on baseline
                      ,ssTriangleInverted ///< \enumimage{ssTriangleInverted.png} an equilateral triangle, standing on corner
                      ,ssCrossSquare      ///< \enumimage{ssCrossSquare.png} a square with a cross inside
                      ,ssPlusSquare       ///< \enumimage{ssPlusSquare.png} a square with a plus inside
                      ,ssCrossCircle      ///< \enumimage{ssCrossCircle.png} a circle with a cross inside
                      ,ssPlusCircle       ///< \enumimage{ssPlusCircle.png} a circle with a plus inside
                      ,ssPeace     ///< \enumimage{ssPeace.png} a circle, with one vertical and two downward diagonal lines
                      ,ssPixmap    ///< a custom pixmap specified by \ref setPixmap, centered on the data point coordinates
                      ,ssCustom    ///< custom painter operations are performed per scatter (As QPainterPath, see \ref setCustomPath)
                    };

  QCPScatterStyle();
  QCPScatterStyle(ScatterShape shape, double size=6);
  QCPScatterStyle(ScatterShape shape, const QColor &color, double size);
  QCPScatterStyle(ScatterShape shape, const QColor &color, const QColor &fill, double size);
  QCPScatterStyle(ScatterShape shape, const QPen &pen, const QBrush &brush, double size);
  QCPScatterStyle(const QPixmap &pixmap);
  QCPScatterStyle(const QPainterPath &customPath, const QPen &pen, const QBrush &brush=Qt::NoBrush, double size=6);
  
  // getters:
  double size() const { return mSize; }
  ScatterShape shape() const { return mShape; }
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  QPixmap pixmap() const { return mPixmap; }
  QPainterPath customPath() const { return mCustomPath; }

  // setters:
  void setSize(double size);
  void setShape(ScatterShape shape);
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setPixmap(const QPixmap &pixmap);
  void setCustomPath(const QPainterPath &customPath);

  // non-property methods:
  bool isNone() const { return mShape == ssNone; }
  bool isPenDefined() const { return mPenDefined; }
  void applyTo(QCPPainter *painter, const QPen &defaultPen) const;
  void drawShape(QCPPainter *painter, QPointF pos) const;
  void drawShape(QCPPainter *painter, double x, double y) const;

protected:
  // property members:
  double mSize;
  ScatterShape mShape;
  QPen mPen;
  QBrush mBrush;
  QPixmap mPixmap;
  QPainterPath mCustomPath;
  
  // non-property members:
  bool mPenDefined;
};
Q_DECLARE_TYPEINFO(QCPScatterStyle, Q_MOVABLE_TYPE);


class QCP_LIB_DECL QCPPainter : public QPainter
{
  Q_GADGET
public:
  /*!
    Defines special modes the painter can operate in. They disable or enable certain subsets of features/fixes/workarounds,
    depending on whether they are wanted on the respective output device.
  */
  enum PainterMode {pmDefault       = 0x00   ///< <tt>0x00</tt> Default mode for painting on screen devices
                    ,pmVectorized   = 0x01   ///< <tt>0x01</tt> Mode for vectorized painting (e.g. PDF export). For example, this prevents some antialiasing fixes.
                    ,pmNoCaching    = 0x02   ///< <tt>0x02</tt> Mode for all sorts of exports (e.g. PNG, PDF,...). For example, this prevents using cached pixmap labels
                    ,pmNonCosmetic  = 0x04   ///< <tt>0x04</tt> Turns pen widths 0 to 1, i.e. disables cosmetic pens. (A cosmetic pen is always drawn with width 1 pixel in the vector image/pdf viewer, independent of zoom.)
                   };
  Q_FLAGS(PainterMode PainterModes)
  Q_DECLARE_FLAGS(PainterModes, PainterMode)
  
  QCPPainter();
  QCPPainter(QPaintDevice *device);
  ~QCPPainter();
  
  // getters:
  bool antialiasing() const { return testRenderHint(QPainter::Antialiasing); }
  PainterModes modes() const { return mModes; }

  // setters:
  void setAntialiasing(bool enabled);
  void setMode(PainterMode mode, bool enabled=true);
  void setModes(PainterModes modes);

  // methods hiding non-virtual base class functions (QPainter bug workarounds):
  bool begin(QPaintDevice *device);
  void setPen(const QPen &pen);
  void setPen(const QColor &color);
  void setPen(Qt::PenStyle penStyle);
  void drawLine(const QLineF &line);
  void drawLine(const QPointF &p1, const QPointF &p2) {drawLine(QLineF(p1, p2));}
  void save();
  void restore();
  
  // non-virtual methods:
  void makeNonCosmetic();
  
protected:
  // property members:
  PainterModes mModes;
  bool mIsAntialiasing;
  
  // non-property members:
  QStack<bool> mAntialiasingStack;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QCPPainter::PainterModes)


class QCP_LIB_DECL QCPLayer : public QObject
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QCustomPlot* parentPlot READ parentPlot)
  Q_PROPERTY(QString name READ name)
  Q_PROPERTY(int index READ index)
  Q_PROPERTY(QList<QCPLayerable*> children READ children)
  /// \endcond
public:
  QCPLayer(QCustomPlot* parentPlot, const QString &layerName);
  ~QCPLayer();
  
  // getters:
  QCustomPlot *parentPlot() const { return mParentPlot; }
  QString name() const { return mName; }
  int index() const { return mIndex; }
  QList<QCPLayerable*> children() const { return mChildren; }
  
protected:
  // property members:
  QCustomPlot *mParentPlot;
  QString mName;
  int mIndex;
  QList<QCPLayerable*> mChildren;
  
  // non-virtual methods:
  void addChild(QCPLayerable *layerable, bool prepend);
  void removeChild(QCPLayerable *layerable);
  
private:
  Q_DISABLE_COPY(QCPLayer)
  
  friend class QCustomPlot;
  friend class QCPLayerable;
};

class QCP_LIB_DECL QCPLayerable : public QObject
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(bool visible READ visible WRITE setVisible)
  Q_PROPERTY(QCustomPlot* parentPlot READ parentPlot)
  Q_PROPERTY(QCPLayerable* parentLayerable READ parentLayerable)
  Q_PROPERTY(QCPLayer* layer READ layer WRITE setLayer)
  Q_PROPERTY(bool antialiased READ antialiased WRITE setAntialiased)
  /// \endcond
public:
  QCPLayerable(QCustomPlot *plot, QString targetLayer="", QCPLayerable *parentLayerable=0);
  ~QCPLayerable();
  
  // getters:
  bool visible() const { return mVisible; }
  QCustomPlot *parentPlot() const { return mParentPlot; }
  QCPLayerable *parentLayerable() const { return mParentLayerable.data(); }
  QCPLayer *layer() const { return mLayer; }
  bool antialiased() const { return mAntialiased; }
  
  // setters:
  void setVisible(bool on);
  bool setLayer(QCPLayer *layer);
  bool setLayer(const QString &layerName);
  void setAntialiased(bool enabled);
  
  // introduced virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const;
  
  // non-property methods:
  bool realVisibility() const;
  
protected:
  // property members:
  bool mVisible;
  QCustomPlot *mParentPlot;
  QPointer<QCPLayerable> mParentLayerable;
  QCPLayer *mLayer;
  bool mAntialiased;
  
  // introduced virtual methods:
  virtual void parentPlotInitialized(QCustomPlot *parentPlot);
  virtual QCP::Interaction selectionCategory() const;
  virtual QRect clipRect() const;
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const = 0;
  virtual void draw(QCPPainter *painter) = 0;
  // events:
  virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged);
  virtual void deselectEvent(bool *selectionStateChanged);
  
  // non-property methods:
  void initializeParentPlot(QCustomPlot *parentPlot);
  void setParentLayerable(QCPLayerable* parentLayerable);
  bool moveToLayer(QCPLayer *layer, bool prepend);
  void applyAntialiasingHint(QCPPainter *painter, bool localAntialiased, QCP::AntialiasedElement overrideElement) const;
  
private:
  Q_DISABLE_COPY(QCPLayerable)
  
  friend class QCustomPlot;
  friend class QCPAxisRect;
};


class QCP_LIB_DECL QCPRange
{
public:
  double lower, upper;
  
  QCPRange();
  QCPRange(double lower, double upper);
  
  double size() const;
  double center() const;
  void normalize();
  void expand(const QCPRange &otherRange);
  QCPRange expanded(const QCPRange &otherRange) const;
  QCPRange sanitizedForLogScale() const;
  QCPRange sanitizedForLinScale() const;
  bool contains(double value) const;
  
  static bool validRange(double lower, double upper);
  static bool validRange(const QCPRange &range);
  static const double minRange; //1e-280;
  static const double maxRange; //1e280;
};
Q_DECLARE_TYPEINFO(QCPRange, Q_MOVABLE_TYPE);


class QCP_LIB_DECL QCPMarginGroup : public QObject
{
  Q_OBJECT
public:
  QCPMarginGroup(QCustomPlot *parentPlot);
  ~QCPMarginGroup();
  
  // non-virtual methods:
  QList<QCPLayoutElement*> elements(QCP::MarginSide side) const { return mChildren.value(side); }
  bool isEmpty() const;
  void clear();
  
protected:
  // non-property members:
  QCustomPlot *mParentPlot;
  QHash<QCP::MarginSide, QList<QCPLayoutElement*> > mChildren;
  
  // non-virtual methods:
  int commonMargin(QCP::MarginSide side) const;
  void addChild(QCP::MarginSide side, QCPLayoutElement *element);
  void removeChild(QCP::MarginSide side, QCPLayoutElement *element);
  
private:
  Q_DISABLE_COPY(QCPMarginGroup)
  
  friend class QCPLayoutElement;
};


class QCP_LIB_DECL QCPLayoutElement : public QCPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QCPLayout* layout READ layout)
  Q_PROPERTY(QRect rect READ rect)
  Q_PROPERTY(QRect outerRect READ outerRect WRITE setOuterRect)
  Q_PROPERTY(QMargins margins READ margins WRITE setMargins)
  Q_PROPERTY(QMargins minimumMargins READ minimumMargins WRITE setMinimumMargins)
  Q_PROPERTY(QSize minimumSize READ minimumSize WRITE setMinimumSize)
  Q_PROPERTY(QSize maximumSize READ maximumSize WRITE setMaximumSize)
  /// \endcond
public:
  explicit QCPLayoutElement(QCustomPlot *parentPlot=0);
  virtual ~QCPLayoutElement();
  
  // getters:
  QCPLayout *layout() const { return mParentLayout; }
  QRect rect() const { return mRect; }
  QRect outerRect() const { return mOuterRect; }
  QMargins margins() const { return mMargins; }
  QMargins minimumMargins() const { return mMinimumMargins; }
  QCP::MarginSides autoMargins() const { return mAutoMargins; }
  QSize minimumSize() const { return mMinimumSize; }
  QSize maximumSize() const { return mMaximumSize; }
  QCPMarginGroup *marginGroup(QCP::MarginSide side) const { return mMarginGroups.value(side, (QCPMarginGroup*)0); }
  QHash<QCP::MarginSide, QCPMarginGroup*> marginGroups() const { return mMarginGroups; }
  
  // setters:
  void setOuterRect(const QRect &rect);
  void setMargins(const QMargins &margins);
  void setMinimumMargins(const QMargins &margins);
  void setAutoMargins(QCP::MarginSides sides);
  void setMinimumSize(const QSize &size);
  void setMinimumSize(int width, int height);
  void setMaximumSize(const QSize &size);
  void setMaximumSize(int width, int height);
  void setMarginGroup(QCP::MarginSides sides, QCPMarginGroup *group);
  
  // introduced virtual methods:
  virtual void update();
  virtual QSize minimumSizeHint() const;
  virtual QSize maximumSizeHint() const;
  virtual QList<QCPLayoutElement*> elements(bool recursive) const;
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const;
  
protected:
  // property members:
  QCPLayout *mParentLayout;
  QSize mMinimumSize, mMaximumSize;
  QRect mRect, mOuterRect;
  QMargins mMargins, mMinimumMargins;
  QCP::MarginSides mAutoMargins;
  QHash<QCP::MarginSide, QCPMarginGroup*> mMarginGroups;
  
  // introduced virtual methods:
  virtual int calculateAutoMargin(QCP::MarginSide side);
  // events:
  virtual void mousePressEvent(QMouseEvent *event) {Q_UNUSED(event)}
  virtual void mouseMoveEvent(QMouseEvent *event) {Q_UNUSED(event)}
  virtual void mouseReleaseEvent(QMouseEvent *event) {Q_UNUSED(event)}
  virtual void mouseDoubleClickEvent(QMouseEvent *event) {Q_UNUSED(event)}
  virtual void wheelEvent(QWheelEvent *event) {Q_UNUSED(event)}
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const { Q_UNUSED(painter) }
  virtual void draw(QCPPainter *painter) { Q_UNUSED(painter) }
  virtual void parentPlotInitialized(QCustomPlot *parentPlot);

private:
  Q_DISABLE_COPY(QCPLayoutElement)
  
  friend class QCustomPlot;
  friend class QCPLayout;
  friend class QCPMarginGroup;
};


class QCP_LIB_DECL QCPLayout : public QCPLayoutElement
{
  Q_OBJECT
public:
  explicit QCPLayout();
  
  // reimplemented virtual methods:
  virtual void update();
  virtual QList<QCPLayoutElement*> elements(bool recursive) const;
  
  // introduced virtual methods:
  virtual int elementCount() const = 0;
  virtual QCPLayoutElement* elementAt(int index) const = 0;
  virtual QCPLayoutElement* takeAt(int index) = 0;
  virtual bool take(QCPLayoutElement* element) = 0;
  virtual void simplify();
  
  // non-virtual methods:
  bool removeAt(int index);
  bool remove(QCPLayoutElement* element);
  void clear();
  
protected:
  // introduced virtual methods:
  virtual void updateLayout();
  
  // non-virtual methods:
  void sizeConstraintsChanged() const;
  void adoptElement(QCPLayoutElement *el);
  void releaseElement(QCPLayoutElement *el);
  QVector<int> getSectionSizes(QVector<int> maxSizes, QVector<int> minSizes, QVector<double> stretchFactors, int totalSize) const;
  
private:
  Q_DISABLE_COPY(QCPLayout)
  friend class QCPLayoutElement;
};


class QCP_LIB_DECL QCPLayoutGrid : public QCPLayout
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(int rowCount READ rowCount)
  Q_PROPERTY(int columnCount READ columnCount)
  Q_PROPERTY(QList<double> columnStretchFactors READ columnStretchFactors WRITE setColumnStretchFactors)
  Q_PROPERTY(QList<double> rowStretchFactors READ rowStretchFactors WRITE setRowStretchFactors)
  Q_PROPERTY(int columnSpacing READ columnSpacing WRITE setColumnSpacing)
  Q_PROPERTY(int rowSpacing READ rowSpacing WRITE setRowSpacing)
  /// \endcond
public:
  explicit QCPLayoutGrid();
  virtual ~QCPLayoutGrid();
  
  // getters:
  int rowCount() const;
  int columnCount() const;
  QList<double> columnStretchFactors() const { return mColumnStretchFactors; }
  QList<double> rowStretchFactors() const { return mRowStretchFactors; }
  int columnSpacing() const { return mColumnSpacing; }
  int rowSpacing() const { return mRowSpacing; }
  
  // setters:
  void setColumnStretchFactor(int column, double factor);
  void setColumnStretchFactors(const QList<double> &factors);
  void setRowStretchFactor(int row, double factor);
  void setRowStretchFactors(const QList<double> &factors);
  void setColumnSpacing(int pixels);
  void setRowSpacing(int pixels);
  
  // reimplemented virtual methods:
  virtual void updateLayout();
  virtual int elementCount() const;
  virtual QCPLayoutElement* elementAt(int index) const;
  virtual QCPLayoutElement* takeAt(int index);
  virtual bool take(QCPLayoutElement* element);
  virtual QList<QCPLayoutElement*> elements(bool recursive) const;
  virtual void simplify();
  virtual QSize minimumSizeHint() const;
  virtual QSize maximumSizeHint() const;
  
  // non-virtual methods:
  QCPLayoutElement *element(int row, int column) const;
  bool addElement(int row, int column, QCPLayoutElement *element);
  bool hasElement(int row, int column);
  void expandTo(int newRowCount, int newColumnCount);
  void insertRow(int newIndex);
  void insertColumn(int newIndex);
  
protected:
  // property members:
  QList<QList<QCPLayoutElement*> > mElements;
  QList<double> mColumnStretchFactors;
  QList<double> mRowStretchFactors;
  int mColumnSpacing, mRowSpacing;
  
  // non-virtual methods:
  void getMinimumRowColSizes(QVector<int> *minColWidths, QVector<int> *minRowHeights) const;
  void getMaximumRowColSizes(QVector<int> *maxColWidths, QVector<int> *maxRowHeights) const;
  
private:
  Q_DISABLE_COPY(QCPLayoutGrid)
};


class QCP_LIB_DECL QCPLayoutInset : public QCPLayout
{
  Q_OBJECT
public:
  /*!
    Defines how the placement and sizing is handled for a certain element in a QCPLayoutInset.
  */
  enum InsetPlacement {ipFree            ///< The element may be positioned/sized arbitrarily, see \ref setInsetRect
                       ,ipBorderAligned  ///< The element is aligned to one of the layout sides, see \ref setInsetAlignment
                      };
  
  explicit QCPLayoutInset();
  virtual ~QCPLayoutInset();
  
  // getters:
  InsetPlacement insetPlacement(int index) const;
  Qt::Alignment insetAlignment(int index) const;
  QRectF insetRect(int index) const;
  
  // setters:
  void setInsetPlacement(int index, InsetPlacement placement);
  void setInsetAlignment(int index, Qt::Alignment alignment);
  void setInsetRect(int index, const QRectF &rect);
  
  // reimplemented virtual methods:
  virtual void updateLayout();
  virtual int elementCount() const;
  virtual QCPLayoutElement* elementAt(int index) const;
  virtual QCPLayoutElement* takeAt(int index);
  virtual bool take(QCPLayoutElement* element);
  virtual void simplify() {}
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const;
  
  // non-virtual methods:
  void addElement(QCPLayoutElement *element, Qt::Alignment alignment);
  void addElement(QCPLayoutElement *element, const QRectF &rect);
  
protected:
  // property members:
  QList<QCPLayoutElement*> mElements;
  QList<InsetPlacement> mInsetPlacement;
  QList<Qt::Alignment> mInsetAlignment;
  QList<QRectF> mInsetRect;
  
private:
  Q_DISABLE_COPY(QCPLayoutInset)
};


class QCP_LIB_DECL QCPLineEnding
{
  Q_GADGET
public:
  /*!
    Defines the type of ending decoration for line-like items, e.g. an arrow.
    
    \image html QCPLineEnding.png
    
    The width and length of these decorations can be controlled with the functions \ref setWidth
    and \ref setLength. Some decorations like \ref esDisc, \ref esSquare, \ref esDiamond and \ref esBar only
    support a width, the length property is ignored.
    
    \see QCPItemLine::setHead, QCPItemLine::setTail, QCPItemCurve::setHead, QCPItemCurve::setTail
  */
  Q_ENUMS(EndingStyle)
  enum EndingStyle { esNone          ///< No ending decoration
                     ,esFlatArrow    ///< A filled arrow head with a straight/flat back (a triangle)
                     ,esSpikeArrow   ///< A filled arrow head with an indented back
                     ,esLineArrow    ///< A non-filled arrow head with open back
                     ,esDisc         ///< A filled circle
                     ,esSquare       ///< A filled square
                     ,esDiamond      ///< A filled diamond (45° rotated square)
                     ,esBar          ///< A bar perpendicular to the line
                     ,esHalfBar      ///< A bar perpendicular to the line sticking out to one side
                     ,esSkewedBar    ///< A bar that is skewed (skew controllable via \ref setLength)
                   };
  
  QCPLineEnding();
  QCPLineEnding(EndingStyle style, double width=8, double length=10, bool inverted=false);
  
  // getters:
  EndingStyle style() const { return mStyle; }
  double width() const { return mWidth; }
  double length() const { return mLength; }
  bool inverted() const { return mInverted; }
  
  // setters:
  void setStyle(EndingStyle style);
  void setWidth(double width);
  void setLength(double length);
  void setInverted(bool inverted);
  
  // non-property methods:
  double boundingDistance() const;
  double realLength() const;
  void draw(QCPPainter *painter, const QVector2D &pos, const QVector2D &dir) const;
  void draw(QCPPainter *painter, const QVector2D &pos, double angle) const;
  
protected:
  // property members:
  EndingStyle mStyle;
  double mWidth, mLength;
  bool mInverted;
};
Q_DECLARE_TYPEINFO(QCPLineEnding, Q_MOVABLE_TYPE);


class QCP_LIB_DECL QCPGrid :public QCPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(bool subGridVisible READ subGridVisible WRITE setSubGridVisible)
  Q_PROPERTY(bool antialiasedSubGrid READ antialiasedSubGrid WRITE setAntialiasedSubGrid)
  Q_PROPERTY(bool antialiasedZeroLine READ antialiasedZeroLine WRITE setAntialiasedZeroLine)
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen subGridPen READ subGridPen WRITE setSubGridPen)
  Q_PROPERTY(QPen zeroLinePen READ zeroLinePen WRITE setZeroLinePen)
  /// \endcond
public:
  QCPGrid(QCPAxis *parentAxis);
  
  // getters:
  bool subGridVisible() const { return mSubGridVisible; }
  bool antialiasedSubGrid() const { return mAntialiasedSubGrid; }
  bool antialiasedZeroLine() const { return mAntialiasedZeroLine; }
  QPen pen() const { return mPen; }
  QPen subGridPen() const { return mSubGridPen; }
  QPen zeroLinePen() const { return mZeroLinePen; }
  
  // setters:
  void setSubGridVisible(bool visible);
  void setAntialiasedSubGrid(bool enabled);
  void setAntialiasedZeroLine(bool enabled);
  void setPen(const QPen &pen);
  void setSubGridPen(const QPen &pen);
  void setZeroLinePen(const QPen &pen);
  
protected:
  // property members:
  bool mSubGridVisible;
  bool mAntialiasedSubGrid, mAntialiasedZeroLine;
  QPen mPen, mSubGridPen, mZeroLinePen;
  // non-property members:
  QCPAxis *mParentAxis;
  
  // reimplemented virtual methods:
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const;
  virtual void draw(QCPPainter *painter);
  
  // non-virtual methods:
  void drawGridLines(QCPPainter *painter) const;
  void drawSubGridLines(QCPPainter *painter) const;
  
  friend class QCPAxis;
};


class QCP_LIB_DECL QCPAxis : public QCPLayerable
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(AxisType axisType READ axisType)
  Q_PROPERTY(QCPAxisRect* axisRect READ axisRect)
  Q_PROPERTY(ScaleType scaleType READ scaleType WRITE setScaleType)
  Q_PROPERTY(double scaleLogBase READ scaleLogBase WRITE setScaleLogBase)
  Q_PROPERTY(QCPRange range READ range WRITE setRange)
  Q_PROPERTY(bool rangeReversed READ rangeReversed WRITE setRangeReversed)
  Q_PROPERTY(bool autoTicks READ autoTicks WRITE setAutoTicks)
  Q_PROPERTY(int autoTickCount READ autoTickCount WRITE setAutoTickCount)
  Q_PROPERTY(bool autoTickLabels READ autoTickLabels WRITE setAutoTickLabels)
  Q_PROPERTY(bool autoTickStep READ autoTickStep WRITE setAutoTickStep)
  Q_PROPERTY(bool autoSubTicks READ autoSubTicks WRITE setAutoSubTicks)
  Q_PROPERTY(bool ticks READ ticks WRITE setTicks)
  Q_PROPERTY(bool tickLabels READ tickLabels WRITE setTickLabels)
  Q_PROPERTY(int tickLabelPadding READ tickLabelPadding WRITE setTickLabelPadding)
  Q_PROPERTY(LabelType tickLabelType READ tickLabelType WRITE setTickLabelType)
  Q_PROPERTY(QFont tickLabelFont READ tickLabelFont WRITE setTickLabelFont)
  Q_PROPERTY(QColor tickLabelColor READ tickLabelColor WRITE setTickLabelColor)
  Q_PROPERTY(double tickLabelRotation READ tickLabelRotation WRITE setTickLabelRotation)
  Q_PROPERTY(QString dateTimeFormat READ dateTimeFormat WRITE setDateTimeFormat)
  Q_PROPERTY(QString numberFormat READ numberFormat WRITE setNumberFormat)
  Q_PROPERTY(int numberPrecision READ numberPrecision WRITE setNumberPrecision)
  Q_PROPERTY(double tickStep READ tickStep WRITE setTickStep)
  Q_PROPERTY(QVector<double> tickVector READ tickVector WRITE setTickVector)
  Q_PROPERTY(QVector<QString> tickVectorLabels READ tickVectorLabels WRITE setTickVectorLabels)
  Q_PROPERTY(int tickLengthIn READ tickLengthIn WRITE setTickLengthIn)
  Q_PROPERTY(int tickLengthOut READ tickLengthOut WRITE setTickLengthOut)
  Q_PROPERTY(int subTickCount READ subTickCount WRITE setSubTickCount)
  Q_PROPERTY(int subTickLengthIn READ subTickLengthIn WRITE setSubTickLengthIn)
  Q_PROPERTY(int subTickLengthOut READ subTickLengthOut WRITE setSubTickLengthOut)
  Q_PROPERTY(QPen basePen READ basePen WRITE setBasePen)
  Q_PROPERTY(QPen tickPen READ tickPen WRITE setTickPen)
  Q_PROPERTY(QPen subTickPen READ subTickPen WRITE setSubTickPen)
  Q_PROPERTY(QFont labelFont READ labelFont WRITE setLabelFont)
  Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor)
  Q_PROPERTY(QString label READ label WRITE setLabel)
  Q_PROPERTY(int labelPadding READ labelPadding WRITE setLabelPadding)
  Q_PROPERTY(int padding READ padding WRITE setPadding)
  Q_PROPERTY(int offset READ offset WRITE setOffset)
  Q_PROPERTY(SelectableParts selectedParts READ selectedParts WRITE setSelectedParts)
  Q_PROPERTY(SelectableParts selectableParts READ selectableParts WRITE setSelectableParts)
  Q_PROPERTY(QFont selectedTickLabelFont READ selectedTickLabelFont WRITE setSelectedTickLabelFont)
  Q_PROPERTY(QFont selectedLabelFont READ selectedLabelFont WRITE setSelectedLabelFont)
  Q_PROPERTY(QColor selectedTickLabelColor READ selectedTickLabelColor WRITE setSelectedTickLabelColor)
  Q_PROPERTY(QColor selectedLabelColor READ selectedLabelColor WRITE setSelectedLabelColor)
  Q_PROPERTY(QPen selectedBasePen READ selectedBasePen WRITE setSelectedBasePen)
  Q_PROPERTY(QPen selectedTickPen READ selectedTickPen WRITE setSelectedTickPen)
  Q_PROPERTY(QPen selectedSubTickPen READ selectedSubTickPen WRITE setSelectedSubTickPen)
  Q_PROPERTY(QCPLineEnding lowerEnding READ lowerEnding WRITE setLowerEnding)
  Q_PROPERTY(QCPLineEnding upperEnding READ upperEnding WRITE setUpperEnding)
  Q_PROPERTY(QCPGrid* grid READ grid)
  /// \endcond
public:
  /*!
    Defines at which side of the axis rect the axis will appear. This also affects how the tick
    marks are drawn, on which side the labels are placed etc.
  */
  enum AxisType { atLeft    = 0x01  ///< <tt>0x01</tt> Axis is vertical and on the left side of the axis rect
                  ,atRight  = 0x02  ///< <tt>0x02</tt> Axis is vertical and on the right side of the axis rect
                  ,atTop    = 0x04  ///< <tt>0x04</tt> Axis is horizontal and on the top side of the axis rect
                  ,atBottom = 0x08  ///< <tt>0x08</tt> Axis is horizontal and on the bottom side of the axis rect
                };
  Q_FLAGS(AxisType AxisTypes)
  Q_DECLARE_FLAGS(AxisTypes, AxisType)
  /*!
    When automatic tick label generation is enabled (\ref setAutoTickLabels), defines how the
    coordinate of the tick is interpreted, i.e. translated into a string.
    
    \see setTickLabelType
  */
  enum LabelType { ltNumber    ///< Tick coordinate is regarded as normal number and will be displayed as such. (see \ref setNumberFormat)
                   ,ltDateTime ///< Tick coordinate is regarded as a date/time (seconds since 1970-01-01T00:00:00 UTC) and will be displayed and formatted as such. (for details, see \ref setDateTimeFormat)
                 };
  Q_ENUMS(LabelType)
  /*!
    Defines the scale of an axis.
    \see setScaleType
  */
  enum ScaleType { stLinear       ///< Linear scaling
                   ,stLogarithmic ///< Logarithmic scaling with correspondingly transformed plots and (major) tick marks at every base power (see \ref setScaleLogBase).
                 };
  Q_ENUMS(ScaleType)
  /*!
    Defines the selectable parts of an axis.
    \see setSelectableParts, setSelectedParts
  */
  enum SelectablePart { spNone        = 0      ///< None of the selectable parts
                        ,spAxis       = 0x001  ///< The axis backbone and tick marks
                        ,spTickLabels = 0x002  ///< Tick labels (numbers) of this axis (as a whole, not individually)
                        ,spAxisLabel  = 0x004  ///< The axis label
                      };
  Q_FLAGS(SelectablePart SelectableParts)
  Q_DECLARE_FLAGS(SelectableParts, SelectablePart)
  
  explicit QCPAxis(QCPAxisRect *parent, AxisType type);
      
  // getters:
  AxisType axisType() const { return mAxisType; }
  QCPAxisRect *axisRect() const { return mAxisRect; }
  ScaleType scaleType() const { return mScaleType; }
  double scaleLogBase() const { return mScaleLogBase; }
  const QCPRange range() const { return mRange; }
  bool rangeReversed() const { return mRangeReversed; }
  bool autoTicks() const { return mAutoTicks; }
  int autoTickCount() const { return mAutoTickCount; }
  bool autoTickLabels() const { return mAutoTickLabels; }
  bool autoTickStep() const { return mAutoTickStep; }
  bool autoSubTicks() const { return mAutoSubTicks; }
  bool ticks() const { return mTicks; }
  bool tickLabels() const { return mTickLabels; }
  int tickLabelPadding() const { return mTickLabelPadding; }
  LabelType tickLabelType() const { return mTickLabelType; }
  QFont tickLabelFont() const { return mTickLabelFont; }
  QColor tickLabelColor() const { return mTickLabelColor; }
  double tickLabelRotation() const { return mTickLabelRotation; }
  QString dateTimeFormat() const { return mDateTimeFormat; }
  Qt::TimeSpec dateTimeSpec() const { return mDateTimeSpec; }
  QString numberFormat() const;
  int numberPrecision() const { return mNumberPrecision; }
  double tickStep() const { return mTickStep; }
  QVector<double> tickVector() const { return mTickVector; }
  QVector<QString> tickVectorLabels() const { return mTickVectorLabels; }
  int tickLengthIn() const { return mTickLengthIn; }
  int tickLengthOut() const { return mTickLengthOut; }
  int subTickCount() const { return mSubTickCount; }
  int subTickLengthIn() const { return mSubTickLengthIn; }
  int subTickLengthOut() const { return mSubTickLengthOut; }
  QPen basePen() const { return mBasePen; }
  QPen tickPen() const { return mTickPen; }
  QPen subTickPen() const { return mSubTickPen; }
  QFont labelFont() const { return mLabelFont; }
  QColor labelColor() const { return mLabelColor; }
  QString label() const { return mLabel; }
  int labelPadding() const { return mLabelPadding; }
  int padding() const { return mPadding; }
  int offset() const { return mOffset; }
  SelectableParts selectedParts() const { return mSelectedParts; }
  SelectableParts selectableParts() const { return mSelectableParts; }
  QFont selectedTickLabelFont() const { return mSelectedTickLabelFont; }
  QFont selectedLabelFont() const { return mSelectedLabelFont; }
  QColor selectedTickLabelColor() const { return mSelectedTickLabelColor; }
  QColor selectedLabelColor() const { return mSelectedLabelColor; }
  QPen selectedBasePen() const { return mSelectedBasePen; }
  QPen selectedTickPen() const { return mSelectedTickPen; }
  QPen selectedSubTickPen() const { return mSelectedSubTickPen; }
  QCPLineEnding lowerEnding() const { return mLowerEnding; }
  QCPLineEnding upperEnding() const { return mUpperEnding; }
  QCPGrid *grid() const { return mGrid; }
  
  // setters:
  void setScaleType(ScaleType type);
  void setScaleLogBase(double base);
  Q_SLOT void setRange(const QCPRange &range);
  void setRange(double lower, double upper);
  void setRange(double position, double size, Qt::AlignmentFlag alignment);
  void setRangeLower(double lower);
  void setRangeUpper(double upper);
  void setRangeReversed(bool reversed);
  void setAutoTicks(bool on);
  void setAutoTickCount(int approximateCount);
  void setAutoTickLabels(bool on);
  void setAutoTickStep(bool on);
  void setAutoSubTicks(bool on);
  void setTicks(bool show);
  void setTickLabels(bool show);
  void setTickLabelPadding(int padding);
  void setTickLabelType(LabelType type);
  void setTickLabelFont(const QFont &font);
  void setTickLabelColor(const QColor &color);
  void setTickLabelRotation(double degrees);
  void setDateTimeFormat(const QString &format);
  void setDateTimeSpec(const Qt::TimeSpec &timeSpec);
  void setNumberFormat(const QString &formatCode);
  void setNumberPrecision(int precision);
  void setTickStep(double step);
  void setTickVector(const QVector<double> &vec);
  void setTickVectorLabels(const QVector<QString> &vec);
  void setTickLength(int inside, int outside=0);
  void setTickLengthIn(int inside);
  void setTickLengthOut(int outside);
  void setSubTickCount(int count);
  void setSubTickLength(int inside, int outside=0);
  void setSubTickLengthIn(int inside);
  void setSubTickLengthOut(int outside);
  void setBasePen(const QPen &pen);
  void setTickPen(const QPen &pen);
  void setSubTickPen(const QPen &pen);
  void setLabelFont(const QFont &font);
  void setLabelColor(const QColor &color);
  void setLabel(const QString &str);
  void setLabelPadding(int padding);
  void setPadding(int padding);
  void setOffset(int offset);
  void setSelectedTickLabelFont(const QFont &font);
  void setSelectedLabelFont(const QFont &font);
  void setSelectedTickLabelColor(const QColor &color);
  void setSelectedLabelColor(const QColor &color);
  void setSelectedBasePen(const QPen &pen);
  void setSelectedTickPen(const QPen &pen);
  void setSelectedSubTickPen(const QPen &pen);
  Q_SLOT void setSelectableParts(const QCPAxis::SelectableParts &selectableParts);
  Q_SLOT void setSelectedParts(const QCPAxis::SelectableParts &selectedParts);
  void setLowerEnding(const QCPLineEnding &ending);
  void setUpperEnding(const QCPLineEnding &ending);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=0) const;
  
  // non-virtual methods:
  Qt::Orientation orientation() const { return mOrientation; }
  void moveRange(double diff);
  void scaleRange(double factor, double center);
  void setScaleRatio(const QCPAxis *otherAxis, double ratio=