#include "SnakeHandler.h"
#include "../../QLogger.h"
#include "../../Display/ErrorHandler.h"
#include <iostream>
#include <memory>


SnakeHandler::SnakeHandler(std::string filename) : m_filename(filename) {

    m_pName = PyUnicode_DecodeFSDefault(filename.c_str());

    m_pModule = PyImport_Import(m_pName);
    Py_DECREF(m_pName);

    if (!m_pModule) {
        QLogger::GetInstance().Log(LOGLEVEL::ERR, "Failed to load python module: ", m_filename);
        PyErr_Print();
    } else {
        QLogger::GetInstance().Log(LOGLEVEL::INFO, "Loaded python module: ", m_filename);
    }
}

SnakeHandler::~SnakeHandler() {
    if (Py_FinalizeEx() < 0) {
        QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename, "Python handler failed to exit successfully");
    }
}

bool SnakeHandler::callFunction(std::string &function, std::vector<std::unique_ptr<base_type>> &arguments) {
    /* pFunc is a new reference */
    m_pFunc = PyObject_GetAttrString(m_pModule, function.c_str());

    if (m_pFunc && PyCallable_Check(m_pFunc)) {
        m_pArgs = PyTuple_New(arguments.size());

        // Dynamic type conversion, a little scary
        // Here we rebuild the class based on the data type
        for (auto &arg : arguments) {
            if (arg->getType() == 'd') {
                d_type<int> *t_ptr = reinterpret_cast<d_type<int>*>(&*arg);
                m_pValue = PyLong_FromLong(t_ptr->getValue());
            } else if (arg->getType() == 's') {
                d_type<const char*> *t_ptr = reinterpret_cast<d_type<const char*>*>(&*arg);
                m_pValue = PyUnicode_FromString(t_ptr->getValue());
            } else if (arg->getType() == 'f') {
                d_type<double> *t_ptr = reinterpret_cast<d_type<double>*>(&*arg);
                m_pValue = PyFloat_FromDouble(t_ptr->getValue());
            }

            if (!m_pValue) {
                Py_DECREF(m_pArgs);
                QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename, "Can't convert argument of type: ", arg->getType() );
                return 0;
            }

            /* pValue reference stolen here: */
            PyTuple_SetItem(m_pArgs, arg->index, m_pValue);
        }

        m_pValue = PyObject_CallObject(m_pFunc, m_pArgs);
        Py_DECREF(m_pArgs);

        if (m_pValue && PyLong_AsLong(m_pValue) >= 0) {
            QLogger::GetInstance().Log(LOGLEVEL::INFO, m_filename, "Function call completed successfully for: ", function, "with result: ", PyLong_AsLong(m_pValue));
            Py_DECREF(m_pValue);
            Py_XDECREF(m_pFunc);
            return 1;
        }
        else {
            Py_DECREF(m_pFunc);
            PyErr_Print();
            QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename, "Function call failed for: ", function);
            ErrorHandler::GetInstance().setError("Docker function call execution failed, check that contains are running...");
            return 0;
        }
    }

    QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename, "Can't find function: ", function);

    if (PyErr_Occurred()) {
        PyErr_Print();
    }
    
    Py_DECREF(m_pFunc);
    return 0;
}