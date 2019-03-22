#pragma once
#include "js.object.h"

/**
 * @brief The js namespace
 * 
 */
namespace js {

/**
 * @brief IndexedProperty
 * 
 */
class IndexedProperty {
  protected:
    value_ref_t this_;
    value_ref_t index_;

  public:
    IndexedProperty() {}
    IndexedProperty(value_ref_t target, value_ref_t index)
        : this_(target), index_(index) {}
    void operator=(const value_ref_t &val) {
		JSERR_TO_EXCEPTION(JsSetIndexedProperty(this_, index_, val));
    }
    operator value_ref_t() const {
        value_ref_t out;
        JsGetIndexedProperty(this_, index_, out.addr());
        return out;
    }
};

/**
 * @brief The accessor of JsArray
 * 
 */
class array_accessor_ : public object_accessor_<_Array> {
  public:
  /**
   * @brief Get the length of the array
   * 
   * @return uint32_t 
   */
    uint32_t Length() {
        propid_t id = PropertyId("length");
        if (!id)
            return 0;
        value_ref_t length = GetProperty(id);
        if (!length.is<_Number>())
            return 0;
        return GetAs<Int>(length);
    }
    /**
     * @brief Get the Item array
     * 
     * @param index The index of the array to get.
     * @return value_ref_t The js value.
     */
    value_ref_t GetItem(Int index) {
        auto i = Just<Int>(index);
        value_ref_t out;
        JsGetIndexedProperty(get(), i, out.addr());
        return out;
    }
    /**
     * @brief Set the Item object
     * 
     * @param index 
     * @param value 
     * @return true 
     * @return false 
     */
    bool SetItem(Int index, value_ref_t value) {
        auto i = Just<Int>(index);
        auto err = JsSetIndexedProperty(get(), i, value);
        return err == JsNoError;
    }
    /**
     * @brief 
     * 
     * @param index 
     * @return IndexedProperty 
     */
    IndexedProperty operator[](Int index) {
        return IndexedProperty(*this, Just<Int>(index));
    }

  public:
    /**
     * @brief Create a new array with special length.
     * 
     * @param len 
     * @return value_ref_t 
     */
    static value_ref_t Create(uint32_t len) {
        value_ref_t out;
        JsCreateArray(len, out.addr());
        return out;
    }
};

using Array = base_value_<array_accessor_>;

}; // namespace js
