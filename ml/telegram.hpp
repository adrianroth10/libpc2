/*! \file ml/telegram.hpp
 *  \brief Masterlink Telegram related classes
 */
#ifndef __ML_TELEGRAM_HPP
#define __ML_TELEGRAM_HPP

#include <vector>
#include <map>
#include <string>

class MasterlinkTelegram;
#include "ml/masterlink.hpp"
#include "pc2/pc2device.hpp"

typedef std::vector<std::string> payload_labels_t;
typedef std::vector<std::pair<bool, uint8_t>> payload_expectations_t;

/*! \class MasterlinkTelegram
 *! \brief Base Masterlink Telegram class
 */
class MasterlinkTelegram {
    public:
        std::map<uint8_t, std::string> node_name = {
            {0x80, "ALL"},
            {0x83, "ALL"},
            {0xC0, "V_MASTER"},
            {0xC1, "A_MASTER"},
            {0xC2, "PC_1"},
            {0x01, "NODE_01"},
            {0x02, "NODE_02"},
            {0x03, "NODE_03"},
            {0x04, "NODE_04"},
            {0x05, "NODE_05"},
            {0x06, "NODE_06"},
            {0x07, "NODE_07"},
            {0x08, "NODE_08"},
            {0x09, "NODE_09"},
            {0x0A, "NODE_0A"},
            {0x0B, "NODE_0B"},
            {0x0C, "NODE_0C"},
            {0x0D, "NODE_0D"},
            {0x0E, "NODE_0E"},
            {0x0F, "NODE_0F"},
            {0x10, "NODE_10"},
            {0x11, "NODE_11"},
            {0x12, "NODE_12"},
            {0x13, "NODE_13"},
        };

        enum payload_types: uint8_t {
            master_present = 0x04,
            display_data = 0x06,
            metadata = 0x0b,
            goto_source = 0x45,
            audio_bus = 0x08,
            status_info = 0x87,
            track_info = 0x44,
        };

        std::map<uint8_t, std::string> payload_type_name = {
            {payload_types::master_present, "MASTER_PRESENT"},
            {0x05, "???"},
            {payload_types::display_data, "DISPLAY_DATA"},
            {payload_types::audio_bus, "AUDIO_BUS"},
            {payload_types::metadata, "METADATA"},
            {0x0d, "BEO4_KEY"},
            {0x10, "STANDBY"},
            {0x11, "RELEASE"},
            {0x12, "???"},
            {0x20, "???"},
            {0x30, "???"},
            {0x3c, "TIMER"},
            {0x40, "CLOCK"},
            {payload_types::track_info, "TRACK_INFO"},
            {payload_types::goto_source, "GOTO_SOURCE"},
            {0x5c, "???"},
            {0x6c, "DISTRIBUTION_REQUEST"},
            {0x82, "TRACK_INFO_LONG"},
            {payload_types::status_info, "STATUS_INFO"},
            {0x94, "DVD_STATUS_INFO"},
            {0x96, "PC_PRESENT"},
        };

        enum telegram_types: uint8_t {
            command = 0x0a,
            request = 0x0b,
            status = 0x14,
            info = 0x2c,
            time = 0x40,
            config = 0x5e,
        };

        std::map<telegram_types, std::string> telegram_type_name = {
            {command, "COMMAND"},
            {request, "REQUEST"},
            {status, "STATUS"},
            {info, "INFO"},
            {time, "TIME"}, // not seen so far, but a forum post suggests this exists...
            {config, "CONFIG"},
        };

        PC2Message serialize();
        std::vector<uint8_t> data;
        std::vector<uint8_t> payload;
        uint8_t dest_node = 0;
        uint8_t dest_src = 0; // some telegrams are addressed to a node's specific source ID
        uint8_t src_node = 0;
        uint8_t src_src = 0; // some telegrams have a specific source ID
        enum payload_types payload_type;
        unsigned int payload_size = 0;
        unsigned int payload_version = 0;
        enum telegram_types telegram_type;

        uint8_t checksum(std::vector<uint8_t> data);
        MasterlinkTelegram(const PC2Telegram & tgram);
        MasterlinkTelegram();
};

namespace DecodedTelegram {
    class DecodedTelegram: public MasterlinkTelegram {
        public:
            virtual std::ostream& debug_repr(std::ostream& outputStream) = 0;
            friend std::ostream& operator <<(std::ostream& outputStream, const DecodedTelegram& m);
            DecodedTelegram(MasterlinkTelegram & tgram): MasterlinkTelegram{tgram} {};
            DecodedTelegram();
    };

    class UnknownTelegram: public DecodedTelegram {
        public:
            std::ostream& debug_repr(std::ostream& outputStream);
            UnknownTelegram(MasterlinkTelegram & tgram): DecodedTelegram{tgram} {};
    };

    class GotoSource: public DecodedTelegram {
        public:
            enum tgram_meanings {
                unknown,
                request_source,
            } tgram_meanings;
            uint8_t requested_source;
            enum tgram_meanings tgram_meaning;
            GotoSource(MasterlinkTelegram & tgram);
            std::ostream& debug_repr(std::ostream& outputStream);
            GotoSource();
    };

    class TrackInfo: public DecodedTelegram {
        public:
            TrackInfo(MasterlinkTelegram & tgram): DecodedTelegram{tgram} { }
            TrackInfo(uint8_t source_id); //generates boilerplate reply telegram for a source
            std::ostream& debug_repr(std::ostream& outputStream);
    };

    class StatusInfo: public DecodedTelegram {
        public:
            StatusInfo(MasterlinkTelegram & tgram): DecodedTelegram{tgram} { }
            StatusInfo(uint8_t source_id); //generates boilerplate reply telegram for a source
            std::ostream& debug_repr(std::ostream& outputStream);
    };

    class DisplayData: public DecodedTelegram {
        public:
            DisplayData(MasterlinkTelegram & tgram): DecodedTelegram{tgram} { }
            std::ostream& debug_repr(std::ostream& outputStream);
    };

    class AudioBus: public DecodedTelegram {
        public:
            enum audio_bus_tgram_meanings {
                unknown,
                request_status,
                status_not_distributing,
                status_distributing,
            } audio_bus_tgram_meanings;
            enum audio_bus_tgram_meanings tgram_meaning;

            AudioBus(MasterlinkTelegram & tgram);
            AudioBus();
            std::ostream& debug_repr(std::ostream& outputStream);
    };

    class MasterPresent: public DecodedTelegram {
        public:
            static MasterPresent reply_from_request(const MasterPresent & tgram);
            MasterPresent(MasterlinkTelegram & tgram): DecodedTelegram{tgram} { };
            MasterPresent();
            std::ostream& debug_repr(std::ostream& outputStream);
    };

    class Metadata: public DecodedTelegram {
        public:
            std::ostream& debug_repr(std::ostream& outputStream);
            payload_labels_t labels = {"Field type ID", "??", "??", "??", "Metadata type: N_RADIO or A_MEM2", "??", "??", "??", "??", "??", "??", "??", "??", "??"};
            payload_expectations_t expectations = {
                {false, 0},
                {true, 0},
                {true, 1},
                {true, 1},
                {false, 0}, // Metadata message type? If radio, gives 1A (ML SRC N_RADIO), if A MEM gives 7A (A_MEM2)
                {true, 5},
                {true, 0},
                {true, 0},
                {true, 0},
                {true, 0xFF},
                {true, 0},
                {true, 0xFF},
                {true, 0},
                {true, 1},
            };
            enum metadata_message_type {
                amem = 0x7a,
                radio = 0xa1
            };

            Metadata(MasterlinkTelegram & tgram);
            bool any_surprises_here();
            enum metadata_field_type {
                genre = 0x01,
                album = 0x02,
                artist = 0x03,
                track = 0x04,
            };

            std::map<uint8_t, std::string> metadata_field_type_label {
                {0x01, "genre"},
                    {0x02, "album"},
                    {0x03, "artist"},
                    {0x04, "track"},
            };

            Masterlink::source src;
            std::string key;
            std::string value;

    };

    class DecodedTelegramFactory {
        public:
            static DecodedTelegram *make(MasterlinkTelegram & tgram);
    };
}

std::ostream& operator <<(std::ostream& outputStream, DecodedTelegram::DecodedTelegram& m);
#endif
