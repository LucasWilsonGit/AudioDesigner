#pragma once

#include <vector>

namespace AudioEngine {

    /** tricks to force 16byte alignment, pass in an allocator that allocates on 16 byte boundaries, e.g block_allocator<my_16byte_struct, 2048> = aligned allocation for 32kibibytes of memory
     *  Not thread safe
     */
    template <class Type, class Alloc, size_t Capacity = 512>
    class sparse_collection {
        using key_t = std::optional<size_t>;

    public:
        struct entry_handle {
        protected:
            friend class sparse_collection<Type, Alloc, Capacity>;
            size_t key_index;
        };

        struct TypeStorage {
            Type value;
            size_t key_index;
        };

        using alloc_t = typename Alloc::template rebind<TypeStorage>::other; 
        using container_t = std::vector<TypeStorage, alloc_t>;

    private:

        

        Alloc m_alloc;
        container_t m_values;
        std::array<key_t, Capacity> m_keys alignas(16);

        size_t get_new_key_idx() const {
            for (size_t i = 0; i < m_keys.size(); i++) {
                if (!m_keys[i].has_value()) {
                    return i;
                }
            }
            throw std::bad_alloc();
        }

    public:
        using reference = Type&;
        using const_reference = Type const&;

        sparse_collection(Alloc const& allocator) :
            m_alloc(allocator),
            m_values(alloc_t(m_alloc))
        {}

        [[nodiscard]] container_t& values() noexcept  {
            return m_values;
        }
        [[nodiscard]] container_t const& cvalues() const noexcept {
            return m_values;
        }

        [[nodiscard]] entry_handle add(Type&& t) {
            size_t kidx = get_new_key_idx();
            size_t vidx = m_values.size();

            m_keys[kidx] = vidx;

            m_values.push_back( TypeStorage(std::forward<Type>(t), kidx) );

            return *(entry_handle*)(&kidx);
        }

        template <class... Args>
        [[nodiscard]] entry_handle emplace(Args&&... arg) {
            return add(Type(std::forward<Args>(arg)...));
        }

        void remove(entry_handle k) {
            if (k.key_index >= Capacity)
                throw std::out_of_range("Attempt to remove key beyond capacity of sparse_collection");

            auto& removing_key = m_keys[k.key_index];
            auto& removing_value = m_values[removing_key.value()];

            auto& old_last_value = m_values.back();
            auto& old_last_key = m_keys[old_last_value.key_index];

            removing_value = std::move(old_last_value);

            old_last_key = removing_key.value();
            removing_key = std::nullopt;

            m_values.pop_back();
        }

        [[nodiscard]] reference get(entry_handle k) {
            auto& key = m_keys[k.key_index];
            return m_values[key.value()].value;
        }
        
        [[nodiscard]] const_reference get(entry_handle k) const  {
            auto& key = m_keys[k.key_index];
            return m_values[key.value()].value;
        }

        auto begin() noexcept { return m_values.begin(); }
        auto end() noexcept { return m_values.end(); }

        auto cbegin() const noexcept { return m_values.cbegin(); }
        auto cend() const noexcept { return m_values.cend(); }
    };

}