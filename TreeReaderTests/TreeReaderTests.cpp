#include "TreeReader.h"
#include "CppUnitTest.h"

#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TreeReaderTests
{
	TEST_CLASS(TreeReaderTests)
	{
	public:
		
		TEST_METHOD(PrintEmptyTree)
		{
			std::wostringstream sstream;
			sstream << TextTree();

			const wchar_t expectedOutput[] = L"";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}

		TEST_METHOD(PrintBuiltTree)
		{
			TextTree tree;

			tree.SourceTextLines = CreateTextLines();

			size_t r0 = tree.AddChild(0, &tree.SourceTextLines->at(0));
			size_t r0c0 = tree.AddChild(r0, &tree.SourceTextLines->at(1));
			size_t r0c1 = tree.AddChild(r0, &tree.SourceTextLines->at(2));
			size_t r0c2 = tree.AddSibling(r0c1, &tree.SourceTextLines->at(3));
			size_t r0c0c0 = tree.AddChild(r0c0, &tree.SourceTextLines->at(4));
			size_t r1 = tree.AddSibling(r0, &tree.SourceTextLines->at(5));

			std::wostringstream sstream;
			sstream << tree;

			const wchar_t expectedOutput[] =
				L"abc\n"
				L"  def\n"
				L"    mno\n"
				L"  ghi\n"
				L"  jkl\n"
				L"pqr\n";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}

		TEST_METHOD(PrintSimpleTree)
		{
			std::wostringstream sstream;
			sstream << CreateSimpleTree();

			const wchar_t expectedOutput[] =
				L"abc\n"
				L"  def\n"
				L"    jkl\n"
				L"  ghi\n"
				L"    mno\n"
				L"      pqr\n"
				L"      stu\n"
				L"        vwx\n";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}

		TEST_METHOD(CountSimpleTree)
		{
			const TextTree tree = CreateSimpleTree();

			Assert::AreEqual<size_t>(2, tree.CountChildren(0));
			Assert::AreEqual<size_t>(1, tree.CountChildren(1));
			Assert::AreEqual<size_t>(1, tree.CountChildren(2));
			Assert::AreEqual<size_t>(0, tree.CountChildren(3));
			Assert::AreEqual<size_t>(2, tree.CountChildren(4));
			Assert::AreEqual<size_t>(0, tree.CountChildren(5));
			Assert::AreEqual<size_t>(1, tree.CountChildren(6));
			Assert::AreEqual<size_t>(0, tree.CountChildren(7));

			Assert::AreEqual<size_t>(1, tree.CountSiblings(0));
			Assert::AreEqual<size_t>(2, tree.CountSiblings(1));
			Assert::AreEqual<size_t>(1, tree.CountSiblings(2));
			Assert::AreEqual<size_t>(1, tree.CountSiblings(3));
			Assert::AreEqual<size_t>(1, tree.CountSiblings(4));
			Assert::AreEqual<size_t>(2, tree.CountSiblings(5));
			Assert::AreEqual<size_t>(1, tree.CountSiblings(6));
			Assert::AreEqual<size_t>(1, tree.CountSiblings(7));
		}

		TEST_METHOD(PrintSimpleTreeWithContainsFilter)
		{
			ContainsTextFilter contains(L"g");

			TextTree filtered;
			FilterTree(CreateSimpleTree(), filtered, contains);

			std::wostringstream sstream;
			sstream << filtered;

			const wchar_t expectedOutput[] = L"ghi\n";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}

		TEST_METHOD(PrintSimpleTreeWithNotFilter)
		{
			ContainsTextFilter contains(L"f");
			NotTextFilter not(contains);

			TextTree filtered;
			FilterTree(CreateSimpleTree(), filtered, not);

			std::wostringstream sstream;
			sstream << filtered;

			const wchar_t expectedOutput[] =
				L"abc\n"
				L"  jkl\n"
				L"  ghi\n"
				L"    mno\n"
				L"      pqr\n"
				L"      stu\n"
				L"        vwx\n";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}

		TEST_METHOD(PrintSimpleTreeWithOrFilter)
		{
			ContainsTextFilter containsF(L"f");
			ContainsTextFilter containsM(L"m");
			OrTextFilter or(containsF, containsM);

			TextTree filtered;
			FilterTree(CreateSimpleTree(), filtered, or);

			std::wostringstream sstream;
			sstream << filtered;

			const wchar_t expectedOutput[] =
				L"def\n"
				L"mno\n";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}

		TEST_METHOD(PrintSimpleTreeWithRemoveChildrenFilter)
		{
			ContainsTextFilter contains(L"g");
			NotTextFilter not(contains);

			TextTreeFilter textTreeFilter(not);
			RemoveChildrenTreeFilter remove(textTreeFilter);

			TextTree filtered;
			FilterTree(CreateSimpleTree(), filtered, remove);

			std::wostringstream sstream;
			sstream << filtered;

			const wchar_t expectedOutput[] =
				L"abc\n"
				L"  def\n"
				L"    jkl\n";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}

		TEST_METHOD(PrintSimpleTreeWithMultiTextFilters)
		{
			MultiTextFilters filters;

			filters.Filters.push_back(std::make_shared<ContainsTextFilter>(L"g"));
			filters.Filters.push_back(std::make_shared<NotTextFilter>(*filters.Filters[0]));
			filters.RootIndex = 1;

			TextTree filtered;
			FilterTree(CreateSimpleTree(), filtered, filters);

			std::wostringstream sstream;
			sstream << filtered;

			const wchar_t expectedOutput[] =
				L"abc\n"
				L"  def\n"
				L"    jkl\n"
				L"  mno\n"
				L"    pqr\n"
				L"    stu\n"
				L"      vwx\n";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}


		TEST_METHOD(PrintSimpleTreeWithMultiTreeFilters)
		{
			MultiTextFilters filters;

			filters.Filters.push_back(std::make_shared<ContainsTextFilter>(L"d"));
			filters.Filters.push_back(std::make_shared<ContainsTextFilter>(L"s"));
			filters.Filters.push_back(std::make_shared<OrTextFilter>(*filters.Filters[0], *filters.Filters[1]));
			filters.RootIndex = 2;

			TextTree filtered;
			FilterTree(CreateSimpleTree(), filtered, filters);

			std::wostringstream sstream;
			sstream << filtered;

			const wchar_t expectedOutput[] =
				L"def\n"
				L"stu\n";
			Assert::AreEqual(expectedOutput, sstream.str().c_str());
		}

	private:

		std::shared_ptr<TextLines> CreateTextLines()
		{
			auto textLines = std::make_shared<TextLines>();

			textLines->push_back(L"abc");
			textLines->push_back(L"def");
			textLines->push_back(L"ghi");
			textLines->push_back(L"jkl");
			textLines->push_back(L"mno");
			textLines->push_back(L"pqr");
			textLines->push_back(L"stu");
			textLines->push_back(L"vwx");

			return textLines;
		}

		TextTree CreateSimpleTree()
		{
			TextTree textTree;

			textTree.SourceTextLines = CreateTextLines();

			textTree.Nodes.push_back(TextTree::Node{ &textTree.SourceTextLines->at(0), size_t(-1),  1 });
			textTree.Nodes.push_back(TextTree::Node{ &textTree.SourceTextLines->at(1),  2,  3 });
			textTree.Nodes.push_back(TextTree::Node{ &textTree.SourceTextLines->at(2), size_t(-1),  4 });
			textTree.Nodes.push_back(TextTree::Node{ &textTree.SourceTextLines->at(3), size_t(-1), size_t(-1) });
			textTree.Nodes.push_back(TextTree::Node{ &textTree.SourceTextLines->at(4), size_t(-1),  5 });
			textTree.Nodes.push_back(TextTree::Node{ &textTree.SourceTextLines->at(5),  6, size_t(-1) });
			textTree.Nodes.push_back(TextTree::Node{ &textTree.SourceTextLines->at(6), size_t(-1),  7 });
			textTree.Nodes.push_back(TextTree::Node{ &textTree.SourceTextLines->at(7), size_t(-1), size_t(-1) });

			return textTree;
		}
	};
}
