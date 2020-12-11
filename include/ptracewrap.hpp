//
// Created by Mital on 13.08.2019.
//

#ifndef PTRACEWRAP_PTRACEWRAP_HPP_
#define PTRACEWRAP_PTRACEWRAP_HPP_

//

#include <cstring>
#include <cstddef>
#include <cerrno>
#include <system_error>
#include <type_traits>
#include <memory>
#include <string>
#include <iterator>

#include <sys/ptrace.h>
#include <sys/types.h>

#ifdef USE_LIBEXPLAIN
extern "C" {
    void explain_message_errno_ptrace(char* message, int message_size, int errnum, int request, ::pid_t pid, void* addr, void* data);
}
#endif

namespace ptracewrap {

#ifdef USE_LIBEXPLAIN

class ptrace_error : public ::std::system_error {
    ptrace_error(::__ptrace_request request, ::pid_t pid, void* addr = nullptr, void* data = nullptr) : ptrace_error(errno, request, pid, addr, data) {}

    ptrace_error(int errnum, ::__ptrace_request request, ::pid_t pid, void* addr = nullptr, void* data = nullptr) :
      ::std::system_error(::std::error_code(errnum, ::std::generic_category()), "ptrace"),
      m_request(request), m_pid(pid), m_addr(addr), m_data(data) {}

    ptrace_error(const ptrace_error& other) :
      ptrace_error(other.get_errno(), other.m_request, other.m_pid, other.m_addr, other.m_data) {
        if (other.m_explanation.size() != 0) {
            m_explanation = other.m_explanation;
        }
    }

    ptrace_error& operator=(const ptrace_error& other) /* noexcept */ {
        if (*this != other) {
            // Both invalid
            // const_cast<std::error_code&>(code()) = other.code();
            // ::new (this) std::system_error(other.code(), "ptrace");
            if (get_errno() != other.get_errno()) {
                throw ::std::runtime_error("Tried to assign ptrace_error to a ptrace_error with a different code");
            }
            m_request = other.m_request;
            m_pid = other.m_pid;
            m_explanation = other.m_explanation;
            m_addr = other.m_addr;
            m_data = other.m_data;
        }
        return *this;
    }

    bool operator==(const ptrace_error& other) const noexcept {
        if (this == &other) {
            return true;
        }
        if (
            m_request == other.m_request && m_pid == other.m_pid &&
            m_addr == other.m_addr && m_data == other.m_data &&
            get_errno() == other.get_errno()
        ) {
            if (m_explanation.size() != 0) {
                if (other.m_explanation.size() == 0) {
                    other.m_explanation = m_explanation;
                }
            } else if (other.m_explanation.size() != 0) {
                m_explanation = other.m_explanation;
            }
            return true;
        }
        return false;
    }

    bool operator!=(const ptrace_error& other) const noexcept {
        return !this->operator==(other);
    }

    int get_errno() const noexcept {
        return code().value();
    }

    ::__ptrace_request get_request() const noexcept {
        return m_request;
    }

    ::pid_t get_pid() const noexcept {
        return m_pid;
    }

    void* get_addr() const noexcept {
        return m_addr;
    }

    void* get_data() const noexcept {
        return m_data;
    }

    const ::std::string& get_explanation() const noexcept {
        if (m_explanation.size() == 0) {
            set_explanation();
        }
        return m_explanation;
    }

    const char* what() const noexcept override {
        return get_explanation().c_str();
    }
private:
    void set_explanation() const noexcept {
        char buf[3001];

        ::explain_message_errno_ptrace(buf, static_cast<int>(sizeof(buf)), get_errno(), m_request, m_pid, m_addr, m_data);
        m_explanation = buf;
    }

    ::__ptrace_request m_request;
    ::pid_t m_pid;
    void* m_addr;
    void* m_data;
    mutable ::std::string m_explanation;
};
#else
class ptrace_error : public ::std::system_error {
public:
    ptrace_error(::__ptrace_request request, ::pid_t pid, void* addr = nullptr, void* data = nullptr) : ptrace_error(errno, request, pid, addr, data) {}

    ptrace_error(int errnum, ::__ptrace_request request, ::pid_t pid, void* addr = nullptr, void* data = nullptr) :
      ::std::system_error(::std::error_code(errnum, ::std::generic_category()), "ptrace"),
      m_request(request), m_pid(pid), m_addr(addr), m_data(data) { }

    ptrace_error(const ptrace_error& other) :
      ptrace_error(other.get_errno(), other.m_request, other.m_pid, other.m_addr, other.m_data) {}

    ptrace_error& operator=(const ptrace_error& other) /* noexcept */ {
        if (*this != other) {
            // Both invalid
            // const_cast<std::error_code&>(code()) = other.code();
            // ::new (this) std::system_error(other.code(), "ptrace");
            if (get_errno() != other.get_errno()) {
                throw ::std::runtime_error("Tried to assign ptrace_error to a ptrace_error with a different code");
            }
            m_request = other.m_request;
            m_pid = other.m_pid;
            m_addr = other.m_addr;
            m_data = other.m_data;
        }
        return *this;
    }

    bool operator==(const ptrace_error& other) const noexcept {
        return this == &other || (
            m_request == other.m_request && m_pid == other.m_pid &&
            m_addr == other.m_addr && m_data == other.m_data &&
            get_errno() == other.get_errno()
        );
    }

    bool operator!=(const ptrace_error& other) const noexcept {
        return !this->operator==(other);
    }

    int get_errno() const noexcept {
        return code().value();
    }

    ::__ptrace_request get_request() const noexcept {
        return m_request;
    }

    ::pid_t get_pid() const noexcept {
        return m_pid;
    }

    void* get_addr() const noexcept {
        return m_addr;
    }

    void* get_data() const noexcept {
        return m_data;
    }

    ::std::string get_explanation() const noexcept {
        return what();
    }
private:
    ::__ptrace_request m_request;
    ::pid_t m_pid;
    void* m_addr;
    void* m_data;
};
#endif

namespace detail {
    inline void memcpy(void* to, const void* from, ::std::size_t n) noexcept {
        ::std::memcpy(to, from, n);
    }

    template<class T, class U>
    typename ::std::enable_if< ::std::is_volatile<T>::value || ::std::is_volatile<U>::value, void>::type
    memcpy(T* to, const U* from, ::std::size_t n) noexcept {
        typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile void*, void*>::type tvp;
        typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile char*, char*>::type tcp;
        typedef typename ::std::conditional< ::std::is_volatile<U>::value, const volatile void*, const void*>::type uvp;
        typedef typename ::std::conditional< ::std::is_volatile<U>::value, const volatile char*, const char*>::type ucp;
        tcp a = static_cast<tcp>(static_cast<tvp>(to));
        ucp b = static_cast<ucp>(static_cast<uvp>(from));
        while (n-- > 0) {
            *a++ = *b++;
        }
    }

    template< ::std::size_t n>
    void memcpy(void* to, const void* from) noexcept {
        ::std::memcpy(to, from, n);
    }

    template< ::std::size_t N, class T, class U>
    typename ::std::enable_if< ::std::is_volatile<T>::value || ::std::is_volatile<U>::value, void>::type
    memcpy(T* to, const U* from) noexcept {
        typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile void*, void*>::type tvp;
        typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile char*, char*>::type tcp;
        typedef typename ::std::conditional< ::std::is_volatile<U>::value, const volatile void*, const void*>::type uvp;
        typedef typename ::std::conditional< ::std::is_volatile<U>::value, const volatile char*, const char*>::type ucp;
        tcp a = static_cast<tcp>(static_cast<tvp>(to));
        ucp b = static_cast<ucp>(static_cast<uvp>(from));
        ::std::size_t n = N;
        while (n-- > 0) {
            *a++ = *b++;
        }
    }

    inline long ptrace_noreset_errno(::__ptrace_request request, ::pid_t pid, void* addr = nullptr, void* data = nullptr) {
        long result = ::ptrace(request, pid, addr, data);
        if (result == -1 && errno != 0) {
            throw ::ptracewrap::ptrace_error(request, pid, addr, data);
        }
        return result;
    }

    template<class T>
    struct void_t {
        typedef void type;
    };
}

typedef ::pid_t pid_t;
typedef ::__ptrace_request ptrace_request;

// Use ptracewrap::ptrace instead of ::ptrace, as the global ptrace is variadic and
// your arguments may not be the correct type
// See ptrace(2) for usage
inline long ptrace(::__ptrace_request request, ::pid_t pid, void* addr = nullptr, void* data = nullptr) noexcept {
    return ::ptrace(request, pid, addr, data);
}

// Throws `ptrace_error` instead of needing to check errno
inline long ptrace_w_error(::__ptrace_request request, ::pid_t pid, void* addr = nullptr, void* data = nullptr) {
    errno = 0;
    long result = ::ptrace(request, pid, addr, data);
    if (result == -1 && errno != 0) {
        throw ::ptracewrap::ptrace_error(request, pid, addr, data);
    }
    return result;
}

// Like ptrace::read, but writes to `to` instead of returning (Works with arrays)
template<class T>
void read_to(::pid_t pid, void* address, T& to) {
    static_assert(::std::is_trivially_copyable<T>::value, "Can only ptrace_read trivial types");
    long data[static_cast< ::std::size_t>(sizeof(T) / sizeof(long)) + (sizeof(T) % sizeof(long) == 0 ? 0 : 1)];
    errno = 0;
    for (long& l : data) {
        l = ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_PEEKDATA, pid, address);
    }
    ::std::memcpy(::std::addressof(to), data, sizeof(T));
}

template<class T>
void read_to(::pid_t pid, const volatile void* address, T& to) {
    return ::ptracewrap::read_to(pid, const_cast<void*>(address), to);
}

// Like ptrace::read_to, but read to contiguous storage of `n` `T`s pointed to by `to`
template<class T>
void read_to(::pid_t pid, void* address, T* to, ::std::size_t n) {
    static_assert(::std::is_trivially_copyable<T>::value, "Can only ptrace_read trivial types");
    static_assert(!::std::is_const<T>::value, "read_to argument 3 (T* to) must be non-const to write to");
    typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile void*, void*>::type vp;
    typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile char*, char*>::type cp;
    ::std::size_t whole_longs = sizeof(T) * n / sizeof(long);
    ::std::size_t remainder = (sizeof(T) * n) % sizeof(long);
    bool has_remainder = remainder != 0;
    errno = 0;
    for (::std::size_t i = 0; i < whole_longs; ++i) {
        long l = ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_PEEKDATA, pid, address);
        address = static_cast<void*>(static_cast<long*>(address) + 1);
        ::ptracewrap::detail::memcpy<sizeof(long)>(static_cast<cp>(static_cast<vp>(to)) + i * sizeof(long), &l);
    }
    if (has_remainder) {
        long l = ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_PEEKDATA, pid, address);
        ::ptracewrap::detail::memcpy(static_cast<cp>(static_cast<vp>(to)) + whole_longs * sizeof(long), &l, remainder);
    }
}

template<class T>
void read_to(::pid_t pid, const volatile void* address, T* to, ::std::size_t n) {
    return ::ptracewrap::read_to(pid, const_cast<void*>(address), to, n);
}

// Reads a value of type `T` from `address` in the process with pid `pid`'s virtual address space
template<class T>
T read(::pid_t pid, void* address) {
    static_assert(!::std::is_reference<T>::value, "Cannot ptrace_read with T as a reference");
    T out;
    ::ptracewrap::read_to(pid, address, out);
    return out;
}

template<class T>
T read(::pid_t pid, const volatile void* address) {
    return ::ptracewrap::read<T>(pid, const_cast<void*>(address));
}

// Writes `data` to `address` in the process with pid `pid`'s virtual address space
template<class T>
void write(::pid_t pid, void* address, const T& data) {
    static_assert(::std::is_trivially_copyable<T>::value, "Can only ptrace_write trivial types");

    constexpr ::std::size_t whole_longs = sizeof(T) / sizeof(long);
    constexpr ::std::size_t remainder_bytes = sizeof(T) % sizeof(long);
    constexpr bool has_remainder = remainder_bytes != 0;
    long ldata[whole_longs + has_remainder];

    ::ptracewrap::detail::memcpy<whole_longs * sizeof(long)>(static_cast<long*>(ldata), ::std::addressof(data));

    if (has_remainder) {
        char last_long[sizeof(long)];
        ::std::memcpy(last_long, static_cast<const char*>(static_cast<const void*>(::std::addressof(data))) + whole_longs * sizeof(long), remainder_bytes);
        void* last_address = static_cast<void*>(static_cast<long*>(address) + whole_longs);
        long rest = ::ptracewrap::ptrace_w_error(PTRACE_PEEKDATA, pid, last_address);
        ::std::memcpy(last_long + remainder_bytes, static_cast<char*>(static_cast<void*>(&rest)) + remainder_bytes, sizeof(long) - remainder_bytes);
        ::std::memcpy(ldata + whole_longs, last_long, sizeof(long));
    }

    for (long& l : ldata) {
        ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_POKEDATA, pid, address, reinterpret_cast<void*>(l));
        address = static_cast<void*>(static_cast<long*>(address) + 1);
    }
}

template<class T>
void write(::pid_t pid, const volatile void* address, const T& data) {
    return ::ptracewrap::write(pid, const_cast<void*>(address), data);
}

template<class T>
void write(::pid_t pid, void* address, const T* from, ::std::size_t n) {
    static_assert(::std::is_trivially_copyable<T>::value, "Can only ptrace_write trivial types");
    typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile void, void>::type v;
    typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile char, char>::type c;

    ::std::size_t whole_longs = sizeof(T) * n / sizeof(long);
    ::std::size_t remainder_bytes = (sizeof(T) * n) % sizeof(long);
    bool has_remainder = remainder_bytes != 0;

    for (::std::size_t i = 0; i < whole_longs; ++i) {
        long to_write;
        ::ptracewrap::detail::memcpy<sizeof(long)>(&to_write, static_cast<const c*>(static_cast<const v*>(from)) + i * sizeof(long));
        ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_POKEDATA, pid, address, reinterpret_cast<void*>(to_write));
        address = static_cast<void*>(static_cast<long*>(address) + 1);
    }

    if (has_remainder) {
        char last_long[sizeof(long)];
        ::ptracewrap::detail::memcpy(last_long, static_cast<const c*>(static_cast<const v*>(from)) + whole_longs * sizeof(long), remainder_bytes);
        void* last_address = static_cast<void*>(static_cast<long*>(address) + whole_longs);
        long rest = ptrace_w_error(PTRACE_PEEKDATA, pid, last_address);
        ::ptracewrap::detail::memcpy(last_long + remainder_bytes, static_cast<char*>(static_cast<void*>(&rest)) + remainder_bytes, sizeof(long) - remainder_bytes);
        long last_long_as_long;
        ::ptracewrap::detail::memcpy<sizeof(long)>(&last_long_as_long, last_long);
        ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_POKEDATA, pid, address, reinterpret_cast<void*>(last_long_as_long));
    }
}

template<class InputIt, class Sentinel = InputIt>
typename ::ptracewrap::detail::void_t<typename ::std::iterator_traits<InputIt>::value_type>::type
write(::pid_t pid, const volatile void* address, InputIt first, Sentinel last) {
    typedef typename ::std::remove_reference<typename ::std::iterator_traits<InputIt>::value_type>::type cv_value_type;
    typedef typename ::std::remove_cv<cv_value_type>::type value_type;
    static_assert(::std::is_trivially_copyable<value_type>::value, "Can only ptrace_write trivial types");

    void* write_address = const_cast<void*>(address);
    char remainder[sizeof(long)];
    ::std::size_t remainder_pos = 0;

    typedef typename ::std::conditional< ::std::is_volatile<cv_value_type>::value, const volatile value_type&, const value_type&>::type reference;
    typedef typename ::std::conditional< ::std::is_volatile<cv_value_type>::value, const volatile void*, const void*>::type vp;
    typedef typename ::std::conditional< ::std::is_volatile<cv_value_type>::value, const volatile char*, const char*>::type cp;

    errno = 0;
    for (; first != last; ++first) {
        reference value = static_cast<reference>(*first);
        ::std::size_t to_write = sizeof(value_type);
        cp pointer = static_cast<cp>(static_cast<vp>(::std::addressof(value)));
        if (remainder_pos) {
            if (to_write >= sizeof(long) - remainder_pos) {
                ::ptracewrap::detail::memcpy(static_cast<char*>(static_cast<void*>(remainder)) + remainder_pos, pointer, sizeof(long) - remainder_pos);
                to_write -= sizeof(long) - remainder_pos;
                pointer += sizeof(long) - remainder_pos;

                long remainder_as_long;
                ::std::memcpy(&remainder_as_long, remainder, sizeof(long));
                ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_POKEDATA, pid, write_address, reinterpret_cast<void*>(remainder_as_long));
                write_address = static_cast<void*>(static_cast<char*>(write_address) + sizeof(long));
            } else {
                ::ptracewrap::detail::memcpy(static_cast<char*>(static_cast<void*>(remainder)) + remainder_pos, pointer, to_write);
                remainder_pos += to_write;
                to_write = 0;
            }
        }

        while (to_write >= sizeof(long)) {
            char data[sizeof(long)];
            ::ptracewrap::detail::memcpy(data, pointer, sizeof(long));
            long data_as_long;
            ::std::memcpy(&data_as_long, data, sizeof(long));
            ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_POKEDATA, pid, write_address, reinterpret_cast<void*>(data_as_long));
            write_address = static_cast<void*>(static_cast<char*>(write_address) + sizeof(long));
            to_write -= sizeof(long);
            if (to_write) pointer += sizeof(long);
        }

        if (to_write) {
            remainder_pos = to_write;
            ::ptracewrap::detail::memcpy(remainder, pointer, to_write);
        }
    }

    if (remainder_pos != 0) {
        long last = ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_PEEKDATA, pid, write_address);
        ::std::memcpy(remainder, static_cast<char*>(static_cast<void*>(&last)) + remainder_pos, sizeof(long) - remainder_pos);
        long remainder_as_long;
        ::std::memcpy(&remainder_as_long, remainder, sizeof(long));
        ::ptracewrap::detail::ptrace_noreset_errno(PTRACE_POKEDATA, pid, write_address, reinterpret_cast<void*>(remainder_as_long));
    }
}

template<class T>
void write(::pid_t pid, const volatile void* address, const T* from, ::std::size_t n) {
    return ::ptracewrap::write(pid, const_cast<void*>(address), from, n);
}

// Like ptrace_read, but relies on undefined behaviour for non trivial types (`memcpy`s non trivial types)
template<class T>
T read_non_trivial(::pid_t pid, const volatile void* address) {
    alignas(T) char out[sizeof(T)];
    ::ptracewrap::read_to<char>(pid, address, out, sizeof(out));
    return static_cast<T&&>(*static_cast<T*>(static_cast<void*>(out)));
}

// Like ptrace_read_to, but relies on undefined behaviour for non trivial types (`memcpy`s non trivial types)
template<class T>
void read_to_non_trivial(::pid_t pid, const volatile void* address, T* to, ::std::size_t n) {
    typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile void*, void*>::type vp;
    typedef typename ::std::conditional< ::std::is_volatile<T>::value, volatile char, char>::type ct;
    typedef ct* cp;
    ::ptracewrap::read_to<ct>(pid, address, static_cast<cp>(static_cast<vp>(to)), sizeof(T) * n);
}

// Like ptrace_read_to, but relies on undefined behaviour for non trivial types (`memcpy`s non trivial types)
template<class T>
void read_to_non_trivial(::pid_t pid, const volatile void* address, T& to) {
    ::ptracewrap::read_to_non_trivial(pid, address, ::std::addressof(to), 1u);
}

template<class T>
void write_non_trivial(::pid_t pid, const volatile void* address, const T* from, ::std::size_t n) {
    typedef typename ::std::conditional< ::std::is_volatile<T>::value, const volatile void*, const void*>::type vp;
    typedef typename ::std::conditional< ::std::is_volatile<T>::value, const volatile char, const char>::type ct;
    typedef ct* cp;
    ::ptracewrap::write<ct>(pid, address, static_cast<cp>(static_cast<vp>(from)), sizeof(T) * n);
}

template<class T>
void write_non_trivial(::pid_t pid, const volatile void* address, const T& data) {
    ::ptracewrap::write_non_trivial(pid, address, ::std::addressof(data), 1u);
}

}

#endif  // PTRACEWRAP_PTRACEWRAP_HPP_
