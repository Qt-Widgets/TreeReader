#include "TreeFilterWidget.h"
#include "QtUtilities.h"

#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qcheckbox.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qpushbutton.h>

#include <QtGui/qvalidator.h>

#include "resource.h"

namespace TreeReaderApp
{
   using namespace TreeReader;
   using namespace std;
   using namespace QtAdditions;

   namespace
   {
      using DeleteCallbackFunction = TreeFilterWidget::DeleteCallbackFunction;

      namespace L
      {
         inline const wchar_t* t(const wchar_t* text)
         {
            return text;
         }
      }

      /////////////////////////////////////////////////////////////////////////
      //
      // Filter panel creation helpers.

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<AcceptTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<StopTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<UntilTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<ContainsTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, &filter->Contained);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<RegexTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, &filter->RegexTextForm);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<NotTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<IfSubTreeTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<IfSiblingTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<CountChildrenTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr, &filter->IncludeSelf, &filter->Count);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<CountSiblingsTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr, &filter->IncludeSelf, &filter->Count);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<OrTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<AndTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<UnderTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr, &filter->IncludeSelf);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<RemoveChildrenTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr, &filter->IncludeSelf);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<LevelRangeTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr, nullptr, &filter->MinLevel, &filter->MaxLevel);
      }

      TreeFilterWidget* CreateFilterPanel(const shared_ptr<NamedTreeFilter>& filter, DeleteCallbackFunction delFunc)
      {
         return new TreeFilterWidget(filter, delFunc, nullptr);
      }
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Filter panel creation.

   TreeFilterWidget* TreeFilterWidget::Create(const TreeFilterPtr& filter, DeleteCallbackFunction delFunc)
   {
      #define CALL_CONVERTER(a) if (auto ptr = dynamic_pointer_cast<a>(filter)) { return CreateFilterPanel(ptr, delFunc); }

      CALL_CONVERTER(AcceptTreeFilter)
      CALL_CONVERTER(StopTreeFilter)
      CALL_CONVERTER(UntilTreeFilter)
      CALL_CONVERTER(ContainsTreeFilter)
      CALL_CONVERTER(RegexTreeFilter)
      CALL_CONVERTER(NotTreeFilter)
      CALL_CONVERTER(IfSubTreeTreeFilter)
      CALL_CONVERTER(IfSiblingTreeFilter)
      CALL_CONVERTER(CountChildrenTreeFilter)
      CALL_CONVERTER(CountSiblingsTreeFilter)
      CALL_CONVERTER(OrTreeFilter)
      CALL_CONVERTER(AndTreeFilter)
      CALL_CONVERTER(UnderTreeFilter)
      CALL_CONVERTER(RemoveChildrenTreeFilter)
      CALL_CONVERTER(LevelRangeTreeFilter)
      CALL_CONVERTER(NamedTreeFilter)

      #undef CALL_CONVERTER

      return nullptr;
   }

   TreeFilterWidget::TreeFilterWidget(const shared_ptr<TreeFilter>& filter, DeleteCallbackFunction delFunc,
      wstring* textContent, bool* includeSelf, size_t* count, size_t* count2)
   : Filter(filter)
   {
      const bool active = (delFunc != nullptr);

      setToolTip(QString::fromStdWString(filter->GetDescription()));
      setBackgroundRole(QPalette::ColorRole::Base);

      auto container_layout = new QVBoxLayout(this);
      container_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
      container_layout->setMargin(4);
      setLayout(container_layout);

      auto name_layout = new QHBoxLayout;
      name_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
      name_layout->setMargin(0);
      container_layout->addLayout(name_layout);

      auto nameLabel = new QLabel(QString::fromStdWString(filter->GetShortName()));
      name_layout->addWidget(nameLabel);

      if (textContent)
      {
         auto textEdit = new QLineEdit(QString::fromStdWString(*textContent));
         textEdit->setMaximumWidth(80);
         textEdit->setEnabled(active);
         name_layout->addWidget(textEdit);

         textEdit->connect(textEdit, &QLineEdit::editingFinished, [textContent, textEdit]()
         {
            *textContent = textEdit->text().toStdWString();
         });
      }

      if (count)
      {
         auto numberEdit = new QLineEdit(QString().setNum(*count));
         numberEdit->setMaximumWidth(count2 ? 40 : 80);
         numberEdit->setValidator(new QIntValidator);
         numberEdit->setEnabled(active);
         name_layout->addWidget(numberEdit);

         numberEdit->connect(numberEdit, &QLineEdit::editingFinished, [count, numberEdit]()
         {
            *count = numberEdit->text().toUInt();
         });
      }

      if (count2)
      {
         auto numberEdit = new QLineEdit(QString().setNum(*count2));
         numberEdit->setMaximumWidth(count ? 40 : 80);
         numberEdit->setValidator(new QIntValidator);
         numberEdit->setEnabled(active);
         name_layout->addWidget(numberEdit);

         numberEdit->connect(numberEdit, &QLineEdit::editingFinished, [count2, numberEdit]()
         {
            *count2 = numberEdit->text().toUInt();
         });
      }

      if (delFunc)
      {
         auto deleteButton = new QPushButton;
         deleteButton->setIcon(QIcon(CreatePixmapFromResource(IDB_FILTER_DELETE)));
         deleteButton->setToolTip(QString::fromWCharArray(L::t(L"Delete this filter")));
         deleteButton->setFlat(true);
         deleteButton->setMaximumSize(QSize(16, 16));
         name_layout->addWidget(deleteButton);

         deleteButton->connect(deleteButton, &QPushButton::clicked, [filter, self=this, delFunc]()
         {
            delFunc(self);
         });
      }

      if (includeSelf)
      {
         auto includeBox = new QCheckBox(QString::fromWCharArray(L::t(L"Include self")));
         includeBox->setEnabled(active);
         container_layout->addWidget(includeBox);

         includeBox->connect(includeBox, &QCheckBox::stateChanged, [includeSelf, includeBox](int)
         {
            *includeSelf = includeBox->isChecked();
         });
      }
   }

   TreeFilterWidget* TreeFilterWidget::Clone() const
   {
      return Clone(nullptr);
   }

   TreeFilterWidget* TreeFilterWidget::Clone(DeleteCallbackFunction delFunc) const
   {
      return Create(Filter->Clone(), delFunc);
   }
}
