#include <iostream>
#include <memory>

template <typename T, typename Deleter = std::default_delete<T>> class shared_ptr; 

template <typename T> class weak_ptr;    

template <typename T>
class ControlBlock {
public:
    ControlBlock(T* ptr): m_ptr(ptr), shared_count(0), weak_count(0) {}

    void add_shared() {
        shared_count++;
    }

    void release_shared() {
        shared_count--;
        {
            shared_count--;
            if (shared_count == 0) {
                delete m_ptr; 
                if (weak_count == 0) {
                    delete this; 
                }
            }
        }
    }

    void release_weak() {
        weak_count--;
        if (shared_count == 0 && weak_count == 0) {
            delete this; 
        }
    }




    void add_weak() {
        weak_count++;
    }

    T* get_ptr() const {
        return m_ptr;
    }

    int get_shared_count() const {
        return shared_count;
    }

    int get_weak_count() const {
        return weak_count;
    }

private:
    T* m_ptr;
    int shared_count = 0;
    int weak_count = 0;

    friend class weak_ptr<T>;
    friend class shared_ptr<T, std::default_delete<T>>;
    friend class shared_ptr<T, std::default_delete<T[]>>;
};

template <typename T, typename Deleter>
class shared_ptr {
public:
    explicit shared_ptr(T* p = nullptr, Deleter d = Deleter()) 
        : m_ptr(p), control_block(new ControlBlock<T>(p)), deleter(d) {
        control_block->add_shared();
    }

    shared_ptr(const shared_ptr& other) 
        : m_ptr(other.m_ptr), control_block(other.control_block), deleter(other.deleter) {
        control_block->add_shared();
    }

    shared_ptr(shared_ptr&& other) noexcept 
        : m_ptr(other.m_ptr), control_block(other.control_block), deleter(std::move(other.deleter)) {
        other.m_ptr = nullptr;
        other.control_block = nullptr;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        m_ptr = other.m_ptr;
        control_block = other.control_block;
        deleter = std::move(other.deleter);
        other.m_ptr = nullptr;
        other.control_block = nullptr;
        return *this;
    }

    shared_ptr& operator=(const shared_ptr& other) {
        if (this != &other) {
            m_ptr = other.m_ptr;
            control_block = other.control_block;
            deleter = other.deleter;
            control_block->add_shared();
        }
        return *this;
    }

    ~shared_ptr() {
        if(control_block) {
            control_block->release_shared();
        }
    }

    T* get() const {
        return m_ptr;
    }

    T& operator*() const {
        return *m_ptr;
    }

    T* operator->() const {
        return m_ptr;
    }

    int use_count() const {
        return control_block->get_shared_count();
    }

    explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }
   ControlBlock<T>* get_control_block() const {
        return control_block;
    }
private:
    T* m_ptr;
    ControlBlock<T>* control_block;
    Deleter deleter;
};

template <typename T>
class weak_ptr {
public:
    weak_ptr() : m_ptr(nullptr), control_block(nullptr) {}

    weak_ptr(const shared_ptr<T>& shared) 
        : m_ptr(shared.get()), control_block(shared.get_control_block()) {
        control_block->add_weak();
    }

    weak_ptr(const weak_ptr& other) 
        : m_ptr(other.m_ptr), control_block(other.control_block) {
        control_block->add_weak();
    }

    weak_ptr(weak_ptr&& other) noexcept 
        : m_ptr(other.m_ptr), control_block(other.control_block) {
        other.m_ptr = nullptr;
        other.control_block = nullptr;
    }

    ~weak_ptr() {
        if(control_block) {
            control_block->release_weak();
        }
    }

    weak_ptr& operator=(const weak_ptr& other) {
        m_ptr = other.m_ptr;
        control_block = other.control_block;
        control_block->add_weak();
        return *this;
    }

    weak_ptr& operator=(weak_ptr&& other) noexcept {
        m_ptr = other.m_ptr;
        control_block = other.control_block;
        other.m_ptr = nullptr;
        other.control_block = nullptr;
        return *this;
    }

    shared_ptr<T> lock() const {
        if(control_block->get_shared_count() == 0) {
            return shared_ptr<T>();
        }
        return shared_ptr<T>(m_ptr);
    }

    int use_count() const {
        return control_block->get_shared_count();
    }

    explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }

private:
    T* m_ptr;
    ControlBlock<T>* control_block;
};

template <typename T>
class my_unique_ptr {
public:
    my_unique_ptr(T *ptr) : ptr(ptr) {}

    ~my_unique_ptr() {
        if(ptr) {
            delete ptr;
        }
    }

    my_unique_ptr(const my_unique_ptr& other) = delete;
    my_unique_ptr& operator=(const my_unique_ptr& other) = delete;

    my_unique_ptr(my_unique_ptr&& other) noexcept : ptr(other.release()) {}

    my_unique_ptr<T>& operator=(my_unique_ptr<T>&& other) noexcept {
        if (this != &other) {
            reset(other.release());
            other.ptr = nullptr;
        }
        return *this;
    }

    T* release() {
        T *temp = ptr;
        ptr = nullptr;
        return temp;
    }

    T* get() {
        return ptr;
    }

    void reset(T *new_ptr = nullptr) {
        if(ptr != new_ptr) {
            delete ptr;
        }
        ptr = new_ptr;
    }

    T& operator*() {
        return *ptr;
    }

    T* operator->() {
        return ptr;
    }

private:
    T *ptr;
};

template <typename T>
class my_auto_ptr {
public:
    my_auto_ptr(T *ptr) : ptr(ptr) {}

    ~my_auto_ptr() {
        if(ptr) {
            delete ptr;
        }
    }

    my_auto_ptr(const my_auto_ptr<T>& other) : ptr(other.release()) {}

    my_auto_ptr(my_auto_ptr<T>&& other) noexcept : ptr(other.release()) {}

    my_auto_ptr<T>& operator=(my_auto_ptr<T>&& other) noexcept {
        if (this != &other) {
            reset(other.release());
        }
        return *this;
    }

    T& operator*() {
        return *ptr;
    }

    T* operator->() {
        return ptr;
    }

    T* get() {
        return ptr;
    }

    T* release() {
        T *temp = ptr;
        ptr = nullptr;
        return temp;
    }

    void reset(T *new_ptr = nullptr) {
        if(ptr != new_ptr) {
            delete ptr;
        }
        ptr = new_ptr;
    }

private:
    T *ptr;
};

int main() {
    shared_ptr<int> sp1(new int(10));
    std::cout << "sp1 value: " << *sp1 << ", use count: " << sp1.use_count() << std::endl;

    weak_ptr<int> wp1(sp1);
    std::cout << "wp1 use count (after weak_ptr created): " << wp1.use_count() << std::endl;

    shared_ptr<int> sp2 = wp1.lock();
    std::cout << "sp2 value (locked from wp1): " << *sp2 << ", use count: " << sp2.use_count() << std::endl;

    my_unique_ptr<int> uptr1(new int(20));
    std::cout << "uptr1 value: " << *uptr1 << std::endl;

    my_unique_ptr<int> uptr2 = std::move(uptr1);
    if (!uptr1.get()) {
        std::cout << "uptr1 is null after move" << std::endl;
    }
    std::cout << "uptr2 value: " << *uptr2 << std::endl;

    my_auto_ptr<int> aptr1(new int(30));
    std::cout << "aptr1 value: " << *aptr1 << std::endl;

    my_auto_ptr<int> aptr2 = std::move(aptr1);
    if (!aptr1.get()) {
        std::cout << "aptr1 is null after move" << std::endl;
    }
    std::cout << "aptr2 value: " << *aptr2 << std::endl;

    return 0;
}
