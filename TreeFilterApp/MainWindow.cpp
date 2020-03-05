#include "MainWindow.h"
#include "TextTreeModel.h"
#include "QtUtilities.h"

#include "TreeFilterMaker.h"

#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qerrormessage.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWinExtras/qwinfunctions.h>

#include "resource.h"

namespace TreeReaderApp
{
   using namespace TreeReader;
   using namespace QtAdditions;
   using namespace std;

   namespace
   {
      namespace L
      {
         inline const wchar_t* t(const wchar_t* text)
         {
            return text;
         }
      }

      static constexpr wchar_t TreeFileTypes[] = L"Text Tree files (*.txt *.log);;Text files (*.txt);;Log files (*.log)";
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Create the main window.

   MainWindow::MainWindow()
   {
      BuildUI();
      FillUI();
      ConnectUI();
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Create the UI elements.

   void MainWindow::BuildUI()
   {
      QToolBar* toolbar = new QToolBar();
         toolbar->setIconSize(QSize(32, 32));

         _loadTreeAction = CreateAction(L::t(L"Load Tree"), IDB_TREE_OPEN, QKeySequence(QKeySequence::StandardKey::Open));
         _loadTreeButton = CreateToolButton(_loadTreeAction);
         toolbar->addWidget(_loadTreeButton);

         _saveTreeAction = CreateAction(L::t(L"Save Tree"), IDB_TREE_SAVE, QKeySequence(QKeySequence::StandardKey::Save));
         _saveTreeButton = CreateToolButton(_saveTreeAction);
         toolbar->addWidget(_saveTreeButton);

         _applyFilterAction = CreateAction(L::t(L"Filter Tree"), IDB_FILTER_APPLY, QKeySequence(QKeySequence::StandardKey::Find));
         _applyFilterButton = CreateToolButton(_applyFilterAction);
         toolbar->addWidget(_applyFilterButton);

         _nameFilterAction = CreateAction(L::t(L"Name Filter"), IDB_FILTER_NAME);
         _nameFilterButton = CreateToolButton(_nameFilterAction);
         toolbar->addWidget(_nameFilterButton);

         toolbar->addSeparator();

         _undoAction = CreateAction(L::t(L"Undo"), IDB_UNDO, QKeySequence(QKeySequence::StandardKey::Undo));
         _undoButton = CreateToolButton(_undoAction);
         _undoAction->setEnabled(false);
         toolbar->addWidget(_undoButton);

         _redoAction = CreateAction(L::t(L"Redo"), IDB_REDO, QKeySequence(QKeySequence::StandardKey::Redo));
         _redoButton = CreateToolButton(_redoAction);
         _redoAction->setEnabled(false);
         toolbar->addWidget(_redoButton);


      auto filtersDock = new QDockWidget(QString::fromWCharArray(L::t(L"Tree Filter")));
         filtersDock->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetFloatable | QDockWidget::DockWidgetFeature::DockWidgetMovable);
         QWidget* filters_container = new QWidget();
         QHBoxLayout* filters_layout = new QHBoxLayout(filters_container);

         _availableFiltersList = new TreeFilterListWidget;
         filters_layout->addWidget(_availableFiltersList);

         _filterEditor = new FilterEditor(filters_container);
         filters_layout->addWidget(_filterEditor);

         filtersDock->setWidget(filters_container);

      _treeView = new QTreeView;
      _treeView->setUniformRowHeights(true);
      _treeView->setHeaderHidden(true);
      _treeView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

      auto cmdDock = new QDockWidget(QString::fromWCharArray(L::t(L"Commands")));
         cmdDock->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetFloatable | QDockWidget::DockWidgetFeature::DockWidgetMovable);
         QWidget* cmd_container = new QWidget();
         QVBoxLayout* cmd_layout = new QVBoxLayout(cmd_container);

         _cmdLine = new QLineEdit(cmd_container);
         cmd_layout->addWidget(_cmdLine);

         cmdDock->setWidget(cmd_container);

      setCentralWidget(_treeView);
      addToolBar(toolbar);
      addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, filtersDock);
      addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, cmdDock);
      setWindowIcon(QIcon(QtWin::fromHICON((HICON)::LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON, 256, 256, 0))));
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Connect the signals of the UI elements.

   void MainWindow::ConnectUI()
   {
      /////////////////////////////////////////////////////////////////////////
      //
      // Undo / redo actions.

      _undoAction->connect(_undoAction, &QAction::triggered, [&]()
      {
         _undoStack.Undo();
         UpdateUndoRedoActions();
      });

      _redoAction->connect(_redoAction, &QAction::triggered, [&]()
      {
         _undoStack.Redo();
         UpdateUndoRedoActions();
      });

      /////////////////////////////////////////////////////////////////////////
      //
      // Load / save actions.

      _loadTreeAction->connect(_loadTreeAction, &QAction::triggered, [self=this]()
      {
         self->LoadTree();
      });

      _saveTreeAction->connect(_saveTreeAction, &QAction::triggered, [self=this]()
      {
         self->SaveFilteredTree();
      });

      _applyFilterAction->connect(_applyFilterAction, &QAction::triggered, [self = this]()
      {
         self->FilterTree();
      });

      _nameFilterAction->connect(_nameFilterAction, &QAction::triggered, [self = this]()
      {
         self->NameFilter();
      });

      /////////////////////////////////////////////////////////////////////////
      //
      // The filter list UI call-backs.

      _filterEditor->FilterChanged = [self=this](const TreeFilterPtr& filter)
      {
         self->CommitToUndo();
         self->_data.Filter = self->_filterEditor->GetEdited();
      };

      /////////////////////////////////////////////////////////////////////////
      //
      // Command line-edit.

      _cmdLine->connect(_cmdLine, &QLineEdit::editingFinished, [self=this]()
      {
         QString text = self->_cmdLine->text();
         wstring result = ParseCommands(text.toStdWString(), self->_data);

         self->FillTextTreeUI();
         self->FillFilterEditorUI();
      });
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Fill the UI with the intial data.

   void MainWindow::FillUI()
   {
      try
      {
         *_data.KnownFilters = ReadNamedFilters(L"filters.txt");
      }
      catch (const exception &)
      {
         // Ignore.
      }

      ClearUndoStack();
      FillAvailableFiltersUI();
   }

   void MainWindow::FillTextTreeUI()
   {
      shared_ptr<TextTree> newTree;
      if (_data.Filtered)
      {
         newTree = _data.Filtered;
      }
      else if (_data.Trees.size() > 0)
      {
         newTree = _data.Trees.back();
      }

      if (!_treeView->model() || !dynamic_cast<TextTreeModel*>(_treeView->model()) || dynamic_cast<TextTreeModel*>(_treeView->model())->Tree != newTree)
      {
         TextTreeModel* model = new TextTreeModel;
         model->Tree = newTree;
         _treeView->setModel(model);
      }

      CommitToUndo();
   }

   void MainWindow::FillFilterEditorUI()
   {
      _filterEditor->SetEdited(_data.Filter);
   }

   void MainWindow::FillAvailableFiltersUI()
   {
      for (const auto& [name, filter] : _data.KnownFilters->All())
         AddNamedFilterToAvailable(filter);

      _availableFiltersList->AddTreeFilter(Accept());
      _availableFiltersList->AddTreeFilter(Stop());
      _availableFiltersList->AddTreeFilter(Until(nullptr));
      _availableFiltersList->AddTreeFilter(Contains(L""));
      _availableFiltersList->AddTreeFilter(Regex(L""));
      _availableFiltersList->AddTreeFilter(Not(nullptr));
      _availableFiltersList->AddTreeFilter(Any(vector<TreeFilterPtr>()));
      _availableFiltersList->AddTreeFilter(All(vector<TreeFilterPtr>()));
      _availableFiltersList->AddTreeFilter(Under(nullptr));
      _availableFiltersList->AddTreeFilter(CountSiblings(nullptr, 0));
      _availableFiltersList->AddTreeFilter(CountChildren(nullptr, 0));
      _availableFiltersList->AddTreeFilter(NoChild(nullptr));
      _availableFiltersList->AddTreeFilter(LevelRange(0, 100));
      _availableFiltersList->AddTreeFilter(IfSubTree(nullptr));
      _availableFiltersList->AddTreeFilter(IfSibling(nullptr));
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Closing and saving.

   void MainWindow::closeEvent(QCloseEvent* ev)
   {
      if (SaveIfRequired(L::t(L"close the window"), L::t(L"closing the window")))
      {
         QWidget::closeEvent(ev);
         try
         {
            WriteNamedFilters(L"filters.txt", *_data.KnownFilters);
         }
         catch (const exception&)
         {
            // Ignore.
         }
      }
      else
      {
         ev->ignore();
      }
   }

   bool MainWindow::SaveIfRequired(const wstring& action, const wstring& actioning)
   {
      if (_data.Filtered && !_data.FilteredWasSaved)
      {
         YesNoCancel answer = AskYesNoCancel(
            L::t(L"Unsaved Text Tree Warning"),
            wstring(L::t(L"The current filtered tree has not been saved.\nDo you want to save it before ")) + actioning + L::t(L"?"),
            this);
         if (answer == YesNoCancel::Cancel)
            return false;
         else if (answer == YesNoCancel::Yes)
            if (!SaveFilteredTree())
               return false;
      }

      return true;
   }

   void MainWindow::LoadTree()
   {
      if (!SaveIfRequired(L::t(L"load a text tree"), L::t(L"loading a text tree")))
         return;

      filesystem::path path = AskOpen(L::t(L"Load Text Tree"), L::t(TreeFileTypes), this);
      _data.LoadTree(path);
      _data.ApplyFilterToTree();

      if (_data.Trees.size() == 0)
         return;

      FillTextTreeUI();
   }

   bool MainWindow::SaveFilteredTree()
   {
      if (!_data.Filtered)
         return true;

      filesystem::path path = AskSave(L::t(L"Save Filtered Text Tree"), L::t(TreeFileTypes), this);

      _data.SaveFilteredTree(path);

      return true;
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Tree filtering.

   void MainWindow::FilterTree()
   {
      _data.Filter = _filterEditor->GetEdited();
      _data.ApplyFilterToTree();

      shared_ptr<TextTree> newTree;
      if (_data.Filtered)
      {
         newTree = _data.Filtered;
      }
      else if (_data.Trees.size() > 0)
      {
         newTree = _data.Trees.back();
      }

      if (!_treeView->model() || !dynamic_cast<TextTreeModel*>(_treeView->model()) || dynamic_cast<TextTreeModel*>(_treeView->model())->Tree != newTree)
      {
         TextTreeModel* model = new TextTreeModel;
         model->Tree = newTree;
         _treeView->setModel(model);
      }

      CommitToUndo();
   }

   void MainWindow::NameFilter()
   {
      auto filter = _filterEditor->GetEdited();
      if (!filter)
         return;

      wstring filterName = AskForText(L::t(L"Name a filter"), L::t(L"Filter Name"), this);
      if (filterName.empty())
         return;

      auto namedFilter = Named(filterName, filter->Clone());
      _data.NameFilter(filterName, namedFilter);
      AddNamedFilterToAvailable(namedFilter);
   }

   void MainWindow::AddNamedFilterToAvailable(const TreeFilterPtr& filter)
   {
      _availableFiltersList->AddTreeFilter(filter, [self=this](TreeFilterWidget* panel)
      {
         if (!panel)
            return;

         if (auto named = dynamic_pointer_cast<NamedTreeFilter>(panel->Filter))
         {
            self->_data.KnownFilters->Remove(named->Name);
            self->_availableFiltersList->RemoveItem(panel);
         }
      });
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Undo / redo tool-bar buttons.

   void MainWindow::DeadedFilters(any& data)
   {
      data = ConvertFiltersToText(_data.Filter);
   }

   void MainWindow::AwakenFilters(const any& data)
   {
      _data.Filter = ConvertTextToFilters(any_cast<wstring>(data), *_data.KnownFilters);;

      FillFilterEditorUI();
   }

   void MainWindow::AwakenToEmptyFilters()
   {
      _data.Filter = nullptr;

      FillFilterEditorUI();
   }

   void MainWindow::ClearUndoStack()
   {
      _undoStack.Clear();
      // Note: allow undoing back to an empty filter list. To enable this, there must be an empty initial commit.
      _undoStack.Commit({ 0, nullptr, [self = this](const any&) { self->AwakenToEmptyFilters(); } });
   }

   void MainWindow::CommitToUndo()
   {
      _undoStack.Commit(
      {
         ConvertFiltersToText(_data.Filter),
         [self=this](any& data) { self->DeadedFilters(data); },
         [self=this](const any& data) { self->AwakenFilters(data); }
      });
      UpdateUndoRedoActions();
   }

   void MainWindow::UpdateUndoRedoActions()
   {
      _undoAction->setEnabled(_undoStack.HasUndo());
      _redoAction->setEnabled(_undoStack.HasRedo());
   }

}
