/******************************
   TWITCH SWITCH v1.0
   for Teensy LC (www.pjrc.com)
   by Notes and Volts
   www.notesandvolts.com
 ******************************/

/**************************************
   ** Upload Settings **
   Board: "Teensy LC"
   USB Type: "Keyboard"
   CPU Speed: "48 Mhz"
   Keyboard Layout: "US English"
 **************************************/

#include <EEPROM.h>

#define NUMBER_BUTTONS 11 //<-Set this to the number of buttons used

// Do not touch these!
#define EEPROM_KEY 127
#define EEPROM_BUTTONS 3
#define EEPROM_MEMBERS 2
#define MAX_KEYNAME 99
#define MIN_KEYNAME 4

int heldTime = 3000; //<-Set this to the number of Milliseconds for 'Held Key'

// These parameters are changed in UTILITY EDIT MODE
byte delayPress = 0; //Delay between Key Press and Modifier Press
byte debounceTime = 0; //Debounce time for all keys


// keyNames: Array holding the Key Names that are displayed in KEY EDIT MODE
char* keyNames[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K" , "L", "M", "N",
                    "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
                    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
                    "ENTER", "ESC", "BACKSPACE", "TAB", "SPACE", "-", "=", "[", "]", "\\",
                    "NON_US_NUM", ";", "QUOTE", "TILDE", "COMMA", "PERIOD", "/", "CAPS_LOCK",
                    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
                    "PRINTSCREEN", "SCROLL_LOCK", "PAUSE", "INSERT", "HOME", "PAGE_UP", "DELETE",
                    "END", "PAGE_DOWN", "RIGHT", "LEFT", "DOWN", "UP", "NUM_LOCK",
                    "KEYPAD /", "KEYPAD *", "KEYPAD -", "KEYPAD +", "KEYPAD ENTER",
                    "KEYPAD 1", "KEYPAD 2", "KEYPAD 3", "KEYPAD 4", "KEYPAD 5", "KEYPAD 6",
                    "KEYPAD 7", "KEYPAD 8", "KEYPAD 9", "KEYPAD 0", "KEYPAD ."
                   };


// Button Class ********************************************
class Button {
    int pin;
    unsigned long prevMillis;
    bool newPress;
    bool held;
    bool cleared;

  public:
    String label;
    byte key;
    byte mod;
    byte Check();
    void Update();
    void Reset();

    Button(int pin_, String label_) {
      pin = pin_;
      pinMode(pin, INPUT_PULLUP);
      label = label_;
      key = 0;
      mod = 0;
      newPress = false;
      held = false;
      cleared = false;
      prevMillis = 0;
    }
};

void Button::Update() {
  if (mod == 0) {
    Keyboard.set_key1(key);
    Keyboard.send_now();
    delay(delayPress);
    Keyboard.set_key1(0);
    Keyboard.send_now();
  }
  else {
    Keyboard.set_modifier(mod);
    Keyboard.send_now();
    delay(delayPress);
    Keyboard.set_key1(key);
    Keyboard.send_now();
    delay(delayPress);
    Keyboard.set_modifier(0);
    Keyboard.send_now();
    Keyboard.set_key1(0);
    Keyboard.send_now();
  }
}

void Button::Reset() {
  newPress = false;
  held = false;
  cleared = true;
}

byte Button::Check() {
  // Key Reset
  if (newPress == false && held == false && cleared == true) {
    if (digitalRead(pin) == LOW) {
      return 0;
    }
    else {
      cleared = false;
      return 0;
    }
  }

  // No Key
  if (newPress == false && held == false && cleared == false) {
    if (digitalRead(pin) == LOW) {
      prevMillis = millis();
      newPress = true;
      return 0;
    }
    else return 0;
  }

  // Key Debounce
  else if (newPress == true && held == false) {
    if ((millis() - prevMillis) < debounceTime) {
      return 0;
    }
    else if (digitalRead(pin) == LOW) {
      newPress = false;
      held = true;
      return 1;
    }
    else {
      newPress = false;
      held = false;
      return 0;
    }
  }

  // Key Held Debounce
  else if (newPress == false && held == true) {
    if (digitalRead(pin) == HIGH) {
      newPress = false;
      held = false;
      return 0;
    }
    else if ((millis() - prevMillis) < heldTime) {
      return 0;
    }
    else {
      return 2;
    }
  }

  // Key Held Release
  else if (newPress == true && held == true) {
    if (digitalRead(pin) == LOW) {
      return 2;
    }
    else {
      newPress = false;
      held = false;
      return 0;
    }
  }

  //Default
  else return 0;
}
//End of Button Class **********************************

byte buttonValues[NUMBER_BUTTONS];
byte mode = 0;

// Describe Button Objects: name(pin number, label)
Button BU1(0, "STREAM");
Button BU2(1, "MIC");
Button BU3(2, "PC");
Button BU4(3, "Scene 5");
Button BU5(4, "Scene 6");
Button BU6(5, "Scene 7");
Button BU7(6, "Scene 8");
Button BU8(7, "Scene 1");
Button BU9(8, "Scene 2");
Button BU10(9, "Scene 3");
Button BU11(10, "Scene 4");
// Add all Button Objects to this Array like this: &name
Button *BUTTONS[] {&BU1, &BU2, &BU3, &BU4, &BU5, &BU6, &BU7, &BU8, &BU9, &BU10, &BU11};

void setup() {
  initRom();
  readRom();
}

void loop() {
  updateButtons();
  setMode();

  switch (mode) {
    case 0:
      normal();
      break;
    case 1:
      edit1();
      break;
    case 2:
      edit2();
      break;
  }
}

void updateButtons() {
  for (int i = 0; i < NUMBER_BUTTONS; i = i + 1) {
    buttonValues[i] = BUTTONS[i]->Check();
  }
}

byte selectButton() {
  resetButtons();
  while (1) {
    updateButtons();
    for (int i = 0; i < NUMBER_BUTTONS; i = i + 1) {
      if (buttonValues[i] == 1) return i;
    }
  }
}// End selectButton

byte setMode() {
  if (buttonValues[0] == 2 && buttonValues[1] == 2 && buttonValues[2] == 2) {
    mode = 1;
  }
  if (buttonValues[7] == 2 && buttonValues[8] == 2 && buttonValues[9] == 2 && buttonValues[10] == 2) {
    mode = 2;
  }
}// End setMode

void resetButtons() {
  for (int i = 0; i < NUMBER_BUTTONS; i = i + 1) {
    BUTTONS[i]->Reset();
  }
}// End resetButtons

void normal() {
  for (int i = 0; i < NUMBER_BUTTONS; i = i + 1) {
    if (buttonValues[i] == 1) BUTTONS[i]->Update();
  }
}// End normal

//KEY EDIT MODE
void edit1() {
  byte editButton = 0;
  byte editKey = 0;
  byte editMod = 0;
  byte editStep = 0;
  bool changed = true;
  bool edited = false;

  resetButtons();
  Keyboard.println();
  //printRom(); //TEST ONLY
  Keyboard.println();
  Keyboard.println("--- KEY EDIT MODE -----------------------------------");
  Keyboard.println("PRESS: the 'MIC' or 'PC' key to ESCAPE without saving");
  Keyboard.println("HOLD: the 'STREAM' key to SAVE changes and EXIT");
  Keyboard.println("-----------------------------------------------------");
  Keyboard.println();
  Keyboard.println("--- COMMANDS -----------------------------------------------------------");
  Keyboard.println("MODIFIER: (Scene 5 = DN)(Scene 6 = UP)");
  Keyboard.println("KEY TYPE: (Scene 1 = DN)(Scene 2 = UP)(Scene 3 = DN 10)(Scene 4 = UP 10)");
  Keyboard.println("------------------------------------------------------------------------");
  Keyboard.println();
  Keyboard.print("Select Button to EDIT: ");
  editButton = selectButton();
  editKey = BUTTONS[editButton]->key;
  editMod = BUTTONS[editButton]->mod;
  Keyboard.println(BUTTONS[editButton]->label);
  resetButtons();
  Keyboard.println();

  while (mode == 1) {
    if (changed == true) {
      Keyboard.print(BUTTONS[editButton]->label);
      Keyboard.print(": Key = ");
      Keyboard.print(keyNames[editKey - MIN_KEYNAME]);
      Keyboard.print("     Modifier = ");
      modKeyPrint(editMod);
      Keyboard.println();
      changed = false;
    }

    updateButtons();

    if (buttonValues[0] == 2) {
      mode = 0;
      resetButtons();
      if (editKey != BUTTONS[editButton]->key) {
        //Keyboard.println("SAVED KEY");
        BUTTONS[editButton]->key = editKey;
        EEPROM.write((editButton * EEPROM_MEMBERS) + EEPROM_BUTTONS, editKey);
      }
      if (editMod != BUTTONS[editButton]->mod) {
        //Keyboard.println("SAVED MOD");
        BUTTONS[editButton]->mod = editMod;
        EEPROM.write((editButton * EEPROM_MEMBERS) + EEPROM_BUTTONS + 1, editMod);
      }
      Keyboard.println();
      Keyboard.println("Changes SAVED");
      Keyboard.println("GOODBYE!");
    }
    else if (buttonValues[1] == 1 || buttonValues[2] == 1) {
      mode = 0;
      resetButtons();
      Keyboard.println();
      Keyboard.println("ESCAPE - Changes NOT SAVED");
      Keyboard.println("GOODBYE!");
    }
    else if (buttonValues[8] == 1) {
      editKey++;
      if (editKey > MAX_KEYNAME) editKey = MIN_KEYNAME;
      changed = true;
      edited == true;
    }
    else if (buttonValues[7] == 1) {
      editKey--;
      if (editKey < MIN_KEYNAME) editKey = MAX_KEYNAME;
      changed = true;

    }
    else if (buttonValues[10] == 1) {
      editKey = editKey + 10;
      if (editKey > MAX_KEYNAME) editKey = MIN_KEYNAME;
      changed = true;
    }
    else if (buttonValues[9] == 1) {
      editKey = editKey - 10;
      if (editKey < MIN_KEYNAME || editKey > MAX_KEYNAME ) editKey = MAX_KEYNAME;
      changed = true;
    }
    else if (buttonValues[4] == 1) {
      editMod++;
      if (editMod > 6) editMod = 0;
      changed = true;
    }
    else if (buttonValues[3] == 1) {
      editMod--;
      if (editMod > 128) editMod = 6;
      changed = true;
    }
  }
}// End edit1

void modKeyPrint(byte modKey) {
  switch (modKey) {
    case 0:
      Keyboard.print("NONE");
      break;
    case 1:
      Keyboard.print("CTRL");
      break;
    case 2:
      Keyboard.print("SHIFT");
      break;
    case 3:
      Keyboard.print("CTRL + SHIFT");
      break;
    case 4:
      Keyboard.print("ALT");
      break;
    case 5:
      Keyboard.print("ALT + CTRL");
      break;
    case 6:
      Keyboard.print("ALT + SHIFT");
      break;
  }
}// End modKeyPrint

//UTILITY EDIT MODE
void edit2() {
  byte editDelay = delayPress;
  byte editDebounce = debounceTime;
  bool changed = true;

  resetButtons();
  Keyboard.println();
  //printRom(); //TEST ONLY
  Keyboard.println();
  Keyboard.println("--- UTILITY MODE ------------------------------------");
  Keyboard.println("PRESS: the 'MIC' or 'PC' key to ESCAPE without saving");
  Keyboard.println("HOLD: the 'STREAM' key to SAVE changes and EXIT");
  Keyboard.println("-----------------------------------------------------");
  Keyboard.println();
  Keyboard.println("--- COMMANDS ----------------------------------------------------------------");
  Keyboard.println("DELAY TIME:    (Scene 5 = DN)(Scene 6 = UP)(Scene 7 = DN 10)(Scene 8 = UP 10)");
  Keyboard.println("DEBOUNCE TIME: (Scene 1 = DN)(Scene 2 = UP)(Scene 3 = DN 10)(Scene 4 = UP 10)");
  Keyboard.println("-----------------------------------------------------------------------------");
  Keyboard.println();
  while (mode == 2) {
    updateButtons();
    if (buttonValues[0] == 2) {
      mode = 0;
      if (editDelay != EEPROM.read(1)) {
        //Keyboard.println("SAVED DELAY");
        delayPress = editDelay;
        EEPROM.write(1, editDelay);
      }
      if (editDebounce != EEPROM.read(2)) {
        //Keyboard.println("SAVED DEBOUNCE");
        debounceTime = editDebounce;
        EEPROM.write(2, editDebounce);
      }
      Keyboard.println();
      Keyboard.println("Changes SAVED");
      Keyboard.println("GOODBYE!");
      resetButtons();
    }
    else if (buttonValues[1] == 1 || buttonValues[2] == 1) {
      mode = 0;
      resetButtons();
      Keyboard.println();
      Keyboard.println("ESCAPE - Changes NOT SAVED");
      Keyboard.println("GOODBYE!");
    }
    else if (changed == true) {
      Keyboard.print("Debounce = ");
      Keyboard.print(editDebounce);
      Keyboard.print("     Delay = ");
      Keyboard.println(editDelay);
      changed = false;
    }
    else if (buttonValues[7] == 1 && editDebounce > 0) {
      editDebounce--;
      changed = true;
    }
    else if (buttonValues[8] == 1 && editDebounce < 255) {
      editDebounce++;
      changed = true;
    }
    else if (buttonValues[9] == 1 && editDebounce > 10) {
      editDebounce = editDebounce - 10;
      changed = true;
    }
    else if (buttonValues[10] == 1 && editDebounce < 245) {
      editDebounce = editDebounce + 10;
      changed = true;
    }
    else if (buttonValues[3] == 1 && editDelay > 0) {
      editDelay--;
      changed = true;
    }
    else if (buttonValues[4] == 1 && editDelay < 255) {
      editDelay++;
      changed = true;
    }
    else if (buttonValues[5] == 1 && editDelay > 10) {
      editDelay = editDelay - 10;
      changed = true;
    }
    else if (buttonValues[6] == 1 && editDelay < 245) {
      editDelay = editDelay + 10;
      changed = true;
    }
  }
}// End edit2()

void initRom() {
  byte key = 4; // first key is 'A'
  byte location = EEPROM_BUTTONS; // Start of buttons in EEPROM

  if (EEPROM.read(0) != EEPROM_KEY) {
    EEPROM.write(0, EEPROM_KEY); // Key
    EEPROM.write(1, 100); // Delay
    EEPROM.write(2, 60); // Debounce

    for (int i = 0; i < NUMBER_BUTTONS; i++) {
      EEPROM.write(location, key);
      EEPROM.write((location + 1), 0);
      key++;
      location = location + 2;
    }
  }
}

void printRom() { // TEST ONLY
  for (int i = 0; i < 30; i++) {
    Keyboard.print(EEPROM.read(i));
    Keyboard.print(" ");
  }
  Keyboard.println();
}

void readRom() {
  byte location = EEPROM_BUTTONS;

  delayPress = EEPROM.read(1);
  debounceTime = EEPROM.read(2);

  for (int i = 0; i < NUMBER_BUTTONS; i++) {
    BUTTONS[i]->key = EEPROM.read(location);
    BUTTONS[i]->mod = EEPROM.read(location + 1);
    location = location + 2;
  }
}

