/*************************************************************************
	> File Name:    mysecret.cpp
	> Author:       D_L
	> Mail:         deel@d-l.top
	> Created Time: 2016/1/11 14:04:04
 ************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <boost/algorithm/string.hpp>
#include "crypto/aes_crypto.h"
//#include "crypto/rsa_crypto.h"
using namespace std;
using namespace crypto;

typedef std::vector<uint8_t> stream;

enum LogLevel {DEBUG = 0, INFO, WARNING, ERROR, FATAL};
std::string gen_log(LogLevel loglevel, const std::string& message) {
    const std::string Level[5] = {
        "[DEBUG]", "[INFO]", "[WARNING]", "[ERROR]", "[FATAL]"
    };
    return Level[loglevel] + ' ' + message;
}
std::string gen_log(LogLevel loglevel, const std::ostringstream& message) {
    return gen_log(loglevel, message.str());
}

/*
template<typename T>
void assert_pretty(T&& condition, const string& message = "") {
    if (! std::forward<T>(condition)) {
        if (message != "") {
            std::cerr << message << std::endl;
        }
        assert(std::forward<T>(condition));
    } 
}
*/

template<typename T>
T&& pretty(T&& condition, const string& message = "") {
    if (! std::forward<T>(condition)) {
        if (message != "") {
            std::cerr << message << std::endl;
        }
    } 
    return std::forward<T>(condition);
}

void usage(void) {
   //mysecret [-e file]
   //mysecret [-d file]
   //mysecret [-h, --help]
   cout << "this is a help message.\n";
}

/*
readfile(is, stream& content) {
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    

    content.resize(length);
    is.read(&content[0], length);

    if (! is) {
        std::cout << "error: only " << is.gcount() << " could be read";
    }
    // is.close()
    
}
*/

std::string& trim(std::string& some) {
    //" \t\n\v\f\r";
    boost::algorithm::trim(some);
    return some;
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        usage();
        return 1;
    }

    enum { ENCODE, DECODE } mode;

    const std::string mode_arg = argv[1]; 
    if (mode_arg == "-e") {
        mode = ENCODE;
    }
    else if (mode_arg == "-d") {
        mode = DECODE;
    }
    else {
        usage();
        return 2;
    }

    const std::string file_in = argv[2]; 
    const std::string file_out = file_in + ".out";

    // step 1. 到当前工作目录找 key 文件
    try {
        //stream key;
        //readfile(ifstream("./key"), key);
        char key_buffer[1024] = {0};
        auto&& keyfile = std::ifstream("./key");
        assert(pretty(keyfile, gen_log(FATAL, "cannot open file: 'key'")));
        keyfile.getline(key_buffer, 
                sizeof(key_buffer) / sizeof(key_buffer[0]));
        std::string key = key_buffer;
        trim(key);
        assert(pretty(key.size(), gen_log(FATAL, "key is empty!")));
        std::cout << gen_log(DEBUG, "key : [" + key + ']') << std::endl;

        stream input, output;
        std::ifstream is (file_in, std::ifstream::binary);
        assert(pretty(is, gen_log(FATAL,
                        "cannot open file: '" + file_in + "'")));
        {
            is.seekg (0, is.end);
            int length = is.tellg();
            is.seekg (0, is.beg);

            char buffer[length];
            is.read (buffer, length);
            if (is) {
                std::cout << gen_log(INFO, "all characters read successfully.")
                    << std::endl;
            }
            else {
                std::ostringstream oss;
                oss << "error: only " << is.gcount() << " could be read";
                assert(pretty(is, gen_log(FATAL, oss)));
            }
            is.close();
            input.assign(buffer, buffer + length);
        }   

        Encryptor encryptor(new Aes(std::string(key)));
        switch (mode) {
        case ENCODE:
            encryptor.encrypt(output, &input[0], input.size());
            break;
        case DECODE:
            encryptor.decrypt(output, &input[0], input.size());
            break;
        default:
            usage();
            return -1;
        }

        std::string output_buffer(output.begin(), output.end());
        //std::ofstream("./file.out", std::ofstream::out) << output_buffer;
        std::ofstream(file_out, std::ios_base::out | std::ios_base::binary)
            << output_buffer;
    }
    catch (...) {
        throw;
    }
    return 0;
}
