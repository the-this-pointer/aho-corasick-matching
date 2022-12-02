/*
* Copyright (C) 2018 Christopher Gilbert.
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

#ifndef AHO_CORASICK_HPP
#define AHO_CORASICK_HPP

#include <algorithm>
#include <cctype>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <queue>
#include <utility>
#include <vector>
#include <limits>

namespace aho_corasick {

	// class interval
	class interval {
		size_t d_start;
		size_t d_end;

	public:
		interval(size_t start, size_t end)
			: d_start(start)
			, d_end(end) {}

		size_t get_start() const { return d_start; }
		size_t get_end() const { return d_end; }
		size_t size() const { return d_end - d_start + 1; }

		bool overlaps_with(const interval& other) const {
			return d_start <= other.d_end && d_end >= other.d_start;
		}

		bool overlaps_with(size_t point) const {
			return d_start <= point && point <= d_end;
		}

		bool operator <(const interval& other) const {
			return get_start() < other.get_start();
		}

		bool operator !=(const interval& other) const {
			return get_start() != other.get_start() || get_end() != other.get_end();
		}

		bool operator ==(const interval& other) const {
			return get_start() == other.get_start() && get_end() == other.get_end();
		}
	};

	// class interval_tree
	template<typename T>
	class interval_tree {
	public:
		using interval_collection = std::vector<T>;

	private:
		// class node
		class node {
			enum direction {
				LEFT, RIGHT
			};
			using node_ptr = std::unique_ptr<node>;

			size_t              d_point;
			node_ptr            d_left;
			node_ptr            d_right;
			interval_collection d_intervals;

		public:
			explicit node(const interval_collection& intervals)
				: d_point(0)
				, d_left(nullptr)
				, d_right(nullptr)
				, d_intervals()
			{
				d_point = determine_median(intervals);
				interval_collection to_left, to_right;
				for (const auto& i : intervals) {
					if (i.get_end() < d_point) {
						to_left.push_back(i);
					} else if (i.get_start() > d_point) {
						to_right.push_back(i);
					} else {
						d_intervals.push_back(i);
					}
				}
				if (to_left.size() > 0) {
					d_left.reset(new node(to_left));
				}
				if (to_right.size() > 0) {
					d_right.reset(new node(to_right));
				}
			}

			size_t determine_median(const interval_collection& intervals) const {
				auto start = std::numeric_limits<size_t>::max();
				auto end   = std::numeric_limits<size_t>::max();
				for (const auto& i : intervals) {
					auto cur_start = i.get_start();
					auto cur_end = i.get_end();
					if (start == std::numeric_limits<size_t>::max() || cur_start < start) {
						start = cur_start;
					}
					if (end == std::numeric_limits<size_t>::max() || cur_end > end) {
						end = cur_end;
					}
				}
				return (start + end) / 2;
			}

			interval_collection find_overlaps(const T& i) {
				interval_collection overlaps;
				if (d_point < i.get_start()) {
					add_to_overlaps(i, overlaps, find_overlapping_ranges(d_right, i));
					add_to_overlaps(i, overlaps, check_right_overlaps(i));
				} else if (d_point > i.get_end()) {
					add_to_overlaps(i, overlaps, find_overlapping_ranges(d_left, i));
					add_to_overlaps(i, overlaps, check_left_overlaps(i));
				} else {
					add_to_overlaps(i, overlaps, d_intervals);
					add_to_overlaps(i, overlaps, find_overlapping_ranges(d_left, i));
					add_to_overlaps(i, overlaps, find_overlapping_ranges(d_right, i));
				}
				return interval_collection(overlaps);
			}

		protected:
			void add_to_overlaps(const T& i, interval_collection& overlaps, interval_collection new_overlaps) const {
				for (const auto& cur : new_overlaps) {
					if (cur != i) {
						overlaps.push_back(cur);
					}
				}
			}

			interval_collection check_left_overlaps(const T& i) const {
				return interval_collection(check_overlaps(i, LEFT));
			}

			interval_collection check_right_overlaps(const T& i) const {
				return interval_collection(check_overlaps(i, RIGHT));
			}

			interval_collection check_overlaps(const T& i, direction d) const {
				interval_collection overlaps;
				for (const auto& cur : d_intervals) {
					switch (d) {
					case LEFT:
						if (cur.get_start() <= i.get_end()) {
							overlaps.push_back(cur);
						}
						break;
					case RIGHT:
						if (cur.get_end() >= i.get_start()) {
							overlaps.push_back(cur);
						}
						break;
					}
				}
				return interval_collection(overlaps);
			}

			interval_collection find_overlapping_ranges(node_ptr& node, const T& i) const {
				if (node) {
					return interval_collection(node->find_overlaps(i));
				}
				return interval_collection();
			}
		};
		node d_root;

	public:
		explicit interval_tree(const interval_collection& intervals)
			: d_root(intervals) {}

		interval_collection remove_overlaps(const interval_collection& intervals) {
			interval_collection result(intervals.begin(), intervals.end());
			std::sort(result.begin(), result.end(), [](const T& a, const T& b) -> bool {
				if (b.size() - a.size() == 0) {
					return a.get_start() > b.get_start();
				}
				return a.size() > b.size();
			});
			std::set<T> remove_tmp;
			for (const auto& i : result) {
				if (remove_tmp.find(i) != remove_tmp.end()) {
					continue;
				}
				auto overlaps = find_overlaps(i);
				for (const auto& overlap : overlaps) {
					remove_tmp.insert(overlap);
				}
			}
			for (const auto& i : remove_tmp) {
				result.erase(
					std::find(result.begin(), result.end(), i)
				);
			}
			std::sort(result.begin(), result.end(), [](const T& a, const T& b) -> bool {
				return a.get_start() < b.get_start();
			});
			return interval_collection(result);
		}

		interval_collection find_overlaps(const T& i) {
			return interval_collection(d_root.find_overlaps(i));
		}
	};

	// class emit
	template<typename CharType>
	class emit: public interval {
	public:
		typedef std::basic_string<CharType>  string_type;
		typedef std::basic_string<CharType>& string_ref_type;

	private:
		string_type d_keyword;
		unsigned    d_index = 0;

	public:
		emit()
			: interval(-1, -1)
			, d_keyword() {}

		emit(size_t start, size_t end, string_type keyword, unsigned index)
			: interval(start, end)
			, d_keyword(keyword), d_index(index) {}

		string_type get_keyword() const { return string_type(d_keyword); }
		unsigned get_index() const { return d_index; }
		bool is_empty() const { return (get_start() == -1 && get_end() == -1); }
	};

	// class token
	template<typename CharType>
	class token {
	public:
		enum token_type{
			TYPE_FRAGMENT,
			TYPE_MATCH,
		};

		using string_type     = std::basic_string<CharType>;
		using string_ref_type = std::basic_string<CharType>&;
		using emit_type       = emit<CharType>;

	private:
		token_type  d_type;
		string_type d_fragment;
		emit_type   d_emit;

	public:
		explicit token(string_ref_type fragment)
			: d_type(TYPE_FRAGMENT)
			, d_fragment(fragment)
			, d_emit() {}

		token(string_ref_type fragment, const emit_type& e)
			: d_type(TYPE_MATCH)
			, d_fragment(fragment)
			, d_emit(e) {}

		bool is_match() const { return (d_type == TYPE_MATCH); }
		string_type get_fragment() const { return string_type(d_fragment); }
		emit_type get_emit() const { return d_emit; }
	};

	// class state
	template<typename CharType>
	class state {
	public:
    typedef CharType                         type;
		typedef state<CharType>*                 ptr;
		typedef std::unique_ptr<state<CharType>> unique_ptr;
		typedef std::basic_string<CharType>      string_type;
		typedef std::basic_string<CharType>&     string_ref_type;
		typedef std::pair<string_type, unsigned> key_index;
		typedef std::set<key_index>              string_collection;
		typedef std::vector<ptr>                 state_collection;
		typedef std::vector<CharType>            transition_collection;

	private:
		size_t                         d_depth;
		ptr                            d_root;
		std::map<CharType, unique_ptr> d_success;
    bool                           d_has_success;
    ptr                            d_failure;
    string_collection              d_emits;
    type                           d_value; // used for matching against +/#
    bool                           d_ending_pattern;

	public:
		state(): state(0, 0) {}

		explicit state(size_t depth, type val)
			: d_depth(depth)
			, d_root(depth == 0 ? this : nullptr)
			, d_success()
			, d_failure(nullptr)
			, d_emits()
      , d_value(val)
      , d_has_success(false)
      {}

		ptr next_state(CharType character) const {
			return next_state(character, false, false);
		}

		ptr next_state_ignore_root_state(CharType character) const {
			return next_state(character, true, true);
		}

		ptr add_state(CharType character) {
			auto next = next_state_ignore_root_state(character);
			if (next == nullptr) {
				next = new state<CharType>(d_depth + 1, character);
				d_success[character].reset(next);
        if (next != this)
          d_has_success = true;
			}
			return next;
		}

		ptr add_state(CharType character, ptr state) {
			auto next = next_state_ignore_root_state(character);
			if (next == nullptr) {
				d_success[character].reset(state);
        if (state != this)
          d_has_success = true;
      }
			return state;
		}

		size_t get_depth() const { return d_depth; }

		void add_emit(string_ref_type keyword, unsigned index) {
			d_emits.insert(std::make_pair(keyword, index));
		}

		void add_emit(const string_collection& emits) {
			for (const auto& e : emits) {
				string_type str(e.first);
				add_emit(str, e.second);
			}
		}

		string_collection get_emits() const { return d_emits; }

    bool ending_pattern() { return d_ending_pattern; }

    void set_ending_pattern(bool ending_pattern) { d_ending_pattern = ending_pattern; }

    CharType value() { return d_value; }

		ptr failure() const { return d_failure; }

		void set_failure(ptr fail_state) { d_failure = fail_state; }

    bool has_success() const {return d_has_success;}

		state_collection get_states() const {
			state_collection result;
			for (auto it = d_success.cbegin(); it != d_success.cend(); ++it) {
				result.push_back(it->second.get());
			}
			return state_collection(result);
		}

		transition_collection get_transitions() const {
			transition_collection result;
			for (auto it = d_success.cbegin(); it != d_success.cend(); ++it) {
				result.push_back(it->first);
			}
			return transition_collection(result);
		}

	private:
		ptr next_state(CharType character, bool ignore_root_state, bool state_insertion) const {
      ptr result = nullptr;

      auto found = d_success.find(character);
      if (found != d_success.end()) {
        result = found->second.get();
      }

			return result;
		}
	};

	template<typename CharType>
	class basic_trie {
	public:
		using string_type = std::basic_string < CharType > ;
		using string_ref_type = std::basic_string<CharType>&;

		typedef state<CharType>         state_type;
		typedef state<CharType>*        state_ptr_type;
		typedef token<CharType>         token_type;
		typedef emit<CharType>          emit_type;
		typedef std::vector<state_ptr_type> state_collection;
		typedef std::vector<token_type> token_collection;
		typedef std::map<emit_type, bool>  emit_collection;

		class config {
			bool d_case_insensitive;

		public:
			config()
				: d_case_insensitive(false) {}

			bool is_case_insensitive() const { return d_case_insensitive; }
			void set_case_insensitive(bool val) { d_case_insensitive = val; }
		};

	private:
		std::unique_ptr<state_type> d_root;
		config                      d_config;
		bool                        d_constructed_failure_states;
		unsigned                    d_num_keywords = 0;

	public:
		basic_trie(): basic_trie(config()) {}

		basic_trie(const config& c)
			: d_root(new state_type())
			, d_config(c)
			, d_constructed_failure_states(false) {}

		basic_trie& case_insensitive() {
			d_config.set_case_insensitive(true);
			return (*this);
		}

		basic_trie& remove_overlaps() {
			d_config.set_allow_overlaps(false);
			return (*this);
		}

		basic_trie& only_whole_words() {
			d_config.set_only_whole_words(true);
			return (*this);
		}

		void insert(string_type keyword) {
			if (keyword.empty())
				return;
			state_ptr_type cur_state = d_root.get();
      state_ptr_type last_multi_wildcard = nullptr;

			for (const auto& ch : keyword) {
				cur_state = cur_state->add_state(ch);

        // Of course! I know that handling failures this way, could bring some bugs for matching topics later,
        // I'll be return and fix this section later...
        // topics with multiple # and +s will have some problems because of this!
        switch (ch) {
          case '.':
            if (last_multi_wildcard)
              cur_state->set_failure(last_multi_wildcard);
            break;
          case '+':
            cur_state->add_state('+', cur_state);
            break;
          case '#':
            cur_state->add_state('#', cur_state);
            last_multi_wildcard = cur_state;
            break;
        }
			}

      if (cur_state != d_root.get())
        cur_state->set_ending_pattern(true);

			cur_state->add_emit(keyword, d_num_keywords++);
			d_constructed_failure_states = false;
		}

		template<class InputIterator>
		void insert(InputIterator first, InputIterator last) {
			for (InputIterator it = first; first != last; ++it) {
				insert(*it);
			}
		}

		token_collection tokenise(string_type text) {
			token_collection tokens;
			auto collected_emits = parse_text(text);
			size_t last_pos = -1;
			for (const auto& e : collected_emits) {
				if (e.get_start() - last_pos > 1) {
					tokens.push_back(create_fragment(e, text, last_pos));
				}
				tokens.push_back(create_match(e, text));
				last_pos = e.get_end();
			}
			if (text.size() - last_pos > 1) {
				tokens.push_back(create_fragment(typename token_type::emit_type(), text, last_pos));
			}
			return token_collection(tokens);
		}

		emit_collection parse_text(string_type text) {
			size_t pos = 0;
			emit_collection collected_emits;

      state_collection prev_states;
      state_collection cur_states;
      prev_states.reserve(32);
      cur_states.reserve(32);
      prev_states.push_back(d_root.get());

      for (auto c : text) {
				if (d_config.is_case_insensitive()) {
					c = std::tolower(c);
				}

        for (auto& cur_state: prev_states)
        {
          auto state = get_state(cur_state, c);
          if (state)
          {
            if (!state->has_success() && pos + 1 == text.length())  // state finished
              store_emits(pos, state, collected_emits);
            cur_states.push_back(state);
          }

          if (!(cur_state->value() == '+' && c == '.')) {
            state = get_state(cur_state, '+');
            if (state) {
              if ((!state->has_success() || state->ending_pattern()) && pos + 1 == text.length())  // state finished
                store_emits(pos, state, collected_emits);
              cur_states.push_back(state);
            }
          }

          state = get_state(cur_state, '#');
          if (state)
          {
            if ((!state->has_success() || state->ending_pattern()) && pos + 1 == text.length())  // state finished
              store_emits(pos, state, collected_emits);
            cur_states.push_back(state);
          }
        }

        prev_states = std::move(cur_states);
        pos++;
			}
			return emit_collection(collected_emits);
		}

	private:
		token_type create_fragment(const typename token_type::emit_type& e, string_ref_type text, size_t last_pos) const {
			auto start = last_pos + 1;
			auto end = (e.is_empty()) ? text.size() : e.get_start();
			auto len = end - start;
			typename token_type::string_type str(text.substr(start, len));
			return token_type(str);
		}

		token_type create_match(const typename token_type::emit_type& e, string_ref_type text) const {
			auto start = e.get_start();
			auto end = e.get_end() + 1;
			auto len = end - start;
			typename token_type::string_type str(text.substr(start, len));
			return token_type(str, e);
		}

		state_ptr_type get_state(state_ptr_type cur_state, CharType c) const {
			state_ptr_type result = cur_state->next_state(c);
			while (result == nullptr) {
				cur_state = cur_state->failure();
        if (cur_state == nullptr)
          break;
				result = cur_state->next_state(c);
			}
			return result;
		}

		void store_emits(size_t pos, state_ptr_type cur_state, emit_collection& collected_emits) const {
			auto emits = cur_state->get_emits();
			if (!emits.empty()) {
				for (const auto& str : emits) {
					auto emit_str = typename emit_type::string_type(str.first);
					collected_emits[emit_type(pos - emit_str.size() + 1, pos, emit_str, str.second)] = true;
				}
			}
		}
	};

	typedef basic_trie<char>     trie;
	typedef basic_trie<wchar_t>  wtrie;


} // namespace aho_corasick

#endif // AHO_CORASICK_HPP
