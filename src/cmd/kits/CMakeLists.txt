# Define the source files of the kits library:
SET(kits_SRCS
    # BASIC TABLE INFRASTRUCTURE
    ${CMAKE_CURRENT_SOURCE_DIR}/field.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/index_desc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/row.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/table_man.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/table_desc.cpp

    # KITS SM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/shore_env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shore_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shore_helper_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/reqs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/daemons.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/skewer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kits_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trx_worker.cpp

    # UTILITIES
    ${CMAKE_CURRENT_SOURCE_DIR}/mersenne.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stl_pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/random_input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/confparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shore_worker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sort.cpp

    # YCSB
    ${CMAKE_CURRENT_SOURCE_DIR}/ycsb/ycsb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ycsb/templates.cpp

    # TPC-B
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcb/tpcb_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcb/tpcb_env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcb/tpcb_input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcb/tpcb_schema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcb/tpcb_schema_man.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcb/tpcb_xct.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcb/templates.cpp

    # TPC-C
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcc/tpcc_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcc/tpcc_env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcc/tpcc_input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcc/tpcc_schema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcc/tpcc_schema_man.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcc/tpcc_xct.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcc/tpcc_random.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpcc/templates.cpp

    # TPC-E
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_env.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_schema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_schema_man.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_broker_volume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_customer_position.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_data_maintenance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_market_feed.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_market_watch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_populate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_security_detail.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_trade_cleanup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_trade_lookup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_trade_order.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_trade_result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_trade_status.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/shore_tpce_xct_trade_update.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/tpce_input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/AddressTable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/BaseLogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/bucketsimulator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/CE.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/CETxnInputGenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/CETxnMixGenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/CustomerSelection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/CustomerTable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/DateTime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/DM.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/EGenGenerateAndLoad.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/EGenLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/EGenLogFormatterTab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/EGenValidate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/EGenVersion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/FlatFileLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/InputFlatFilesStructure.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/locking.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/MEE.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/MEEPriceBoard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/MEESecurity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/MEETickerTape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/MEETradingFloor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/Money.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/Person.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/progressmeter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/progressmeterinterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/Random.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/ReadRowFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/ReadRowFunctions_istream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/ReadRowFunctions_sscanf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/strutil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/threading.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/TradeGen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tpce/egen/WheelTime.cpp

    # KITS ZAPPS INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/kits_cmd.cpp
   )

# Add the library of this directory kits:
ADD_LIBRARY(kits ${kits_SRCS})

IF(LINK_TIME_OPTIMIZATION)
    SET_PROPERTY(TARGET kits PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
ENDIF(LINK_TIME_OPTIMIZATION)

# Set the shared librariy used by the library kits:
TARGET_LINK_LIBRARIES(kits)
