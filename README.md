# Aho-Corasick Implementation For Topic Matching Purposes (C++)

The code is taken from [here](https://github.com/cjgdev/aho_corasick) and
you can see the examples of the algorithm there. I modified the source code and
added wildcard matching for a personal project to examine the performance. It's not
a standard modification of the algorithm.


It's not supposed to be a high-performance source code, and nothing but the algorithm
is not related to the speed here. Contribution appreciated for contribution to increase the performance.

Given the inputs:

```
hi.mom
hi.there
hi.alex.how.are.you?
hi.james.how.are.you?
hi.james.bond.how.are.you?
im.patrick
im.patrick.bond
im.james.bond
im.not.james.bond
```

And some patterns:

```
hi.#
hi.there
hi.mom
hi.+.how.are.you?
im.james.bond
im.+.bond
im.#.bond
im.#
```

The output for matching test program:

```
*** Aho-Corasick Matching Test ***
Generating input text ... done
Generating search patterns ... done
Generating trie ... done
Running =====================================================
>> result [hi.alex.how.are.you?] : true, 0ms
match: hi.+.how.are.you?
match: hi.#
=====================================================
>> result [hi.james.bond.how.are.you?] : true, 0ms
match: hi.#
=====================================================
>> result [hi.james.how.are.you?] : true, 0ms
match: hi.+.how.are.you?
match: hi.#
=====================================================
>> result [hi.mom] : true, 0ms
match: hi.mom
match: hi.#
=====================================================
>> result [hi.there] : true, 0ms
match: hi.there
match: hi.#
=====================================================
>> result [im.james.bond] : true, 0ms
match: im.james.bond
match: im.+.bond
match: im.#
=====================================================
>> result [im.not.james.bond] : true, 0ms
match: im.#.bond
match: im.#
=====================================================
>> result [im.patrick] : true, 0ms
match: im.#
=====================================================
>> result [im.patrick.bond] : true, 0ms
match: im.+.bond
match: im.#
 done
```

## License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Further Reading

- [Wikipedia](https://en.wikipedia.org/wiki/Aho%E2%80%93Corasick_string_matching_algorithm)
- [Whitepaper](ftp://163.13.200.222/assistant/bearhero/prog/%A8%E4%A5%A6/ac_bm.pdf)