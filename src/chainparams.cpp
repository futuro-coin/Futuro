// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175, ver=1, hashPrevBlock=0000000000000000000000000000000000000000000000000000000000000000, hashMerkleRoot=3a7e038922f88887c9337b643cbf91c11e12707dd2f75fe3aa7d2d9d3fdf7ba3, nTime=1517356800, nBits=1e0ffff0, nNonce=227856, vtx=1)
 *   CTransaction(hash=3a7e038922, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(0000000000000000000000000000000000000000000000000000000000000000, 4294967295), coinbase 04ffff001d01044c584e5a20486572616c642033312f4a616e2f3230313820436f6f6b696e672074686520426f6f6b7320706f64636173743a2057687920626974636f696e20636f756c64206265206d61696e73747265616d2062792032303139)
 *     CTxOut(nValue=50.00000000, scriptPubKey=76a914793346dba7d4bc56cfe2c674)
 *
 * */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "NZ Herald 31/Jan/2018 Cooking the Books podcast: Why bitcoin could be mainstream by 2019";
    const CScript genesisOutputScript = CScript() << OP_DUP << OP_HASH160 << ParseHex("793346dba7d4bc56cfe2c6742bdb02f3144130bd")  <<  OP_EQUALVERIFY << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */


class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 2 * 525600; // Halving per 2 years (one block per minute - number of blocks per year is 360 * 24 * 60 = 525600)
        consensus.nInstantSendKeepLock = 24;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175");
        consensus.powLimit = uint256S("00000fffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetSpacing = 60;         // FuturoCoin: value given in seconds, i.e. 1 minute
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916;   // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016;      // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;  // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;      // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1505557879;     // 2017-09-16
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1537093879;     // 2018-09-16

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1508025600;       // Oct 15th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1539561600;     // Oct 15th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 3226;      // 80% of 4032

        // Deployment of FIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nStartTime = 1584662400; // Mar 20th, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nTimeout = 1616198400; // Mar 20th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nThreshold = 50; // 50% of 100

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        /**
         * For FuturoCoin the message start string is based on "FTRO" string shifted
         * by 0x80 to reach ASCII extended table values.
         */
        pchMessageStart[0] = 0xCF; // 'O' + 0x80
        pchMessageStart[1] = 0xD2; // 'R' + 0x80
        pchMessageStart[2] = 0xD4; // 'T' + 0x80
        pchMessageStart[3] = 0xC6; // 'F' + 0x80
        vAlertPubKey = ParseHex("04ce461ade8b748e623c0018d212a7bf618ea7efd0f4341b33257a998355802924add8931ac6e5e1e113f87308f0e279e5a22968a3588fe949cd07b5e5c7c9c7be");
        nDefaultPort = 9009;
        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1517356800UL, 227856UL, 0x1e0ffff0, 1, 50 * COIN);

        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175"));
        assert(genesis.hashMerkleRoot == uint256S("0x3a7e038922f88887c9337b643cbf91c11e12707dd2f75fe3aa7d2d9d3fdf7ba3"));

        vSeeds.push_back(CDNSSeedData("futurocoin.com", "mainnet-dnsseed.futurocoin.com"));
        vSeeds.push_back(CDNSSeedData("fncoins.com", "mainnet-dnsseed.fncoins.com"));

        // FuturoCoin addresses start with 'F'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,36);
        // FuturoCoin script addresses start with '6'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,13);
        // FuturoCoin private keys start with '6' or 'F'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,164);
        // FuturoCoin BIP32 pubkeys start with 'xpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        // FuturoCoin BIP32 prvkeys start with 'xprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // FuturoCoin BIP44 coin type is '0x88888'
        nExtCoinType = 0x88888;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false; // ToDo: set to true
        fTestnetToBeDeprecatedFieldRPC = false;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour
        strSporkPubKey = "04cc4d35e3133012760bbfeb44ce7da3323710b0c0f07c19fa95f7e3c4d70bf167767d3a02731fe5aadd168bf14b77217c55f16aec1e47365fabfad03558d427ce";
        strMasternodeListManagerPubKey = "04218eb27f45a2bf4b6efd354bb3084ec28446ecde62e0c9bd42073df858db50cf3e1ad7ba5b49b24423c356c2bcaf2ba30d26d4cf25d6d90c611676664fd8a77d";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 0, uint256S("0x00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175")),
            1517356800UL,     // * UNIX timestamp of last checkpoint block
            0,//1998064,    // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
            2800            // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 2 * 525600; // Halving per 2 years (one block per minute - number of blocks per year is 360 * 24 * 60 = 525600)
        consensus.nInstantSendKeepLock = 6;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175");
        consensus.powLimit = uint256S("00000fffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetSpacing = 60;     // FuturoCoin: value given in seconds, i.e. 1 minute
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1506556800; // September 28th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538092800; // September 28th, 2018

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1505692800; // Sep 18th, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1537228800; // Sep 18th, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 50; // 50% of 100

        // Deployment of FIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nStartTime = 1584316800; // Mar 12th, 2020
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nTimeout = 1615507200; // Mar 12th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nWindowSize = 50;
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nThreshold = 25; // 50% of 50

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175");

        /**
         * For FuturoCoin the message start string is based on "FTRT" string shifted
         * by 0x80 to reach ASCII extended table values.
         */
        pchMessageStart[0] = 0xD4; // 'T' + 0x80
        pchMessageStart[1] = 0xD2; // 'R' + 0x80
        pchMessageStart[2] = 0xD4; // 'T' + 0x80
        pchMessageStart[3] = 0xC6; // 'F' + 0x80
        vAlertPubKey = ParseHex("04df473119137884b6b57ea26dc53fad605f3f14315cd0041da30651e777d5f8a4228e3e701508aba3216685657465bbd64a5200163f3a4135af15b3189ccbc6dc");
        nDefaultPort = 19009;
        nMaxTipAge = 0x7fffffff; // allow mining on top of old blocks for testnet
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1517356800UL, 227856UL, 0x1e0ffff0, 1, 50 * COIN);

        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175"));
        assert(genesis.hashMerkleRoot == uint256S("0x3a7e038922f88887c9337b643cbf91c11e12707dd2f75fe3aa7d2d9d3fdf7ba3"));

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("futurocoin.com", "testnet-dnsseed.futurocoin.com"));
        vSeeds.push_back(CDNSSeedData("fncoins.com", "testnet-dnsseed.fncoins.com"));

        // Testnet FuturoCoin addresses start with 'f'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,95);
        // Testnet FuturoCoin script addresses start with '7' or '8'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,17);
        // Testnet private keys start with '8' or 'c'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,223);
        // Testnet FuturoCoin BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Testnet FuturoCoin BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Testnet FuturoCoin BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes
        strSporkPubKey = "046aa9cc60b75a5b92cdbdf7cb8e9eaadb31ece7618af061f264e4884603ba699aa293d02433aa90df2ff4dd060cfd86f13cc9ca29ee44b4d4c57d8f1f87fc2860";
        strMasternodeListManagerPubKey = "04b23afd335c7ccc9590d1d592d8340051c6e02b244e8fbde947fb17e005a98e37fcc60f9e13be6d95dc1fc5a80dcfafc8decde4dfbbbf59635adb22973bf45f06";

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 0, uint256S("0x00000bb3fa6e7040e32c0f8e8218e928c37e177139b62ae1daba803e0543c175")),
            1517356800UL,    // * UNIX timestamp of last checkpoint block
            0,//168590,    // * total number of transactions between genesis and last checkpoint
                           //   (the tx=... number in the SetBestChain debug.log lines)
            500            // * estimated number of transactions per day after checkpoint
        };

    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 30; // Halving per 30 minutes (one block per minute)
        consensus.nInstantSendKeepLock = 6;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetSpacing = 60;     // FuturoCoin: value given in seconds, i.e. 1 minute
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_FIP0001].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x418b3663e91147e6bdc6e4890da9a62075d435ead27abdaa2ae90b0230f82eed");

        /**
        * For FuturoCoin the message start string is based on "FTRR" string shifted
        * by 0x80 to reach ASCII extended table values.
        */
        pchMessageStart[0] = 0xD2; // 'R' + 0x80
        pchMessageStart[1] = 0xD2;   // 'R' + 0x80
        pchMessageStart[2] = 0xD4;   // 'T' + 0x80
        pchMessageStart[3] = 0xC6;   // 'F' + 0x80

        vAlertPubKey = ParseHex("048a35711848f57780759431ba221d05a5734f4b6fa23fd85c7fca28f8e5b4e62d198fb9d890f0d1606686e43534a50d2ccbadd06b9c1b740184442cb93df05841");

        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 0; // never delay GETHEADERS in regtests
        nDefaultPort = 19004;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1517356800UL, 2UL, 0x207fffff, 1, 50 * COIN);

        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == uint256S("0x418b3663e91147e6bdc6e4890da9a62075d435ead27abdaa2ae90b0230f82eed"));
        assert(genesis.hashMerkleRoot == uint256S("0x3a7e038922f88887c9337b643cbf91c11e12707dd2f75fe3aa7d2d9d3fdf7ba3"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        strSporkPubKey = "0449ba732f730a35dd8406f32b9513b6ebe662ae38124cbd89367309b0004184ff280c9d0544e6a07a2fd4d78cb2716e026e1424c9566e458f1a701b66ebef4e45";
        strMasternodeListManagerPubKey = "04847d2dc194ad41885cb07bce518fc461734f98b4bc50d35cd7969348b1df8a5f77d0a876fc4c905740e72ead2f17ca1841b2af70a4913b8e998b33c973d4123b";

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x418b3663e91147e6bdc6e4890da9a62075d435ead27abdaa2ae90b0230f82eed")),
            1517356800UL,
            0,
            0
        };
        // Regtest FuturoCoin addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Regtest FuturoCoin script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Regtest private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Regtest FuturoCoin BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Regtest FuturoCoin BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Regtest FuturoCoin BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;
   }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}
