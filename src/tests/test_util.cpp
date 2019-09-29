#include "tests/util.h"
#include "core/sputil.h"

int main(int, char **) {
	haunted::tests::testing unit;
	spjalla::tests::test_util(unit);	
}

namespace spjalla::tests {
	void test_util(haunted::tests::testing &unit) {
		using namespace std::string_literals;

		unit.check({
			{{"foo"s, 0},           { 0,  0}},
			{{"foo"s, 1},           { 0,  1}},
			{{"foo"s, 2},           { 0,  2}},
			{{"foo"s, 3},           { 0,  3}},
			{{"foo"s, 4},           {-2, -1}},
			{{"foo bar"s, 3},       { 0,  3}},
			{{"foo bar"s, 4},       { 1,  0}},
			{{"foo bar"s, 6},       { 1,  2}},
			{{"foo bar"s, 7},       { 1,  3}},
			{{"foo bar"s, 8},       {-3, -1}},
			{{"foo  bar"s, 4},      {-2, -1}},
			{{"foo "s, 4},          {-2, -1}},
			{{"foo    "s, 4},       {-2, -1}},
			{{"foo    "s, 7},       {-2, -1}},
			{{" foo"s, 0},          {-1, -1}},
			{{"  foo"s, 0},         {-1, -1}},
			{{"  foo"s, 1},         {-1, -1}},
			{{"  foo"s, 2},         { 0,  0}},
			{{"foo  bar  baz"s, 4}, {-2, -1}},
			{{"foo  bar  baz"s, 9}, {-3, -1}},
		}, &util::word_indices, "util::word_indices");
	}
}