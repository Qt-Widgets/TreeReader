#include "QWidgetScrollListWidget.h"
#include "QWidgetListMimeData.h"
#include "QtUtilities.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qlayout.h>

#include <QtGui/qdrag.h>
#include <QtGui/qpainter.h>
#include <QtGui/qwindow.h>
#include <QtGui/qvalidator.h>

#include <QtGui/qevent.h>

namespace QtAdditions
{
   using namespace std;

   /////////////////////////////////////////////////////////////////////////
   //
   // Scrollable list of widgets.

   QWidgetScrollListWidget::QWidgetScrollListWidget(QWidget* widget, QWidget* parent)
   : QScrollArea(parent)
   {
      setWidget(widget);
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setWidgetResizable(true);
      setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
      setSizeAdjustPolicy(SizeAdjustPolicy::AdjustToContents);
   }

}

