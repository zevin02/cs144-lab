#ifndef SPONGE_LIBSPONGE_FILE_DESCRIPTOR_HH
#define SPONGE_LIBSPONGE_FILE_DESCRIPTOR_HH

#include "buffer.hh"

#include <array>
#include <cstddef>
#include <limits>
#include <memory>

//! A reference-counted handle to a file descriptor
class FileDescriptor {
    //! \brief A handle on a kernel file descriptor.
    //! \details FileDescriptor objects contain a std::shared_ptr to a FDWrapper.
    class FDWrapper {
      public:
        int _fd;                    //!< The file descriptor number returned by the kernel,通过内核返回的一个文件描述符数字
        bool _eof = false;          //!< Flag indicating whether FDWrapper::_fd is at EOF,fd是否处在文件末尾
        bool _closed = false;       //!< Flag indicating whether FDWrapper::_fd has been closed，fd是否已经关闭
        unsigned _read_count = 0;   //!< The number of times FDWrapper::_fd has been read，fd读取文件花费的时间
        unsigned _write_count = 0;  //!< The numberof times FDWrapper::_fd has been written，fd写文件花的时间

        //! Construct from a file descriptor number returned by the kernel
        //可以使用构造函数
        explicit FDWrapper(const int fd);//内核来获得fd来构造
        //! Closes the file descriptor upon destruction
        ~FDWrapper();//稀构函数来关闭文件描述符
        //! Calls [close(2)](\ref man2::close) on FDWrapper::_fd
        void close();//手动关闭文件描述符

        //! \name
        //! An FDWrapper cannot be copied or moved

        //!@{
        //不能被拷贝构造，赋值，移动构造，赋值

        FDWrapper(const FDWrapper &other) = delete;
        FDWrapper &operator=(const FDWrapper &other) = delete;
        FDWrapper(FDWrapper &&other) = delete;
        FDWrapper &operator=(FDWrapper &&other) = delete;
        //!@}
    };
  //这个时private，无法被子类继承

    //! A reference-counted handle to a shared FDWrapper
    std::shared_ptr<FDWrapper> _internal_fd;//只有一个指向文件描述符的结构体指针，可以被拷贝

    // private constructor used to duplicate the FileDescriptor (increase the reference count)
    explicit FileDescriptor(std::shared_ptr<FDWrapper> other_shared_ptr);//可以使用私有拷贝构造，

  protected:
  //给子类可以使用，别人不能使用
    void register_read() { ++_internal_fd->_read_count; }    //!< increment read count
    void register_write() { ++_internal_fd->_write_count; }  //!< increment write count

  public:
    //! Construct from a file descriptor number returned by the kernel
    explicit FileDescriptor(const int fd);

    //! Free the std::shared_ptr; the FDWrapper destructor calls close() when the refcount goes to zero.
    ~FileDescriptor() = default;

    //! Read up to `limit` bytes
    //读取limit字节后放回
    std::string read(const size_t limit = std::numeric_limits<size_t>::max());

    //! Read up to `limit` bytes into `str` (caller can allocate storage)
    //把读取的limit的字节都放到str里面
    void read(std::string &str, const size_t limit = std::numeric_limits<size_t>::max());

    //! Write a string, possibly blocking until all is written
    //往套接子里面写数据，write_all判断是否阻塞的写，直到全部写完
    
    //write重载了很多方法
    size_t write(const char *str, const bool write_all = true) { return write(BufferViewList(str), write_all); }

    //! Write a string, possibly blocking until all is written
    size_t write(const std::string &str, const bool write_all = true) { return write(BufferViewList(str), write_all); }

    //! Write a buffer (or list of buffers), possibly blocking until all is written
    size_t write(BufferViewList buffer, const bool write_all = true);

    //! Close the underlying file descriptor
    //关闭文件描述符，调用里面定义的结构体来操作
    void close() { _internal_fd->close(); }

    //! Copy a FileDescriptor explicitly, increasing the FDWrapper refcount
    //拷贝一个相同的filedescriptor

    FileDescriptor duplicate() const;

    //! Set blocking(true) or non-blocking(false)
    //设置文件描述符号是否阻塞
    void set_blocking(const bool blocking_state);

    //! \name FDWrapper accessors
    //!@{

    //! underlying descriptor number
    //获得我这个文件描述符对应的数字

    int fd_num() const { return _internal_fd->_fd; }

    //! EOF flag state
    //获得他是否处在eof

    bool eof() const { return _internal_fd->_eof; }

    //! closed flag state
    //判断文件描述法是否已经关闭
    bool closed() const { return _internal_fd->_closed; }

    //! number of reads
    unsigned int read_count() const { return _internal_fd->_read_count; }

    //! number of writes
    unsigned int write_count() const { return _internal_fd->_write_count; }
    //!@}

    //! \name Copy/move constructor/assignment operators
    //! FileDescriptor can be moved, but cannot be copied (but see duplicate())
    //!@{

    //因为使用的时指针，所以我们不能使用拷贝构造系列，只能使用移动构造系列
    
    FileDescriptor(const FileDescriptor &other) = delete;             //!< \brief copy construction is forbidden
    FileDescriptor &operator=(const FileDescriptor &other) = delete;  //!< \brief copy assignment is forbidden
    FileDescriptor(FileDescriptor &&other) = default;                 //!< \brief move construction is allowed
    FileDescriptor &operator=(FileDescriptor &&other) = default;      //!< \brief move assignment is allowed
    //!@}
};

//! \class FileDescriptor
//! In addition, FileDescriptor tracks EOF state and calls to FileDescriptor::read and
//! FileDescriptor::write, which EventLoop uses to detect busy loop conditions.
//!
//! For an example of FileDescriptor use, see the EventLoop class documentation.

#endif  // SPONGE_LIBSPONGE_FILE_DESCRIPTOR_HH
