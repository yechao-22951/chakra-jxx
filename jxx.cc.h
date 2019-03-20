#pragma once

#include "jxx.class.h"

#include <map>
#include <unordered_map>

namespace js {

    class cache_t {
    public:
        using string_map_t = std::unordered_map<String, Durable<Value>>;
        using proto_map_t = std::unordered_map<String, Durable<Object>>;
        using symbol_map_t = std::unordered_map<String, Durable<Symbol>>;
        using propid_map_t = std::unordered_map<String, Durable<value_ref_t>>;


        string_map_t strings_;
        proto_map_t protos_;
        symbol_map_t symbols_;
        propid_map_t propids_;

    public:
        Value get_string(const String& str) {
            auto it = strings_.find(str);
            if (it != strings_.end())
                return it->second;
            Value js_str = just_is_(str);
            if (!js_str)
                return js_str;
            strings_[str] = (js_str);
            return js_str;
        }
        Value get_string(const CharPtr pstr) {
            String str(pstr);
            return get_string(str);
        }

        value_ref_t get_propid(const String & str) {
            auto it = propids_.find(str);
            if (it != propids_.end())
                return it->second;
            value_ref_t propid = PropertyId(str);
            if (!propid)
                return propid;
            propids_[str] = (propid);
            return propid;
        }
        value_ref_t get_propid(const CharPtr pstr) {
            String str(pstr);
            return get_string(std::move(str));
        }


        // proto
        Object get_proto(const String & str) {
            auto it = protos_.find(str);
            if (it != protos_.end())
                return it->second;
            return Object();
        }
        void put_proto(String && str, Object proto) {
            if (!proto) {
                protos_.erase(str);
            }
            else {
                protos_[std::move(str)] = Durable(proto);
            }
        }

        // symbol
        Symbol get_symbol(const String & token) {
            auto it = symbols_.find(token);
            if (it != symbols_.end())
                return it->second;
            Value str = get_string(token);
            if (!str)
                return value_ref_t();
            Symbol symbol;
            auto err = JsCreateSymbol(str, symbol.addr());
            if (err)
                return symbol;
            symbols_[token] = symbol;
            return symbol;
        }
        Symbol get_symbol(CharPtr token) { return get_symbol(String(token)); }

        void clear() {
            strings_.clear();
            protos_.clear();
            symbols_.clear();
            // propids_.clear();
        }
    };

}; // namespace js