#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;
    Optional(const T& value);
    Optional(T&& value);
    Optional(const Optional& other);
    Optional(Optional&& other);

    Optional& operator=(const T& value);
    Optional& operator=(T&& rhs);
    Optional& operator=(const Optional& rhs);
    Optional& operator=(Optional&& rhs);

    ~Optional();

    bool HasValue() const;
    
    template <typename... Args>
    void Emplace(Args&&... args);

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*();
    const T& operator*() const;
    T* operator->();
    const T* operator->() const;

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value();
    const T& Value() const;

    void Reset();

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};

template <typename T>
Optional<T>::Optional(const T& value)
    : is_initialized_(true) {
    new (data_) T(value);
}

template <typename T>
Optional<T>::Optional(T&& value)
    : is_initialized_(true) {
    new (data_) T(std::move(value));
}

template <typename T>
Optional<T>::Optional(const Optional& other)
    : is_initialized_(other.is_initialized_) {
    if (is_initialized_) {
        new (data_) T(*other);
    }
}

template <typename T>
Optional<T>::Optional(Optional&& other)
    : is_initialized_(other.is_initialized_) {
    if (is_initialized_) {
        new (data_) T(std::move(*other));
    }
}

template <typename T>
Optional<T>::~Optional() {
    Reset();
}

template <typename T>
Optional<T>& Optional<T>::operator=(const T& value) {
    if (!is_initialized_) {
        new (data_) T(value);
        is_initialized_ = true;
    }
    else {
        Value() = value;
    }
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(T&& value) {
    if (is_initialized_) {
        Value() = std::move(value);
    }
    else {
        new (data_) T(std::move(value));
        is_initialized_ = true;
    }
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(const Optional& rhs) {
    if (!is_initialized_) {
        if (rhs.is_initialized_) {
            new (data_) T(*rhs);
            is_initialized_ = rhs.is_initialized_;
        }
    }
    else {
        if (rhs.is_initialized_) {
            Value() = *rhs;
        }
        else {
            Reset();
        }
    }
    return *this;
}

template <typename T>
Optional<T>& Optional<T>::operator=(Optional&& rhs) {
    if (!is_initialized_) {
        if (rhs.is_initialized_) {
            new (data_) T(std::move(*rhs));
            is_initialized_ = rhs.is_initialized_;
        }
    }
    else {
        if (rhs.is_initialized_) {
            Value() = std::move(*rhs);
        }
        else {
            Reset();
        }
    }
    return *this;
}

template <typename T>
bool Optional<T>::HasValue() const {
    return this->is_initialized_;
}

template <typename T>
template <typename... Args>
void Optional<T>::Emplace(Args&&... args) {
    if (is_initialized_) {
        Value().~T();  // Удаляем текущее значение
    }
    new (&data_[0]) T(std::forward<Args>(args)...);  // Создаем новое значение
    is_initialized_ = true;
}

template <typename T>
T& Optional<T>::operator*() {
    return Value();
}

template <typename T>
const T& Optional<T>::operator*() const {
    return Value();
}

template <typename T>
T* Optional<T>::operator->() {
    return &Value();
}

template <typename T>
const T* Optional<T>::operator->() const {
    return &Value();
}

template <typename T>
T& Optional<T>::Value() {
    if (!is_initialized_) {
        throw BadOptionalAccess();
    }
    return *reinterpret_cast<T*>(data_);
}

template <typename T>
const T& Optional<T>::Value() const {
    if (!is_initialized_) {
        throw BadOptionalAccess();
    }
    return *reinterpret_cast<const T*>(data_);
}

template <typename T>
void Optional<T>::Reset() {
    if (is_initialized_) {
        Value().~T();
        is_initialized_ = false;
    }
}
