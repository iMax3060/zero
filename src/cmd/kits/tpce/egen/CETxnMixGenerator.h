/*
 * Legal Notice
 *
 * This document and associated source code (the "Work") is a part of a
 * benchmark specification maintained by the TPC.
 *
 * The TPC reserves all right, title, and interest to the Work as provided
 * under U.S. and international laws, including without limitation all patent
 * and trademark rights therein.
 *
 * No Warranty
 *
 * 1.1 TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THE INFORMATION
 *     CONTAINED HEREIN IS PROVIDED "AS IS" AND WITH ALL FAULTS, AND THE
 *     AUTHORS AND DEVELOPERS OF THE WORK HEREBY DISCLAIM ALL OTHER
 *     WARRANTIES AND CONDITIONS, EITHER EXPRESS, IMPLIED OR STATUTORY,
 *     INCLUDING, BUT NOT LIMITED TO, ANY (IF ANY) IMPLIED WARRANTIES,
 *     DUTIES OR CONDITIONS OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
 *     PURPOSE, OF ACCURACY OR COMPLETENESS OF RESPONSES, OF RESULTS, OF
 *     WORKMANLIKE EFFORT, OF LACK OF VIRUSES, AND OF LACK OF NEGLIGENCE.
 *     ALSO, THERE IS NO WARRANTY OR CONDITION OF TITLE, QUIET ENJOYMENT,
 *     QUIET POSSESSION, CORRESPONDENCE TO DESCRIPTION OR NON-INFRINGEMENT
 *     WITH REGARD TO THE WORK.
 * 1.2 IN NO EVENT WILL ANY AUTHOR OR DEVELOPER OF THE WORK BE LIABLE TO
 *     ANY OTHER PARTY FOR ANY DAMAGES, INCLUDING BUT NOT LIMITED TO THE
 *     COST OF PROCURING SUBSTITUTE GOODS OR SERVICES, LOST PROFITS, LOSS
 *     OF USE, LOSS OF DATA, OR ANY INCIDENTAL, CONSEQUENTIAL, DIRECT,
 *     INDIRECT, OR SPECIAL DAMAGES WHETHER UNDER CONTRACT, TORT, WARRANTY,
 *     OR OTHERWISE, ARISING IN ANY WAY OUT OF THIS OR ANY OTHER AGREEMENT
 *     RELATING TO THE WORK, WHETHER OR NOT SUCH AUTHOR OR DEVELOPER HAD
 *     ADVANCE NOTICE OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 * Contributors
 * - Doug Johnson, Cecil Reames, Matt Emmerton
 */

/******************************************************************************
*   Description:        EGenDriverCE class to generate transaction types for
*                       execution
******************************************************************************/

#ifndef CE_TXN_MIX_GENERATOR_H
#define CE_TXN_MIX_GENERATOR_H

#include "EGenUtilities_stdafx.h"
#include "DriverParamSettings.h"
#include "EGenLogger.h"

namespace tpce {

    class CCETxnMixGenerator {
    private:
        const PDriverCETxnSettings m_pDriverCETxnSettings;
        CRandom m_rnd;
        CBaseLogger *m_pLogger;

        // Transaction mixes are expressed out of a total of 1000.
        //
        // NOTE that Trade-Result and Market-Feed are not generated by this class
        // as possible runtime transaction types. They happen as an automatic
        // by-product of Trade-Order transactions.

        INT32 m_CETransactionMixTotal;

        /*INT32                 m_BrokerVolumeMixLimit;
        INT32                   m_CustomerPositionMixLimit;
        INT32                   m_MarketWatchMixLimit;
        INT32                   m_SecurityDetailMixLimit;
        INT32                   m_TradeLookupMixLimit;
        INT32                   m_TradeOrderMixLimit;
        INT32                   m_TradeStatusMixLimit;
        INT32                   m_TradeUpdateMixLimit;*/

        // Array of transaction types used for "shuffle a deck of cards"
        // algorithm (also known as Knuth shuffle).
        //
        INT32 m_iTxnArrayCurrentIndex;
        char *m_pTxnArray;

    public:
        static const INT32 INVALID_TRANSACTION_TYPE = -1;
        static const INT32 SECURITY_DETAIL = 0;
        static const INT32 BROKER_VOLUME = 1;
        static const INT32 CUSTOMER_POSITION = 2;
        static const INT32 MARKET_WATCH = 3;
        static const INT32 TRADE_STATUS = 4;
        static const INT32 TRADE_LOOKUP = 5;
        static const INT32 TRADE_ORDER = 6;
        static const INT32 TRADE_UPDATE = 7;
        //Trade-Result and Market-Feed are included for completness.
        static const INT32 MARKET_FEED = 8;
        static const INT32 TRADE_RESULT = 9;

        CCETxnMixGenerator(const PDriverCETxnSettings pTxnParamSettings, CBaseLogger *pLogger);

        CCETxnMixGenerator(const PDriverCETxnSettings pTxnParamSettings, RNGSEED RNGSeed, CBaseLogger *pLogger);

        ~CCETxnMixGenerator();

        RNGSEED GetRNGSeed(void);

        void SetRNGSeed(RNGSEED RNGSeed);

        void UpdateTunables(void);

        int GenerateNextTxnType(void);
    };

}   // namespace TPCE

#endif  // #ifndef CE_TXN_MIX_GENERATOR_H
