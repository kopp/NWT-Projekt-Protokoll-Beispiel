/* Teste UART interface:
    Nutzer hat nach Start des Arduinos eine bestimmte Zeit, um Kommunikation
    herzustellen.  Wenn er das nicht tut, dann geht ein anderes Programm los.
*/


const int LED_YELLOW = 7;
const int LED_BLUE = 6;

const long WAIT_DURATION_FOR_CONNECTION_S = 40;
const int NUMBER_OF_INITIAL_MESSAGES = 20;

String command;


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
    Serial.println("Did not receive a command -- going to measurement mode.");
  }

  return false;
}


//! Infinite loop waiting for commands.
//! This assumes, that a command was given.
void command_loop()
{
  Serial.setTimeout(10000); // remind user every 10 s to send a command
  
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
      // TODO
      Serial.println("--> printed everything");
    } else if (command.equals("print temperature")) {
      // TODO
      Serial.println("--> printed temperature");
    } else if (command.equals("print humidity")) {
      // TODO
      Serial.println("--> printed humidity");
    } else if (command.equals("clear")) {
      // TODO data.clear();
      Serial.println("--> Cleared all data");
    } else if (command.equals("length")) {
      Serial.print("--> length: ");
      // TODO Serial.println(data.size());
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


void setup() {
  Serial.begin(9600);

  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  // Wait for serial command
  digitalWrite(LED_BLUE, HIGH);


  bool command_issued = wait_for_serial_command();

  if (command_issued) {
    command_loop();
  }

  // otherwise, continue to loop()
  digitalWrite(LED_BLUE, LOW);
}




void loop() {

  // bullshit program
  digitalWrite(LED_YELLOW, HIGH);
  delay(1000);
  digitalWrite(LED_YELLOW, LOW);
  delay(750);

}
