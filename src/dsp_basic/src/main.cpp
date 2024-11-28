#include "AudioEngine/sockapi.hpp"
#include "AudioEngine/address.hpp"
#include "AudioEngine/shm.hpp"
#include "AudioEngine/monitoring.hpp"
#include "AudioEngine/dsp.hpp"
#include "AudioEngine/config.hpp"

#include <winsock2.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

#include <iostream>
#include <limits>
#include <variant>
#include <numbers>

std::string get_cwd() {
    char currentDir[MAX_PATH];
    DWORD result;
    if ( (result = GetCurrentDirectoryA(MAX_PATH, currentDir)) != 0 )
        return currentDir;
    else
        throw std::runtime_error("Failed to load current environment execution directory");
}

auto load_config(std::string const& path) {
    AudioEngine::dsp_cfg_parser<char, 16,
            AudioEngine::dsp_cfg_bool_parser_impl,
            AudioEngine::dsp_cfg_monitor_input_parser_impl
        > parser(path);

    return parser.get_config_fields();
}

std::vector<ma_device_info> get_playout_devices(ma_context& context) {
    ma_device_info *out_devices;
    ma_uint32 out_device_count = 0;

    if (ma_context_get_devices(&context, &out_devices, &out_device_count, nullptr, nullptr) != MA_SUCCESS) {
        throw AudioEngine::dsp_error("Failed to retrieve device infos from miniaudio context");
    }

    std::vector<ma_device_info> res(out_device_count);
    for (ma_uint32 i = 0; i < out_device_count; i++) {
        res[i] = std::move(out_devices[i]);
    }   

    return res;
}



template <class T>
concept dsp_buffer = requires (T const& readable, T& writable, std::span<typename T::ValueType> data, size_t offs, size_t len) {
    typename T::ValueType; //T must have ::ValueType
    { readable.get(offs) } -> std::same_as<typename T::ValueType&>;
    { readable.view(offs, len) } -> std::same_as<std::span<typename T::ValueType>>;
    { writable.store(data) } -> std::same_as<void>;
    { writable.store(offs, data) } -> std::same_as<void>;
};

template <class SampleType, class Allocator = std::allocator<SampleType>>
class pcm_buffer {
public:
    using Alloc_T = typename std::allocator_traits<Allocator>::rebind_alloc<SampleType>;
    using ValueType = SampleType;
private:
    std::optional<Alloc_T> m_allocator;
    SampleType *m_buffer;
    ma_uint32 m_channels;
    ma_uint32 m_frame_count;
    size_t m_size;
    
public:

    pcm_buffer(ma_uint32 channels, ma_uint32 frame_count, Alloc_T const& alloc)
    :   m_allocator(alloc),
        m_buffer(m_allocator.allocate(channels * frame_count)),
        m_channels(channels),
        m_frame_count(frame_count),
        m_size(m_channels * m_frame_count)
    {}

    ValueType& get(size_t offset) const {
        if (offset >= m_size) [[unlikely]]
            throw std::runtime_error("Offset outside of buffer");
        return m_buffer[offset];
    }

    std::span<ValueType> view(size_t offset, size_t length) const {
        if ((offset + length) >= m_size)
            throw AudioEngine::dsp_error(format("view from {} to {} would exceed buffer ({} samples)", offset, offset + length, m_size));
        return std::span<ValueType>(m_buffer[offset], m_buffer[offset+length]);
    }

    void store(size_t offset, std::span<ValueType> const& data) {
        if constexpr (std::is_trivially_constructible_v<ValueType>) {
            std::memcpy(&m_buffer[offset], data.data(), data.size_bytes());
        }
        else {
            if (offset + data.size() >= m_size)
                throw std::out_of_range("offset + data.size() >= m_size");

            size_t counter = 0;
            for (auto& elem : data) {
                m_buffer[offset + counter] = std::move(elem);
            }
        }
    }

    void store(std::span<ValueType> const& data) {
        store(0, data);
    }

    [[nodiscard]] size_t size() const noexcept {
        return m_size;
    }

    SampleType *data() const noexcept { return m_buffer; }
};

template <dsp_buffer BufferT>
class circular_buffer_writer {
    BufferT& m_buffer;
    std::streampos m_pos;

    using ValueType = typename BufferT::ValueType;
public:
    circular_buffer_writer(BufferT& buff)
    :   m_buffer(buff),
        m_pos(0)
    {}

    circular_buffer_writer<BufferT>& operator<<(ValueType const& v) {
        m_buffer.get(m_pos) = v;
        m_pos = (m_pos + 1) % m_buffer.size();

        return *this;
    }

    circular_buffer_writer<BufferT>& operator<<(std::span<ValueType> const& v) {
        ValueType *data = m_buffer.data();

        int64_t headroom = m_buffer.size() - m_pos;

        size_t left = std::min(headroom, v.size());
        std::memcpy( &m_buffer.get(m_pos), v.data(), left );

        size_t right = v.size() - left;
        std::memcpy( m_buffer.data(), &v[left], right);

        return *this;
    }

    [[nodiscard]] operator bool() const noexcept {
        return true;
    }
};  

template <dsp_buffer BufferT>
class circular_buffer_reader {
    BufferT const& m_buffer;
    std::streampos m_pos;

    using ValueType = typename BufferT::ValueType;
public:
    circular_buffer_reader(BufferT const& buff)
    :   m_buffer(buff),
        m_pos(0)
    {}

    circular_buffer_reader<BufferT>& operator>>(ValueType& dst) {
        dst = m_buffer.get(m_pos);
        m_pos = (m_pos + 1) % m_buffer.size(); 
        return *this;
    }

    circular_buffer_reader<BufferT>& operator>>(std::span<ValueType>& dst) {
        ValueType *data = m_buffer.data();
        std::span<ValueType> dataspan = std::span<ValueType>(data, m_buffer.size());

        int64_t headroom = m_buffer.size() - m_pos;

        size_t left = std::min((uint64_t)headroom, dst.size());
        auto leftspan = dataspan.first(left);
        std::memcpy(dst.data(), leftspan.data(), leftspan.size());

        size_t right = dst.size() - left;
        if (right > 0) {
            auto rightspan = dataspan.subspan(left, right);
            std::memcpy(&dst[left], rightspan.data(), rightspan.size());
        }

        return *this;
    }

    [[nodiscard]] operator bool() const noexcept {
        return true;
    }
};

using sample_t = int16_t;
using pcm_buff_t = pcm_buffer<sample_t, std::allocator<sample_t>>;
using pcm_buff_reader_t = circular_buffer_reader<pcm_buff_t>;

struct play_data_callback_userdata {
    pcm_buff_reader_t in_pcm_stream;
};

void play_data_callback(ma_device *p_device, void *p_output, void const *p_input, ma_uint32 frame_count) {
    ma_uint32 channels = p_device->playback.channels;

    auto *play_data = (play_data_callback_userdata*)p_device->pUserData;
    auto &reader = play_data->in_pcm_stream;

    std::span<sample_t> out_span( (sample_t*)p_output, frame_count * channels);
    if (!(reader >> out_span)) {
        throw AudioEngine::dsp_error("Failed to write to output buffer");
    }
}

void generate_sin_wave(int16_t *buffer, size_t sample_count) {
    int16_t *abuffer = std::assume_aligned<32>(buffer);

    for (size_t i = 0; i < sample_count; i++) {
        float ts = i/48000;
        abuffer[i] = (int16_t)(std::numeric_limits<int16_t>::max() * sin(ts * std::numbers::pi));
    }
}

int main() {
    //If this fails then we might as well just exit.
    Net::init();

    try {
        Net::address_ipv4 addr{"127.0.0.1"};
        Net::end_point ep(addr, 10681);
        
        using shm_t = Memory::_shm<
            Memory::win32mmapapi<Memory::pagesize_2MB>, 
            Memory::shm_size::MEGABYTEx256
        >;

        shm_t shm("256mb_storage_dsp_basic", PAGE_READWRITE);
        AudioEngine::Monitoring::probe_service service(shm.data(), shm_t::page_size);
        
        AudioEngine::Monitoring::probe_description pd{
            .name = "SETUP_PROBE", 
            .unit = "", 
            .flags = 0
        };
        decltype(service)::probe_handle_t probe_handle = service.add_probe(pd);
    
        auto config = load_config(get_cwd() + "/conf.cfg");

        







        ma_context context;

        if ( ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
            throw AudioEngine::dsp_error("Failed to load miniaudio context");
        }

        auto playout_devices = get_playout_devices(context);
        for (auto& device_info : playout_devices) {
            std::cout << format("{}\n", device_info.name);
        }

        ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
        cfg.playback.format = ma_format_s16;
        cfg.playback.channels = 2;
        cfg.playback.pDeviceID = &playout_devices[0].id;
        cfg.sampleRate = 48000;
        cfg.dataCallback = play_data_callback;
        cfg.pUserData = nullptr;

        ma_context_uninit(&context);










    }
    catch (Net::net_error const& e) {
        std::cout << e.what() << "\n";
    }

    Net::cleanup();

    return 0;
}