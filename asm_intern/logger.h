#ifndef __LOGGERH
#define __LOGGERH

#include <sstream>
#include <string>

namespace dave
{
    class logger;
    struct end {};

    class log_stream {
    private:
        logger *_logger;
        std::stringstream _stm;
    public:
        log_stream(logger *logger);

        template<typename T> friend log_stream& operator <<(log_stream &ls, const T &value) {
            ls._stm << value;
            return ls;
        }

        void flush();
    };

    void operator <<(log_stream &ls, const end&);

    class logger {
    private:
        log_stream _stm;
    protected:
        virtual void write(const std::string &msg) = 0;
    public:
        std::string _filename;
        size_t _line_no;

        logger()
        : _stm(this)
        {}

        void log(const char *msg) {
            std::stringstream stm;
            if (!_filename.empty()) {
                stm << _filename;
                if (_line_no != 0) {
                    stm << ':' << _line_no;
                }
                stm << " - ";
            }
            stm << msg;
            write(stm.str());
        }
        void log(const std::string &msg) {
            log(msg.c_str());
        }
        log_stream& log() {
            return _stm;
        }

        friend class log_stream;

        static logger* def;
    };

    class stm_logger : public logger {
    private:
        std::ostream &_stm;
    protected:
        virtual void write(const std::string &msg) override {
            _stm << msg << std::endl;
        }
    public:
        stm_logger(std::ostream &stm)
        : _stm(stm)
        {}
    };
}

#endif