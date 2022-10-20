#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <vector>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#pragma once

#include <string>

class base_type {
public:
    base_type(char t, int index) : type(t), index(index) {}
    char type;
    int index;

    char getType() { return type; }
    virtual std::string getArg() = 0;
};

/*
    Possible types:
    d - int
    s - string
    f - double
*/
template <class T>
class d_type : public base_type {
public:
    d_type(char t, std::string a, T val, int index) : base_type(t, index) {
        arg = a;
        value = val;
    }
    std::string arg;
    T value;

    virtual std::string getArg() {
        return arg;
    }

    virtual T getValue() { 
        return value;
    }
};

class SnakeHandler {
private:
    PyObject *m_pName, *m_pModule, *m_pFunc;
    PyObject *m_pArgs, *m_pValue;
    std::mutex m_mutex;
    
    std::string m_filename;
public:
    SnakeHandler(std::string filename);
    ~SnakeHandler();

    bool callFunction(const std::string &function, const std::vector<std::unique_ptr<base_type>> &arguments, bool &finishedFlag);
};