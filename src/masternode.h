// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MASTERNODE_H
#define MASTERNODE_H

#include "key.h"
#include "validation.h"
#include "spork.h"
#include "base58.h"

class CMasternode;
class CMasternodeBroadcast;
class CConnman;

static const int MASTERNODE_CHECK_SECONDS               =   5;
static const int MASTERNODE_MIN_MNB_SECONDS             =   5 * 60;
static const int MASTERNODE_MIN_MNP_SECONDS             =  10 * 60;
static const int MASTERNODE_EXPIRATION_SECONDS          =  65 * 60;
static const int MASTERNODE_NEW_START_REQUIRED_SECONDS  = 180 * 60;

static const int MASTERNODE_POSE_BAN_MAX_SCORE          = 5;

//
// The Masternode Ping Class : Contains a different serialize method for sending pings from masternodes throughout the network
//

class CMasternodePing
{
public:
    CPubKey pubKeyMasternode{};
    CTxIn vin{};
    uint256 blockHash{};
    int64_t sigTime{}; //mnb message times
    std::vector<unsigned char> vchSig{};

    CMasternodePing() = default;

    CMasternodePing(const COutPoint& outpoint);
    CMasternodePing(const CPubKey& pubKey);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED active
            READWRITE(vin);
        } else {
            READWRITE(pubKeyMasternode);
        }

        READWRITE(blockHash);
        READWRITE(sigTime);
        READWRITE(vchSig);
    }

    uint256 GetHash() const
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);

        if (fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED active
            ss << vin;
        } else {
            ss << pubKeyMasternode;
        }

        ss << sigTime;
        return ss.GetHash();
    }

    bool IsExpired() const { return GetAdjustedTime() - sigTime > MASTERNODE_NEW_START_REQUIRED_SECONDS; }

    bool SignMnRel(const CKey& keyMasternode, const CPubKey& pubKeyMasternode);
    bool Sign(const CKey& keyMasternode, const CPubKey& pubKeyMasternode);
    bool CheckSignatureMnRel(CPubKey& pubKeyMasternode, int &nDos);
    bool CheckSignature(CPubKey& pubKeyMasternode, int &nDos);
    bool SimpleCheckMnRel(int& nDos);
    bool SimpleCheck(int& nDos);
    bool CheckAndUpdateMnRel(CMasternode* pmn, bool fFromNewBroadcast, int& nDos, CConnman& connman);
    bool CheckAndUpdate(CMasternode* pmn, bool fFromNewBroadcast, int& nDos, CConnman& connman);
    void Relay(CConnman& connman);
};

inline bool operator==(const CMasternodePing& a, const CMasternodePing& b)
{
    return (fMasterNodesReleased ? a.vin == b.vin : a.pubKeyMasternode == b.pubKeyMasternode) && a.blockHash == b.blockHash;
}
inline bool operator!=(const CMasternodePing& a, const CMasternodePing& b)
{
    return !(a == b);
}

struct masternode_info_t
{
    // Note: all these constructors can be removed once C++14 is enabled.
    // (in C++11 the member initializers wrongly disqualify this as an aggregate)
    masternode_info_t() = default;
    masternode_info_t(masternode_info_t const&) = default;

    masternode_info_t(int activeState, int protoVer, int64_t sTime) :
        nActiveState{activeState}, nProtocolVersion{protoVer}, sigTime{sTime} {}

    // SPORK_14_MNODES_RELEASE_ENABLED
    masternode_info_t(int activeState, int protoVer, int64_t sTime,
                      COutPoint const& outpoint, CService const& addr,
                      CPubKey const& pkCollAddr, CPubKey const& pkMN,
                      int64_t tWatchdogV = 0):
        nActiveState{activeState}, nProtocolVersion{protoVer}, sigTime{sTime},
        vin{outpoint}, addr{addr},
        pubKeyCollateralAddress{pkCollAddr}, pubKeyMasternode{pkMN} {}

    // SPORK_14_MNODES_RELEASE_ENABLED
    masternode_info_t(int activeState, int protoVer, int64_t sTime,
                      COutPoint const& outpoint, CService const& addr,
                      CPubKey const& pkCollAddr, CPubKey const& pkMN,
                      CBitcoinAddress const& payeeAddress,
                      int64_t tWatchdogV = 0):
        nActiveState{activeState}, nProtocolVersion{protoVer}, sigTime{sTime},
        vin{outpoint}, addr{addr},
        pubKeyCollateralAddress{pkCollAddr}, pubKeyMasternode{pkMN}, payee{payeeAddress} {}

    masternode_info_t(int activeState, int protoVer, int64_t sTime,
                     CService const& addr,
                     CPubKey const& pkMN,
                     CBitcoinAddress const& payeeAddress,
                     int64_t tWatchdogV = 0):
        nActiveState{activeState}, nProtocolVersion{protoVer}, sigTime{sTime},
        addr{addr},
        pubKeyMasternode{pkMN}, payee{payeeAddress} {}

    int nActiveState = 0;
    int nProtocolVersion = 0;
    int64_t sigTime = 0; //mnb message time

    CTxIn vin{};
    CService addr{};
    CPubKey pubKeyCollateralAddress{};
    CPubKey pubKeyMasternode{};
    CBitcoinAddress payee{};

    int64_t nLastDsq = 0; //the dsq count from the last dsq broadcast of this node
    int64_t nTimeLastChecked = 0;
    int64_t nTimeLastPaid = 0;
    int64_t nTimeLastPing = 0; //* not in CMN
    bool fInfoValid = false; //* not in CMN
};

//
// The Masternode Class. It contains the signature to prove it's the one who
// own that ip address and code for calculating the payment election.
//
class CMasternode : public masternode_info_t
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

public:
    enum state {
        MASTERNODE_PRE_ENABLED,
        MASTERNODE_ENABLED,
        MASTERNODE_EXPIRED,
        MASTERNODE_OUTPOINT_SPENT,
        MASTERNODE_UPDATE_REQUIRED,
        MASTERNODE_NEW_START_REQUIRED,
        MASTERNODE_POSE_BAN
    };

    enum CollateralStatus {
        COLLATERAL_OK,
        COLLATERAL_UTXO_NOT_FOUND,
        COLLATERAL_INVALID_AMOUNT
    };


    CMasternodePing lastPing{};
    std::vector<unsigned char> vchSig{};

    uint256 nCollateralMinConfBlockHash{};
    int nBlockLastPaid{};
    int nPoSeBanScore{};
    int nPoSeBanHeight{};
    bool fUnitTest = false;

    CMasternode();
    CMasternode(const CMasternode& other);
    CMasternode(const CMasternodeBroadcast& mnb);
    CMasternode(CService addrNew, COutPoint outpointNew, CPubKey pubKeyCollateralAddressNew, CPubKey pubKeyMasternodeNew, int nProtocolVersionIn);
    CMasternode(CService addrNew, CPubKey pubKeyMasternodeNew, CBitcoinAddress payee, int nProtocolVersionIn);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        LOCK(cs);

        if (fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED active
            READWRITE(vin);
            READWRITE(addr);
            READWRITE(pubKeyCollateralAddress);
        } else {
            READWRITE(addr);
        }

        READWRITE(pubKeyMasternode);

        if (!fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED not active
            READWRITE(payee);
        }

        READWRITE(lastPing);
        READWRITE(vchSig);
        READWRITE(sigTime);
        READWRITE(nLastDsq);
        READWRITE(nTimeLastChecked);
        READWRITE(nTimeLastPaid);
        READWRITE(nActiveState);
        READWRITE(nCollateralMinConfBlockHash);
        READWRITE(nBlockLastPaid);
        READWRITE(nProtocolVersion);
        READWRITE(nPoSeBanScore);
        READWRITE(nPoSeBanHeight);
        READWRITE(fUnitTest);
    }

    // CALCULATE A RANK AGAINST OF GIVEN BLOCK
    arith_uint256 CalculateScore(const uint256& blockHash);

    bool UpdateFromNewBroadcast(CMasternodeBroadcast& mnb, CConnman& connman);

    static CollateralStatus CheckCollateral(const COutPoint& outpoint);
    static CollateralStatus CheckCollateral(const COutPoint& outpoint, int& nHeightRet);

    void CheckMnRel(bool fForce = false);
    void Check(bool fForce = false);

    bool IsBroadcastedWithin(int nSeconds) { return GetAdjustedTime() - sigTime < nSeconds; }

    bool IsPingedWithin(int nSeconds, int64_t nTimeToCheckAt = -1)
    {
        if(lastPing == CMasternodePing()) return false;

        if(nTimeToCheckAt == -1) {
            nTimeToCheckAt = GetAdjustedTime();
        }
        return nTimeToCheckAt - lastPing.sigTime < nSeconds;
    }

    bool IsEnabled() { return nActiveState == MASTERNODE_ENABLED; }
    bool IsPreEnabled() { return nActiveState == MASTERNODE_PRE_ENABLED; }
    bool IsPoSeBanned() { return nActiveState == MASTERNODE_POSE_BAN; }
    // NOTE: this one relies on nPoSeBanScore, not on nActiveState as everything else here
    bool IsPoSeVerified() { return nPoSeBanScore <= -MASTERNODE_POSE_BAN_MAX_SCORE; }
    bool IsExpired() { return nActiveState == MASTERNODE_EXPIRED; }
    bool IsOutpointSpent() { return nActiveState == MASTERNODE_OUTPOINT_SPENT; }
    bool IsUpdateRequired() { return nActiveState == MASTERNODE_UPDATE_REQUIRED; }
    bool IsNewStartRequired() { return nActiveState == MASTERNODE_NEW_START_REQUIRED; }

    static bool IsValidStateForAutoStart(int nActiveStateIn)
    {
        return  nActiveStateIn == MASTERNODE_ENABLED ||
                nActiveStateIn == MASTERNODE_PRE_ENABLED ||
                nActiveStateIn == MASTERNODE_EXPIRED;
    }

    bool IsValidForPayment()
    {
        if (nActiveState == MASTERNODE_ENABLED) {
            return true;
        }

        return false;
    }

    /// Is the input associated with collateral public key? (and there is 100000 FUTURO - checking if valid masternode)
    bool IsInputAssociatedWithPubkey();

    bool IsValidNetAddr();
    static bool IsValidNetAddr(CService addrIn);

    void IncreasePoSeBanScore() { if(nPoSeBanScore < MASTERNODE_POSE_BAN_MAX_SCORE) nPoSeBanScore++; }
    void DecreasePoSeBanScore() { if(nPoSeBanScore > -MASTERNODE_POSE_BAN_MAX_SCORE) nPoSeBanScore--; }
    void PoSeBan() { nPoSeBanScore = MASTERNODE_POSE_BAN_MAX_SCORE; }

    masternode_info_t GetInfo();

    static std::string StateToString(int nStateIn);
    std::string GetStateString() const;
    std::string GetStatus() const;

    int GetLastPaidTime() { return nTimeLastPaid; }
    int GetLastPaidBlock() { return nBlockLastPaid; }
    void UpdateLastPaid(const CBlockIndex *pindex, int nMaxBlocksToScanBack);

    CMasternode& operator=(CMasternode const& from)
    {
        static_cast<masternode_info_t&>(*this)=from;
        lastPing = from.lastPing;
        vchSig = from.vchSig;
        nCollateralMinConfBlockHash = from.nCollateralMinConfBlockHash;
        nBlockLastPaid = from.nBlockLastPaid;
        nPoSeBanScore = from.nPoSeBanScore;
        nPoSeBanHeight = from.nPoSeBanHeight;
        fUnitTest = from.fUnitTest;
        return *this;
    }
};

inline bool operator==(const CMasternode& a, const CMasternode& b)
{
    return fMasterNodesReleased ? a.vin == b.vin : a.pubKeyMasternode == b.pubKeyMasternode;
}
inline bool operator!=(const CMasternode& a, const CMasternode& b)
{
    return fMasterNodesReleased ? !(a.vin == b.vin) : !(a.pubKeyMasternode == b.pubKeyMasternode && a.vin == b.vin);
}


//
// The Masternode Broadcast Class : Contains a different serialize method for sending masternodes through the network
//

class CMasternodeBroadcast : public CMasternode
{
public:

    bool fRecovery;

    CMasternodeBroadcast() : CMasternode(), fRecovery(false) {}
    CMasternodeBroadcast(const CMasternode& mn) : CMasternode(mn), fRecovery(false) {}
    CMasternodeBroadcast(CService addrNew, CPubKey pubKeyMasternodeNew, CBitcoinAddress payee, int nProtocolVersionIn) :
        CMasternode(addrNew, pubKeyMasternodeNew, payee, nProtocolVersionIn), fRecovery(false) {}
    CMasternodeBroadcast(CService addrNew, COutPoint outpointNew, CPubKey pubKeyCollateralAddressNew, CPubKey pubKeyMasternodeNew, int nProtocolVersionIn) :
        CMasternode(addrNew, outpointNew, pubKeyCollateralAddressNew, pubKeyMasternodeNew, nProtocolVersionIn), fRecovery(false) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED active
            READWRITE(vin);
            READWRITE(addr);
            READWRITE(pubKeyCollateralAddress);
        } else {
            READWRITE(addr);
        }

        READWRITE(pubKeyMasternode);

        if (!fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED not active
        	READWRITE(payee);
        }

        READWRITE(vchSig);
        READWRITE(sigTime);
        READWRITE(nProtocolVersion);
        READWRITE(lastPing);
    }

    uint256 GetHash() const
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);

        if (fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED active
            ss << vin;
            ss << pubKeyCollateralAddress;
        } else {
            ss << pubKeyMasternode;
            ss << payee;
        }

        ss << sigTime;
        return ss.GetHash();
    }

    /// Create Masternode broadcast, needs to be relayed manually after that
    static bool Create(const CService& service,
                       const CKey& keyMasternodeNew,
                       const CPubKey& pubKeyMasternodeNew,
                       const CBitcoinAddress& payee,
                       std::string &strErrorRet, CMasternodeBroadcast &mnbRet);
    static bool Create(std::string strService, std::string strKey, std::string strPayee,
                       std::string& strErrorRet, CMasternodeBroadcast &mnbRet,
                       bool fOffline = false);

    static bool Create(const COutPoint& outpoint, const CService& service,
                       const CKey& keyCollateralAddressNew, const CPubKey& pubKeyCollateralAddressNew,
                       const CKey& keyMasternodeNew, const CPubKey& pubKeyMasternodeNew,
                       std::string &strErrorRet, CMasternodeBroadcast &mnbRet);
    static bool Create(std::string strService, std::string strKey, std::string strTxHash,
                       std::string strOutputIndex, std::string& strErrorRet,
                       CMasternodeBroadcast &mnbRet, bool fOffline = false);

    bool SimpleCheckMnRel(int& nDos);
    bool SimpleCheck(int& nDos);
    bool UpdateMnRel(CMasternode* pmn, int& nDos, CConnman& connman);
    bool Update(CMasternode* pmn, int& nDos, CConnman& connman);
    bool CheckOutpoint(int& nDos);
    bool CheckMasternode(int& nDos);

    bool SignMnRel(const CKey& keyCollateralAddress);
    bool Sign(const CKey& keyMasternode);
    bool CheckSignatureMnRel(int& nDos);
    bool CheckSignature(int& nDos);
    void Relay(CConnman& connman);
};

class CMasternodeVerification
{
public:
    CTxIn vin1{};
    CTxIn vin2{};
    CPubKey pubKey1{};
    CPubKey pubKey2{};
    CService addr{};
    int nonce{};
    int nBlockHeight{};
    std::vector<unsigned char> vchSig1{};
    std::vector<unsigned char> vchSig2{};

    CMasternodeVerification() = default;

    CMasternodeVerification(CService addr, int nonce, int nBlockHeight) :
        addr(addr),
        nonce(nonce),
        nBlockHeight(nBlockHeight)
    {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED active
            READWRITE(vin1);
            READWRITE(vin2);
        } else {
            READWRITE(pubKey1);
            READWRITE(pubKey2);
        }

        READWRITE(addr);
        READWRITE(nonce);
        READWRITE(nBlockHeight);
        READWRITE(vchSig1);
        READWRITE(vchSig2);
    }

    uint256 GetHash() const
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);

        if (fMasterNodesReleased) {
            // SPORK_14_MNODES_RELEASE_ENABLED active
            ss << vin1;
            ss << vin2;
        } else {
            ss << pubKey1;
            ss << pubKey2;
        }

        ss << addr;
        ss << nonce;
        ss << nBlockHeight;
        return ss.GetHash();
    }

    void Relay() const
    {
        CInv inv(MSG_MASTERNODE_VERIFY, GetHash());
        g_connman->RelayInv(inv);
    }
};

#endif
