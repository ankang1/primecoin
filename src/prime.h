// Copyright (c) 2013 Primecoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PRIMECOIN_PRIME_H
#define PRIMECOIN_PRIME_H

#include "main.h"

static const CBigNum bnOne = 1;
static const CBigNum bnPrimeMax = (bnOne << 2039) - 1;
static const CBigNum bnPrimeMin = (bnOne << 256);
static const unsigned int nMaxSieveSize = 1000000u;

// Generate small prime table
void GeneratePrimeTable();

// Compute primorial number p#
void Primorial(unsigned int p, CBigNum& bnPrimorial);
// Compute the first primorial number greater than or equal to bn
void PrimorialAt(CBigNum& bn, CBigNum& bnPrimorial);

// Test probable prime chain for: bnPrimeChainOrigin
// fFermatTest
//   true - Use only Fermat tests
//   false - Use Fermat-Euler-Lagrange-Lifchitz tests
// Return value:
//   true - Probable prime chain found (one of nChainLength meeting target)
//   false - prime chain too short (none of nChainLength meeting target)
bool ProbablePrimeChainTest(const CBigNum& bnPrimeChainOrigin, unsigned int nBits, bool fFermatTest, unsigned int& nChainLengthCunningham1, unsigned int& nChainLengthCunningham2, unsigned int& nChainLengthBiTwin);

static const unsigned int nFractionalBits = 24;
static const uint64 nFractionalDifficultyMax = (1llu << (nFractionalBits + 32));
static const uint64 nFractionalDifficultyMin = (1llu << 32);
static const uint64 nFractionalDifficultyThreshold = (1llu << (8 + 32));
static const unsigned int nTargetMinLength = 2;
static const unsigned int nProofOfWorkLimit = (nTargetMinLength << nFractionalBits);
unsigned int TargetGetLength(unsigned int nBits);
bool TargetSetLength(unsigned int nLength, unsigned int& nBits);
unsigned int TargetGetFractional(unsigned int nBits);
uint64 TargetGetFractionalDifficulty(unsigned int nBits);
bool TargetSetFractionalDifficulty(uint64 nFractionalDifficulty, unsigned int& nBits);
std::string TargetToString(unsigned int nBits);
unsigned int TargetFromInt(unsigned int nLength);
bool TargetGetMint(unsigned int nBits, uint64& nMint);
bool TargetGetNext(unsigned int nBits, int64 nInterval, int64 nTargetSpacing, int64 nActualSpacing, unsigned int& nBitsNext);

// Mine probable prime chain of form: n = h * p# +/- 1
bool MineProbablePrimeChain(CBlock& block, CBigNum& bnFixedMultiplier, bool& fNewBlock, unsigned int& nTriedMultiplier, unsigned int& nProbableChainLength, unsigned int& nTests, unsigned int& nPrimesHit);

// Size of a big number (in bits), times 65536
// Can be used as an approximate log scale for numbers up to 2 ** 65536 - 1
bool LogScale(const CBigNum& bn, unsigned int& nLogScale);

// Check prime proof-of-work
enum // prime chain type
{
    PRIME_CHAIN_CUNNINGHAM1 = 1u,
    PRIME_CHAIN_CUNNINGHAM2 = 2u,
    PRIME_CHAIN_BI_TWIN     = 3u,
};
bool CheckPrimeProofOfWork(uint256 hashBlockHeader, unsigned int nBits, const CBigNum& bnPrimeChainMultiplier);

// prime target difficulty value for visualization
double GetPrimeDifficulty(unsigned int nBits);
// prime chain type and length value
std::string GetPrimeChainName(CBigNum& bnPrimeChainOrigin);

// Sieve of Eratosthenes for proof-of-work mining
class CSieveOfEratosthenes
{
    unsigned int nSieveSize; // size of the sieve
    unsigned int nBits; // target of the prime chain to search for
    uint256 hashBlockHeader; // block header hash
    CBigNum bnFixedFactor; // fixed factor to derive the chain
    unsigned int nPrimeChainType; // prime chain type being sieved

    // bitmap of the sieve, index represents the variable part of multiplier
    std::vector<bool> vfCompositeInChain;

    unsigned int nPrimeSeq; // prime sequence number currently being processed
    unsigned int nCandidateMultiplier; // current candidate for power test
    unsigned int nCandidates; // number of candidates for power tests

public:
    CSieveOfEratosthenes(unsigned int nSieveSize, unsigned int nBits, unsigned int nPrimeChainType, uint256 hashBlockHeader, CBigNum& bnFixedMultiplier)
    {
        this->nSieveSize = nSieveSize;
        this->nBits = nBits;
        this->nPrimeChainType = nPrimeChainType;
        this->hashBlockHeader = hashBlockHeader;
        this->bnFixedFactor = bnFixedMultiplier * CBigNum(hashBlockHeader);
        nPrimeSeq = 0;
        vfCompositeInChain = std::vector<bool> (1000000, false);
        nCandidateMultiplier = 0;
        nCandidates = nSieveSize;
    }

    // Get total number of candidates for power test
    unsigned int GetCandidateCount()
    {
        return nCandidates;
    }

    // Scan for the next candidate multiplier (variable part)
    // Return values:
    //   True - found next candidate; nVariableMultiplier has the candidate
    //   False - scan complete, no more candidate and reset scan
    bool GetNextCandidateMultiplier(unsigned int& nVariableMultiplier)
    {
        while (true)
        {
            nCandidateMultiplier++;
            if (nCandidateMultiplier >= nSieveSize)
            {
                nCandidateMultiplier = 0;
                return false;
            }
            if (!vfCompositeInChain[nCandidateMultiplier])
            {
                nVariableMultiplier = nCandidateMultiplier;
                return true;
            }
        }
    }

    // Weave the sieve for the next prime in table
    // Return values:
    //   True  - weaved another prime; nComposite - number of composites removed
    //   False - sieve already completed
    bool Weave(unsigned int& nComposite);
};

#endif
