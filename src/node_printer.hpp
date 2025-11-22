#ifndef NODE_PRINTER_HPP
#define NODE_PRINTER_HPP
#include "macros.hh"


MY_NODE_MODULE_CALLBACK(SayMyName);
MY_NODE_MODULE_CALLBACK(getPrinters);
MY_NODE_MODULE_CALLBACK(getDefaultPrinterName);
MY_NODE_MODULE_CALLBACK(printDirect);
MY_NODE_MODULE_CALLBACK(getPrinter);
MY_NODE_MODULE_CALLBACK(printFile);
MY_NODE_MODULE_CALLBACK(getSupportedPrintFormats);



/**
 * @brief A base class template for managing a pointer to a value of type Type.
 * 
 * This class provides basic functionality for managing a pointer to a value,
 * including construction, destruction, and access methods.
 * 
 * @tparam Type The type of the value being managed.
 */

template<typename Type>
class MemValueBase
{
public:
    /**
     * @brief Default constructor. Initializes the pointer to NULL.
     */
    MemValueBase(): _value(NULL) {}

    /**
     * @brief Virtual destructor. The allocated memory will be deallocated.
     */
    virtual ~MemValueBase() {}
    /**
     * @brief Gets the pointer to the value.
     * 
     * @return A pointer to the value of type Type.
     */
    Type * get() {return _value; }
    /**
     * @brief Overloaded arrow operator to access the value.
     * 
     * @return A pointer to the value of type Type.
     */
    Type * operator ->() { return &_value; }
    /**
     * @brief Conversion operator to check if the pointer is not NULL.
     * 
     * @return True if the pointer is not NULL, false otherwise.
     */
    operator bool() const { return (_value != NULL); }
protected:
    Type *_value;  /**< Pointer to the value of type Type. */
    /**
     * @brief Virtual method to free the allocated memory.
     * 
     * This method can be overridden by derived classes to provide custom
     * memory deallocation logic.
     */
    virtual void free() {};
};

/**
 * Try to extract a string or buffer from a N-API value.
 * @param value - source N-API value
 * @param oData - destination data
 * @return TRUE if value is a string or buffer, FALSE otherwise
 */

bool getStringOrBufferFromNapiValue(const Napi::Value& value, std::string& oData);


#endif
