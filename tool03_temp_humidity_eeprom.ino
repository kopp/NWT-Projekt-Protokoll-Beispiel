#include <EEPROM.h>

#include "DHT_U.h"
#include "DHT.h"



const int LED_YELLOW = 7;
const int LED_BLUE = 6;
const int DHT11_PIN = 8;


/* testing
const long WAIT_DURATION_FOR_CONNECTION_S = 10;
const int NUMBER_OF_INITIAL_MESSAGES = 5;
const long DELAY_BETWEEN_MEASUREMENTS_S = 10;
const long DELAY_BEFORE_MEASUREMENT_MODE_S = 3;
*/

/* productive */
const long WAIT_DURATION_FOR_CONNECTION_S = 40;
const int NUMBER_OF_INITIAL_MESSAGES = 20;
const long DELAY_BETWEEN_MEASUREMENTS_S = 300;
const long DELAY_BEFORE_MEASUREMENT_MODE_S = 15;



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


DHT dht(DHT11_PIN, DHT11);

String command;

EEPROM_DeltaEncoder<0> temperature_log;
EEPROM_DeltaEncoder<512> humidity_log;


//! Wait for serial command.
//! Return, whether a command was issued.
bool wait_for_serial_command()
{
  Serial.setTimeout(WAIT_DURATION_FOR_CONNECTION_S * 1000 / NUMBER_OF_INITIAL_MESSAGES);

  for (int i = 0; i < NUMBER_OF_INITIAL_MESSAGES; ++i) {
    Serial.print("Waiting for command (");
    Serial.print(i + 1);
    Serial.print(" / ");
    Serial.print(NUMBER_OF_INITIAL_MESSAGES);
    Serial.println(")!");

    command = Serial.readStringUntil('\n');
    if (command.length() > 0) {
      Serial.print(" > ");
      Serial.println(command);
      return true;
    }
  }

  if (Serial) {
    Serial.println("Did not receive a command -- going into measurement mode!");
    delay(DELAY_BEFORE_MEASUREMENT_MODE_S * 2 / 3);
    Serial.println("now for real!");
    delay(DELAY_BEFORE_MEASUREMENT_MODE_S / 3);
  }

  return false;
}


int get_length_of_shortest_log()
{
  int length_temperature = temperature_log.getNumberOfValues();
  int length_humidity = humidity_log.getNumberOfValues();
  if (length_temperature <= length_humidity) {
    return length_temperature;
  } else {
    return length_humidity;
  }
}


long measurementTimeInSForIndex(int i) {
  return WAIT_DURATION_FOR_CONNECTION_S + DELAY_BEFORE_MEASUREMENT_MODE_S +
    i * DELAY_BETWEEN_MEASUREMENTS_S;
}


//! Infinite loop waiting for commands.
//! This assumes, that a command was given.
void command_loop()
{
  Serial.setTimeout(10000);

  while (true) {
    if (command.equals("help")) {
      Serial.println("Known commands: ");
      Serial.println("  - help");
      Serial.println("  - print all          -- print all stored content");
      Serial.println("  - print temperature  -- print all stored temperatures");
      Serial.println("  - print humidity     -- print all stored humidities");
      Serial.println("  - length             -- print number of data entries");
      Serial.println("  - clear              -- clear data in eeprom");
    } else if (command.equals("print all")) {
      for (int i = 0; i < get_length_of_shortest_log(); ++i) {
        Serial.print(measurementTimeInSForIndex(i));
        Serial.print(" ; ");
        Serial.print(temperature_log.getValue(i));
        Serial.print(" ; ");
        Serial.println(humidity_log.getValue(i));
      }
      Serial.println("--> done printing everything");
    } else if (command.equals("print temperature")) {
      for (int i = 0; i < temperature_log.getNumberOfValues(); ++i) {
        Serial.print(measurementTimeInSForIndex(i));
        Serial.print(" ; ");
        Serial.println(temperature_log.getValue(i));
      }
      Serial.println("--> done printing temperature");
    } else if (command.equals("print humidity")) {
      for (int i = 0; i < humidity_log.getNumberOfValues(); ++i) {
        Serial.print(measurementTimeInSForIndex(i));
        Serial.print(" ; ");
        Serial.println(humidity_log.getValue(i));
      }
      Serial.println("--> done printing temperature");
    } else if (command.equals("clear")) {
      temperature_log.initializeWithFirstValue(0);
      humidity_log.initializeWithFirstValue(0);
      Serial.println("--> all data cleared");
    } else if (command.equals("length")) {
      Serial.print("--> length: ");
      Serial.println(get_length_of_shortest_log());
    } else if (command.length() != 0) {
      Serial.println("Unknown command : -(");
      Serial.println("You can type help to get a list of available commands.");
    } else {
      // no command given
      Serial.println("Waiting for command...");
    }

    command = Serial.readStringUntil('\n');
  }
}


void signal_measurement_error() {
  digitalWrite(LED_YELLOW, HIGH);
  delay(100);
  digitalWrite(LED_YELLOW, LOW);
  delay(100);
  digitalWrite(LED_YELLOW, HIGH);
  delay(100);
  digitalWrite(LED_YELLOW, LOW);
  delay(100);
  digitalWrite(LED_YELLOW, HIGH);
  delay(100);
  digitalWrite(LED_YELLOW, LOW);
  delay(100);
  digitalWrite(LED_YELLOW, HIGH);
  delay(100);
  digitalWrite(LED_YELLOW, LOW);
}


void signal_storage_exhausted() {
  digitalWrite(LED_BLUE, HIGH);
  delay(100);
  digitalWrite(LED_BLUE, LOW);
  delay(100);
}


#define assert(X) do { if (X) { Serial.println(#X " worked"); } else { Serial.println(#X " FAILED"); } } while(0)


void test_EEPROM_DeltaEncoder()
{
  assert(true);
  EEPROM_DeltaEncoder<0> temperature;
  temperature.initializeWithFirstValue(23);
  float test = temperature.getValue(0);
  Serial.println(test);
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


void setup_for_tests() // rename to setup() to run tests
{
  Serial.begin(9600);
  while (! Serial) {
    ;
  }
  test_EEPROM_DeltaEncoder();
  Serial.println("....");
  test_EEPROM_DeltaEncoder();
  Serial.println("..done..");
  while (true) {
    ;
  }
}


unsigned long timestamp_last_measurement_ms = 0;

void setup() {
  Serial.begin(9600);

  dht.begin();

  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  // Wait for serial command
  digitalWrite(LED_BLUE, HIGH);


  bool command_issued = wait_for_serial_command();

  if (command_issued) {
    command_loop();
  }

  // otherwise, continue with measurement
  digitalWrite(LED_BLUE, LOW);

  digitalWrite(LED_YELLOW, HIGH);
  delay(100);

  float relative_humidity = dht.readHumidity();
  float temperature_in_C = dht.readTemperature();

  if (isnan(relative_humidity) || isnan(temperature_in_C)) {
    while (true) {
      signal_measurement_error();
    }
  }
  else {
    temperature_log.initializeWithFirstValue(temperature_in_C);
    humidity_log.initializeWithFirstValue(relative_humidity);
    timestamp_last_measurement_ms = millis();
    if (Serial) {
      Serial.print("measurement initialized to ");
      Serial.print(temperature_in_C);
      Serial.print(" (");
      Serial.print(temperature_log.getValue(0));
      Serial.print("), ");
      Serial.print(relative_humidity);
      Serial.print(" (");
      Serial.print(humidity_log.getValue(0));
      Serial.println(").");
    }
  }

  delay(100);
  digitalWrite(LED_YELLOW, LOW);

}



void loop() {

  // compute, when the next measurement is due
  unsigned long timestemp_next_measurement_ms =
    timestamp_last_measurement_ms + DELAY_BETWEEN_MEASUREMENTS_S * 1000;
  unsigned long timestamp_now_ms = millis();

  // wait for that time
  // Note: here the controller could go to sleep as well!
  delay(timestemp_next_measurement_ms - timestamp_now_ms);


  digitalWrite(LED_YELLOW, HIGH);

  float relative_humidity = dht.readHumidity();
  float temperature_in_C = dht.readTemperature();
  timestamp_last_measurement_ms = millis();

  if (isnan(relative_humidity) || isnan(temperature_in_C)) {
    signal_measurement_error();
    return;
  }
  else {
    bool writing_worked = temperature_log.addValue(temperature_in_C);
    writing_worked = writing_worked and humidity_log.addValue(relative_humidity);

    if (writing_worked)
    {
      if (Serial) {
        Serial.print("added measurement ");
        Serial.print(temperature_in_C);
        Serial.print(", ");
        Serial.print(relative_humidity);
        Serial.println();
      }

      delay(600); // some delay so that one can see the LED
      digitalWrite(LED_YELLOW, LOW);
      return;
    }
    else
    {
      while (true) {
        signal_storage_exhausted();
      }
    }

  }

}
