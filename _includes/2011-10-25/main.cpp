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
	EvaluationResult evaluateCandidate(
			Number expectation, const Number epsilon = 0.1) {

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
	tCandidates.search<true, Candidates::CORRECT>(2011);
	std::cout << "Done" << std::endl;
	return 0;
}

