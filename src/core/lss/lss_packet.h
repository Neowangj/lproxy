#ifndef _LSS_PACKET_H_1
#define _LSS_PACKET_H_1
/*************************************************************************
	> File Name:    lss_packet.h
	> Author:       D_L
	> Mail:         deel@d-l.top
	> Created Time: 2015/12/4 10:50:30
 ************************************************************************/

#include <stdint.h>
//#include <vector>
#include <string>

#include <lss/typedefine.h>

namespace lproxy {

struct __packet {
public:
    byte   version;
    byte   type;
    byte   data_len_high_byte,  data_len_low_byte;
    data_t data;
public:
    __packet(void) : version(0xff), type(0xff), 
                       data_len_high_byte(0x00), data_len_low_byte(0x00),
                       data() {}
    
    __packet(byte version_,  byte pack_type_, 
           data_len_t data_len_, const data_t& data_)

            : version(version_), type(pack_type_), 
              data_len_high_byte((data_len_ >> 8) & 0xff),
              data_len_low_byte(data_len_ & 0xff), data(data_) {}

    __packet(byte version_, byte pack_type_,  byte data_len_high_, 
           byte data_len_low_, const data_t& data_)

            : version(version_), type(pack_type_), 
              data_len_high_byte(data_len_high_),
              data_len_low_byte(data_len_low_), data(data_) {}

    __packet(byte version_,  byte pack_type_, 
           data_len_t data_len_, data_t&& data_)

            : version(version_), type(pack_type_), 
              data_len_high_byte((data_len_ >> 8) & 0xff),
              data_len_low_byte(data_len_ & 0xff), 
              data(std::move(data_)) {}

    __packet(byte version_, byte pack_type_,  byte data_len_high_, 
           byte data_len_low_, data_t&& data_)

            : version(version_), type(pack_type_), 
              data_len_high_byte(data_len_high_),
              data_len_low_byte(data_len_low_), data(std::move(data_)) {}

    __packet(const __packet& that) {
        if (this != &that) {
            version = that.version;
            type    = that.type;
            data_len_high_byte = that.data_len_high_byte;
            data_len_low_byte  = that.data_len_low_byte;
            data.assign(that.data.begin(), that.data.end());
        }
    }
    __packet(__packet&& that) {
        if (this != &that) {
            version = that.version;
            type    = that.type;
            data_len_high_byte = that.data_len_high_byte;
            data_len_low_byte  = that.data_len_low_byte;
            data = std::move(that.data);
        }
    }
    __packet& operator= (const __packet& that) {
        if (this != &that) {
            version = that.version;
            type    = that.type;
            data_len_high_byte = that.data_len_high_byte;
            data_len_low_byte  = that.data_len_low_byte;
            data.assign(that.data.begin(), that.data.end());
        }
        return *this;
    }
    __packet& operator= (__packet&& that) {
        if (this != &that) {
            version = that.version;
            type    = that.type;
            data_len_high_byte = that.data_len_high_byte;
            data_len_low_byte  = that.data_len_low_byte;
            data = std::move(that.data);
        }
        return *this;
    }

    virtual ~__packet(void) {}

    
}; // struct lproxy::__packet

class request_and_reply_base_class {
public:
    virtual data_len_t data_len(void) const = 0;
    virtual ~request_and_reply_base_class() {}
};

struct __request_type : request_and_reply_base_class {
    enum PackType {
        hello    = 0x00,
        exchange = 0x02,
        data     = 0x06,
        zipdata  = 0x17,
        bad      = 0xff
    };
}; // struct lproxy::__request_type

struct __reply_type : request_and_reply_base_class {
    enum PackType {
        hello    = 0x01,
        exchange = 0x03,
        deny     = 0x04,
        timeout  = 0x05,
        data     = 0x06,
        zipdata  = 0x17,
        bad      = 0xff
    };
}; // struct lproxy::__reply_type


namespace local {

// local 端用来发给 server 端的数据包
class request : public __request_type {
public:
    request(void) : pack() {}
    request(byte version,        byte pack_type, 
               data_len_t data_len, const data_t& data)
        : pack(version, pack_type, data_len, data) {}

    request(byte version, byte pack_type,  byte data_len_high, 
               byte data_len_low, const data_t& data)
        : pack(version, pack_type, data_len_high, data_len_low, data) {}

    request(byte version,        byte pack_type, 
               data_len_t data_len, data_t&& data)
        : pack(version, pack_type, data_len, std::move(data)) {}

    request(byte version, byte pack_type,  byte data_len_high, 
               byte data_len_low, data_t&& data)
        : pack(version, pack_type, data_len_high, 
                data_len_low, std::move(data)) {}

    request(const request& that) : pack(that.pack) {}
    request(request&& that) : pack(std::move(that.pack)) {}

    virtual ~request(void) {}

    request& operator= (const request& that) {
        if (this != &that) {
             pack = that.pack;   
        }
        return *this;
    }
    request& operator= (request&& that) {
        if (this != &that) {
            pack = std::move(that.pack);
        }
        return *this;
    }

    request& assign(byte version,   byte pack_type,
            data_len_t data_len, const data_t& data) {
        this->pack = __packet(version, pack_type, data_len, data);
        return *this;
    }
    request& assign(byte version,   byte pack_type,
            data_len_t data_len, data_t&& data) {
        this->pack = __packet(version, pack_type, data_len, std::move(data));
        return *this;
    }

    virtual data_len_t data_len(void) const override {
        data_len_t len = pack.data_len_high_byte;
        return ((len << 8) & 0xff00) | pack.data_len_low_byte;
    }


    boost::array<boost::asio::const_buffer, 5> buffers(void) const {
        boost::array<boost::asio::const_buffer, 5> bufs = 
        {
            {
                boost::asio::buffer(&(pack.version), 1), 
                boost::asio::buffer(&(pack.type),    1), 
                boost::asio::buffer(&(pack.data_len_high_byte),  1), 
                boost::asio::buffer(&(pack.data_len_low_byte),   1), 
                boost::asio::buffer(pack.data), 
            }
        };
        return bufs;
    }

private:
    __packet pack;
}; // class lproxy::local::request

// local 端用来接收 server 端的数据包
class reply : public __reply_type {
public:
    reply(void) : pack(), pack_data_size_setting(false) {}
    explicit reply(const std::size_t data_initial_length) 
        : pack(__packet(0xff, 0xff, 0x00, 0x00, 
                    data_t(data_initial_length, 0))), 
          pack_data_size_setting(true) {} 
    virtual ~reply(void) {}
    
    explicit reply(const reply& that) : pack(that.pack) {}
    explicit reply(reply&& that) : pack(std::move(that.pack)) {}
    explicit reply(const __packet& _pack) : pack(_pack) {}
    explicit reply(__packet&& _pack) : pack(std::move(_pack)) {}
    reply& operator= (const reply& that) {
        if (this != &that) {
             pack = that.pack;   
        }
        return *this;
    }
    reply& operator= (reply&& that) {
        if (this != &that) {
            pack = std::move(that.pack);
        }
        return *this;
    }


    void set_data_size(std::size_t size, byte c = 0) {
        pack.data.resize(size, c);
        pack_data_size_setting = true;
        //_data_size = size;
    }
    void assign_data(std::size_t size, byte c = 0) {
        pack.data.assign(size, c);
        pack_data_size_setting = true;
    }

    // just for reading
    boost::array<boost::asio::mutable_buffer, 5> buffers(void) {
        assert(pack_data_size_setting);
        /*
        pack.version = 0xff;
        pack.type    = 0xff;
        pack.data_len_high_byte = 0x00;
        pack.data_len_high_byte = 0x00;
        pack.data.assign(_data_size, (char)0);
        */
        boost::array<boost::asio::mutable_buffer, 5> bufs =
        {
            {
                boost::asio::buffer(&(pack.version), 1), 
                boost::asio::buffer(&(pack.type),  1), 
                boost::asio::buffer(&(pack.data_len_high_byte),  1), 
                boost::asio::buffer(&(pack.data_len_low_byte),   1), 
                boost::asio::buffer(&(pack.data[0]), pack.data.size()), 
            }
        };
        return bufs;
    }

    byte version(void) {
        return pack.version;
    }

    reply::PackType type(void) {
        return (reply::PackType)pack.type;
    }
    
    virtual data_len_t data_len(void) const override {
        data_len_t len = pack.data_len_high_byte;
        return ((len << 8) & 0xff00) | pack.data_len_low_byte;
    }

    data_t& get_data(void) {
        return pack.data;
    }
    const data_t& get_data(void) const {
        return const_cast<reply*>(this)->get_data();
    }
    
private:
    __packet pack;
    bool pack_data_size_setting = false;
    //std::size_t    _data_size;
}; // class lproxy::local::reply


} // namespace lproxy::local

namespace server {

// server 端用来接收 local 端发来的数据包
class request : public __request_type {
public:
    request(void) : pack(), pack_data_size_setting(false) {}
    explicit request(const std::size_t data_initial_length) 
        : pack(__packet(0xff, 0xff, 0x00, 0x00, 
                    data_t(data_initial_length, 0))),
          pack_data_size_setting(true) {} 
    virtual ~request(void) {}

    explicit request(const request& that) : pack(that.pack) {}
    explicit request(request&& that) : pack(std::move(that.pack)) {}
    explicit request(const __packet& _pack) : pack(_pack) {}
    explicit request(__packet&& _pack) : pack(std::move(_pack)) {}
    request& operator= (const request& that) {
        if (this != &that) {
             pack = that.pack;   
        }
        return *this;
    }
    request& operator= (request&& that) {
        if (this != &that) {
            pack = std::move(that.pack);
        }
        return *this;
    }

    void set_data_size(std::size_t size, byte c = 0) {
        pack.data.resize(size, c);
        pack_data_size_setting = true;
        //_data_size = size;
    }
    void assign_data(std::size_t size, byte c = 0) {
        pack.data.assign(size, c);
        pack_data_size_setting = true;
    }

    // just for reading
    boost::array<boost::asio::mutable_buffer, 5> buffers(void) {
        assert(pack_data_size_setting);
        /*
        pack.version = 0xff;
        pack.type    = 0xff;
        pack.data_len_high_byte = 0x00;
        pack.data_len_high_byte = 0x00;
        pack.data.assign(_data_size, (char)0);
        */
        boost::array<boost::asio::mutable_buffer, 5> bufs =
        {
            {
                boost::asio::buffer(&(pack.version), 1), 
                boost::asio::buffer(&(pack.type),  1), 
                boost::asio::buffer(&(pack.data_len_high_byte),  1), 
                boost::asio::buffer(&(pack.data_len_low_byte),   1), 
                boost::asio::buffer(&(pack.data[0]), pack.data.size()), 
            }
        };
        return bufs;
    }

    byte version(void) {
        return pack.version;
    }

    request::PackType type(void) {
        return (request::PackType)pack.type;
    }
    
    virtual data_len_t data_len(void) const override {
        data_len_t len = pack.data_len_high_byte;
        return ((len << 8) & 0xff00) | pack.data_len_low_byte;
    }

    data_t& get_data(void) {
        return pack.data;
    }
    const data_t& get_data(void) const {
        return const_cast<request*>(this)->get_data();
    }
private:
    __packet pack;
    bool pack_data_size_setting = false;
    //std::size_t _data_size;
}; // class lproxy::server::request

// server 端用来发给 local 端的数据
class reply : public __reply_type {
public:
    reply(void) : pack() {}
    reply(byte version,        byte pack_type, 
               data_len_t data_len, const data_t& data)
        : pack(version, pack_type, data_len, data) {}

    reply(byte version, byte pack_type,  byte data_len_high, 
               byte data_len_low, const data_t& data)
        : pack(version, pack_type, data_len_high, data_len_low, data) {}

    reply(byte version,        byte pack_type, 
               data_len_t data_len, data_t&& data)
        : pack(version, pack_type, data_len, std::move(data)) {}

    reply(byte version, byte pack_type,  byte data_len_high, 
               byte data_len_low, data_t&& data)
        : pack(version, pack_type, data_len_high, 
                data_len_low, std::move(data)) {}


    reply(const reply& that) : pack(that.pack) {}
    reply(reply&& that) : pack(std::move(that.pack)) {}

    virtual ~reply(void) {}

    reply& operator= (const reply& that) {
        if (this != &that) {
             pack = that.pack;   
        }
        return *this;
    }
    reply& operator= (reply&& that) {
        if (this != &that) {
            pack = std::move(that.pack);
        }
        return *this;
    }


    reply& assign(byte version,   byte pack_type,
            data_len_t data_len, const data_t& data) {
        this->pack = __packet(version, pack_type, data_len, data);
        return *this;
    }
    reply& assign(byte version,   byte pack_type,
            data_len_t data_len, data_t&& data) {
        this->pack = __packet(version, pack_type, data_len, std::move(data));
        return *this;
    }

    virtual data_len_t data_len(void) const override {
        data_len_t len = pack.data_len_high_byte;
        return ((len << 8) & 0xff00) | pack.data_len_low_byte;
    }


    boost::array<boost::asio::const_buffer, 5> buffers(void) const {
        boost::array<boost::asio::const_buffer, 5> bufs = 
        {
            {
                boost::asio::buffer(&(pack.version), 1), 
                boost::asio::buffer(&(pack.type),    1), 
                boost::asio::buffer(&(pack.data_len_high_byte),  1), 
                boost::asio::buffer(&(pack.data_len_low_byte),   1), 
                boost::asio::buffer(pack.data), 
            }
        };
        return bufs;
    }

private:
    __packet pack;
}; // class lproxy::server::reply

} // namespace lproxy::server

} // namespace lproxy

#endif // _LSS_PACKET_H_1
