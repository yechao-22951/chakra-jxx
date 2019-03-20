# chakra-jxx
ChakraCore C++ binding helper

## jsrt wrapper

All js.* files are jsrt function wrapper, all things in namespace `js`.

- `js::value_ref_t`
  
  value_ref_t is an object try to instand of JsValueRef, JsContextRef, JsPropertyIdRef.   
  Because value_ref_t is not a C++ primitive type, it will not 

- `js::_as_the<_TYPE_MASK>`

    `_as_the<>` is use to check the type of `JsValueRef`. It accept a type mask.  
    If `JsValueRef` is `JS_INVALID_REFERENCE`, the type mask is `_Optional`.  
    If `JsValueRef` is not a real javascript value, it will be `_Invalid`.  

    `_as_the<>` can be construct from `param_t`. They are used to implement **Magic** native function.
    **Magic** native function is normal c/c++ function, just like this:
    ```C++
    value_ref_t connect( 
        call_info_t & info,                     // the first argument must be call_info_t.
        _as_the<_Object> socket,                // this parameter must be a javascript Object.
        _as_the<_String> remote,                // this parameter must be a javascript string(not String Object).
        _as_the<_Number|_String> port           // this parameter must be a javascript Number or string.
        _as_the<_Object|_Optional> options      // this parameter is optional, if it is set, it must be Object.
    );

    value_ref_t printf( 
        call_info_t & info,                     // the first argument must be call_info_t.
        _as_the<_String_> format,               // this parameter must be a javascript string.
        more_list__ more                        // this parameter more_list__ is an array of rest arguments.
    );
    ```