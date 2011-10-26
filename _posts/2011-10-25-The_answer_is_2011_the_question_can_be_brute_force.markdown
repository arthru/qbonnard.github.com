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

#include <iostream>
#include <cmath>
#include <string>
#include <sstream>

// data structure to count from 0 to UPTO^SIZE in base UPTO+1
// the first FROMINDEX positions remain 0
template <typename T, int SIZE, T UPTO, int FROMINDEX = 0>
struct ExhaustiveEnumeration {
public:
	ExhaustiveEnumeration() {
		for(int i=0; i<SIZE; ++i) mData[i]=0;
	}
	T get(int pIndex) const { return mData[pIndex]; }

	bool increment() {
		int i = FROMINDEX;
		while(i < SIZE && mData[i] >= UPTO) mData[i++] = 0;
		if (i >= SIZE) return false;
		++mData[i];
		return true;
	}

protected:
	T mData[SIZE];
};

// datastructure to count from 0 to 012..SIZE-1 in "base" 123..SIZE
template <typename T, int SIZE>
struct ExhaustiveEnumerationUpToIndex {
public:
	ExhaustiveEnumerationUpToIndex() {
		for(int i=0; i<SIZE; ++i) mData[i]=0;
	}
	T get(int pIndex) const { return mData[pIndex]; }

	bool increment() {
		int i = 1;
		while(i < SIZE && mData[i] >= i) mData[i++] = 0;
		if (i >= SIZE) return false;
		++mData[i];
		return true;
	}

protected:
	T mData[SIZE];
};

namespace {
	// I'm too lazy to use some kind of BigInt...
	typedef double Number;

	// ...but not enough to hard code factorial
	const int MAXFACTARG = 20;
	const long factorial[1+MAXFACTARG] = {1,
		1,2,6,24,120,
		720,5040,40320,362880,3628800,
		39916800, 479001600, 6227020800, 87178291200, 1307674368000,
		20922789888000, 355687428096000, 6402373705728000, 121645100408832000,
		2432902008176640000};

	// floating point equality
	inline bool equals(Number a, Number b, Number epsilon) {
		Number tDelta = a-b;
		return (-epsilon < tDelta && tDelta < epsilon);
	}

	// test whether a floating point is (clos enough from) an integer
	inline bool isInt(Number a, Number epsilon) {
		return equals(a, static_cast<Number>((long) a), epsilon);
	}
}

// n = NOPERANDS, f = MAXFRAC, s = MAXSQRT
template<int NOPERANDS = 5, char MAXFRAC = 2, char MAXSQRT = 2>
class CandidateEnumeration {
public:
	CandidateEnumeration():mPairPositions(),mOps(),mFacs(),mSqrt() {}

	// see decode(EvaluationResult) for a description
	enum EvaluationResult {
		CORRECT = 1,
		DIVISION = 2,
		FAC_TOO_MUCH = 4,
		FAC_NON_INT = 8,
		FAC_TOO_BIG = 16,
		SQRT = 32,
		INCORRECT = 64};

	// mainly do{evaluateCandidate()} while (generateNextCandidate());
	// the rest is just progress output
	// 
	// ALL choose whether all candidates should be tried,
	// or if the search can be stopped after the first success
	// 
	// VERBOSITY is the bitwise or combination of the EvaluationResult types
	// which should be printed
	template <bool ALL, int VERBOSITY>
	void search(Number expectation, const Number epsilon = 0.1) {
		EvaluationResult result;
		long tRemaingCandidates = factorial[NBINOPERATIONS]
				          *pow(NBINOPERATIONTYPES,NBINOPERATIONS)
				          *pow(MAXFRAC+1,NBINOPERATIONS+NOPERANDS-2)
				          *pow(MAXSQRT+1,NBINOPERATIONS+NOPERANDS-1);
		std::cout << tRemaingCandidates << " candidates" << std::endl;
		do {
			result = evaluateCandidate(expectation, epsilon);
			if (result & VERBOSITY) {
				std::cout << decode(result) << ": ";
				printCandidate();
			}

			--tRemaingCandidates;
			if (ALL && !(tRemaingCandidates%100000000)) {
				std::cout << tRemaingCandidates << " remaining candidates"
						<< std::endl;
			}
		} while (generateNextCandidate() && (ALL || result != CORRECT));
	}

	void printCandidate() {
		std::string tCandidate[NOPERANDS];
		for (int i = 0; i < NOPERANDS; ++i) {
			std::stringstream tOperand;
			tOperand << (i+1);
			tCandidate[i] = buildNode(tOperand.str(),
					mSqrt.get(i), mFacs.get(i));
		}
		for (int i = NBINOPERATIONS-1; i >= 0; --i) {
			int tPairPositionIndex = mPairPositions.get(i);
			tCandidate[tPairPositionIndex] = '('
					+ tCandidate[tPairPositionIndex]
					+ decode(mOps.get(i))
					+ tCandidate[tPairPositionIndex+1] + ')';
			tCandidate[tPairPositionIndex] = buildNode(
					tCandidate[tPairPositionIndex],
					mSqrt.get(NOPERANDS+i), mFacs.get(NOPERANDS+i));
			for(int j=tPairPositionIndex+1; j<NBINOPERATIONS; ++j) {
				tCandidate[j] = tCandidate[j+1];
			}
		}
		std::cout << tCandidate[0] << std::endl;
	}

	// helper of printCandidate()
	std::string buildNode(std::string pCore, int pNSqrt, int pNFac) {
		if (pNSqrt>0) pCore = '('+pCore+')';
		for (int k = 0; k<pNSqrt; ++k) pCore="√"+pCore;
		if (pNFac>0) pCore = '('+pCore+')';
		for (int k = 0; k<pNFac; ++k) pCore+='!';
		return pCore;
	}

protected:

	// decode the current candidate and check whether it's the expected result
	// or abort if an operation is not possible
	EvaluationResult evaluateCandidate(Number expectation, const Number epsilon = 0.1) {

		// initial numbers and corresponding factorials and square roots
		Number tResults[NOPERANDS];
		for (int i = 0; i<NOPERANDS; ++i) {
			tResults[i]=i+1;

			for (int k = 0; k<mSqrt.get(i); ++k) tResults[i] = sqrt(tResults[i]);

			for (int k = 0; k<mFacs.get(i); ++k) {
				if (tResults[i] > MAXFACTARG) return FAC_TOO_MUCH;
				tResults[i] = factorial[(int) tResults[i]];
			}
		}

		// fetch the operation type and operand pair, compute result
		// and apply corresponding factorials and square roots
		// 
		// the result is stored in the cell of the first operand,
		// and the intermediary results after the second operand are shifted
		for (int i = NBINOPERATIONS-1; i >= 0; --i) {

			// binary operation
			int tPairPositionIndex = mPairPositions.get(i);
			switch (mOps.get(i)) {
				case 0: tResults[tPairPositionIndex] +=
						tResults[tPairPositionIndex+1]; break;
				case 1: tResults[tPairPositionIndex] -=
						tResults[tPairPositionIndex+1]; break;
				case 2: tResults[tPairPositionIndex] *=
						tResults[tPairPositionIndex+1]; break;
				case 3: tResults[tPairPositionIndex] /=
						tResults[tPairPositionIndex+1]; break;
				case 4: tResults[tPairPositionIndex] =
						pow(tResults[tPairPositionIndex],
								tResults[tPairPositionIndex+1]); break;
				default: std::cerr << "Unexpected opcode:"
						<< (int) mOps.get(i) << std::endl; break;
			}
			if (std::isnan(tResults[tPairPositionIndex])
				|| std::isinf(tResults[tPairPositionIndex])) return DIVISION;

			// square roots
			if (mSqrt.get(NOPERANDS+i) > 0
					&& tResults[tPairPositionIndex] < 0.0) return SQRT;
			for (int k = 0; k<mSqrt.get(NOPERANDS+i); ++k) {
				tResults[tPairPositionIndex] =
						sqrt(tResults[tPairPositionIndex]);
			}

			// factorial
			if (mFacs.get(NOPERANDS+i) > 0
					&& !isInt(tResults[tPairPositionIndex], epsilon)) {
				return FAC_NON_INT;
			}
			for (int k = 0; k<mFacs.get(NOPERANDS+i); ++k) {
				if (0 > tResults[tPairPositionIndex]
				    || tResults[tPairPositionIndex] > MAXFACTARG) {
					return FAC_TOO_BIG;
				}
				tResults[tPairPositionIndex] =
						factorial[(int) tResults[tPairPositionIndex]];
			}

			// shifting the intermediate results
			for(int j=tPairPositionIndex+1; j<NBINOPERATIONS; ++j) {
				tResults[j] = tResults[j+1];
			}
		}

		return equals(tResults[0],expectation, epsilon)?CORRECT:INCORRECT;
	}

	// chains the increments
	bool generateNextCandidate() {
		return mOps.increment()
				|| mPairPositions.increment()
				|| mFacs.increment()
				|| mSqrt.increment();
	}

	// opcode to char
	static char decode(char op) {
		switch (op) {
			case 0: return '+';
			case 1: return '-';
			case 2: return '*';
			case 3: return '/';
			case 4: return '^';
			default: return '?';
		}
	}

	// describes an EvaluationResult
	static const char *decode(EvaluationResult result) {
		switch (result) {
		case CORRECT:
			return "Solution found";
		case DIVISION:
			return "Division impossible to perform on an operator";
		case FAC_TOO_MUCH:
			return "Factorial impossible to perform on a base operand";
		case FAC_NON_INT:
			return "Factorial impossible to perform on non integer";
		case FAC_TOO_BIG:
			return "Factorial impossible to perform on an operation result";
		case SQRT:
			return "Square root impossible to perform";
		case INCORRECT:
			return "Invalid candidate";
		default:
			return "Unexpected result code";
		}
	}

	// count of binary operations (n-1)
	static const int NBINOPERATIONS = NOPERANDS-1;
	// count of types of binary operations (k)
	static const int NBINOPERATIONTYPES = 5;

	// position of the pair of operands of each binary operation
	ExhaustiveEnumerationUpToIndex<char, NBINOPERATIONS> mPairPositions;
	// type of each binary operation
	ExhaustiveEnumeration<char, NBINOPERATIONS, NBINOPERATIONTYPES-1> mOps;
	// count of factorials to apply on each initial number or operation result
	// it's useless to try to increase the count of factorials applied to 1 and
	// 2, so we skip the first 2 indices in the enumeration
	ExhaustiveEnumeration<char, NOPERANDS+NBINOPERATIONS, MAXFRAC, 2> mFacs;
	// count of square roots to apply on each initial number or operation result
	// it's useless to try to increase the count of square root applied to 1
	// so we skip the first  index in the enumeration
	ExhaustiveEnumeration<char, NOPERANDS+NBINOPERATIONS, MAXSQRT, 1> mSqrt;
};

int main() {
	typedef CandidateEnumeration<5,2,2> Candidates;
	Candidates tCandidates;
	tCandidates.search<true, Candidates::CORRECT>(2011, 0.001);
	std::cout << "Done" << std::endl;
	return 0;
}

{% endhighlight %}

Limitations
===========

The main limitation of this code is that the number representation (`double`) does not allow to compute the factorial of numbers above 20, which aborts the computation of potentially correct candidates. The use of `double` also results in false positive due to rounding errors.

The limitation to this brute force algorithm in general is that there is obviously no end in the increase of the number of application of factorial and square root. I can't think of an upper bound on `f` and `s` after which there is no possible solution anymore.

That leaves the door open to solutions with less than 5 initial numbers. I would bet that there aren't any, but I'm not sure how to prove it.

Finally, this code can easily be used to play "The answer is 2012". I wonder if there are years for which this code is unacceptabely long, i.e. years that would require "a lot" of initial numbers...
