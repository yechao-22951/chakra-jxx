#pragma once

#include "jxx.class.h"

#include <map>
#include <unordered_map>

namespace js {

    class ValueCache {
    public:
        enum {
            String,
            Prototype,
            Symbol,
            _Max_,
        };
    public:
        using value_map_t = std::unordered_map<String, Durable<Value>>;
        using string_map_t = std::unordered_map<String, Durable<Value>>;
        using proto_map_t = std::unordered_map<String, Durable<Object>>;
        using symbol_map_t = std::unordered_map<String, Durable<Symbol>>;
        using propid_map_t = std::unordered_map<String, Durable<value_ref_t>>;

        string_map_t strings_;
        proto_map_t protos_;
        symbol_map_t symbols_;
        propid_map_t propids_;


        value_map_t value_maps_[_Max_];
    public:
        Value get_string(const String& pstr) {
            auto it = strings_.find(pstr);
            if (it != strings_.end())
                return it->second;
            Value js_str = Just(pstr);
            if (!js_str)
                return js_str;
            strings_[pstr] = (js_str);
            return js_str;
        }
        Value get_string(CharPtr pstr, size_t len) {
            if( !len ) len = strlen(pstr);
            return get_string(String(pstr,len));
        }

        value_ref_t get_propid(const String & pstr) {
            auto it = propids_.find(pstr);
            if (it != propids_.end())
                return it->second;
            value_ref_t propid = PropertyId(pstr);
            if (!propid)
                return propid;
            propids_[pstr] = (propid);
            return propid;
        }
        value_ref_t get_propid(const CharPtr pstr) {
            return get_string(String(pstr));
        }


        // proto
        Object get_proto(const String & pstr) {
            auto it = protos_.find(pstr);
            if (it != protos_.end())
                return it->second;
            return Object();
        }
        void put_proto(String && pstr, Object proto) {
            if (!proto) {
                protos_.erase(pstr);
            }
            else {
                protos_[std::move(pstr)] = Durable(proto);
            }
        }

        // symbol
        Symbol get_symbol(const String & token) {
            auto it = symbols_.find(token);
            if (it != symbols_.end())
                return it->second;
            Value pstr = get_string(token);
            if (!pstr)
                return value_ref_t();
            Symbol symbol;
            auto err = JsCreateSymbol(pstr, symbol.addr());
            if (err)
                return symbol;
            symbols_[token] = symbol;
            return symbol;
        }
        Symbol get_symbol(CharPtr token, size_t len) { 
            len = len ? len : strlen(token);
            return get_symbol(String(token, len));
        }

        void clear() {
            strings_.clear();
            protos_.clear();
            symbols_.clear();
            // propids_.clear();
        }
    };

}; // namespace js