#ifndef BITVECTOR_HPP
#define BITVECTOR_HPP


#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <type_traits>
#include "my_move.hpp"
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

    class ConstBitReference
    {
    public:
//---------------------------------------------------------------------------------

        ConstBitReference(uint8_t *src, size_t shift = MAX_SHIFT)
          : src_(src),
            shift_(shift)
        {
            assert(src != nullptr);
        }

        ConstBitReference(const ConstBitReference &other) = default;
        ConstBitReference(const BitReference &other)
          : src_(const_cast<const uint8_t *> (other.src_)),
            shift_(other.shift_)
        {}

        ConstBitReference &operator =(const ConstBitReference &other) = delete;
        ConstBitReference &operator =(bool value) = delete;

        ~ConstBitReference()
        {
            src_ = reinterpret_cast<uint8_t *> (const_cast<char *> (DESTR_PTR));
            shift_  = 0;    
        }
//---------------------------------------------------------------------------------

        operator bool() const
        {
            return *src_ & (1 << shift_);
        }

        bool operator ==(const ConstBitReference &other) const
        {
            return (bool) *this == (bool) other; 
        }

        bool operator !=(const ConstBitReference &other) const
        {
            return !operator ==(other);
        }

        bool operator <(const ConstBitReference &other) const
        {
            return !((bool) *this) && (bool) other;
        }

        bool operator >(const ConstBitReference &other) const
        {
            return other.operator <(*this);
        }

        bool operator <=(const ConstBitReference &other) const
        {
            return !operator >(other);
        }

        bool operator >=(const ConstBitReference &other) const
        {
            return !operator <(other);
        }

    private:
//-----------------------------------Variables-------------------------------------

        const uint8_t *src_ = reinterpret_cast<const uint8_t *> (UNINIT_PTR);
        size_t shift_ = 0;
    };

    class BitReference
    {
        friend class ConstBitReference;

    public:
//---------------------------------------------------------------------------------

        BitReference(uint8_t *src, size_t shift = MAX_SHIFT)
          : src_(src),
            shift_(shift)
        {
            assert(src != nullptr);
        }

        BitReference(const BitReference &other) = default;


        BitReference &operator =(const BitReference &other)
        {
            return *this = (bool) other;
        }

        BitReference &operator =(bool value)
        {
            if (value)
            {
                *src_ |=  (1 << shift_);
            }
            else
            {
                *src_ &= ~(1 << shift_);
            }

            return *this;
        }

        ~BitReference()
        {
            src_ = reinterpret_cast<uint8_t *> (const_cast<char *> (DESTR_PTR));
            shift_  = 0;    
        }

//---------------------------------------------------------------------------------
        operator bool() const
        {
            return *src_ & (1 << shift_);
        }

        bool operator ==(const BitReference &other) const
        {
            return (bool) *this == (bool) other; 
        }

        bool operator !=(const BitReference &other) const
        {
            return !operator ==(other);
        }

        bool operator <(const BitReference &other) const
        {
            return !((bool) *this) && (bool) other;
        }

        bool operator >(const BitReference &other) const
        {
            return other.operator <(*this);
        }

        bool operator <=(const BitReference &other) const
        {
            return !operator >(other);
        }

        bool operator >=(const BitReference &other) const
        {
            return !operator <(other);
        }

    private:
//-----------------------------------Variables-------------------------------------

        uint8_t *src_ = reinterpret_cast<uint8_t *> (const_cast<char *> (UNINIT_PTR));
        size_t shift_ = 0;
    };


    template<bool flag, class IsTrue, class IsFalse>
    struct choose;

    template<class IsTrue, class IsFalse>
    struct choose<false, IsTrue, IsFalse>
    {
        typedef IsFalse type;
    };

    template<class IsTrue, class IsFalse>
    struct choose<true, IsTrue, IsFalse>
    {
        typedef IsTrue type;
    };

    template<bool IsConst>
    class Iterator                      
    {
        friend class Iterator<true>;
        friend class Iterator<false>;

        typedef typename choose<IsConst, ConstBitReference,   BitReference>::type Reference;
        typedef typename choose<IsConst, ConstBitReference *, BitReference *>::type Pointer;
        typedef typename choose<IsConst, const Vector<bool> *, Vector<bool> *>::type ContainerPtr;

    public:

        using iterator_category = std::random_access_iterator_tag;
        using value_type =  bool;
        using difference_type = ptrdiff_t;
        using pointer = Pointer;
        using reference = Reference;
//---------------------------------------------------------------------------------

        Iterator()
          : data_(reinterpret_cast<uint8_t *> (const_cast<char *> (UNINIT_PTR))),
            shift_(MAX_SHIFT),
            container_(reinterpret_cast<ContainerPtr> (const_cast<const ContainerPtr> (UNINIT_PTR)))
        {}
    
        Iterator(ContainerPtr vector, uint8_t *data, size_t shift = MAX_SHIFT)
          : data_(data),
            shift_(shift),
            container_(vector)
        {
            assert(vector != nullptr);
            assert(data   != nullptr);
        }

        Iterator(const Iterator<false> &other)
          : data_(other.data_),
            shift_(other.shift_),
            container_(other.container_)
        {}

        Iterator &operator =(const Iterator<false> &other)
        {
            data_  = other.data_;
            shift_ = other.shift_;
            container_ = other.container_;

            return *this;
        }

        ~Iterator()
        {
            data_ = const_cast<uint8_t *> (reinterpret_cast<const uint8_t *> (DESTR_PTR));
            shift_ = 0;
            container_ = reinterpret_cast<ContainerPtr> (reinterpret_cast<uint64_t> (DESTR_PTR));
        }
//---------------------------------------------------------------------------------

        reference operator *() const
        {
            return reference(data_, shift_);
        }

        Iterator &operator ++()
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

        Iterator operator ++(int)
        {
            Iterator prev = *this;
            ++(*this);

            return prev;
        }

        Iterator &operator --()
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

        Iterator operator --(int)
        {
            Iterator prev = *this;
            --(*this);

            return prev;
        }

        Iterator &operator +=(difference_type value)
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

        Iterator &operator -=(difference_type value)
        {
            return operator +=(-value);
        }

        Iterator operator +(difference_type value) const
        {
            Iterator result = *this;

            return result += value;
        }

        friend Iterator operator +(difference_type value, const Iterator &other)
        {
            return other + value;
        }

        Iterator operator -(difference_type value) const
        {
            Iterator result = *this;

            return result -= value;
        }

        difference_type operator -(const Iterator &other) const
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
        bool operator ==(const Iterator<true> &other) const
        {
            return (data_ == other.data_) && (shift_ == other.shift_); 
        }    
        bool operator ==(const Iterator<false> &other) const
        {
            return operator ==(Iterator<true>(other));
        }

        bool operator !=(const Iterator<true> &other) const
        {
            return !operator ==(other);
        }
        bool operator !=(const Iterator<false> &other) const
        {
            return operator !=(Iterator<true>(other));
        }

        bool operator <(const Iterator<true> &other) const
        {
            return (data_ < other.data_) || ((data_ == other.data_) && (shift_ > other.shift_));
        }
        bool operator <(const Iterator<false> &other) const
        {
            return operator <(Iterator<true>(other));
        }

        bool operator >(const Iterator<true> &other) const
        {
            return other < *this;
        }
        bool operator >(const Iterator<false> &other) const
        {
            return operator <(Iterator<true>(other));
        }

        bool operator <=(const Iterator<true> &other) const
        {
            return !operator >(other);
        }
        bool operator <=(const Iterator<false> &other) const
        {
            return !operator >(Iterator<true>(other));
        }

        bool operator >=(const Iterator<true> &other) const
        {
            return !operator <(other);
        }
        bool operator >=(const Iterator<false> &other) const
        {
            return !operator <(Iterator<true>(other));
        }

    private:
//-------------------------------------Variables-----------------------------------

        uint8_t *data_ = nullptr;
        size_t shift_ = 0;

        ContainerPtr container_ = nullptr;
    };

public:
//---------------------------------------------------------------------------------

    Vector()
      : capacity_       (0),
        booked_capacity_(0),
        size_           (0),
        data_           (reinterpret_cast<uint8_t *> (const_cast<char *> (UNINIT_PTR)))
    {}

    Vector(const size_t reserved_size, const bool value = false)
      : capacity_       (round_to_eight_multiple(reserved_size)),
        booked_capacity_(reserved_size),
        size_           (reserved_size),
        data_(new uint8_t[bits_to_bytes_quantity(capacity_)])
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
        Iterator<false> data_copy_to(this, data_);
        Iterator<true> data_copy_from(this, other.data_);
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

        Iterator<false> data_copy_to( this, data_);
        Iterator<true> data_copy_from(this, other.data_);
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

//--------------------------------Iterators----------------------------------------

    Iterator<false> begin()
    {
        return Iterator<false>(this, data_);
    }

    Iterator<true> cbegin() const
    {
        return Iterator<true>(this, data_);
    }

    Iterator<false> end()
    {
        BitsAndBytes end(size_);

        return Iterator<false> (this, data_ + end.bytes_, MAX_SHIFT - end.bits_);
    }

    Iterator<true> cend() const
    {
        BitsAndBytes end(size_);

        return Iterator<true>(this, data_ + end.bytes_, MAX_SHIFT - end.bits_);
    }
//--------------------------------Modifiers----------------------------------------

    void clear()
    {
        size_ = 0;
    }

    Iterator<false> insert(Iterator<true> pos, bool value)
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
        Iterator<false> data_copy_to (this, data_ + data_copy_to_index.bytes_,  MAX_SHIFT - data_copy_to_index.bits_);
        Iterator<true> data_copy_from(this, data_ + data_copy_from_index.bytes_,MAX_SHIFT - data_copy_from_index.bits_);
        copy_data_(data_copy_to, data_copy_from, size_ - index);

        ++size_;

        set_bit_value_(static_cast<size_t> (index), value);
        
        return begin() + index;
    }

    Iterator<false> erase(Iterator<true> pos)
    {
        ptrdiff_t index = pos - cbegin();
        if ((index < 0) || (index > static_cast<ptrdiff_t> (size_)))
        {
            std::cerr << "ERROR(Vector<bool> " << this << "): attempt to erase out of bounds" << std::endl;

            return end();
        }

        BitsAndBytes data_copy_to_index(index);
        BitsAndBytes data_copy_from_index(index + 1);
        Iterator<false> data_copy_to (this, data_ + data_copy_to_index.bytes_,   MAX_SHIFT - data_copy_to_index.bits_);
        Iterator<true> data_copy_from(this, data_ + data_copy_from_index.bytes_, MAX_SHIFT - data_copy_from_index.bits_);
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

    void copy_data_(Iterator<false> dest, Iterator<true> src, size_t quantity)
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

        Iterator<true>  data_copy_from(this, data_);
        Iterator<false> data_copy_to(this, new_data);
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
        capacity_ = POISONED_SIZE_T;
        size_     = POISONED_SIZE_T;
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

    size_t capacity_        = 0;
    size_t booked_capacity_ = 0;
    size_t size_            = 0;

    uint8_t *data_ = reinterpret_cast<uint8_t *> (const_cast<char *> (UNINIT_PTR));
};


#endif
