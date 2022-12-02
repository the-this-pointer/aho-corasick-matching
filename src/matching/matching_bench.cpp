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
#include <algorithm>
#include <iterator>
#include <sstream>

using namespace std;
namespace ac = aho_corasick;
using trie = ac::trie;

// taken from: https://github.com/KolorowyAleksander/mailbox/blob/master/include/utilities.h
namespace utilities {

  std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      result.push_back(item);
    }
    return result;
  }

  bool matching(std::string matchingKey, std::string primaryKey) {
    std::vector<std::string> primary = utilities::split(primaryKey, '.');
    std::vector<std::string> matching = utilities::split(matchingKey, '.');
    if (primary.size() == matching.size()) {  // same size
      for (int i = 0; i < primary.size(); i++) {
        if (primary[i] != matching[i]) {  // primary nd matching do not match
          if (primary[i] != "*" && primary[i] != "#") {  // not equal * or #
            return false;
          }
        }
      }
    } else {  // not same size
      int current_matching = 0;
      if (std::find(primary.begin(), primary.end(), "#") != primary.end()) {
        for (int i = 0; i < primary.size(); i++) {
          if (primary[i] != matching[current_matching]) {
            if (primary[i] == "*") {
              current_matching++;
              if (i == primary.size() - 1) {
                return true;
              }
            }
            if (primary[i] == "#") {          // primary is #
              if (i == primary.size() - 1) {  //# is on last place
                return true;
              }
              if (primary[i + 1] == "*") {
                i++;
                current_matching += 2;
              }
              auto it = std::find(matching.begin() + current_matching,
                                  matching.end(), primary[i + 1]);
              if (it != matching.end()) {
                current_matching = it - (matching.begin() + current_matching + 1);
                i++;
              } else {
                return false;
              }
            } else {  // are different, not #
              return false;
            }
          } else {
            current_matching++;
          }
        }
      } else {  //# does not exists
        return false;
      }
    }
    return true;
  }
}

string gen_str(size_t len) {
  static const char alphanum[] =
      "0123456789"
      "!@~$%^&*"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  string str;
  for (int i = 0; i < len; ++i) {
    str.append(1, alphanum[rand() % (sizeof(alphanum) - 1)]);
  }
  return string(str);
}

size_t bench_naive(vector<string> text_strings, vector<string> patterns) {
  size_t count = 0;
  for (auto& text : text_strings) {
    for (auto& pattern : patterns) {
      bool res = utilities::matching(text, pattern);
      if (res) {
        count++;
      }
    }
  }
  return count;
}

size_t bench_aho_corasick(vector<string> text_strings, trie& t) {
  size_t count = 0;
  for (auto& text : text_strings) {
    auto matches = t.parse_text(text);
    if (!matches.empty())
      count ++;
  }
  return count;
}


int main(int argc, char** argv) {
  cout << "*** Aho-Corasick Matching Test ***" << endl;

  cout << "Generating input text ...";
  set<string> input_strings;
  while (input_strings.size() < 10) {
    std::string input = "ptr.";
    for(int i = 0; i < 5; i++)
    {
      input.append(gen_str((rand() % 7)+3));
      input.append(".");
    }
    input.pop_back();
    input_strings.insert(input);

    //cout << "pattern: " << input << endl;
  }
  vector<string> input_vector(input_strings.begin(), input_strings.end());
  cout << " done" << endl;

  cout << "Generating search patterns ...";
  set<string> patterns;
  while (patterns.size() < 100000) {
    std::string pattern = "ptr.";
    for(int i = 0; i < 5; i++)
    {
      int rnd = rand() % 5;
      if (rnd == 0)
        pattern.append("#.");
      else if (rnd == 1)
        pattern.append("+.");
      else
      {
        pattern.append(gen_str((rand() % 7)+3));
        pattern.append(".");
      }
    }
    pattern.pop_back();
    patterns.insert(pattern);

    //cout << "pattern: " << pattern << endl;
  }
  vector<string> pattern_vector(patterns.begin(), patterns.end());
  cout << " done" << endl;

  cout << "Generating trie ...";
  trie t;
  for (auto& pattern : patterns) {
    t.insert(pattern);
  }
  cout << " done" << endl;

  map<size_t, tuple<chrono::high_resolution_clock::duration, chrono::high_resolution_clock::duration>> timings;

  cout << "Running ";
  cout << boolalpha;
  for (size_t i = 10; i > 0; --i) {
    cout << ".";
    auto start_time = chrono::high_resolution_clock::now();
    size_t count_1 = bench_naive(input_vector, pattern_vector);
    auto end_time = chrono::high_resolution_clock::now();
    auto time_1 = end_time - start_time;

    start_time = chrono::high_resolution_clock::now();
    size_t count_2 = bench_aho_corasick(input_vector, t);
    end_time = chrono::high_resolution_clock::now();
    auto time_2 = end_time - start_time;

    if (count_1 != count_2) {
      cout << "failed" << endl;
    }

    timings[i] = make_tuple(time_1, time_2);
  }
  cout << " done" << endl;

  cout << "Results: " << endl;
  for (auto& i : timings) {
    cout << "  loop #" << i.first;
    cout << ", naive: " << chrono::duration_cast<chrono::milliseconds>(get<0>(i.second)).count();
    cout << "ms, ac: " << chrono::duration_cast<chrono::milliseconds>(get<1>(i.second)).count() << "ms";
    cout << endl;
  }

  return 0;
}