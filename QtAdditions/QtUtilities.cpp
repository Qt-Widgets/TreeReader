#include "QtUtilities.h"

#include <QtWinExtras/qwinfunctions.h>

#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qerrormessage.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qinputdialog.h>

#include <fstream>

namespace QtAdditions
{
   using namespace std;

   QPixmap CreatePixmapFromResource(int res)
   {
      QPixmap pixmap = QtWin::fromHBITMAP((HBITMAP)::LoadBitmap(GetModuleHandle(0), MAKEINTRESOURCE(res)), QtWin::HBitmapNoAlpha);
      pixmap.setMask(pixmap.createMaskFromColor(QColor(255, 255, 255, 255)));
      return pixmap;
   }

   QToolButton* CreateToolButton()
   {
      QToolButton* button = new QToolButton;
      button->setIconSize(QSize(64, 64));
      button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
      return button;
   }

   QToolButton* CreateToolButton(const wchar_t* text, int icon, QKeySequence key, const wchar_t* tooltip, function<void()> callback)
   {
      return CreateToolButton(CreateAction(text, icon, key, tooltip, callback));
   }

   QToolButton* CreateToolButton(QAction* action)
   {
      QToolButton* button = CreateToolButton();
      button->setDefaultAction(action);
      return button;
   }

   QAction* CreateAction(const wchar_t* text, int icon, QKeySequence key, const wchar_t* tooltip, function<void()> callback)
   {
      QAction* action = new QAction;
      if (text && text[0])
         action->setText(QString::fromWCharArray(text));
      if (icon > 0)
         action->setIcon(QIcon(CreatePixmapFromResource(icon)));
      if (!key.isEmpty())
         action->setShortcut(key);
      if (tooltip && tooltip[0])
         action->setToolTip(QString::fromWCharArray(tooltip));
      if (callback)
         action->connect(action, &QAction::triggered, callback);
      return action;
   }

   filesystem::path AskOpen(const wchar_t* title, const wchar_t* file_types, QWidget* parent)
   {
      if (!title)
         title = L"";

      if (!file_types)
         file_types = L"";

      const QString qfn = QFileDialog::getOpenFileName(
         parent, QString::fromWCharArray(title), QString(), QString::fromWCharArray(file_types));

      return qfn.toStdWString();
   }

   filesystem::path AskSave(const wchar_t* title, const wchar_t* file_types, const wchar_t* initialContent, QWidget* parent)
   {
      if (!title)
         title = L"";

      if (!file_types)
         file_types = L"";

      if (!initialContent)
         initialContent = L"";

      const QString qfn = QFileDialog::getSaveFileName(
         parent, QString::fromWCharArray(title), QString::fromWCharArray(initialContent), QString::fromWCharArray(file_types));

      return qfn.toStdWString();
   }

   // Ask yes/no/cancel.
   YesNoCancel AskYesNoCancel(const wstring& title, const wstring& text, QWidget* parent)
   {
      return AskYesNoCancel(title.c_str(), text.c_str(), parent);
   }

   YesNoCancel AskYesNoCancel(const wchar_t* title, const wchar_t* text, QWidget* parent)
   {
      if (!title)
         title = L"";

      if (!text)
         text = L"";

      QMessageBox box;
      box.setWindowTitle(QString::fromWCharArray(title));
      box.setText(QString::fromWCharArray(text));
      box.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel | QMessageBox::StandardButton::No );
      box.setDefaultButton(QMessageBox::StandardButton::Cancel);
      switch (box.exec())
      {
         case QMessageBox::StandardButton::Yes:
            return YesNoCancel::Yes;
         case QMessageBox::StandardButton::No:
            return YesNoCancel::No;
         case QMessageBox::StandardButton::Cancel:
         default:
            return YesNoCancel::Cancel;
      }
   }

   wstring AskForText(const wchar_t* title, const wchar_t* label, const wchar_t* initialContent, QWidget* parent)
   {
      if (!title)
         title = L"";

      if (!label)
         label = L"";

      if (!initialContent)
         initialContent = L"";

      QString text = QInputDialog::getText(parent, QString::fromWCharArray(title), QString::fromWCharArray(label),
         QLineEdit::Normal, QString::fromWCharArray(initialContent));

      return text.toStdWString();
   }
}

// vim: sw=3 : sts=3 : et : sta : 
