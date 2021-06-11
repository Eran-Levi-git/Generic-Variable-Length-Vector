#include <cstddef>
#include <iterator>
#include <iostream>
#include <cmath>

#ifndef EX6__VL_VECTOR_H_
#define EX6__VL_VECTOR_H_

#define DEFAULT_VLV_STATIC_CAP 16
#define ONE_ELEMENT 1
#define DEFAULT_VLV_SIZE 0
#define CAP_CALC(size, k) (3 * (size + k)) / 2)

template<typename T, size_t StaticCapacity=DEFAULT_VLV_STATIC_CAP>
class VLVector {
  size_t size_;
  size_t capacity_;
  T static_vector_[StaticCapacity];
  T *dynamic_vector_;

 public:
  ~VLVector() {
    if (not(IsStatic())) {
      delete[] dynamic_vector_;
    }
  }

  VLVector() : size_(DEFAULT_VLV_SIZE), capacity_(StaticCapacity), dynamic_vector_(nullptr) {}

  VLVector(VLVector &other)
      : size_(other.size_), capacity_(other.capacity_) {
    if (other.IsStatic()) {
      std::copy(other.begin(), other.end(), static_vector_);
    } else {
      T *temp = new T[other.capacity_];
      std::copy(other.begin(), other.end(), temp);
      dynamic_vector_ = temp;
    }
  }

  template<typename InputIterator>
  VLVector(InputIterator first, InputIterator last) {
    this->Insert(this->cbegin(), first, last);
  }

  size_t Size() const noexcept{ return size_; }

  size_t Capacity() const noexcept{ return capacity_; }

  bool Empty() const noexcept { return (size_ == 0); }

  T At(size_t index) const noexcept(false) {
    if (index >= capacity_) {
      throw std::out_of_range("Index is out of range");
    }
    if (IsStatic()) {
      return static_vector_[index];
    } else {
      return dynamic_vector_[index];
    }
  }

  void PushBack(T elem) {
    if (size_ < StaticCapacity) {
      static_vector_[size_] = elem;
    } else {
      if (size_ == capacity_) {
        IncreaseDynamicSize();
      }
      dynamic_vector_[size_] = elem;
    }
    ++size_;
  }

  void PopBack() {
    if (size_ != DEFAULT_VLV_SIZE) {
      --size_;
    }
  }

  template<typename InputIterator>
  InputIterator Erase(InputIterator it) {
    return Erase(it, it + ONE_ELEMENT);
  }

  template<typename InputIterator>
  InputIterator Erase(InputIterator first, InputIterator last) {
    size_t dist_from_beginning_to_first = std::distance(this->begin(), first);
    size_t num_of_elements_to_erase = std::distance(first, last);
    std::copy(last, this->end(), first);
    size_ -= num_of_elements_to_erase;
    if (size_ <= StaticCapacity) {
      std::copy(this->begin(), this->end(), static_vector_);
      delete[] dynamic_vector_;
    }
    return this->begin() + dist_from_beginning_to_first + num_of_elements_to_erase;
  }

  template<typename InputIterator>
  void Insert(InputIterator it, T elem) {
    T *elem_it = &elem;
    Insert(it, elem_it, elem_it + ONE_ELEMENT);
  }

  template<typename InputIterator>
  void Insert(T* position, InputIterator first, InputIterator last) {
    size_t num_of_new_elements = std::distance(first, last);
    size_t new_size = size_ + num_of_new_elements;
    if (new_size <= StaticCapacity) { // insert to static vector
      std::copy(position, (static_vector_ + size_), position + num_of_new_elements); // coping the end (leaving gap)
      std::copy(first, last, position); // inserting new range in the middle
    } else { // dynamic situation
      if (new_size < capacity_) { // theres enough place
        std::copy(position, (dynamic_vector_ + size_), position + num_of_new_elements); // coping the end (leaving gap)
        std::copy(first, last, position); // inserting new range in the middle
      } else { // increase dynamic size
        size_t new_capacity = CapC(size_, num_of_new_elements);
        T *temp = new T[new_capacity];
        if (IsStatic()) { // moving from static to dynamic
          size_t distance_to_position = std::distance(static_vector_, position);
          std::copy(static_vector_, static_vector_ + distance_to_position, temp); // coping beginning to temp
          std::copy(static_vector_ + distance_to_position,
                    (static_vector_ + size_),
                    temp + distance_to_position + num_of_new_elements);
          std::copy(first, last, temp + distance_to_position); // inserting new range in the middle
          dynamic_vector_ = temp;
        } else { // moving from dynamic to dynamic
          size_t distance_to_position = std::distance(dynamic_vector_, position);
          std::copy(dynamic_vector_, dynamic_vector_ + distance_to_position, temp); // coping beginning to temp
          std::copy(dynamic_vector_ + distance_to_position,
                    (dynamic_vector_ + size_),
                    temp + distance_to_position + num_of_new_elements);
          std::copy(first, last, temp + distance_to_position); // inserting new range in the middle
          delete[] dynamic_vector_; // deleting old dynamic allocation
          dynamic_vector_ = temp;
        }
        capacity_ = new_capacity;
      }
    }

    size_ += num_of_new_elements;
  }

  T *Data() noexcept {
    if (IsStatic()) {
      return static_vector_;
    } else {
      return dynamic_vector_;
    }
  }

  void Clear() {
    if (not(IsStatic())) {
      delete[] dynamic_vector_;
    }
    size_ = DEFAULT_VLV_SIZE;
    capacity_ = StaticCapacity;
  }

  VLVector &operator=(const VLVector &other) {
    if (this != &other) {
      Clear();
      size_ = other.size_;
      capacity_ = other.capacity_;
      if (other.IsStatic()) {
        std::copy(std::begin(other), std::end(other), static_vector_);
      } else {
        T *temp = new T[other.capacity_];
        std::copy(std::begin(other), std::end(other), temp);
        dynamic_vector_ = temp;
      }
    }
    return *this;
  }

  T operator[](size_t index) const noexcept {
    if (IsStatic()) {
      return static_vector_[index];
    } else {
      return dynamic_vector_[index];
    }
  }

  bool operator==(VLVector &other) const noexcept {
    if (size_ != other.size_){
      return false;
    }
    for (size_t i = 0; i <size_ ; ++i) {
      if (this->At(i) != other.At(i)){
        return false;
      }
    }
    return true;
  }

  bool operator!=(VLVector &other) const noexcept {
    if (size_ != other.size_){
      return true;
    }
    for (size_t i = 0; i <size_ ; ++i) {
      if (this->At(i) != other.At(i)){
        return true;
      }
    }
    return false;
  }

  /** Iterator functions section - begin, end, etc **/
  T *begin() {
    return Data();
  }
  T *begin() const {
    return Data();
  }
  T *cbegin() const {
    return Data();
  }
  T *end() {
    return (Data() + size_);
  }
  T *end() const {
    return (Data() + size_);
  }
  T *cend() const {
    return (Data() + size_);
  }
  std::reverse_iterator<T *> rbegin() {
    return std::reverse_iterator<T *>(end());
  }
  std::reverse_iterator<T *> rbegin() const {
    return std::reverse_iterator<T *>(end());
  }
  std::reverse_iterator<T *> rend() {
    return std::reverse_iterator<T *>(begin());
  }
  std::reverse_iterator<T *> rend() const {
    return std::reverse_iterator<T *>(begin());
  }

  /** prints the vector */
  void MyPrint() const noexcept {
    if (IsStatic()) {
      for (size_t i = 0; i < size_; ++i) {
        std::cout << static_vector_[i] << std::endl;
      }
    } else {
      for (size_t i = 0; i < size_; ++i) {
        std::cout << dynamic_vector_[i] << std::endl;
      }
    }
  }

  /** cap_c formula for vector's capacity */
  size_t CapC(size_t size, size_t k) const noexcept{
    if (size + k <= StaticCapacity) {
      return (size + k);
    }
    return (floor(CAP_CALC(size, k));
  }

  /** increasing dynamic array according to cap_c formula  */
  void IncreaseDynamicSize() {
    size_t new_capacity = CapC(size_, ONE_ELEMENT);
    if (size_ == StaticCapacity) { // moving from static to dynamic
      T *temp = new T[new_capacity];
      std::copy(static_vector_, static_vector_ + size_, temp);
      dynamic_vector_ = temp;
    } else { // moving from dynamic to dynamic
      T *temp = new T[new_capacity];
      std::copy(dynamic_vector_, dynamic_vector_ + size_, temp);
      delete [] dynamic_vector_;
      dynamic_vector_ = temp;
    }
    capacity_ = new_capacity;
  }

  /** tells if vector is static or dynamic. */
  bool IsStatic() const noexcept{
    if (size_ > StaticCapacity) {
      return false;
    } else {
      return true;
    }
  }

};

#endif //EX6__VL_VECTOR_H_




