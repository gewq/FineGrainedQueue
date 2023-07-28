#include "ListIsEmpty_Exception.h"



ListIsEmpty_Exception::ListIsEmpty_Exception() : std::exception()
{
}



const char* ListIsEmpty_Exception::what() const noexcept
{
	return "Error: list is empty";
}