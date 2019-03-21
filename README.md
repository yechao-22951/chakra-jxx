# chakra-jxx
ChakraCore C++ binding helper

## Plan

1. Wrap most JSRT APIs.
2. Create a framework for nodejs-like system, including ioloop, module system, native module support and utils.  
   Combine ASIO and Chakra.
3. Implements some Low-Level APIs for FileSystem, Network, and Windows Reisttry(maybe). At the same time, an simple
   privileges system will be build ( native-level sandbox ).
4. Using javascript to implement the most nodejs traits.
5. Import co-routine supporting.
6. ...

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

- Simple Type System : JxxClass, I use it to defense Type-Confusion and implement inherition.

    - `JxxClassTemplateNE<This_,Parents_...>`  
        c++ class tempalte without js-exports
    - `JxxClassTemplate<<This_,Parents_...>`  
        c++ class tempalte with js-exports
    - `JxxOf<T>`                                  
        generic wrapper for any c++ struct

    ```C++
    //
    // JxxClassTemplate
    //
    class Console : public JxxClassTemplate<Console, IJxxObject> 
    {
    public:
        DEFINE_CLASS_NAME("org.mode.buildin.console");
    public:
        value_ref_t log(call_info_t&, more_list__ args) { 
            return JS_INVALID_REFERENCE;
        }
    public:
        // export log function
        JXX_EXPORT_METHOD(Console, log);            
    };

    //
    // JxxOf
    //

    // directly wrap asio::ip::tcp::socket
    using tcp_socket_t = JxxOf<tcp::socket>;        

    _as_the<_NotCare> AsyncRead(_as_the<_Object> socket, _as_the<_BufferLike> buffer)
    {
        Durable<ExtObject> jsSocket(socket);
        if (!jsSocket) return Undefined();

        // TryGetAs will check c++ object type.
        JxxObjectPtr<tcp_socket_t> cxxSocket = jsSocket->TryGetAs<tcp_socket_t>();
        if (!cxxSocket) return Undefined();

        Durable<value_ref_t> jsBuffer = buffer;
        content_t cxxBuffer = GetContent(jsBuffer.get());
        cxxSocket->async_read_some(asio::buffer(cxxBuffer.data, cxxBuffer.size),
            [jsSocket, jsBuffer](std::error_code ec, size_t read)
            {
                Function callback = (value_ref_t)jsSocket->GetProperty("onData");
                if (!callback) return;
                callback.Call(
                    jsSocket.get(),
                    just_is_(ec.value()),
                    just_is_(read),
                    jsBuffer.get()
                );
            });
        return Undefined();
    };
    ```
