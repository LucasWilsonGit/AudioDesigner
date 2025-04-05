

#include "AudioEngine/sockapi.hpp"
#include "AudioEngine/address.hpp"
#include "AudioEngine/shm.hpp"
#include "AudioEngine/monitoring.hpp"
#include "AudioEngine/dsp.hpp"
#include "AudioEngine/config.hpp"

#include "AudioEngine/buffers/pcm_buffer.hpp"
#include "AudioEngine/buffers/circular_streams.hpp"

#define _WINSOCKAPI_  // Stops `winsock.h` from loading
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MINIAUDIO_IMPLEMENTATION
#include "AudioEngine/miniaudio_utils.hpp"

#include <iostream>
#include <limits>
#include <variant>
#include <numbers>
#include <thread>
#include <filesystem>
#include <cwctype>



std::string get_cwd() {
    char currentDir[MAX_PATH];
    DWORD result;
    if ( (result = GetCurrentDirectoryA(MAX_PATH, currentDir)) != 0 )
        return currentDir;
    else
        throw std::runtime_error("Failed to load current environment execution directory");
}

auto load_config(std::string const& path) {
    AudioEngine::parser_from_tuple<char, 16, AudioEngine::base_cfg_parsers>::type parser(path);

    return parser.get_config();
}

std::vector<ma_device_info> get_playout_devices(ma_context& context) {
    ma_device_info *out_devices;
    ma_uint32 out_device_count = 0;

    AudioEngine::ma_call(ma_context_get_devices(&context, &out_devices, &out_device_count, nullptr, nullptr));

    std::vector<ma_device_info> res(out_device_count);
    for (ma_uint32 i = 0; i < out_device_count; i++) {
        res[i] = std::move(out_devices[i]);
    }   

    return res;
}







using sample_t = int16_t;
using pcm_buff_t = AudioEngine::pcm_buffer<sample_t, std::allocator<sample_t>>;
using pcm_buff_reader_t = AudioEngine::circular_buffer_reader<pcm_buff_t>;

struct play_data_callback_userdata {
    play_data_callback_userdata() = delete;

    pcm_buff_reader_t in_pcm_stream;
    play_data_callback_userdata(pcm_buff_reader_t&& stream) : in_pcm_stream(std::move(stream)) {}
};

void play_data_callback(ma_device *p_device, void *p_output, void const */*p_input*/, ma_uint32 frame_count) {
    ma_uint32 channels = p_device->playback.channels;

    auto *play_data = (play_data_callback_userdata*)p_device->pUserData;
    auto &reader = play_data->in_pcm_stream;

    std::span<sample_t> out_span( (sample_t*)p_output, frame_count * channels);
    if (!(reader >> out_span)) {
        throw AudioEngine::dsp_error("Failed to write to output buffer");
    }
}

sample_t sin_at_sample(size_t sample_idx, size_t sample_rate, size_t hertz) {
    float ts = static_cast<float>(sample_idx) / static_cast<float>(sample_rate);
    return (sample_t)(std::numeric_limits<sample_t>::max() * sin(ts * std::numbers::pi * static_cast<double>(hertz)));
}

void generate_sin_wave(sample_t *buffer, size_t frame_count, size_t sample_rate, int64_t channel_count, size_t hertz) {

    int64_t count = 0;
    for (size_t i = 0; i < frame_count; i++) {

        sample_t val = sin_at_sample(i, sample_rate, hertz);
        
        for (int j = 0; j < channel_count; j++) {
            buffer[i*channel_count+j] = val;
        }
        count += channel_count;
    }

    std::cout << format("Generated {} samples\n", count);
}

class dsp_sine_generator_plugin {
public:
    using cfg_parser_types = std::tuple<>; //just use the default base_cfg_parsers types
};


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
        AudioEngine::Monitoring::probe_service service(shm.data(), shm_t::page_size); //uses slightly under ~2MB of data which should be one page
        
        AudioEngine::Monitoring::probe_description pd{
            .name = "SETUP_PROBE", 
            .unit = "", 
            .flags = 0
        };
        decltype(service)::probe_handle_t probe_handle = service.add_probe(pd);
    
        auto config = load_config(get_cwd() + "/conf.cfg");

        uint32_t cfg_sample_rate = config.get<uint32_t>("PlayoutSampleRate");
        uint8_t cfg_channels = config.get<uint8_t>("PlayoutChannels");
        int64_t cfg_loop_ms = config.get<int64_t>("LoopDurationMs");
        bool cfg_output_file_enabled = config.get<bool>("DbgAudioOutputEnabled");
        std::string cfg_output_file = config.get<std::string>("DbgAudioOutput");
        int64_t cfg_duration_ms = config.get<int64_t>("SessionDurationMs");
        int64_t cfg_hertz = config.get<int64_t>("SineWaveHertz");

        std::cout << format("Play {}ms loop of {} channel audio at {} Hz for a looped session duration of {}\n", cfg_loop_ms, cfg_channels, cfg_sample_rate, cfg_duration_ms);

        pcm_buff_t buffer(static_cast<uint8_t>(cfg_channels), static_cast<size_t>(cfg_sample_rate / 1000 * cfg_loop_ms), std::allocator<sample_t>());
        auto writer = AudioEngine::circular_buffer_writer(buffer);

        size_t monosize = static_cast<size_t>((cfg_loop_ms * cfg_sample_rate) / 1000);
        /*size_t blockcount = 100;
        size_t blocksize = static_cast<size_t>((cfg_channels * monosize) / blockcount);
        */

        sample_t *buf = new sample_t[monosize * cfg_channels];
        generate_sin_wave(buf, monosize, static_cast<size_t>(cfg_sample_rate), cfg_channels, static_cast<size_t>(cfg_hertz));
        std::cout << format("monosize {} channels {}\n", monosize, cfg_channels);
        writer << std::span<sample_t>(buf, monosize * cfg_channels);
        delete[] buf;

        //write buf into file if enabled in the configuration
        if (cfg_output_file_enabled) {
            std::ios_base::sync_with_stdio(false);
            auto myfile = std::fstream(cfg_output_file, std::ios::out | std::ios::binary);
            myfile.write((char*)buffer.data(), static_cast<std::streamsize>(buffer.size_bytes()));
            myfile.close();
        }







        //allocate on 16 byte alignment from a pool of 128 kibibytes of memory in a single large page
        using miniaudio_allocator = AudioEngine::block_allocator<AudioEngine::s16, 8192>;
        miniaudio_allocator mallocator(reinterpret_cast<void*>(shm.get_page(1))); //use 128 kibibytes of the seccond page

        //example of heap allocating a ma_wrapper via my block_allocator
        auto ctx_allocator = std::allocator_traits<miniaudio_allocator>::rebind_alloc<ma_context>(mallocator);
        ma_context *ctx = ctx_allocator.allocate(1);
        AudioEngine::ma_call(ma_context_init(NULL, 0, NULL, ctx));
        AudioEngine::ma_wrapper<ma_context, ma_context_uninit, decltype(ctx_allocator)> context(ctx, std::move(ctx_allocator));

        auto playout_devices = get_playout_devices(context);
        for (auto& device_info : playout_devices) {
            std::cout << format("{}\n", device_info.name);
        }

        play_data_callback_userdata data = play_data_callback_userdata(pcm_buff_reader_t(buffer));

        ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
        cfg.playback.format = ma_format_s16;
        cfg.playback.channels = cfg_channels;
        cfg.playback.pDeviceID = &playout_devices[0].id;
        cfg.sampleRate = cfg_sample_rate;
        cfg.dataCallback = play_data_callback;
        cfg.pUserData = &data;

        //example of heap allocated and we can leave the default assumed deallocator for ma_wrapper (default_delete<T>)
        ma_device *_out_device = new ma_device();
        AudioEngine::ma_call(ma_device_init(context, &cfg, _out_device));
        AudioEngine::ma_wrapper<ma_device, ma_device_uninit> out_device(_out_device);

        AudioEngine::ma_call(ma_device_start(out_device));

        std::this_thread::sleep_for(std::chrono::milliseconds(cfg_duration_ms));

        //cleanup unneeded, the wrapper dtors clean up my mess 
        //deconstructed in reverse order of construction means I dont need to worry about ordering

    }
    catch (Net::net_error const& e) {
        std::cout << e.what() << "\n";
    }
    catch (std::out_of_range const& e) {
        std::cout << format("Exception: {}\n", e.what());
    }
    catch (AudioEngine::dsp_error const& e) {
        std::cout << e.what() << "\n";
    }

    Net::cleanup();

    return 0;
}