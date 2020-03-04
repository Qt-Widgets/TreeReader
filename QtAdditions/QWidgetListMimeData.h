#pragma once

#include <QtCore/qmimedata.h>
#include <QtCore/qpoint.h>

namespace QtAdditions
{
   struct QWidgetListItem;

   struct QWidgetListMimeData : public QMimeData
   {
      static constexpr char MimeType[] = "application/x-qwidget-list-item";

      QWidgetListItem* Widget = nullptr;
      QPoint HotSpot;
   };
}
