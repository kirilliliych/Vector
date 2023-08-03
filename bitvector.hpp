#ifndef BITVECTOR_HPP
#define BITVECTOR_HPP


#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include "mymove.hpp"
#include "vector.hpp"


const size_t BITS_IN_BYTE = 8;
const size_t MAX_SHIFT    = 7;
const size_t BITS_TO_BYTES_OFFSET = 3;


size_t round_to_eight_multiple(size_t number)
{
    return (number + 7) & (~7ull);
}

size_t bits_to_bytes_quantity(size_t bits_quantity)
{
    return bits_quantity >> 3;
}


struct BitsAndBytes
{
//---------------------------------------------------------------------------------
    BitsAndBytes()
      : BitsAndBytes(0, 0)
    {}

    BitsAndBytes(size_t bytes, size_t bits)
      : bytes_(bytes),
        bits_(bits)
    {}

    BitsAndBytes(size_t bits_quantity)
    {
        *this = convert_bits(bits_quantity);
    }

    BitsAndBytes operator +(int value) const
    {
        BitsAndBytes result{};
        int bits_quantity_result = static_cast<int> ((bytes_ << BITS_TO_BYTES_OFFSET) + bits_) + value;
        if (bits_quantity_result > 0)
        {
            result = convert_bits(static_cast<size_t> (bits_quantity_result));
        }

        return result;
    }

    BitsAndBytes operator -(int value) const
    {
        return operator +(-value);
    }

    BitsAndBytes convert_bits(size_t bits_quantity) const
    {
        size_t bytes = bits_to_bytes_quantity(bits_quantity);

        return BitsAndBytes(bytes, bits_quantity & MAX_SHIFT);
    }    

    size_t bytes_ = 0;
    size_t bits_  = 0;
};


template<>
class Vector<bool>
{
    class BitReference;

//     class ConstBitReference
//     {
//     public:
// //---------------------------------------------------------------------------------
//         ConstBitReference(uint8_t *src, size_t shift = MAX_SHIFT)
//           : src_(src),
//             shift_(shift)
//         {
//             assert(src != nullptr);
//         }

//         ConstBitReference(const ConstBitReference &other) = default;
//         ConstBitReference(const BitReference &other)
//           : src_(const_cast<const uint8_t *> (other.src_)),
//             shift_(other.shift_)
//         {}

//         ConstBitReference &operator =(const ConstBitReference &other) = delete;
//         ConstBitReference &operator =(bool value) = delete;

//         ~ConstBitReference()
//         {
//             src_ = reinterpret_cast<uint8_t *> (const_cast<char *> (DESTR_PTR));
//             shift_  = POISONED_UINT64_T;    
//         }
// //---------------------------------------------------------------------------------
//         operator bool() const
//         {
//             return *src_ & (1 << shift_);
//         }

//         bool operator ==(const ConstBitReference &other) const
//         {
//             return (bool) *this == (bool) other; 
//         }

//         bool operator !=(const ConstBitReference &other) const
//         {
//             return !operator ==(other);
//         }

//         bool operator <(const ConstBitReference &other) const
//         {
//             return !((bool) *this) && (bool) other;
//         }

//         bool operator >(const ConstBitReference &other) const
//         {
//             return other.operator <(*this);
//         }

//         bool operator <=(const ConstBitReference &other) const
//         {
//             return !operator >(other);
//         }

//         bool operator >=(const ConstBitReference &other) const
//         {
//             return !operator <(other);
//         }

//     private:
// //-----------------------------------Variables-------------------------------------
//         const uint8_t *src_ = reinterpret_cast<const uint8_t *> (UNINIT_PTR);
//         size_t shift_ = 0;
//     };

//     class BitReference
//     {
//         friend class ConstBitReference;

//     public:
// //---------------------------------------------------------------------------------
//         BitReference(uint8_t *src, size_t shift = MAX_SHIFT)
//           : src_(src),
//             shift_(shift)
//         {
//             assert(src != nullptr);
//         }

//         BitReference(const BitReference &other) = default;


//         BitReference &operator =(const BitReference &other)
//         {
//             return *this = (bool) other;
//         }

//         BitReference &operator =(bool value)
//         {
//             if (value)
//             {
//                 *src_ |=  (1 << shift_);
//             }
//             else
//             {
//                 *src_ &= ~(1 << shift_);
//             }

//             return *this;
//         }

//         ~BitReference()
//         {
//             src_ = reinterpret_cast<uint8_t *> (const_cast<char *> (DESTR_PTR));
//             shift_  = 0;    
//         }
// //---------------------------------------------------------------------------------
//         operator bool() const
//         {
//             return *src_ & (1 << shift_);
//         }

//         bool operator ==(const BitReference &other) const
//         {
//             return (bool) *this == (bool) other; 
//         }

//         bool operator !=(const BitReference &other) const
//         {
//             return !operator ==(other);
//         }

//         bool operator <(const BitReference &other) const
//         {
//             return !((bool) *this) && (bool) other;
//         }

//         bool operator >(const BitReference &other) const
//         {
//             return other.operator <(*this);
//         }

//         bool operator <=(const BitReference &other) const
//         {
//             return !operator >(other);
//         }

//         bool operator >=(const BitReference &other) const
//         {
//             return !operator <(other);
//         }

//     private:
// //-----------------------------------Variables-------------------------------------
//         uint8_t *src_ = reinterpret_cast<uint8_t *> (const_cast<char *> (UNINIT_PTR));
//         size_t shift_ = 0;
//     };

    class BitReference
    {
    public:
//---------------------------------------------------------------------------------
        BitReference(uint8_t *byte, size_t shift = MAX_SHIFT)
          : byte_(byte),
            shift_(shift)
        {
            assert(byte != nullptr);
            assert(shift <= MAX_SHIFT);
        }

        BitReference(const BitReference &other)
          : byte_(other.byte_),
            shift_(other.shift_)
        {}

        BitReference(BitReference &&other)
          : byte_(other.byte_),
            shift_(other.shift_)
        {
            other.byte_  = const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (MOVED_REMAINDERS_PTR));
            other.shift_ = 0;
        }

        BitReference &operator =(bool bit_value)
        {
            set_bit_(bit_value);            

            return *this;
        }

        BitReference &operator =(const BitReference &other)
        {
            *this = (bool) other;

            return *this;
        }

        BitReference &operator =(BitReference &&other)
        {
            *this = other;

            return *this;
        }

        ~BitReference()
        {
             byte_ = const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (DESTR_PTR));
            shift_ = POISONED_UINT64_T;
        }

    public:
//---------------------------------------------------------------------------------
    operator bool() const
    {
        return get_bit_();
    }

    private:

    bool get_bit_() const
    {
        return (*byte_ >> shift_) & 0x1;
    }

    void set_bit_(bool bit_value)
    {
        *byte_ = bit_value ? (*byte_ | (0x1 << shift_)) : (*byte_ & ~(0x1 << shift_));
    }

    private:
//----------------------------------Variables--------------------------------------
        uint8_t *byte_ = reinterpret_cast<uint8_t *> (const_cast<char *> (UNINIT_PTR));
        size_t shift_  = 0;
    };

    template<typename Container, typename ItType>
    class BitIterator
    {
        static const bool is_const = std::is_same_v<const typename Container::value_type, ItType>;

        friend typename std::conditional_t<is_const,
                        BitIterator<std::remove_const_t<Container>, std::remove_const_t<ItType>>,
                        BitIterator<const Container, const ItType>>;

        // typedef typename choose<IsConst, ConstBitReference,   BitReference>::type Reference;
        // typedef typename choose<IsConst, ConstBitReference *, BitReference *>::type Pointer;
        // typedef typename choose<IsConst, const Vector<bool> *, Vector<bool> *>::type ContainerPtr;

    public:
        using iterator_category = std::contiguous_iterator_tag;
        using value_type = bool;
        using difference_type = ptrdiff_t;
        using reference = std::conditional_t<is_const, typename Container::const_reference,
                                                       typename Container::reference>;
        using pointer   = std::conditional_t<is_const, typename Container::const_pointer,
                                                       typename Container::pointer>;
//---------------------------------------------------------------------------------
        BitIterator()
          : container_(reinterpret_cast<Container *> (UNINIT_PTR)),
            data_(const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (UNINIT_PTR))),
            shift_(MAX_SHIFT)
        {}
    
        BitIterator(Container *container, uint8_t *data, size_t shift = MAX_SHIFT)
          : container_(container),
            data_(data),
            shift_(shift)
        {
            assert(container != nullptr);
            assert(data   != nullptr);
        }

        template<typename OtherContainer, typename OtherItType>
        BitIterator(const BitIterator<OtherContainer, OtherItType> &other)
          : container_(reinterpret_cast<Container *> (const_cast<uint64_t *> (reinterpret_cast<const uint64_t *> (other.container_)))),
            data_(other.data_),
            shift_(other.shift_)
        {}

        template<typename OtherContainer, typename OtherItType>
        BitIterator(BitIterator<OtherContainer, OtherItType> &&other)
        {
            *this = std::move(other);
        }

        template<typename OtherContainer, typename OtherItType>
        BitIterator &operator =(const BitIterator<OtherContainer, OtherItType> &other)
        {
            if (reinterpret_cast<void *> (this) != reinterpret_cast<void *> (other))
            {
                container_ = other.container_;
                data_  = other.data_;
                shift_ = other.shift_;
            }

            return *this;
        }

        template<typename OtherContainer, typename OtherItType>
        BitIterator &operator =(BitIterator<OtherContainer, OtherItType> &&other)
        {
            if (*this != other)
            {
                container_ = other.container_;
                data_  = other.data_;
                shift_ = other.shift_;

                other.container_ = reinterpret_cast<OtherContainer *> (const_cast<uint64_t *> (reinterpret_cast<const uint64_t *> (MOVED_REMAINDERS_PTR)));
                other.data_      = const_cast<uint8_t *> (reinterpret_cast<const uint8_t *>(MOVED_REMAINDERS_PTR));
                other.shift_     = POISONED_UINT64_T;
            }

            return *this;
        }

        ~BitIterator()
        {
            container_ = const_cast<Container *> (reinterpret_cast<const Container *> (DESTR_PTR));
            data_ = const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (DESTR_PTR));
            shift_ = POISONED_UINT64_T;
        }
//---------------------------------------------------------------------------------
        reference operator *() const
        {
            return reference(data_, shift_);
        }

        BitIterator &operator ++()
        {
            if (shift_ == 0)
            {
                ++data_;
                shift_ = MAX_SHIFT;
            }
            else
            {
                --shift_;
            }

            return *this;
        }

        BitIterator operator ++(int)
        {
            BitIterator prev = *this;
            ++(*this);

            return prev;
        }

        BitIterator &operator --()
        {
            if (shift_ == MAX_SHIFT)
            {
                --data_;
                shift_ = 0;
            }
            else
            {
                ++shift_;
            }

            return *this;
        }

        BitIterator operator --(int)
        {
            BitIterator prev = *this;
            --(*this);

            return prev;
        }

        BitIterator &operator +=(difference_type value)
        {
            if (value > 0)
            {
                while (value > 0)
                {
                    ++(*this);

                    --value;
                }
            }
            else
            {
                while (value < 0)
                {
                    --(*this);

                    ++value;
                }
            }

            return *this;
        }

        BitIterator &operator -=(difference_type value)
        {
            return operator +=(-value);
        }

        BitIterator operator +(difference_type value) const
        {
            BitIterator result = *this;

            return result += value;
        }

        friend BitIterator operator +(difference_type value, const BitIterator &other)
        {
            return other + value;
        }

        BitIterator operator -(difference_type value) const
        {
            BitIterator result = *this;

            return result -= value;
        }

        difference_type operator -(const BitIterator &other) const
        {
            return (data_ - other.data_) * static_cast<difference_type> (BITS_IN_BYTE) +
                                           static_cast<difference_type> (other.shift_) -
                                           static_cast<difference_type> (shift_);
        }

        reference operator [](difference_type value) const
        {
            return *(*this + value);
        }

                                                                                                    // dude just make spaceship lul
        // bool operator ==(const BitIterator<true> &other) const
        // {
        //     return (data_ == other.data_) && (shift_ == other.shift_); 
        // }    
        // bool operator ==(const BitIterator<false> &other) const
        // {
        //     return operator ==(BitIterator<true>(other));
        // }
        template<typename OtherContainer, typename OtherItType>
        bool operator ==(const BitIterator<OtherContainer, OtherItType> &other) const
        {
            return (data_ == other.data_) && (shift_ == other.shift_);
        }

        // bool operator !=(const BitIterator<true> &other) const
        // {
        //     return !operator ==(other);
        // }
        // bool operator !=(const BitIterator<false> &other) const
        // {
        //     return operator !=(BitIterator<true>(other));
        // }
        template<typename OtherContainer, typename OtherItType>
        bool operator !=(const BitIterator<OtherContainer, OtherItType> &other) const
        {
            return !operator ==(other);
        }

        // bool operator <(const BitIterator<true> &other) const
        // {
        //     return (data_ < other.data_) || ((data_ == other.data_) && (shift_ > other.shift_));
        // }
        // bool operator <(const BitIterator<false> &other) const
        // {
        //     return operator <(BitIterator<true>(other));
        // }
        template<typename OtherContainer, typename OtherItType>
        bool operator <(const BitIterator<OtherContainer, OtherItType> &other) const
        {
            return (data_ < other.data_) || ((data_ == other.data_) && (shift_ > other.shift_));
        }

        // bool operator >(const BitIterator<true> &other) const
        // {
        //     return other < *this;
        // }
        // bool operator >(const BitIterator<false> &other) const
        // {
        //     return operator <(BitIterator<true>(other));
        // }
        template<typename OtherContainer, typename OtherItType>
        bool operator >(const BitIterator<OtherContainer, OtherItType> &other) const
        {
            return other < *this;
        }

        // bool operator <=(const BitIterator<true> &other) const
        // {
        //     return !operator >(other);
        // }
        // bool operator <=(const BitIterator<false> &other) const
        // {
        //     return !operator >(BitIterator<true>(other));
        // }
        template<typename OtherContainer, typename OtherItType>
        bool operator <=(const BitIterator<OtherContainer, OtherItType> &other) const
        {
            return !operator >(other);
        }

        // bool operator >=(const BitIterator<true> &other) const
        // {
        //     return !operator <(other);
        // }
        // bool operator >=(const BitIterator<false> &other) const
        // {
        //     return !operator <(BitIterator<true>(other));
        // }
        template<typename OtherContainer, typename OtherItType>
        bool operator >=(const BitIterator<OtherContainer, OtherItType> &other) const
        {
            return !operator <(other);
        }

    private:
//-------------------------------------Variables-----------------------------------
        Container *container_ = nullptr;

        uint8_t *data_ = nullptr;
        size_t shift_  = 0;
    };

public:

    using value_type        = bool;
    using pointer           = bool *;
    using const_pointer     = const bool *;
    using reference         = BitReference;
    using const_reference   = const BitReference;
    using difference_type   = std::ptrdiff_t;

    using Iterator = BitIterator<Vector<bool>, bool>;

    using ConstIterator = BitIterator<const Vector<bool>, const bool>;
//---------------------------------------------------------------------------------
    Vector()
      : capacity_       (0),
        booked_capacity_(0),
        size_           (0),
        data_           (reinterpret_cast<uint8_t *> (const_cast<char *> (UNINIT_PTR)))
    {}

    Vector(const std::initializer_list<bool> &init_list)
    {
        size_t list_size = init_list.size();
        for (size_t index = 0; index < list_size; ++index)
        {
            push_back(*(init_list.begin() + index));
        }
    }

    Vector(const size_t reserved_size, bool value = false)
      : capacity_       (round_to_eight_multiple(reserved_size)),
        booked_capacity_(reserved_size),
        size_           (reserved_size),
        data_(new uint8_t[bits_to_bytes_quantity(capacity_)]{value})
    {
        if (reserved_size != 0)
        {   
            init_elements_(0, reserved_size, value);
        }
    }

    Vector(const Vector<bool> &other)
      : capacity_(round_to_eight_multiple(other.capacity_)),
        booked_capacity_(other.capacity_),
        size_(other.size_),
        data_(new uint8_t[bits_to_bytes_quantity(capacity_)])
    {
        // BitIterator<false> data_copy_to(this, data_);
        // BitIterator<true> data_copy_from(this, other.data_);
        Iterator data_copy_to(this, data_);
        ConstIterator data_copy_from(this, other.data_);
        copy_data_(data_copy_to, data_copy_from, booked_capacity_);
    }

    Vector(Vector<bool> &&other)
    {
        *this = std::move(other);
    }

    Vector<bool> &operator =(const Vector<bool> &other)
    {
        if ((data_ != const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (UNINIT_PTR))) && data_is_valid_())
        {
            delete [] data_;
        }

        capacity_        = other.capacity_;
        booked_capacity_ = other.booked_capacity_;
        size_            = other.size_;
        data_            = new uint8_t[bits_to_bytes_quantity(capacity_)];

        // BitIterator<false> data_copy_to( this, data_);
        // BitIterator<true> data_copy_from(this, other.data_);
        // copy_data_(data_copy_to, data_copy_from, booked_capacity_);
        Iterator data_copy_to(this, data_);
        ConstIterator data_copy_from(this, other.data_);
        copy_data_(data_copy_to, data_copy_from, booked_capacity_);

        return *this;
    }

    Vector<bool> &operator =(Vector<bool> &&other)
    {
        std::swap(capacity_, other.capacity_);
        std::swap(booked_capacity_, other.booked_capacity_);
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);

        return *this;
    }

    ~Vector()
    {
        delete [] data_;

        destroy_fields_();
    }
//---------------------------------Dump--------------------------------------------
    void dump(size_t from = 0, size_t to = VECTOR_MAX_CAPACITY + 1)
    {
        if (to == VECTOR_MAX_CAPACITY + 1)
        {
            to = capacity_;
        }

        std::cout << std::endl;
        std::cout << "---------------DUMP---------------" << std::endl;
        std::cout << "Vector<bool>[" << this << "]" << std::endl;
        std::cout << "capacity_: " << capacity_ << std::endl;
        std::cout << "booked_capacity_: " << booked_capacity_ << std::endl;
        std::cout << "size_: " << size_ << std::endl;
        printf("data_: %p\n\n", data_);

        std::string value = "";
        for (; from < to; ++from)
        {
            std::cout << "vector[" << from << "] = ";
            if (get_bit_value_(from))
            {
                std::cout << "1" << std::endl;
                value.push_back('1');
            }
            else
            {
                std::cout << "0" << std::endl;
                value.push_back('0');
            }
        }
        std::cout << value << "\n---------------DUMP ENDED---------------\n\n" << std::endl;
    }
//--------------------------------Verificator--------------------------------------
    void verificator()
    {
        assert(size_ <= booked_capacity_);
        assert(booked_capacity_ <= capacity_);
        assert(capacity_ <= VECTOR_MAX_CAPACITY);
        assert(data_ != nullptr);
        assert(data_ != const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (INVALID_PTR)));
    }
//--------------------------------Size and capacity--------------------------------
    bool empty() const
    {
        return size_ == 0;
    }

    bool size() const
    {
        return size_;
    }

    size_t max_size() const
    {
        return VECTOR_MAX_CAPACITY;
    }

    size_t capacity() const
    {
        return booked_capacity_;
    }

    void reserve(size_t reserved_capacity)
    {
        if (reserved_capacity <= capacity_)
        {
            if (booked_capacity_ < reserved_capacity)
            {
                booked_capacity_ = reserved_capacity;
            }

            return;
        }

        size_t actual_capacity = 0;
        uint8_t *new_data = vector_realloc_(reserved_capacity, &actual_capacity);

        if ((data_ != const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (UNINIT_PTR))) && data_is_valid_())
        {
            delete [] data_;
        }

        data_            = new_data;
        booked_capacity_ = reserved_capacity;
        capacity_        = actual_capacity;
    }

    void shrink_to_fit()
    {
        if (capacity_ - size_ <= BITS_IN_BYTE)
        {
            return;
        }

        size_t actual_capacity = 0;
        uint8_t *new_data = vector_realloc_(size_, &actual_capacity);

        if ((data_ != const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (UNINIT_PTR))) && data_is_valid_())
        {
            delete [] data_;
        }

        data_            = new_data;
        booked_capacity_ = size_;
        capacity_        = actual_capacity;
    }
//-------------------------------Element access----------------------------------
    const BitReference operator [](size_t index) const
    {
        return const_cast<Vector<bool> *> (this)->operator[](index);
    }

    BitReference operator [](size_t index)
    {
        assert(index < booked_capacity_);

        BitsAndBytes shift(index);

        return BitReference(data_ + shift.bytes_, MAX_SHIFT - shift.bits_);
    }

    const BitReference at(size_t index) const
    {
        return const_cast<Vector<bool> *> (this)->at(index);
    }

    BitReference at(size_t index)
    {
        if (index < booked_capacity_)
        {
            return operator[](index);
        }

        std::cerr << "ERROR(BitVector " << this << "): attempt to obtain value out of bounds" << std::endl;
    }

    bool front() const
    {
        return const_cast<Vector<bool> *> (this)->front();
    }

    bool front()
    {
        return operator[](0);
    }

    bool back() const
    {
        return const_cast<Vector<bool> *> (this)->back();
    }

    bool back()
    {
        return operator[](size_ - 1);
    }

    const uint8_t *data() const
    {
        return const_cast<const uint8_t *> (const_cast<Vector<bool> *> (this)->data());
    }

    uint8_t *data()
    {
        return data_;
    }

//---------------------------------Iterators---------------------------------------
    // BitIterator<false> begin()
    // {
    //     return BitIterator<false>(this, data_);
    // }
    Iterator begin()
    {
        // return BitIterator<Vector<bool>, value_type>(this, data_);
        return Iterator(this, data_);
    }

    ConstIterator begin() const
    {
        return cbegin();
    }

    // BitIterator<true> cbegin() const
    // {
    //     return BitIterator<true>(this, data_);
    // }
    ConstIterator cbegin() const
    {
        return ConstIterator(this, data_);
    }

    // BitIterator<false> end()
    // {
    //     BitsAndBytes end(size_);

    //     return BitIterator<false> (this, data_ + end.bytes_, MAX_SHIFT - end.bits_);
    // }
    Iterator end()
    {
        BitsAndBytes end(size_);

        return Iterator(this, data_ + end.bytes_, MAX_SHIFT - end.bits_);
    }

    ConstIterator end() const
    {
        return cend();
    }

    // BitIterator<true> cend() const
    // {
    //     BitsAndBytes end(size_);

    //     return BitIterator<true>(this, data_ + end.bytes_, MAX_SHIFT - end.bits_);
    // }
    ConstIterator cend() const
    {
        BitsAndBytes end(size_);

        return ConstIterator(this, data_ + end.bytes_, MAX_SHIFT - end.bits_);
    }

    std::reverse_iterator<ConstIterator> crbegin() const
    {
        return std::make_reverse_iterator(cend());
    }

    std::reverse_iterator<Iterator> rbegin()
    {
        return std::make_reverse_iterator(end());
    }

    std::reverse_iterator<ConstIterator> rbegin() const
    {
        return crbegin();
    }

    std::reverse_iterator<ConstIterator> crend() const
    {
        return std::make_reverse_iterator(cbegin());
    }

    std::reverse_iterator<Iterator> rend()
    {
        return std::make_reverse_iterator(begin());
    }

    std::reverse_iterator<ConstIterator> rend() const
    {
        return crend();
    }

//----------------------------------Modifiers--------------------------------------
    void clear()
    {
        size_ = 0;
    }

    // BitIterator<false> insert(BitIterator<true> pos, bool value)
    Iterator insert(ConstIterator pos, bool value)
    {
        ptrdiff_t index = pos - cbegin();
        if ((index < 0) || (index > static_cast<ptrdiff_t> (size_)))
        {
            std::cerr << "ERROR(Vector<bool> " << this << "): attempt to insert out of bounds" << std::endl;

            return end();
        }

        reserve(size_ + 1);
        init_elements_(size_, size_ + 1);
        BitsAndBytes data_copy_to_index(index + 1);
        BitsAndBytes data_copy_from_index(index);
        // BitIterator<false> data_copy_to (this, data_ + data_copy_to_index.bytes_,  MAX_SHIFT - data_copy_to_index.bits_);
        // BitIterator<true> data_copy_from(this, data_ + data_copy_from_index.bytes_,MAX_SHIFT - data_copy_from_index.bits_);
        Iterator data_copy_to (this, data_ + data_copy_to_index.bytes_,  MAX_SHIFT - data_copy_to_index.bits_);
        ConstIterator data_copy_from(this, data_ + data_copy_from_index.bytes_,MAX_SHIFT - data_copy_from_index.bits_);
        copy_data_(data_copy_to, data_copy_from, size_ - index);

        ++size_;

        set_bit_value_(static_cast<size_t> (index), value);
        
        return begin() + index;
    }

    // BitIterator<false> erase(BitIterator<true> pos)
    Iterator erase(ConstIterator pos)
    {
        ptrdiff_t index = pos - cbegin();
        if ((index < 0) || (index > static_cast<ptrdiff_t> (size_)))
        {
            std::cerr << "ERROR(Vector<bool> " << this << "): attempt to erase out of bounds" << std::endl;

            return end();
        }

        BitsAndBytes data_copy_to_index(index);
        BitsAndBytes data_copy_from_index(index + 1);
        // BitIterator<false> data_copy_to (this, data_ + data_copy_to_index.bytes_,   MAX_SHIFT - data_copy_to_index.bits_);
        // BitIterator<true> data_copy_from(this, data_ + data_copy_from_index.bytes_, MAX_SHIFT - data_copy_from_index.bits_);
        Iterator data_copy_to (this, data_ + data_copy_to_index.bytes_,   MAX_SHIFT - data_copy_to_index.bits_);
        ConstIterator data_copy_from(this, data_ + data_copy_from_index.bytes_, MAX_SHIFT - data_copy_from_index.bits_);
        copy_data_(data_copy_to, data_copy_from, size_ - index - 1);

        --size_;

        return begin() + index;
    }

    void push_back(bool value)
    {
        insert(begin() + size_, value);
    }

    void pop_back()
    {
        if (size_ == 0)
        {
            std::cerr << "ERROR(Vector<bool> " << this << "): null pop attempt" << std::endl; 

            return;
        }

        erase(begin() + size_ - 1);
    }

    void resize(size_t new_size, bool value = false)
    {
        if (new_size > VECTOR_MAX_CAPACITY)
        {
            std::cerr << "ERROR(BitVector " << this << "): resizing requires too much memory" << std::endl;

            return;
        }

        if (new_size <= size_)
        {
            size_ = new_size;

            return;
        }

        if (new_size <= capacity_)
        {
            init_elements_(size_, new_size, value);

            size_ = new_size;
            if (new_size > booked_capacity_)
            {
                booked_capacity_ = new_size;
            }

            return;
        }

        size_t actual_capacity = 0;
        uint8_t *new_data = vector_realloc_(new_size, &actual_capacity);

        if ((data_ != const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (UNINIT_PTR))) && data_is_valid_())
        {
            delete [] data_;
        }

        data_            = new_data;
        capacity_        = actual_capacity;
        booked_capacity_ = new_size;
        size_            = new_size;

    }

    void swap(Vector<bool> &other)
    {
        Vector<bool> temp = my_move(other);
        other = my_move(*this);
        *this = my_move(temp);
    }

private:
//--------------------------------Utilitary functions------------------------------
    bool get_bit_value_(size_t where)
    {
        BitsAndBytes pos(where);
        BitReference where_to_get(data_ + pos.bytes_, MAX_SHIFT - pos.bits_);

        return (bool) where_to_get;
    }

    void set_bit_value_(size_t where, bool value)
    {
        BitsAndBytes pos(where);
        BitReference where_to_set(data_ + pos.bytes_, MAX_SHIFT - pos.bits_);
        where_to_set = value;
    }

    void init_elements_(size_t from, size_t to, const bool value = false)
    {
        for (size_t index = from; index < to; ++index)
        {
            set_bit_value_(index, value);
        }
    }

    // void copy_data_(BitIterator<false> dest, BitIterator<true> src, size_t quantity)
    void copy_data_(Iterator dest, ConstIterator src, size_t quantity)
    {
        if (dest == src)
        {
            return;
        }

        if (dest < src)
        {
            for (int counter = 0; counter < static_cast<int> (quantity); ++counter)
            {
                *(dest + counter) = *(src + counter);
            }
        }
        else
        {
            for (int counter = static_cast<int> (quantity - 1); counter >= 0; --counter)
            {
                *(dest + counter) = *(src + counter); 
            }
        }
    }

    uint8_t *vector_realloc_(size_t new_capacity, size_t *actual_capacity)
    {
        assert(actual_capacity != nullptr);

        *actual_capacity = round_to_eight_multiple(new_capacity);
        uint8_t *new_data = new uint8_t[bits_to_bytes_quantity(*actual_capacity)];

        // BitIterator<true>  data_copy_from(this, data_);
        // BitIterator<false> data_copy_to(this, new_data);
        Iterator data_copy_from(this, data_);
        ConstIterator data_copy_to(this, new_data);
        copy_data_(data_copy_to, data_copy_from, size_);

        return new_data;
    }

    bool data_is_valid_() const
    {
        return (data_ != const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (DESTR_PTR))) &&
               (data_ != const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (INVALID_PTR)));
    }

    void destroy_fields_()
    {
        capacity_ = POISONED_UINT64_T;
        size_     = POISONED_UINT64_T;
        data_     = const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (DESTR_PTR));
    }

    // int vector_cmp_(const Vector<bool> &other)
    // {   
    //     size_t v1_size = v1.size();
    //     size_t v2_size = v2.size();
    //     size_t min_size = std::min(v1_size, v2_size);

    //     auto v1_iter = v1.cbegin();
    //     auto v2_iter = v2.cbegin();
    
    //     for (size_t cur_elem_index = 0; cur_elem_index < min_size; ++cur_elem_index)
    //     {   
    //         const Type *v1_data_ptr = v1.data() + cur_elem_index;
    //         const Type *v2_data_ptr = v2.data() + cur_elem_index;

    //         if (*v1_data_ptr > *v2_data_ptr)
    //         {
    //             return 1;
    //         }
    //         if (*v1_data_ptr < *v2_data_ptr)
    //         {
    //             return -1;
    //         }
    //     }

    //     if (v1_size > min_size)
    //     {
    //         return 1;
    //     }
    //     if (v2_size > min_size)
    //     {
    //         return -1;
    //     }

    //     return 0;
    // }

private:
//-----------------------------------Variables-------------------------------------
    static constexpr uint64_t VECTOR_MAX_CAPACITY       = 1024;
    static constexpr uint64_t DEFAULT_RESIZE_MULTIPLIER = 2;

    size_t capacity_        = 0;
    size_t booked_capacity_ = 0;
    size_t size_            = 0;

    uint8_t *data_ = reinterpret_cast<uint8_t *> (const_cast<char *> (UNINIT_PTR));
};


#endif
