#pragma once
#include "AudioEngine/core.hpp"
#include "AudioEngine/block_allocator.hpp"
#include "AudioEngine/sparse_collection.hpp"

#include <array>
#include <vector>
#include <chrono>
#include <memory>
#include <new>
#include <optional>

namespace AudioEngine {
    namespace Monitoring {

        template <class T>
        struct data_point {
            std::chrono::steady_clock::time_point timestamp;
            T value;

            data_point(T&& val) : value(std::forward<T>(val)) {}

            auto operator<=>(const data_point& other) const {
                return value <=> other.value;
            }
        };
        using datapoint_t = data_point<int64_t>;

        struct probe_metadata {
            uint16_t flags;
            char const* unit;
            std::optional<std::string> last_str; 
        };

        //fits exactly into one 64 byte cache line to avoid false sharing, it was close to 64 anyway and I wanted to have m_name to be on a 16 byte alignment without changing it's offset
        template <class DataAllocT = std::allocator<std::byte>>
        class probe {
        public:
            using datapoint_alloc_t = typename std::allocator_traits<DataAllocT>::template rebind_alloc<datapoint_t>;
            using collection_t = std::vector<datapoint_t, datapoint_alloc_t>;
        private:
            
            
            char const *m_name;//0 bytes offset 
            probe_metadata *m_metadata = nullptr; //8 bytes offset
            collection_t m_collection; //16 bytes offset, indeterminate size

            //to force 16 byte alignment
            static constexpr size_t padval = 16 + sizeof(collection_t);
            
            //force 16 byte alignment (Turn on iff pragma pack 1)
            char pad[ ((padval + 15 ) & ~15ull)]; //pad structure size to first multiple of 16

        public:
            probe(char const *name, probe_metadata *meta, datapoint_alloc_t& allocator) :
                m_name(name),
                m_metadata(meta),
                m_collection(allocator)
            {}

            probe_metadata const *get_metadata() const noexcept { return m_metadata; }
            probe_metadata *get_metadata() noexcept { return m_metadata; }
            char const *get_name() const noexcept { return m_name; }
            collection_t& get_data() noexcept { return m_collection; }
            
            template <class T>
            bool add_value(data_point<T>&& data) { 

                std::cout << "1 (" << data.value << ")\n";
                if (m_collection.size() > 0 && data.value == m_collection.back().value)
                    return false;
                
                std::cout << "2\n";
                auto it = std::lower_bound(
                    m_collection.begin(), 
                    m_collection.end(), 
                    data, 
                    [](const data_point<T>& l, const data_point<T>& r) { 
                        return l.timestamp < r.timestamp; 
                    });

                std::cout << "3\n";
                m_collection.insert(it, std::move(data));  
                return true;
            }
        };

        struct probe_description {
            char const* name;
            char const* unit;
            uint16_t flags;
        };

        class probe_service {
        public:
            static constexpr size_t max_probe = 512;
            static constexpr size_t avg_data_count = 128;

            using data_point_allocator_t = block_allocator<datapoint_t, max_probe * avg_data_count>; //1 mebibyte
            using probe_t = probe<data_point_allocator_t>;
            

            using probe_allocator_t = block_allocator< probe_t, max_probe>; //24 kibibytes
            using metadata_allocator_t = block_allocator<probe_metadata, max_probe>; //8 kibibytes

        private:
            struct alloc_buff_layout_s {
                alignas(probe_t) std::array<std::byte, max_probe * sizeof(probe_t)> probe_storage;
                std::array<probe_metadata, max_probe> metadata_storage;
                alignas(data_point<int64_t>) std::array<std::byte, max_probe * avg_data_count * sizeof(data_point<int64_t>)> data_storage;
            };

            using probe_collection_t = sparse_collection<probe_t, probe_allocator_t>;
            char const* m_name;
            void* m_storage;
            alloc_buff_layout_s *m_p_alloc_buffer_storage;

            probe_allocator_t m_probe_allocator;
            data_point_allocator_t m_datapoint_allocator;
            metadata_allocator_t m_meta_allocator;

            probe_collection_t m_probes;

            alloc_buff_layout_s* make_storage(void* buffer, size_t buffer_size) {
                void* aligned = buffer;
                if (std::align(alignof(alloc_buff_layout_s), sizeof(alloc_buff_layout_s), aligned, buffer_size) == nullptr) {
                    throw std::bad_alloc();
                }

                return new (aligned) alloc_buff_layout_s;
            }
        public:
            using probe_handle_t = probe_collection_t::entry_handle;
        private:

            [[nodiscard]] probe_handle_t find_probe(char const* name) {
                for (auto& e : m_probes) {
                    if (std::string_view(e.value.get_name()).find(name) != std::string_view::npos) {
                        return *(probe_handle_t*)(&e.key_index);
                    }
                }

                throw  dsp_error( format("Failed to find probe with name {}", name) );
            }

        public:
            

            /**
             * *probe_service*
             * @param data_buffer Pointer to 2MB of memory for the service to allocate from
             */
            probe_service(void* buffer, size_t buffer_size) :
                m_p_alloc_buffer_storage(make_storage(buffer, buffer_size)),

                m_probe_allocator(probe_allocator_t(m_p_alloc_buffer_storage->probe_storage.data())),
                m_datapoint_allocator(data_point_allocator_t(m_p_alloc_buffer_storage->probe_storage.data())),
                m_meta_allocator(metadata_allocator_t(m_p_alloc_buffer_storage->metadata_storage.data())),

                m_probes(m_probe_allocator)
            {}

            ~probe_service() {}

            //need a method to register a probe
            //need a method to push data at a probe

            [[nodiscard]] probe_handle_t add_probe(probe_description const& description) {

                probe_metadata *meta = m_meta_allocator.allocate(1);
                meta->flags = description.flags;
                meta->unit = description.unit;

                return m_probes.emplace(description.name, meta, m_datapoint_allocator);
            }

            void remove_probe(probe_handle_t handle) {
                //TODO: Swap to using unique_ptr for probe_metadata with a deleter so that this isn't quite so fragile. 
                auto& probe = m_probes.get(handle);
                probe_metadata const *meta = nullptr;
                if ( (meta = probe.get_metadata()) ) {
                    meta->~probe_metadata();
                    m_meta_allocator.deallocate((probe_metadata*)meta, 1); //C-style cast to get rid of const-ness but this is OK for deallocation I feel
                }

                m_probes.remove(handle);
            }

            

            bool send_probe_value(char const* probe_name, std::string&& v) {
                probe_t& probe = m_probes.get(find_probe(probe_name));
                probe_metadata *meta = probe.get_metadata();
                if (meta->last_str && v == meta->last_str.value()) {
                    return false;
                }
                else {
                    meta->last_str = std::move(v);
                    return true;
                }
            }

            bool send_probe_value(char const* probe_name, int64_t&& v) {
                probe_t& probe = m_probes.get(find_probe(probe_name));
                std::cout << "v is: " << v << "\n";
                return probe.add_value(datapoint_t(std::move(v)));
            }

            auto& get_probe_data(probe_handle_t const& handle) {
                return m_probes.get(handle).get_data();
            }

        };
    }
}