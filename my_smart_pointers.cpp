#include <iostream>
#include <memory>

class my_auto_ptr 
{
    public:
        my_auto_ptr(int *ptr): ptr(ptr) {}
        ~my_auto_ptr() {
            if(ptr) {
                delete ptr;
            }
        }
        //my_auto_ptr(const my_auto_ptr& other) = delete;
        //my_auto_ptr& operator=(const my_auto_ptr& other) = delete;
        my_auto_ptr(const my_auto_ptr& other)
        {   
            if(other.ptr) {
                ptr = new int(*other.ptr);
            }
            else (ptr = nullptr);
        }
        my_auto_ptr(my_auto_ptr&& other) noexcept: ptr(other.ptr) {
            other.ptr = nullptr;
        }
        my_auto_ptr& operator=(my_auto_ptr&& other)noexcept {
            if (this == &other) {
                return *this;
            }            
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
            return *this;
        }
        int& operator*() {
            return *ptr;
        }
        int* operator->() {
            return ptr;
        }
    private:
        int *ptr; 
};
