#ifndef __ML_TELEGRAM_HPP
#define __ML_TELEGRAM_HPP

#include <vector>
#include <map>
#include <string>
#include "pc2/pc2device.hpp"

typedef std::vector<std::string> payload_labels_t;
typedef std::vector<std::pair<bool, uint8_t>> payload_expectations_t;

class Masterlink {
    public:
    enum source {
        a_aux = 0x97,
        a_mem = 0x79,
        a_mem2 = 0x7a,
        a_tape = 0x79,
        cd = 0x8d,
        doorcam = 0x3e,
        dtv = 0x1f,
        dtv2 = 0x33,
        dvd = 0x29,
        dvd2 = 0x16,
        n_radio = 0xa1,
        pc = 0x47,
        phono = 0xa1,
        radio = 0x6f,
        sat = 0x1f,
        tv = 0x0b,
        v_aux = 0x33,
        v_aux2 = 0x3e,
        v_mem = 0x15,
        v_tape = 0x15,
        v_tape2 = 0x16,
    };

    std::map<Masterlink::source, std::string> source_name = {
        {Masterlink::source::tv, "TV"},
        {Masterlink::source::v_mem, "V_MEM"},
        {Masterlink::source::v_tape, "V_TAPE"},
        {Masterlink::source::pc, "PC"},
        {Masterlink::source::dvd2, "DVD2"},
        {Masterlink::source::a_mem2, "A.MEM2"},
        {Masterlink::source::doorcam, "DOORCAM"},
        {Masterlink::source::v_aux2, "V.AUX2"},
        {Masterlink::source::v_tape2, "V.TAPE2"},
        {Masterlink::source::cd, "CD"},
        {Masterlink::source::sat, "SAT"},
        {Masterlink::source::dtv, "DTV"},
    };
};

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
            {0x44, "TRACK_INFO"},
            {payload_types::goto_source, "GOTO_SOURCE"},
            {0x5c, "???"},
            {0x6c, "DISTRIBUTION_REQUEST"},
            {0x82, "TRACK_INFO_LONG"},
            {0x87, "STATUS_INFO"},
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
        MasterlinkTelegram(PC2Telegram & tgram);
        MasterlinkTelegram();
};

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

class GotoSourceTelegram: public DecodedTelegram {
    public:
        GotoSourceTelegram(MasterlinkTelegram & tgram): DecodedTelegram{tgram} { };
        std::ostream& debug_repr(std::ostream& outputStream);
        GotoSourceTelegram();
};

class DisplayDataMessage: public DecodedTelegram {
    public:
        DisplayDataMessage(MasterlinkTelegram & tgram): DecodedTelegram{tgram} { }
        std::ostream& debug_repr(std::ostream& outputStream);
};

class AudioBusTelegram: public DecodedTelegram {
    public:
        enum audio_bus_tgram_meanings {
            unknown,
            request_status,
            status_not_distributing,
            status_distributing,
        } audio_bus_tgram_meanings;
        enum audio_bus_tgram_meanings tgram_meaning;

        AudioBusTelegram(MasterlinkTelegram & tgram);
        AudioBusTelegram();
        std::ostream& debug_repr(std::ostream& outputStream);
};

class MasterPresentTelegram: public DecodedTelegram {
    public:
        MasterPresentTelegram(MasterlinkTelegram & tgram): DecodedTelegram{tgram} { };
        MasterPresentTelegram();
        std::ostream& debug_repr(std::ostream& outputStream);
};

class DecodedTelegramFactory {
    public:
        static DecodedTelegram *make(MasterlinkTelegram & tgram);
};
std::ostream& operator <<(std::ostream& outputStream, DecodedTelegram& m);
#endif
