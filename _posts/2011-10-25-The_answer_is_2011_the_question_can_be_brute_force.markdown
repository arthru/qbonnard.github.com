---
layout: post
title: The answer is 2011. The question can be brute force.
---

This post describes a brute-force method to find that `(1+2)!! + 3!^4 - 5 = 2011` and gives an implementation in C++.

The story
=========

The [Fondation Cartier](http://fondation.cartier.com/?_lang=en) in Paris created an exhibition called _Mathematics - A Beautiful Elsewhere_ and invited mathematicians and artists. Among them, [Takeshi Kitano](http://en.wikipedia.org/wiki/Takeshi_Kitano) proposed a game called "La réponse est 2011" (The answer is 2011). The rules are simple:

* The player can use the as many number as needed, but in the natural order: 1, 2, 3, 4, 5, etc.
* These numbers can be the operands of `+`, `-`, `*`, `/`, `^` (power), `!` (factorial), and square root. Parenthesis can be added freely to order these operations.
* The result should be 2011, and the formula should use as few numbers as possible.

Takeshi Kitano proposed several answers, the shortest being:

`(1+2+3)^4 + (5*6*7*8) – (9*10*11) + 12 + 13 = 2011`

The company of Frédéric, my Ph.D. supervisor, designed the touchscreen allowing to play this game at the exhibition. Frédéric, who tells [his side of the story](http://fkaplan.wordpress.com/2011/10/24/la-reponse-est-2011/) in his blog (in French), was warning me at a coffee break that designing an addictive game (just try it!) is actually quite dangerous. When someone realized that the given answer could be improved, the company started losing all its developers in a obsessive compulsive vortex. He also told me that the current best answer used only 6 numbers. My mission in life became clear: save my fellow nerds using brute-force. Turns out, I just ruined their happiness. I'll try to make up for it by sharing the fun I had (solving the problem, not ruining their happiness). 

Enumerating all the candidates
==============================

Well, the method is pretty clear: brute force means trying every possibility and checking its validity. The only tricky part is how to enumerate all the candidates. This is simpler if we consider only the binary operations first, and consider factorial and square root in a second step.

Noting that applying one of the binary operations will leave us with one less term, we will have to apply `n-1` operations on `n` initial numbers. The orders of the initial numbers is fixed, but the parentheses allow the operation to be between any subsequent terms. In other words, the first operation has `n-1` possible pairs of operands, the second operation has `n-2`, etc. Each operation can be one of the `k` different (here `k = 5`), which gives us

`(n-1)! * k^n`

possibilities to try. It's good that `n` is upper-bounded by `6`...

Now the problem is that unary operations can be applied as often as possible. They can be applied to any of the initial numbers, and to any of the results of the binary operations. However, no factorial is a square (except 1), so we can apply the square root first (as many times as we want), and then the factorial, without trying to interleave them. If we apply up to `s` times square root on each possible term (i.e. the `n` initial numbers and the `n-1` results of binary operations), and up to `f` times the factorial on each possible term, that gives us a grand total of

`(n-1)! * k^n * (s+1)^(n+n-1) * (f+1)^(n+n-1)`

possibilities. It's really good that `n` is upper-bounded by `6`...

To summarize, we can describe a candidate with:

* a value in the `[1,n-1]` range, a value in the `[1,n-2]` range, etc. to code which pair of subsequent operand is used in each of the `n-1` binary operation,
* `n-1` values in the `[1,k]` range to code the type of each of the `n-1` binary operations,
* `n` values in the `[0,s]` range to code how many square roots are applied to the `n` initial values, and `n-1` more in the `[0,s]` range for the square root applications of each result of the binary operations,
* similarly, `n + n-1` values in the `[0,f]` range for the factorials.

Implementation overview
=======================

For the last three item of the previous list, I wrote `ExhaustiveEnumeration`, a data structure that basically counts in the base corresponding to the range. For example, if `n=5` and `f = 2`, the data structure corresponding to the last item of the previous list will count from `000000000` to `222222222` in base `3`.
For the first item of the list, the same idea of counting in a different base is used, except the base varies with the position. For example, if `n=5`, the data structure will count from `0000` to `0123`. Both data structures simply provide an accessor to the content, and a method that increments the content and notifies whether a full cycle has just been completed.

Using these data structure, I wrote `CandidateEnumeration`, a class that fully describes a candidate as described in the previous list. It has a search method that evaluates the current candidate, and generates the next until it fits the criteria. To generate the next candidate, it chains the increment methods of the `ExhaustiveEnumeration`: if one indicates that a full cycle has been completed, it calls the next.

The evaluation method creates an array of the `n` initial numbers, and applies the operations coded in the `ExhaustiveEnumeration` members, until the array is reduced to the ultimate result. If an illegal operation (e.g. divide by 0 or negative square root) is detected, the evaluation is aborted.

The rest of the code is basically here only to output the result or the progress.

Code
====

{% highlight cpp %}
{% include 2011-10-25/main.cpp %}
{% endhighlight %}

Limitations
===========

The main limitation of this code is that the number representation (`double`) does not allow to compute the factorial of numbers above 20, which aborts the computation of potentially correct candidates. The use of `double` also results in false positive due to rounding errors.

The limitation to this brute force algorithm in general is that there is obviously no end in the increase of the number of application of factorial and square root. I can't think of an upper bound on `f` and `s` after which there is no possible solution anymore.

That leaves the door open to solutions with less than 5 initial numbers. I would bet that there aren't any, but I'm not sure how to prove it.

Finally, this code can easily be used to play "The answer is 2012". I wonder if there are years for which this code is unacceptabely long, i.e. years that would require "a lot" of initial numbers...
