/*
* Copyright (C) 2022 Rsomething.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "aho_corasick/aho_corasick.hpp"
#include <chrono>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace std;
namespace ac = aho_corasick;
using trie = ac::trie;

int main(int argc, char** argv) {
	cout << "*** Aho-Corasick Matching Test ***" << endl;

  cout << "Generating input text ...";
  set<string> input_strings;
  input_strings.insert("hi.mom");
  input_strings.insert("hi.there");
  input_strings.insert("hi.alex.how.are.you?");
  input_strings.insert("hi.james.how.are.you?");
  input_strings.insert("hi.james.bond.how.are.you?");
  input_strings.insert("im.patrick");
  input_strings.insert("im.patrick.bond");
  input_strings.insert("im.james.bond");
  input_strings.insert("im.not.james.bond");
  vector<string> input_vector(input_strings.begin(), input_strings.end());
  cout << " done" << endl;

  cout << "Generating search patterns ...";
	set<string> patterns;
  patterns.insert("hi.#");
  patterns.insert("hi.there");
  patterns.insert("hi.mom");
  patterns.insert("hi.+.how.are.you?");
  patterns.insert("im.james.bond");
  patterns.insert("im.+.bond");
  patterns.insert("im.#.bond");
  patterns.insert("im.#");
  cout << " done" << endl;

	cout << "Generating trie ...";
	trie t;
	for (auto& pattern : patterns) {
		t.insert(pattern);
	}
	cout << " done" << endl;

	cout << "Running ";
	cout << boolalpha;
  for (auto& text : input_vector) {
    auto result = false;
    auto start_time = chrono::high_resolution_clock::now();

    auto matches = t.parse_text(text);
    result = matches.size() > 0;

    auto end_time = chrono::high_resolution_clock::now();
    auto time = end_time - start_time;

    cout << "=====================================================" << endl;
    cout << ">> result [" << text << "] : " << result << ", " << chrono::duration_cast<chrono::milliseconds>(time).count() << "ms" << endl;
    for(const auto& match: matches)
    {
      cout << "match: " << match.first.get_keyword() << endl;
    }
  }
  cout << " done" << endl;

	return 0;
}