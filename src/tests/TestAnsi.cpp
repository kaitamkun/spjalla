#include "haunted/tests/Test.h"
#include "lib/formicine/ansi.h"

namespace Spjalla::Tests {
	void test_ansi(Haunted::Tests::Testing &unit) {
		using namespace std::string_literals;
		
		unit.check({
			{{"hello^bthere"s}, "hello\e[1mthere"},
			{{"foo bar"s}, "foo bar"},
			{{"foo^^bar"s}, "foo^bar"},
			{{"foo^0bar"s}, "foo\e[0mbar"},
			{{"foo^[blue]bar"s}, "foo\e[34mbar"},
			{{"foo^[yeen]bar^[/f]baz"s}, "foo\e[38;5;112mbar\e[39mbaz"},
			{{"^[yellow]foo^[:blue]bar^[/f]baz^[/b]quux"s}, "\e[33mfoo\e[44mbar\e[39mbaz\e[49mquux"},
		}, &ansi::format, "ansi::format");
	}
}

int main(int, char **) {
	Haunted::Tests::Testing unit;
	Spjalla::Tests::test_ansi(unit);	
}
