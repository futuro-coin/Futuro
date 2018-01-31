// Copyright (c) 2014-2017 The Dash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "validation.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "masternode-list.h"
#include "masternode-sync.h"

#include <boost/lexical_cast.hpp>

class CMasternodeListMessage;
class CMasternodeListManager;

CMasternodeListManager masternodeListManager;

std::map<uint256, CMasternodeListMessage> masternodesInventoryMap;

void CMasternodeListManager::ProcessMNList(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
    if (strCommand == NetMsgType::MNLISTUPDATE) {
        CMasternodeListMessage msg;
        vRecv >> msg;

        uint256 hash = msg.GetHash();
        uint256 pubKeyHash = msg.nPubKey.GetHash();

        std::string strLogMsg;
        {
            LOCK(cs_main);
            pfrom->setAskFor.erase(hash);
            if(!chainActive.Tip()) return;
            strLogMsg = strprintf("MNLISTUPDATE -- pubKey: %s active: %d bestHeight: %d peer=%d",
                                  HexStr(msg.nPubKey), msg.nActive, chainActive.Height(), pfrom->id);
        }

        if(mapNodesActive.count(pubKeyHash)) {
            if (mapNodesActive[pubKeyHash].nTimeSigned >= msg.nTimeSigned) {
                LogPrint("mnlistupdate", "%s seen\n", strLogMsg);
                return;
            } else {
                LogPrintf("%s updated\n", strLogMsg);
            }
        } else {
            LogPrintf("%s new\n", strLogMsg);
        }

        if(!msg.CheckSignature()) {
            LogPrintf("CMasternodeListManager::ProcessMNList -- invalid signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        masternodesInventoryMap[hash] = msg;
        mapNodesActive[pubKeyHash] = msg;
        msg.Relay(connman);

    } else if (strCommand == NetMsgType::GETMNLIST) {

        std::map<uint256, CMasternodeListMessage>::iterator it = mapNodesActive.begin();

        while (it != mapNodesActive.end()) {
            connman.PushMessage(pfrom, NetMsgType::MNLISTUPDATE, it->second);
            it++;
        }
    }
}

bool CMasternodeListManager::UpdateMNList(const CPubKey& nPubKey, bool nActive, CConnman& connman)
{
    CMasternodeListMessage msg = CMasternodeListMessage(nPubKey, GetAdjustedTime(), nActive);

    if (msg.Sign(strMasterPrivKey)) {
        msg.Relay(connman);
        masternodesInventoryMap[msg.GetHash()] = msg;
        mapNodesActive[msg.nPubKey.GetHash()] = msg;
        return true;
    }

    return false;
}

bool CMasternodeListManager::IsMNActive(const CPubKey& nPubKey) {
    return mapNodesActive.count(nPubKey.GetHash()) &&
            mapNodesActive[nPubKey.GetHash()].nActive;
}

const std::map<uint256, CMasternodeListMessage>& CMasternodeListManager::GetMNList() const {
    return mapNodesActive;
}

bool CMasternodeListManager::SetPrivKey(std::string strPrivKey)
{
    CMasternodeListMessage msg;

    msg.Sign(strPrivKey);

    if(msg.CheckSignature()){
        // Test signing successful, proceed
        LogPrintf("CMasternodeListManager::SetPrivKey -- Successfully initialized as masternode list message signer\n");
        strMasterPrivKey = strPrivKey;
        return true;
    } else {
        return false;
    }
}

void CMasternodeListManager::Clear()
{
    LOCK(cs);
    mapNodesActive.clear();
}

std::string CMasternodeListManager::ToString() const
{
    bool active = true;

    std::ostringstream info;

    info << "Masternode manager list: count : " << (int)mapNodesActive.size()
            << ", active masternodes: "
            << std::count_if(std::begin(mapNodesActive),
                             std::end(mapNodesActive),
                             [active](std::pair<uint256, CMasternodeListMessage> const &p) {
                                return p.second.nActive == active;
                             });

    return info.str();
}

void CMasternodeListManager::CheckAndRemove()
{
    if (!masternodeSync.IsMasternodeListSynced()) return;

    LOCK(cs);

    std::map<uint256, CMasternodeListMessage>::iterator it = mapNodesActive.begin();

    while (it != mapNodesActive.end()) {
        CMasternodeListMessage msg = (*it).second;

        if (!IsMNActive(msg.nPubKey)) {
            LogPrint("mnlist", "CMasternodeListManager::CheckAndRemove -- "
                    "Removing inactive Masternode: pubKey=%s\n", HexStr(msg.nPubKey));
            mapNodesActive.erase(it++);
        } else {
            ++it;
        }
    }
    LogPrintf("CMasternodeListManager::CheckAndRemove -- %s\n", ToString());
}

bool CMasternodeListMessage::Sign(std::string strSignKey)
{
    CKey key;
    CPubKey pubkey;
    std::string strError = "";
    std::string strMessage = HexStr(nPubKey) + boost::lexical_cast<std::string>(nTimeSigned) + boost::lexical_cast<std::string>(nActive);

    if(!CMessageSigner::GetKeysFromSecret(strSignKey, key, pubkey)) {
        LogPrintf("CMasternodeListMessage::Sign -- GetKeysFromSecret() failed, invalid key %s\n", strSignKey);
        return false;
    }

    if(!CMessageSigner::SignMessage(strMessage, vchSig, key)) {
        LogPrintf("CMasternodeListMessage::Sign -- SignMessage() failed\n");
        return false;
    }

    if(!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
        LogPrintf("CMasternodeListMessage::Sign -- VerifyMessage() failed, error: %s\n", strError);
        return false;
    }

    return true;
}

bool CMasternodeListMessage::CheckSignature()
{
    //note: need to investigate why this is failing
    std::string strError = "";
    std::string strMessage = HexStr(nPubKey) + boost::lexical_cast<std::string>(nTimeSigned) + boost::lexical_cast<std::string>(nActive);

    CPubKey pubkey(ParseHex(Params().MasternodeListManagerPubKey()));

    if(!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
        LogPrintf("CMasternodeListMessage::CheckSignature -- VerifyMessage() failed, error: %s\n", strError);
        return false;
    }

    return true;
}

void CMasternodeListMessage::Relay(CConnman& connman)
{
    CInv inv(MSG_MNLIST, GetHash());
    connman.RelayInv(inv);
}
