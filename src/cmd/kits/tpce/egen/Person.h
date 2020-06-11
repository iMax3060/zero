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
 * - Sergey Vasilevskiy
 */

/*
*   Person class for the Customer table.
*/
#ifndef PERSON_H
#define PERSON_H

#include "EGenStandardTypes.h"
#include "Random.h"
#include "InputFlatFilesDeclarations.h"
#include "InputFlatFilesStructure.h"

namespace tpce {

// Used for generating tax ID strings.
    const int TaxIDFmt_len = 14;
    const char TaxIDFmt[TaxIDFmt_len + 1] = "nnnaannnnaannn";

    class CPerson {
    private:

        TLastNamesFile *m_LastNames;
        TMaleFirstNamesFile *m_MaleFirstNames;
        TFemaleFirstNamesFile *m_FemaleFirstNames;

        CRandom m_rnd;
        bool m_bCacheEnabled;
        int m_iCacheSize;
        TIdent m_iCacheOffset;
        char **m_CacheFirstName;
        char **m_CacheLastName;

    public:
        CPerson(CInputFiles inputFiles, TIdent iStartFromCustomer, bool bCacheEnabled = false);

        ~CPerson();

        void InitNextLoadUnit(TIdent iCacheOffsetIncrement = iDefaultLoadUnitSize);

        char *GetLastName(TIdent CID);

        char *GetFirstName(TIdent CID);

        char GetMiddleName(TIdent CID);

        char GetGender(TIdent CID); //'M' or 'F'
        bool IsMaleGender(TIdent CID);  //TRUE if male, FALSE if female
        void GetTaxID(TIdent CID, char *buf);

        //get first name, last name, and tax id
        void GetFirstLastAndTaxID(TIdent C_ID, char *szFirstName, char *szLastName, char *szTaxID);
    };

}   // namespace TPCE

#endif //PERSON_H
