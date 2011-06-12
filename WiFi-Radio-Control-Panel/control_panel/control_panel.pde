#include <SPI.h>
#include <Ethernet.h>
#include <EthernetDNS.h>
#include <Twitter.h>
#include <LiquidCrystal.h>

// use the pins as specified in the schematic
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#define TWITTER_AUTH_CODE "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
Twitter twitter(TWITTER_AUTH_CODE);

// four bits used by the rotary encoder
// the analog pins can actually be used as digital IO,
// just refer to the numbers A0-A5 as their digital equivalents: 14-19
#define ENC_BIT_0 14
#define ENC_BIT_1 15
#define ENC_BIT_2 16
#define ENC_BIT_3 17

// MAC and IP address of the Arduino's ethernet adapter
// just make up something unique.
byte mac[]    = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[]     = { 192,168,0,15 };

// IP address of the radio server
byte server[] = { 192,168,0,10 };
// port on the radio server on which MPD is listening
#define MPD_PORT 6600

// how many lines are there on the screen?
#define SCREEN_LINES   2
// how many columns, i.e., how many characters can be displayed?
#define SCREEN_COLUMNS 16
// how many characters should I reserve for the scroll buffers?
// (note that null terminator limits the actual string length 
// to one less than this number)
#define SCROLL_BUF_LEN 128

// scrollBuf contains a buffer of text for each line,
// which will be scrolled from right to left
char scrollBuf[SCREEN_LINES][SCROLL_BUF_LEN] = { 0 };

#define NUM_STATIONS 10
#define STOP "Stop"
// points to the currently playing station
uint8_t currentStation = 0;
// names for the stations
const char* stations[NUM_STATIONS] = {
  STOP,
  "Digitalis",
  "Drone Zone",
  "Groove Salad",
  "Illinois Street Lounge",
  "Lush",
  "Secret Agent",
  STOP, // extra slots, for now, use stop
  STOP,
  STOP,
};

// ursl for each of the stations
const char* urls[] = {
  "", // blank for stop
  "http://ice.somafm.com/digitalis",
  "http://ice.somafm.com/dronezone",
  "http://ice.somafm.com/groovesalad",
  "http://ice.somafm.com/illstreet",
  "http://ice.somafm.com/lush",
  "http://ice.somafm.com/secretagent",
  "", // extra slots, for now, use stop
  "",
  ""
};

// create a global "client" object, which will act 
// as the Arduino's local client to the radio server
Client radioServer(server, MPD_PORT); 

// duration of debounce delay, in milliseconds
// i.e., time that input value must remain consistent 
// before it is counted as a new "input"
#define DEBOUNCE_TIME 10
uint8_t debounceEncoder() {
  static uint32_t timer = 0; 
  static uint8_t  debounced = false;
  static uint8_t  currentValue = 0;

  uint8_t bit0 = !digitalRead(ENC_BIT_0);
  uint8_t bit1 = !digitalRead(ENC_BIT_1);
  uint8_t bit2 = !digitalRead(ENC_BIT_2);
  uint8_t bit3 = !digitalRead(ENC_BIT_3);
  uint8_t newValue = 0;
  newValue = (bit3 << 3) | (bit2 << 2) | (bit1 << 1) | bit0;
  if (newValue != currentValue && debounced) { 
    timer = millis() + DEBOUNCE_TIME; 
    debounced = false;
  }
  if (millis() > timer && !debounced) {
    currentValue = newValue;
    debounced = true;
  }
  return currentValue;
}

// in milliseconds, how long to wait before scrolling display
// one character to the left.
#define SCROLL_DELAY 100

// render display maintains 
void renderDisplay(bool force = false) {
  // startPos maintains, for each line of the display, 
  // how many characters the display string has been scrolled.
  // it includes the blank spaces (SCREEN_COLUMNS of them, to be exact)
  // that exist between the scrolled messaged, so it must be offset
  // by SCREEN_COLUMNS to find the actual position in the display string
  // that is the first to be displayed (if the beginning of the string 
  // itself has been scrolled off the screen to the left)
  static uint8_t startPos[SCREEN_LINES] = { 0 };

  static uint32_t nextUpdate = 0;
  if ( (millis() > nextUpdate) || force) {
    nextUpdate = millis() + SCROLL_DELAY;

    // for each line of the display
    for (uint8_t i = 0; i < SCREEN_LINES; i++) {

      // declare a print buffer that will hold the line
      // as it is built up, to replace line i on the display
      // the size is the number of columns + 1, to allow room
      // for the null terminator
      char pBuf[SCREEN_COLUMNS+1] = { 0 }; // initialize array to null
      memset(pBuf, ' ', SCREEN_COLUMNS);   // set full line to blank spaces
      pBuf[SCREEN_COLUMNS] = '\0';         // sanity check for null terminator, could probably remove.

/*
      // debug output
      Serial.print("startPos[");
      Serial.print(i, DEC);
      Serial.print("]:");
      Serial.println(startPos[i], DEC);
*/      
      // pBufOffset is the offset into the print buffer to begin the display string,
      // i.e., how many blank spaces before the first character in the display string
      // it is computed by taking greater of the startPosition of the current line (offset
      // by the number of columns) and 0, so that as startPos increases (as the string is scrolled off the screen to the left), 
      uint8_t pBufOffset      = max(SCREEN_COLUMNS - startPos[i], 0);
      uint8_t scrollBufOffset = max(startPos[i] - SCREEN_COLUMNS, 0);
      uint8_t len = min((SCREEN_COLUMNS - pBufOffset), strlen(scrollBuf[i]) - scrollBufOffset);

/*      
      Serial.print("min(pBufOffset, strlen(scrollBuf[i])): ");
      Serial.println(len, DEC);
      Serial.print("strlen(pBuf): ");
      Serial.println(strlen(pBuf), DEC);
*/      
      // we copy into the pBufOffset'th character into the pBuf string,
      // from the scrollBufOffset'th character in the scrollbuffer string,
      strncpy(pBuf + pBufOffset, scrollBuf[i] + scrollBufOffset, len);
      // add a null terminator (may not be necessary)
      pBuf[pBufOffset + len] = '\0';      
/*      
      Serial.print("scrollBuf[");
      Serial.print(i, DEC);
      Serial.print("]:");
      Serial.println(scrollBuf[i]);
      Serial.print("pBufOffset: ");
      Serial.println(pBufOffset, DEC);
      Serial.print("scrollBufOffset: ");
      Serial.println(scrollBufOffset, DEC);
      Serial.print("pBuf: ");
      Serial.println(pBuf);
*/      
      // fill any remaining space after the contents pBuffer with spaces
      if (strlen(pBuf) < SCREEN_COLUMNS) {
        memset(pBuf + strlen(pBuf), ' ', SCREEN_COLUMNS - strlen(pBuf));
      }
      // add a null terminator
      pBuf[SCREEN_COLUMNS] = '\0';
/*      
      Serial.println("about to print: ");
      Serial.println(pBuf);
      Serial.print("to line ");
      Serial.println(i, DEC);
*/      
      // set the cursor to the beginning of the appropriate line
      // and send the display buffer to be printed
      lcd.setCursor(0, i);
      lcd.print(pBuf);      
      
      // increment (or reset) the start position
      if (startPos[i] < strlen(scrollBuf[i]) + SCREEN_COLUMNS) {
        startPos[i]++; 
      } else {
        startPos[i] = 0;
      }
    }
  }
}

// read a single line of text from the radioServer (until a newline is read)
// into buffer, with a maximum length of len
// returns true if it read anything, false if the radioServer didn't send anything
bool getLine(char* buf, uint8_t len) {
  uint8_t i = 0;
  if (!radioServer.available()) {
    return false;
  }
  // have to do len-1 to leave room for null character at end
  while(radioServer.available() && i < len - 1) {
    char c = radioServer.read();
    if (c != '\n') {
      buf[i++] = c;
    } else {
      break;
    }
  }
  buf[i] = '\0'; // end string with a null;
//  Serial.print("read line: ");
//  Serial.println(buf);
  return true;
}

// try and post the given message on twitter, print errors to Serial if unsuccessful
void tweet(const char* msg) {
  Serial.print("Posting tweet: '");
  Serial.print(msg);
  Serial.println("'");
  if (twitter.post(msg)) {
    int stat = twitter.wait();
    if (stat == 200) {
      Serial.println("Twitter OK.");
    } else {
      Serial.print("failed : code ");
      Serial.println(stat);
    }
  } else {
    Serial.println("twitter connection failed.");
  }
}

// the prefix of the response we're looking for, and how long we expect it to be
#define PREFIX "Title: "
#define PREFIX_LEN 7

// how many seconds between updates of the currently-playing song
#define SONG_UPDATE_TIME 5

// this buffer holds the most recent twitter message,
// so we don't try to send the same message more than once.
char twitterMsg[SCROLL_BUF_LEN] = { '\0' };

// periodically request current status from the server
// parse out the current artist and song, and pass it to the scroll buffer
void updateSongTitle(bool force = false) {
  // a buffer to read in the responses from the radio server, until we get the one we want
  char songName[SCROLL_BUF_LEN] = { '\0' };

  static uint32_t nextUpdate = 0;
  uint32_t curSec = millis() / 1000;
  if ((curSec > nextUpdate) || force) {
    nextUpdate = curSec + SONG_UPDATE_TIME;

    // send a command to the radioServer, requesting info about the currently-playing song  
    radioServer.println("currentsong");
    bool inSongName = false;
    bool doneWithSongName = false;
    // wait for the server to respond
    delay(10);
    
    // read lines one at a time from the server's response, into the songName buffer
    while(getLine(songName, SCROLL_BUF_LEN)) {
      // check if the temporary buffer the field has the right prefix
      // if so, flush the remainder of the server's response, and break out of the loop
      if (strncmp(songName, PREFIX, PREFIX_LEN) == 0) {
        radioServer.flush();
        break;
      }
    }
    // sanity check, in case we made it through the whole response without finding any matching lines
    // if the buffer has the right prefix
    if (strncmp(songName, PREFIX, PREFIX_LEN) == 0) {
//      Serial.print("found song name: ");
//      Serial.print(songName + PREFIX_LEN);
//      Serial.println("!");
//      Serial.print("strlen(songName):");
//      Serial.println(strlen(songName));
//      Serial.print("min(strlen(songName), SCROLL_BUF_LEN - 1): ");
//      Serial.println(min(strlen(songName), SCROLL_BUF_LEN - 1));
      
      // offset by PREFIX_LEN bytes, to skip the characters in "Title: " (which we don't want on the display)
      // copy into the 2nd line of the scroll buffer (index 1), starting with the songName after the prefix
      // copy either the full length of the songline, or at most, the maximum length of the buffer.
      strncpy(scrollBuf[1], songName + PREFIX_LEN, min(strlen(songName + PREFIX_LEN), SCROLL_BUF_LEN - 1));
//      Serial.print("scrollBuf[1]: ");
//      Serial.println(scrollBuf[1]);

      // build up a new twitter message, and if it's new, post it.
      char newTwitterMsg[SCROLL_BUF_LEN] = { '\0' };
      snprintf(newTwitterMsg, SCROLL_BUF_LEN - 1, "Now playing on station %s: %s", stations[currentStation], (songName + PREFIX_LEN));
      if (strncmp(newTwitterMsg, twitterMsg, strlen(newTwitterMsg)) != 0) {
        Serial.print("Tweeting: '");
        Serial.print(newTwitterMsg);
        Serial.println("'");
        tweet(newTwitterMsg);
        strncpy(twitterMsg, newTwitterMsg, min(strlen(newTwitterMsg), SCROLL_BUF_LEN - 1));        
//      } else {
//        Serial.print("Didn't tweet this message: '");
//        Serial.print(newTwitterMsg);
//        Serial.println("' : ("); 
      }
    } 
  }
}

// check to see if the (debounced) encoder value has changed (due to a new selection)
// and handle the new selection  
void checkDial() {
  // how often between updates
  static uint32_t changeTimeout = 1000;
  // timer to count mark when the next update should be
  static uint32_t changeTimer   = 0;
  // what the previous value of the encoder is
  static uint8_t  oldVal        = 0;
  // indicates when we need to change the station
  static bool     changed       = false;

  // check to see if the encoder has changed, if so, set the timer;
  if (debounceEncoder() != oldVal) {
    oldVal      = debounceEncoder();
    changeTimer = millis() + changeTimeout;
    changed     = false;
  }

  // once the timer expires, go ahead and change the station
  if (millis() > changeTimer && !changed) {
//    Serial.print("New value: ");
//    Serial.println(oldVal, DEC);
//    Serial.println("----------");
    
    // mod value from encoder by number of stations, to make sure it is in range.
    currentStation = oldVal % NUM_STATIONS;
//    Serial.print("Selected station: ");
//    Serial.println(stations[currentStation]);

    // if instructed to stop, send stop command to the radio server
    if (strcmp(stations[currentStation], STOP) == 0) { // last index
//      Serial.print("stopping station.");
      // command the radio server to stop the current station      
      radioServer.println("stop");
    } else {
      // copy the station name into the first line (0 index) of the scroll buffer.   
      strncpy(scrollBuf[0], stations[currentStation], min(strlen(stations[currentStation]), SCROLL_BUF_LEN-1));
//      Serial.print("Starting station: ");
//      Serial.println(stations[currentStation]);
//      Serial.print("scrollBuf[0]: ");
//      Serial.println(scrollBuf[0]);
      
      // command the radio server to stop the previous station      
      radioServer.println("stop");
      // command the radio server to clear the playlist (so the previous station is removed)
      radioServer.println("clear");
      // command the radio server to add the new station to the playlist
      radioServer.print("add ");
      radioServer.println(urls[currentStation]);
      // command the radio server to play the playlist
      radioServer.println("play");

      // force an update of the currently playing song
      // force an update of the display      
      updateSongTitle(true);
      renderDisplay(true);
    }
    changed = true;
  }
}

// Arduino setup routine, happens once, before anything else
void setup() {
  // set up the pins for the rotary encoder
  pinMode(ENC_BIT_0, INPUT);
  digitalWrite(ENC_BIT_0, HIGH); // enable internal pullup
  pinMode(ENC_BIT_1, INPUT);
  digitalWrite(ENC_BIT_1, HIGH); // enable internal pullup
  pinMode(ENC_BIT_2, INPUT);
  digitalWrite(ENC_BIT_2, HIGH); // enable internal pullup
  pinMode(ENC_BIT_3, INPUT);
  digitalWrite(ENC_BIT_3, HIGH); // enable internal pullup

  // start the Ethernet interface
  Ethernet.begin(mac, ip);
  // initialize the serial interface
  Serial.begin(9600);
  
  // initialize the LCD
  lcd.begin(SCREEN_COLUMNS, SCREEN_LINES);

  // wait a bit for it all to take
  delay(1000);

  // establish a connection to the radioServer.
  Serial.println("connecting...");
  lcd.setCursor(0, 0);
  lcd.print("connecting...");
  if (radioServer.connect()) {
    Serial.println("connected!");
    lcd.setCursor(0, 1);
    lcd.print("success!");
  } else {
    Serial.println("radio connection failed.");
    lcd.setCursor(0, 1);
    lcd.print("failed.");
  }
  
  // force an update the first time through.
  checkDial();
  updateSongTitle(true);
  renderDisplay(true);
}

// Arduino loop routine, called repeatedly after setup.
void loop() {

  // check to see if the selection has changed,
  // update the song title (if necessary)
  // render the next scroll step of the display (if necessary)
  checkDial();
  updateSongTitle();
  renderDisplay();
  
  // flush output from radioServer
  radioServer.flush();

  // if something goes wrong and we lose the connection, display an error and hang in infinite loop
  if (!radioServer.connected()) {
    lcd.clear();
    lcd.print("connection lost.");
    Serial.println();
    Serial.println("disconnected.");
    radioServer.stop();
    for (;;) 
      ;
  }
}

