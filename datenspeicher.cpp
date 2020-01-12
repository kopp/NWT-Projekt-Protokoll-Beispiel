#include <cstdint>
#include <vector>
#include <cassert>
#include <cstring>
#include <iostream>

using byte = uint8_t;


struct EEPROM_Mock
{
    static constexpr int SIZE = 1024;

    byte data[SIZE];

    byte read(int index) { return data[index]; }
    void write(int address, byte value) {
        assert(0 <= address);
        assert(address < SIZE);
        data[address] = value;
    }

    template<typename Data>
    Data& put(int address, Data& value) {
        assert(0 <= address);
        assert(address + sizeof(Data) <= SIZE);
        std::memcpy(data + address, &value, sizeof(Data));
        return value;
    }

    template<typename Data>
    Data& get(int address, Data& value) {
        assert(0 <= address);
        assert(address + sizeof(Data) <= SIZE);
        std::memcpy(&value, data + address, sizeof(Data));
        return value;
    }

};


void test_EEPROM_Mock()
{
    EEPROM_Mock EEPROM;
    EEPROM.write(0, 5);
    assert(5 == EEPROM.read(0));
    EEPROM.write(100, 2);
    assert(2 == EEPROM.read(100));

    float f = 3.14;
    EEPROM.put(2, f);
    float g = 9.81;
    EEPROM.get(2, g);
    assert(f == g);
}


EEPROM_Mock EEPROM;


template<int LocationInEeprom>
class EEPROM_DeltaEncoder
{
    using ValueType = float;

    static constexpr int MAX_INDEX = 500;
    static constexpr byte CELL_EMPTY = 0xFF;
    static constexpr int BEGIN_EEPROM_ADDRESS = LocationInEeprom;

    ValueType initialValueCache;
    bool initialValueCached = false;

    //! Points one behind last used index
    int endIndexCache;
    bool endIndexCached = false;

public:

    //! store first value and write all cells to EMPTY
    void initializeWithFirstValue(float value)
    {
        EEPROM.put(BEGIN_EEPROM_ADDRESS, value);
        for (int i = 1; i <= MAX_INDEX; ++i)
        {
            EEPROM.write(addressForIndex(i), CELL_EMPTY);
        }
    }

    //! Add new value; stores that value Delta-encoded
    //! Return whether it was possible to store the value.
    bool addValue(ValueType value)
    {
        int index_first_empty_cell = getNumberOfValues();
        if (index_first_empty_cell > MAX_INDEX)
        {
            return false;
        }
        else
        {
            byte delta = computeDeltaOfInitialValueWith(value);
            EEPROM.write(addressForIndex(index_first_empty_cell), delta);
            storeLastUsedCellIndex(index_first_empty_cell);
            return true;
        }
    }

    //! Return number of values stored
    int getNumberOfValues()
    {
        if (not endIndexCached) {
            for (int i = 1; i <= MAX_INDEX; ++i) {
                if (CELL_EMPTY == EEPROM.read(addressForIndex(i)))
                {
                    endIndexCache = i;
                    endIndexCached = true;
                    break;
                }
            }
        }
        if (not endIndexCached) {
            endIndexCached = true;
            endIndexCache = MAX_INDEX + 1;
        }
        return endIndexCache;
    }

    //! Return the value with the given index
    //! Index 0 is initial value.
    //! You will get total garbage if you use an index outside of [0, getNumberOfValues()[.
    ValueType getValue(int index)
    {
        if (index == 0) {
            return getInitialValue();
        }
        else {
            byte delta = EEPROM.read(addressForIndex(index));
            ValueType value = computeValueFromDeltaWithInitialValue(delta);
            return value;
        }
    }

private:
    //! return initial value -- may buffer result
    ValueType getInitialValue()
    {
        if (not initialValueCached)
        {
            EEPROM.get(BEGIN_EEPROM_ADDRESS, initialValueCache);
            initialValueCached = true;
        }
        return initialValueCache;
    }

    static int addressForIndex(int index)
    {
        return BEGIN_EEPROM_ADDRESS + sizeof(ValueType) + index - 1;
    }

    void storeLastUsedCellIndex(int index)
    {
        endIndexCached = true;
        endIndexCache = index + 1;
    }


    //! Compute the delta encoding to store.
    //! This particular function assumes, that the values to encode are rather close to the initial value; about +/- 10.
    byte computeDeltaOfInitialValueWith(float value)
    {
        float initial = getInitialValue();
        float delta = initial - value;

        float delta_shifted = delta * 20;
        int8_t delta_encoded = static_cast<int8_t>(delta_shifted);
        return static_cast<byte>(delta_encoded);
    }

    ValueType computeValueFromDeltaWithInitialValue(byte delta)
    {
        int8_t delta_encoded = static_cast<int8_t>(delta);
        float delta_unshifted = static_cast<float>(delta_encoded) / 20;

        float initial = getInitialValue();
        float value = initial - delta_unshifted;
        return value;
    }

};


void test_EEPROM_DeltaEncoder()
{
    EEPROM_DeltaEncoder<0> temperature;
    temperature.initializeWithFirstValue(23);
    float test = temperature.getValue(0);
    assert(23 == test);

    int num = temperature.getNumberOfValues();
    assert(1 == num);

    bool worked = temperature.addValue(24);
    assert(worked);
    num = temperature.getNumberOfValues();
    assert(2 == num);

    test = temperature.getValue(1);
    assert(24 == test);

    worked = temperature.addValue(25); assert(worked);
    test = temperature.getValue(2);
    assert(25 == test);

    worked = temperature.addValue(26); assert(worked);
    worked = temperature.addValue(27); assert(worked);
    worked = temperature.addValue(28); assert(worked);
    num = temperature.getNumberOfValues();
    assert(6 == num);

}


int main()
{
    test_EEPROM_Mock();
    test_EEPROM_DeltaEncoder();
}
