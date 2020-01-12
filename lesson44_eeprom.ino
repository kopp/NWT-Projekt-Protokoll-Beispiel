#include <EEPROM.h>


/* Kommandozeileninterpreter, der auf dem EEPROM lesen und schreiben kann.
   Bisher ist ein kleiner Buffer implementiert, an den man nur anhaengen kann.

   Verwendung:
   - Auf Arduino programmieren
   - Serial anschliessen und dort Kommandos eingeben -- `help` um alle
     Kommandos angezeigt zu bekommen.
   - Die Werte sollten nach einem Reset/Strom weg/hin immer noch vorhanden sein!
 */



String command;
const long time_to_type_command_s = 10;

struct EEPROM_buffer {
    static constexpr int OFFSET_DATA{2};
    const int START_ADDRESS;
    const int ADDRESS_LENGTH_LSB;
    const int& ADDRESS_LENGTH_MSB;
    const int LENGTH;

    EEPROM_buffer(const int start_address, const int size_bytes)
      : START_ADDRESS(start_address)
      , ADDRESS_LENGTH_LSB(start_address + 1)
      , ADDRESS_LENGTH_MSB(START_ADDRESS)
      , LENGTH(size_bytes)
    {
    }

    void clear() {
      EEPROM.write(ADDRESS_LENGTH_MSB, 0);
      EEPROM.write(ADDRESS_LENGTH_LSB, 0);
    }

    int size() const {
      unsigned char length_msb = EEPROM.read(ADDRESS_LENGTH_MSB);
      unsigned char length_lsb = EEPROM.read(ADDRESS_LENGTH_LSB);
      int length = (length_msb << 8) + length_lsb;
      return length;
    }

    bool push(unsigned char value) {
      if (size() >= LENGTH) {
        return false;
      } else {
        int address = next_writing_address();
        EEPROM.write(address, value);
        increment_size();
        return true;
      }
    }

    void inspect_raw(int number_of_bytes) const {
      Serial.print("Start address: ");
      Serial.println(START_ADDRESS);
      for (int i = 0; i < number_of_bytes; ++i) {
        Serial.print(EEPROM.read(START_ADDRESS + i), HEX);
        Serial.print(" ");
      }
      Serial.println("----");
    }

    void print() const {
      Serial.print(size());
      Serial.print(" elements: [");
      for (int i = 0; i < size(); ++i) {
        Serial.print(EEPROM.read(START_ADDRESS + OFFSET_DATA + i));
        Serial.print(", ");
      }
      Serial.println("]");
    }

  private:

    int next_writing_address() {
      return size() + OFFSET_DATA; // 2 bytes reserved for address
    }
    void increment_size() {
      unsigned char length_lsb = EEPROM.read(ADDRESS_LENGTH_LSB);
      length_lsb += 1;
      EEPROM.write(ADDRESS_LENGTH_LSB, length_lsb);

      if (length_lsb == 0) { // overflow occured
        unsigned char length_msb = EEPROM.read(ADDRESS_LENGTH_MSB);
        length_msb += 1;
        EEPROM.write(ADDRESS_LENGTH_MSB, length_msb);
      }

    }

};


EEPROM_buffer data(0, 50);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

}



void loop() {

  Serial.println("Ready to accept a command.");

  Serial.setTimeout(time_to_type_command_s * 1000);
  command = Serial.readStringUntil('\n');

  if (command.length() > 0) {
    Serial.print(" > ");
    Serial.println(command);
  }


  if (command.equals("help")) {
    Serial.println("Known commands: ");
    Serial.println("  - help");
    Serial.println("  - append <value>  -- write given value to eeprom");
    Serial.println("  - length          -- show number of values in eeprom");
    Serial.println("  - print           -- print content");
    Serial.println("  - clear           -- clear data in eeprom");
    Serial.println("  - inspect <bytes> -- show given number of bytes in eeprom");
  } else if (command.equals("clear")) {
    data.clear();
    Serial.println("-- > Cleared EEPROM");
  } else if (command.equals("length")) {
    Serial.print("--> ");
    Serial.println(data.size());
  } else if (command.equals("print")) {
    Serial.print("--> ");
    data.print();
  } else if (command.startsWith("inspect ")) {
    String value_str = command.substring(8);
    int value = value_str.toInt();
    data.inspect_raw(value);
  } else if (command.startsWith("append ")) {
    String value_str = command.substring(7);
    int value_int = value_str.toInt();
    unsigned char value = value_int & 0xFF;
    bool worked = data.push(value);
    if (worked) {
      Serial.print("-- > Appended ");
      Serial.println(value);
    } else {
      Serial.println("-- > Unable to append value!");
    }
  } else if (command.length() != 0) {
    Serial.println("Unknown command : -(");
    Serial.println("You can type help to get a list of available commands.");
  } else {
    // no command given
  }


  //delay(5000);  // because the next part is a loop:

}
