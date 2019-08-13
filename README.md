# ptracewrap

A C++ header-only wrapper around ptrace(2)

Compatible with C++11.

See `man 2 ptrace` for more information on how to use ptrace.

## Functions

```c++
long ptracewrap::ptrace(
    __ptrace_request request,
    pid_t pid,
    void* addr = nullptr,
    void* data = nullptr
) noexcept;
```

Calls ptrace(2) (`::ptrace`).

> Note: Since `::ptrace` is implemented as a variadic function on may systems, to call it properly, you have to do
> `::ptrace(request, pid, (void*) nullptr, (void*) nullptr)`, but this removes the hassle

```c++
long ptracewrap::ptrace(
    __ptrace_request request,
    pid_t pid,
    void* addr = nullptr,
    void* data = nullptr
);
```

Instead of having to check `errno`, throws a `ptracewrap::ptrace_error` on error instead. Still squashes `errno`.

```c++
template<class T>
T read(pid_t pid, void* address); 
```

Reads a `T` object from `address` in `pid`'s address space (Using `PTRACE_PEEKDATA`).
Can throw `ptracewrap::ptrace_error`.

> Note: All functions that take a `void*` that is the address in another process can also be called with
> a qualified pointer (e.g. `const void*` or `volatile char*`), since they don't actually read from the pointer

```c++
template<class T>
void read_to(pid_t pid, void* address, T& to);

template<class T>
void read_to(pid_t pid, void* address, T* to, std::size_t n);
```

Read a single object or `n` objects of type `T` (respectively), written to `to`, sequentially from `address` in
`pid`'s address space.
Can throw `ptracewrap::ptrace_error`.

```c++
template<class T>
void write(pid_t pid, void* address, const T& data);
```

Write the `T` object, `data`, to `address` in `pid`'s address space (Using `PTRACE_POKEDATA`).
Can throw `ptracewrap::ptrace_error`.

> Note: If `sizeof(T)` is not a multiple of `sizeof(long)` (Which is the unit that data is "poked" in), the last long
> will be read to make sure unrelated data isn't set. E.g. if `sizeof(long) == 8`, writing a `char[9]` will write the
> first 8 bytes, read the next 8 bytes and set the first byte, then write it again.

```c++
template<class T>
void write(pid_t pid, void* address, const T* from, std::size_t n);
```

Writes `n` objects of type `T`, pointed to by `from`, sequentially to `address` in process with pid `pid`'s
address space (Using `PTRACE_POKEDATA`).
Can throw `ptracewrap::ptrace_error`.

### Unsafe functions

All of the above functions require a type to be trivially copyable (Safe to `std::memcpy` with). If you need to read or
write types that are not trivially copyable, you should manage the members that aren't (e.g. if you wanted to view a
`vector`, find the offset of the `begin()` pointer and read that as pointers are trivial). If you still want to
use this with non-trivially-copyable types, these functions can be used (But expect them to not work 100% of the time)

```c++
template<class T>
T read_non_trivial(pid_t pid, void* address);

template<class T>
void read_to_non_trivial(pid_t pid, void* address, T* to, std::size_t n);

template<class T>
void read_to_non_trivial(pid_t pid, void* address, T& to);

template<class T>
void write_non_trivial(pid_t pid, void* address, const T* from, std::size_t n);

template<class T>
void write_non_trivial(pid_t pid, void* address, const T& data);
```

All of these do the exact same thing as their "trivial" counterparts, sans a `static_assert` to make sure that
the type is trivially copyable. They still throw `ptracewrap::ptrace_error`.

## Classes

```c++
class ptracewrap::ptrace_error : public std::system_error {
public:
    // Call with the same arguments passed to `ptrace(2)`
    ptrace_error(__ptrace_request request, pid_t pid, void* addr = nullptr, void* data = nullptr);
    // Same as above, except with an `errno` if it has changed
    ptrace_error(int errnum, __ptrace_request request, pid_t pid, void* addr = nullptr, void* data = nullptr);
    
    // Returns the passed `errnum` or `errno` at construction
    int get_errno() const noexcept;

    // Return the arguments passed to `ptrace(2)`
    __ptrace_request get_request() const noexcept;
    pid_t get_pid() const noexcept;
    void* get_addr() const noexcept;
    void* get_data() const noexcept;
    
    /* see below */ get_explanation() const noexcept;
};
```

If `USE_LIBEXPLAIN` is defined, `get_explanation()` will return a `const std::string&` of the message created
by libexplain, and `what()` will be this string.

Otherwise, `get_explanation()` will return `std::string`, initialised from `what()`.
