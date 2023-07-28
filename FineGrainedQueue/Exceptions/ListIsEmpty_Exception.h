/**
\file ListIsEmpty_Exception.h
\brief Класс ListIsEmpty_Exception - класс-обработчик исключения "Список пуст"
*/

#pragma once

#include <string>
#include <exception>

class ListIsEmpty_Exception : public std::exception {
  public:
    ListIsEmpty_Exception();

    virtual const char* what() const noexcept override;
};