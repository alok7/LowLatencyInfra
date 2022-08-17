#pragma once

namespace AAX {
  namespace Config {
    constexpr char PRIMARY_REALTIME_HOST[] = "239.100.140.136";
    constexpr char PRIMARY_REALTIME_PORT[] = "10498";
    constexpr char SECONDARY_REALTIME_HOST[] = "239.100.140.137";
    constexpr char SECONDARY_REALTIME_PORT[] = "10499";
    constexpr char SNAPSHOT_PORT[] = "10497";
    constexpr char SNAPSHOT_HOST[] = "10.31.51.49";
    constexpr char COMP_ID[] = "TTSMD1";
    constexpr char PASSWD[] = "0Bug";
    const static constexpr size_t SNAPSHOT_REQUEST_INTERVAL = 1000000; // in microseconds
    const static constexpr size_t COLLECT_LOGS_INTERVAL = 10000000; // in microseconds
    const static constexpr size_t TRADE_QUEUE_SIZE = 32768;
    const static constexpr size_t DEPTHBOOK_QUEUE_SIZE = 1048576;
    const static constexpr size_t MAX_PACKET_LENGTH = 4096;
    const static constexpr size_t ACTIVE_REALTIME_CACHE_SIZE = 0;
    const static constexpr size_t BACKUP_REALTIME_CACHE_SIZE = 8192;
    const static constexpr size_t INITIAL_MESSAGE_BUFFER_SIZE = 32; //buffer this amount of messages before requesting for TCP snapshot
  }
}
