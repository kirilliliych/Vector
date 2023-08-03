#ifndef VECTOR_HPP
#define VECTOR_HPP


#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <new>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include "chunkalloc.hpp"
#include "dynamicalloc.hpp"
#include "myforward.hpp"
#include "mymove.hpp"
#include "specialvalues.hpp"


template<typename Type>
class Vector;

template<typename Container, typename ItType>
class VectorBaseIterator : public std::iterator<std::contiguous_iterator_tag, ItType>
{
    static const bool is_const = std::is_same_v<const typename Container::value_type, ItType>;

    friend typename std::conditional_t<is_const,
                    VectorBaseIterator<std::remove_const_t<Container>, std::remove_const_t<ItType>>,
                    VectorBaseIterator<const Container, const ItType>>;

    using reference = std::conditional_t<is_const, typename Container::const_reference,
                                                   typename Container::reference>;
    using pointer   = std::conditional_t<is_const, typename Container::const_pointer,
                                                   typename Container::pointer>;
    using difference_type = std::ptrdiff_t;

public:
//---------------------------------------------------------------------------------
    VectorBaseIterator()
      : container_(reinterpret_cast<Container *> (UNINIT_PTR)),
        index_(0)
    {}

    VectorBaseIterator(Container *vector, uint64_t index)
      : container_(vector),
        index_(index)
    {
        assert(vector != nullptr);
    }

    template<typename OtherContainer, typename OtherItType>
    VectorBaseIterator(const VectorBaseIterator<OtherContainer, OtherItType> &other)
      : container_(other.container_),
        index_(other.index)
    {}

    template<typename OtherContainer, typename OtherItType>
    VectorBaseIterator(VectorBaseIterator<OtherContainer, OtherItType> &&other)
    {
        // container_ = other.container_;
        // index_     = other.index_;

        *this = std::move(other);
    }

    template<typename OtherContainer, typename OtherItType>
    VectorBaseIterator &operator =(const VectorBaseIterator<OtherContainer, OtherItType> &other)
    {
        if (reinterpret_cast<void *> (this) != reinterpret_cast<void *> (&other))
        {
            container_ = other.container_;
            index_     = other.index_;
        }

        return *this;
    }

    template<typename OtherContainer, typename OtherItType>
    VectorBaseIterator &operator =(VectorBaseIterator<OtherContainer, OtherItType> &&other)
    {
        if (reinterpret_cast<void *> (this) != reinterpret_cast<void *> (&other))
        {
            container_ = other.container_;
            index_     = other.index_;

            other.container_ = const_cast<OtherContainer *> (reinterpret_cast<const OtherContainer *> (MOVED_REMAINDERS_PTR));
            other.index_     = POISONED_UINT64_T;
        }

        return *this;
    }

    ~VectorBaseIterator()
    {
        container_ = reinterpret_cast<Container *> (reinterpret_cast<uint64_t> (DESTR_PTR));
        index_     = POISONED_UINT64_T;
    }
//---------------------------------------------------------------------------------
    reference operator *() const
    {
        return operator [](index_);
    }

    pointer operator ->() const
    {
        return &operator [](index_);
    }

    VectorBaseIterator &operator +=(difference_type value)
    {
        index_ += value;

        return *this;
    }

    VectorBaseIterator &operator -=(difference_type value)
    {
        return operator +=(-value);
    }

    VectorBaseIterator &operator ++()
    {
        return operator +=(1);
    }

    VectorBaseIterator operator ++(int)
    {
        VectorBaseIterator prev = *this;
        ++*this;

        return prev;
    }

    VectorBaseIterator &operator --()
    {
        return operator -=(1);
    }

    VectorBaseIterator operator --(int)
    {
        VectorBaseIterator prev = *this;
        --*this;

        return prev;
    }

    VectorBaseIterator operator +(difference_type value) const
    {
        return VectorBaseIterator<Container, ItType>(container_, index_ + value);
    }

    friend VectorBaseIterator operator +(difference_type value, const VectorBaseIterator &other)
    {
        return other.operator +(value);
    }

    VectorBaseIterator operator -(difference_type value) const
    {
        return operator +(-value);
    }

    difference_type operator -(const VectorBaseIterator &other) const
    {
        return index_ - other.index_;
    }

    reference operator [](uint64_t index) const
    {
        return container_->operator [](index);
    }

    template<typename OtherContainer, typename OtherItType>
    bool operator ==(const VectorBaseIterator<OtherContainer, OtherItType> &other) const
    {
        return index_ == other.index_;
    }

    template<typename OtherContainer, typename OtherItType>
    bool operator !=(const VectorBaseIterator<OtherContainer, OtherItType> &other) const
    {
        return !operator ==(other);
    }

    template<typename OtherContainer, typename OtherItType>
    bool operator <(const VectorBaseIterator<OtherContainer, OtherItType> &other) const
    {
        return index_ < other.index_;
    }

    template<typename OtherContainer, typename OtherItType>
    bool operator >(const VectorBaseIterator<OtherContainer, OtherItType> &other) const
    {
        return other.operator <(*this);
    }

    template<typename OtherContainer, typename OtherItType>
    bool operator <=(const VectorBaseIterator<OtherContainer, OtherItType> &other) const
    {
        return !operator >(other);
    }

    template<typename OtherContainer, typename OtherItType>
    bool operator >=(const VectorBaseIterator<OtherContainer, OtherItType> &other) const
    {
        return !operator <(other);
    }

private:
//-----------------------------------Variables-------------------------------------
    Container *container_ = nullptr;
    int index_ = 0;
};

//-----------------------------------Class Vector----------------------------------
template<typename Type>
class Vector
{
public:
    using value_type        = Type;
    using pointer           = Type *;
    using const_pointer     = const Type *;
    using reference         = Type &;
    using const_reference   = const Type &;
    using iterator_category = std::contiguous_iterator_tag;

    template<typename Vector>
    using Iterator = VectorBaseIterator<Vector, value_type>;
    
    template<typename Vector>
    using ConstIterator = VectorBaseIterator<const Vector, const value_type>;

//---------------------------------------------------------------------------------
    Vector()
      : capacity_(0),
        size_    (0),
        data_    (const_cast<char *> (UNINIT_PTR))
    {}

    Vector(const std::initializer_list<Type> &init_list)
    {
        uint64_t list_size = init_list.size();
        for (uint64_t index = 0; index < list_size; ++index)
        {
            push_back(*(init_list.begin() + index));
        }
    }

    Vector(uint64_t reserved_size, const Type &value = Type())
      : Vector()
    {
        if (reserved_size != 0)
        {
            capacity_ = calculate_enough_capacity_(reserved_size);
            data_ = new char[capacity_ * sizeof(Type)];

            size_ = reserved_size;
            
            init_elements_(0, reserved_size, value);
        }
    }

    Vector(const Vector<Type> &other)
      : capacity_(other.capacity_),
        size_    (other.size_)
    {
        data_ = new char[capacity_ * sizeof(Type)];

        copy_data_to_uninit_place_(data_, other.data_, other.size_);
    }

    Vector<Type> &operator =(const Vector &other)
    {
        destroy_existing_elems_(0, size_);
        if (data_ != const_cast<char *> (UNINIT_PTR) && data_is_valid_())
        {
            delete [] data_;
        }

        capacity_ = other.capacity_;
        size_     = other.size_;
        data_ = new char[capacity_ * sizeof(Type)];
        copy_data_to_uninit_place_(data_, other.data_,  other.size_);

        return *this;
    }

    Vector(Vector<Type> &&other)
    {
        *this = std::move(other);
    }

    Vector<Type> &operator =(Vector<Type> &&other)
    {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);

        return *this;
    }

    ~Vector()
    {
        destroy_existing_elems_(0, size_);
        
        if ((data_ != const_cast<char *> (UNINIT_PTR)) && data_is_valid_())
        {
            delete [] data_;
        }

        destroy_fields_();
    }

//--------------------------------------Dump---------------------------------------
    void dump(void (*dump_elem)(const Type &value), uint64_t from = 0, uint64_t to = VECTOR_MAX_CAPACITY + 1) const
    {   
        if (to == VECTOR_MAX_CAPACITY + 1)
        {
            to = capacity_;
        }

        std::cout << "Vector[" << this << "]" << std::endl;
        std::cout << "capacity_: " << capacity_ << std::endl;
        std::cout << "size_: " << size_ << std::endl << std::endl;

        for (; from < to; ++from)
        {
            std::cout << "vector[" << from << "] = ";
            dump_elem(reinterpret_cast<const Type &> (data_[from * sizeof(Type)]));
            std::cout << "\t\t\t&vector[" << from << "] = " << reinterpret_cast<void *> (&data_[from * sizeof(Type)]) << std::endl;
        }

        std::cout << std::endl;
    }
//-----------------------------------Verificator-----------------------------------
    void verificator()
    {
        assert(size_ <= capacity_);
        assert(capacity_ <= VECTOR_MAX_CAPACITY);
        assert(data_ != nullptr);
        assert(data_ != INVALID_PTR);
    }
//----------------------------------Size and capacity------------------------------
    bool empty() const
    {
        return size_ == 0;
    }

    uint64_t size() const
    {
        return size_;
    }

    uint64_t max_size() const
    {
        return VECTOR_MAX_CAPACITY;
    }

    uint64_t capacity() const
    {
        return capacity_;
    }

    void reserve(uint64_t reserved_capacity)
    {
        if (reserved_capacity <= capacity_)
        {
            return;
        }

        char *new_data = vector_realloc_(reserved_capacity);

        if ((data_ != const_cast<char *> (UNINIT_PTR)) && data_is_valid_())
        {
            delete [] data_;
        }

        data_     = new_data;
        capacity_ = reserved_capacity;
    }

    void shrink_to_fit()
    {
        if (capacity_ == size_)
        {
            return;
        }

        char *new_data = vector_realloc_(size_);

        if ((data_ != const_cast<char *> (UNINIT_PTR)) && data_is_valid_())
        {
            delete [] data_;
        }

        data_     = new_data;
        capacity_ = size_;
    }
//---------------------------------Accessing elements------------------------------
    const Type &operator [](uint64_t index) const
    {
        return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->operator[](index));
    }

    Type &operator [](uint64_t index)
    {
        assert(index <= size_);

        return reinterpret_cast<Type &> (data_[index * sizeof(Type)]);
    }

    const Type &at(uint64_t index) const
    {
       return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->at(index));
    }

    Type &at(uint64_t index)
    {
        assert(index < size_);

        return operator [](index);
    }

    const Type &front() const
    {
        return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->front());
    }

    Type &front()
    {
        return reinterpret_cast<Type &> (data_[0]);
    }

    const Type &back() const
    {
        return const_cast<const Type &>(const_cast<Vector<Type> *>(this)->back());
    }

    Type &back()
    {
        return reinterpret_cast<Type &> (data_[(size_ - 1) * sizeof(Type)]);
    }

    const Type *data() const
    {
        return const_cast<const Type *>(const_cast<Vector<Type> *>(this)->data());
    }

    Type *data()
    {
        return reinterpret_cast<Type *> (data_);
    }

//------------------------------------Iterators------------------------------------
    ConstIterator<Vector> cbegin() const
    {
        return ConstIterator<Vector>(this, 0);
    }

    Iterator<Vector> begin()
    {
        return Iterator<Vector>(this, 0);
    }

    ConstIterator<Vector> begin() const
    {
        return cbegin();
    }

    ConstIterator<Vector> cend() const
    {
        return ConstIterator<Vector>(this, size_);
    }

    Iterator<Vector> end()
    {
        return Iterator<Vector>(this, size_);
    }

    ConstIterator<Vector> end() const
    {
        return cend();
    }

    std::reverse_iterator<ConstIterator<Vector>> crbegin() const
    {
        return std::make_reverse_iterator(cend());
    }
    
    std::reverse_iterator<Iterator<Vector>> rbegin()
    {
        return std::make_reverse_iterator(end());
    }

    std::reverse_iterator<ConstIterator<Vector>> rbegin() const
    {
        return crbegin();
    }

    std::reverse_iterator<ConstIterator<Vector>> crend() const
    {
        return std::make_reverse_iterator(cbegin());
    }

    std::reverse_iterator<Iterator<Vector>> rend()
    {
        return std::make_reverse_iterator(begin());
    }

    std::reverse_iterator<ConstIterator<Vector>> rend() const
    {
        return crend();
    }
//-----------------------------------Modifiers-------------------------------------
    void clear()
    {
        destroy_existing_elems_(0, size_);

        size_ = 0;
    }

    Iterator<Vector> insert(ConstIterator<Vector> pos, const Type &value)
    {
        std::ptrdiff_t index = pos - cbegin();
        if ((index < 0) || (index > static_cast<std::ptrdiff_t> (size_)))
        {
            std::cerr << "ERROR(Vector " << typeid(*this).name() << "): attempt to insert out of bounds" << std::endl;

            return end();
        }

        reserve(size_ > 0 ? size_ * DEFAULT_RESIZE_MULTIPLIER : 1);
        init_elements_(size_, size_ + 1);
        
        move_data_(begin() + index + 1, cbegin() + index, size_ - index);
    
        ++size_;
    
        *(begin() + index) = value;

        return begin() + index;
    }

    template<typename... Args>
    Iterator<Vector> emplace(ConstIterator<Vector> pos, Args &&... args)
    {
        std::ptrdiff_t index = pos - begin();
        if (index > size_)
        {
            return end();
        }
       
        if (index < size_)
        {
            destroy_existing_elems_(index, index + 1);
        }
        
        new (data_ + index * sizeof(Type)) Type(my_forward<Args>(args)...);

        return begin() + index;
    }

    template<typename... Args>
    Type &emplace_back(Args &&... args)
    {
        emplace(cend(), my_forward<Args>(args)...);
    }

    Iterator<Vector> erase(ConstIterator<Vector> pos)
    {
        std::ptrdiff_t index = pos - begin();
        if ((index < 0) || (index > static_cast<std::ptrdiff_t> (size_)))
        {
            std::cerr << "ERROR(Vector " << typeid(*this).name() << "): attempt to erase out of bounds" << std::endl;

            return end();
        }

        move_data_(begin() + index, begin() + index + 1, size_ - index - 1);
        destroy_existing_elems_(size_ - 1, size_);

        --size_;

        return begin() + index;
    }

    void push_back(const Type &value)
    {
        insert(cbegin() + size_, value);
    }

    void pop_back()
    {
        if (size_ == 0)
        {
            std::cerr << "ERROR(Vector " << typeid(*this).name() << "): null pop attempt" << std::endl;

            return;
        }

        erase(begin() + size_ - 1);
    }

    void resize(uint64_t new_size, const Type &value = Type())
    {   
        if (new_size > VECTOR_MAX_CAPACITY)
        {
            std::cerr << "ERROR(Vector " << typeid(*this).name() << "): attempt to resize to more than VECTOR_MAX_CAPACITY (which is "
                      << VECTOR_MAX_CAPACITY << ")" << std::endl; 

            return;
        }
        if (new_size <= size_)                                                       // new size is smaller or equal to previous
        {
            destroy_existing_elems_(new_size, size_);

            size_ = new_size;

            return;
        }
        if (new_size <= capacity_)                                                   // new size is bigger than previous but smaller or equal to capacity
        {
            init_elements_(size_, new_size, value);

            size_ = new_size;

            return;
        }

        uint64_t new_capacity = calculate_enough_capacity_(new_size);                 // new size is bigger than capacity
        char *new_data = vector_realloc_(new_capacity);

        if ((data_ != const_cast<char *> (UNINIT_PTR)) && data_is_valid_())
        {
            delete [] data_;
        }

        data_     = new_data;
        init_elements_(size_, new_size, value);

        capacity_ = new_capacity;
        size_     = new_size;
    }

    void swap(Vector &other)
    {
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

    bool operator ==(const Vector<Type> &other) const = default;
    bool operator !=(const Vector<Type> &other) const = default;
    bool operator  <(const Vector<Type> &other) const = default;
    bool operator  >(const Vector<Type> &other) const = default;
    bool operator <=(const Vector<Type> &other) const = default;
    bool operator >=(const Vector<Type> &other) const = default;

    auto operator <=>(const Vector<Type> &other) const
    {
        return vector_cmp_(other);
    }

private:
//-----------------------------------Utilitary functions---------------------------
    uint64_t calculate_enough_capacity_(uint64_t required_size) 
    {
        uint64_t capacity = 1;

        for (uint64_t base = 32; base > 0; base >>= 1)
        {
            if (required_size >= (capacity << base))
            {
                capacity <<= base;
            }
        }

        capacity <<= 1;

        return capacity > VECTOR_MAX_CAPACITY ? VECTOR_MAX_CAPACITY : capacity;
    }

    void init_elements_(uint64_t from, uint64_t to, const Type &value = Type())
    {                                                   
        for (uint64_t vector_elem_index = from; vector_elem_index < to; ++vector_elem_index)
        {
            new (data_ + vector_elem_index * sizeof(Type)) Type(value); 
        }                                                                       
    }

    void copy_data_to_uninit_place_(char *dest, const char *src, uint64_t quantity)
    {
        if ((dest == nullptr) || (src == nullptr))
        {
            return;
        }

        for (uint64_t index = 0; index < quantity; ++index)
        {
            const Type &elem_to_copy = reinterpret_cast<const Type &> (src[index * sizeof(Type)]);
            new (dest + index * sizeof(Type)) Type(elem_to_copy);
        }
    }

    void copy_data_(char *dest, const char *src, uint64_t quantity)
    {
        if ((dest == nullptr) || (src == nullptr))
        {
            return;
        }

        Type *casted_dest      = reinterpret_cast<Type *> (dest);
        const Type *casted_src = reinterpret_cast<const Type *> (src); 

        if (dest == src)
        {
            return;
        }

        bool from_left_to_right = false;
        if (dest > src)
        {
            from_left_to_right = true;
            casted_dest += quantity - 1;
            casted_src  += quantity - 1;
        }
        
        for (uint64_t counter = 0; counter < quantity; ++counter)
        {
            std::ptrdiff_t shift = from_left_to_right ? -counter : counter;
            *(casted_dest + shift) = *(casted_src + shift);
        }
    }

    void move_data_(Iterator<Vector> dest, ConstIterator<Vector> src, uint64_t quantity)
    {
        if (dest == src)
        {
            return;
        }

        bool from_left_to_right = false;
        if (dest > src)
        {
            from_left_to_right = true;
            dest += quantity - 1;
            src  += quantity - 1;
        }

        for (uint64_t counter = 0; counter < quantity; ++counter)
        {
            std::ptrdiff_t shift = from_left_to_right ? -counter : counter;
            *(dest + shift) = my_move(*(src + shift));
        }
    }

    char *vector_realloc_(uint64_t new_capacity)
    {
        char *new_data = new char[new_capacity * sizeof(Type)];
        copy_data_to_uninit_place_(new_data, data_, size_);
        destroy_existing_elems_(0, size_);

        return new_data;
    }

    bool data_is_valid_() const
    {
        return (data_ != const_cast<char *> (DESTR_PTR))  &&
               (data_ != const_cast<char *> (INVALID_PTR) &&
               (data_ != const_cast<char *> (MOVED_REMAINDERS_PTR)));
    }

    void destroy_existing_elems_(uint64_t from, uint64_t to)
    {
        for (uint64_t index = from; index < to; ++index)
        {
            (reinterpret_cast<Type *> (data_))[index].~Type();
        }
    }

    void destroy_fields_()
    {
        capacity_ = POISONED_UINT64_T;
        size_     = POISONED_UINT64_T;
        data_     = const_cast<char *> (DESTR_PTR);
    }

    std::strong_ordering vector_cmp_(const Vector<Type> &other) const
    {
        uint64_t this_size  = size();
        uint64_t other_size = other.size();
        uint64_t min_size   = std::min(this_size, other_size);
        for (uint64_t cur_elem_index = 0; cur_elem_index < min_size; ++cur_elem_index)
        {
            const Type *this_data_ptr  = data()       + cur_elem_index;
            const Type *other_data_ptr = other.data() + cur_elem_index;

            if (*this_data_ptr > *other_data_ptr)
            {
                return std::strong_ordering::greater;
            }
            if (*this_data_ptr < *other_data_ptr)
            {
                return std::strong_ordering::less;
            }
        }

        if (this_size > min_size)
        {
            return std::strong_ordering::greater;
        }
        if (other_size > min_size)
        {
            return std::strong_ordering::less;
        }

        return std::strong_ordering::equal;
    }

private:
//----------------------------Variables--------------------------------------------
    static constexpr uint64_t VECTOR_MAX_CAPACITY       = 1024;
    static constexpr uint64_t DEFAULT_RESIZE_MULTIPLIER = 2;

    uint64_t capacity_  = 0;
    uint64_t size_      = 0;
    
    char *data_ = const_cast<char *> (UNINIT_PTR);
};


#endif
