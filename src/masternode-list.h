// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MASTERNODE_LIST_H
#define MASTERNODE_LIST_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"
#include "pubkey.h"

class CMasternodeListMessage;
class CMasternodeListManager;

extern std::map<uint256, CMasternodeListMessage> masternodesInventoryMap;// <inv.hash, msg>
extern CMasternodeListManager masternodeListManager;

class CMasternodeListMessage
{
private:
    std::vector<unsigned char> vchSig;

public:
    CPubKey nPubKey;
    int64_t nTimeSigned;
    bool nActive;

    CMasternodeListMessage(CPubKey nPubKey, int64_t nTimeSigned, bool nActive) :
        nPubKey(nPubKey),
        nTimeSigned(nTimeSigned),
        nActive(nActive)
        {}

    CMasternodeListMessage() :
        nPubKey(),
        nTimeSigned(0),
        nActive(0)
        {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nPubKey);
        READWRITE(nTimeSigned);
        READWRITE(nActive);
        READWRITE(vchSig);
    }

    uint256 GetHash() const
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        ss << nPubKey;
        ss << nTimeSigned;
        ss << nActive;
        return ss.GetHash();
    }

    bool Sign(std::string strSignKey);
    bool CheckSignature();
    void Relay(CConnman& connman);
};

class CMasternodeListManager
{
private:
    // critical section to protect the inner data structures
    mutable CCriticalSection cs;

    std::vector<unsigned char> vchSig;
    std::string strMasterPrivKey;
    std::map<uint256, CMasternodeListMessage> mapNodesActive;// <pubKey.hash, msg>

public:
    CMasternodeListManager() {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(vchSig);
        READWRITE(mapNodesActive);
    }

    void ProcessMNList(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
    bool UpdateMNList(const CPubKey& nPubKey, bool nActive, CConnman& connman);
    bool IsMNActive(const CPubKey& nPubKey);
    const std::map<uint256, CMasternodeListMessage>& GetMNList() const;

    bool SetPrivKey(std::string strPrivKey);

    void Clear();
    std::string ToString() const;
    void CheckAndRemove();
};

#endif
