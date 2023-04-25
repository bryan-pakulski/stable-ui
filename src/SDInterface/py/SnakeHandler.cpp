#include "SnakeHandler.h"
#include "../../QLogger.h"
#include "../../Display/ErrorHandler.h"
#include "../../Helpers/States.h"
#include "boolobject.h"
#include "object.h"
#include <iostream>
#include <memory>

SnakeHandler::SnakeHandler(std::string filename) : m_filename(filename) {
  // Register threaded state with GIL
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  m_pName = PyUnicode_DecodeFSDefault(filename.c_str());

  m_pModule = PyImport_Import(m_pName);
  Py_DECREF(m_pName);
  PyGILState_Release(gstate);

  if (!m_pModule) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "SnakeHandler::SnakeHandler failed to load python module: ", m_filename);
    PyErr_Print();
  } else {
    QLogger::GetInstance().Log(LOGLEVEL::INFO, "SnakeHandler::SnakeHandler loaded python module: ", m_filename);
  }
}

SnakeHandler::~SnakeHandler() {}

bool SnakeHandler::asyncCall(const std::string function, std::shared_ptr<PyArgs> arguments, int &state) {
  // Register threaded state with GIL
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  PyObject *l_pName = PyUnicode_DecodeFSDefault(m_filename.c_str());
  PyObject *l_pModule = PyImport_Import(l_pName);
  Py_DECREF(l_pName);

  if (!l_pModule) {
    QLogger::GetInstance().Log(LOGLEVEL::ERR, "SnakeHandler::asyncCall failed to load python module: ", m_filename);
    PyErr_Print();
  }

  /* pFunc is a new reference */
  PyObject *l_pFunc = PyObject_GetAttrString(l_pModule, function.c_str());

  if (l_pFunc && PyCallable_Check(l_pFunc)) {

    // Attempt to load arguments
    PyObject *l_pArgs = PyTuple_New(arguments->size());
    if (popArguments(arguments, l_pArgs) != 0) {
      PyGILState_Release(gstate);
      return EXECUTION_STATE::FAILED;
    }

    // Call function with arguments
    PyObject *l_pValue = PyObject_CallObject(l_pFunc, l_pArgs);
    Py_DECREF(l_pArgs);

    if (l_pValue && PyLong_AsLong(l_pValue) == 0) {
      Py_DECREF(l_pValue);
      Py_XDECREF(l_pFunc);
      state = EXECUTION_STATE::SUCCESS;
      PyGILState_Release(gstate);
      return 1;
    } else {
      Py_DECREF(l_pFunc);
      PyErr_Print();
      QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename, "SnakeHandler::asyncCall call failed for: ", function);
      state = EXECUTION_STATE::FAILED;
      PyGILState_Release(gstate);
      return 0;
    }
  }

  QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename, "SnakeHandler::asyncCall Can't find function: ", function);

  if (PyErr_Occurred()) {
    PyErr_Print();
  } else {
    Py_DECREF(l_pFunc);
  }

  PyGILState_Release(gstate);
  state = EXECUTION_STATE::FAILED;

  return 0;
}

// Retrieve arguments from shared_pointer
// This will clear the vector
bool SnakeHandler::popArguments(std::shared_ptr<PyArgs> arguments, PyObject *pargs) {
  // Dynamic type conversion, a little scary
  // Here we rebuild the class based on the data type
  for (auto &arg : *arguments) {
    if (arg->getType() == 'd') {
      d_type<int> *t_ptr = reinterpret_cast<d_type<int> *>(&*arg);
      m_pValue = PyLong_FromLong(t_ptr->getValue());
    } else if (arg->getType() == 's') {
      d_type<std::string> *t_ptr = reinterpret_cast<d_type<std::string> *>(&*arg);
      m_pValue = PyUnicode_FromString(t_ptr->getValue().c_str());
    } else if (arg->getType() == 'f') {
      d_type<double> *t_ptr = reinterpret_cast<d_type<double> *>(&*arg);
      m_pValue = PyFloat_FromDouble(t_ptr->getValue());
    }

    if (!m_pValue) {
      Py_DECREF(pargs);
      QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename,
                                 "SnakeHandler::callFunction Can't convert argument of type: ", arg->getType());
      return EXECUTION_STATE::FAILED;
    }

    /* pValue reference stolen here: */
    PyTuple_SetItem(pargs, arg->index, m_pValue);
  }

  // Free old arguments
  arguments->clear();
  return 0;
}

bool SnakeHandler::callFunction(const std::string function, std::shared_ptr<PyArgs> arguments, int &state) {
  std::lock_guard<std::mutex> guard(m_mutex);
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  /* pFunc is a new reference */
  m_pFunc = PyObject_GetAttrString(m_pModule, function.c_str());

  if (m_pFunc && PyCallable_Check(m_pFunc)) {

    // Attempt to load arguments
    m_pArgs = PyTuple_New(arguments->size());
    if (popArguments(arguments, m_pArgs) != 0) {
      PyGILState_Release(gstate);
      return EXECUTION_STATE::FAILED;
    }

    // Call function with arguments
    m_pValue = PyObject_CallObject(m_pFunc, m_pArgs);
    Py_DECREF(m_pArgs);

    if (m_pValue && PyLong_AsLong(m_pValue) == 0) {
      Py_DECREF(m_pValue);
      Py_XDECREF(m_pFunc);
      state = EXECUTION_STATE::SUCCESS;
      // Free old arguments
      arguments->clear();
      PyGILState_Release(gstate);
      return 1;
    } else {
      Py_DECREF(m_pFunc);
      PyErr_Print();
      QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename, "SnakeHandler::callFunction call failed for: ", function);
      state = EXECUTION_STATE::FAILED;
      PyGILState_Release(gstate);
      return 0;
    }
  }

  QLogger::GetInstance().Log(LOGLEVEL::ERR, m_filename, "SnakeHandler::callFunction Can't find function: ", function);

  if (PyErr_Occurred()) {
    PyErr_Print();
  } else {
    Py_DECREF(m_pFunc);
  }

  state = EXECUTION_STATE::FAILED;
  PyGILState_Release(gstate);
  return 0;
}