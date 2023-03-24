#include "pid.h"
#include "ADC.h"
#include "Parser.h"

// PID
pid my_pid {0.01, 0.15, 0, 0.01};
float r {25.0};


float pwmValue {0.0};
double lux;

// IO
const int LED_PIN = 28;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(12);
  analogWriteResolution(12);
  Serial.begin(9600);
}

void loop() {
  // Set the LED
  /*if ( Serial.available() ){
    r = Serial.parseInt(SKIP_ALL);
  }
  analogWrite(LED_PIN, r);
*/
  /*if (Serial.available()) {
    char c = Serial.read();
    if(c == "\n"){
      serialInput = "";
    }
    else
    {
      serialInput += c;
    }
  }*/
   
  
  const int MAX_MESSAGE_LENGTH = 20; // Maximum allowed message length
  const int MAX_WORD_LENGTH = 5; // Maximum allowed message length
  
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n'); // Read the incoming message
    message.trim(); // Remove any whitespace from the beginning and end of the message

    if (message.length() > MAX_MESSAGE_LENGTH) {
      Serial.println("Error: Message too long"); // Print an error message if the message is too long
      return; // Exit the loop without processing the message
    }

    // Split the message into words
    char messageBuf[MAX_MESSAGE_LENGTH+1]; // Assume the message has no more than MAX_MESSAGE_LENGTH + null terminator character '\0'
    message.toCharArray(messageBuf, MAX_MESSAGE_LENGTH+1); // Convert the message to a char array
    char* word = strtok(messageBuf, " "); // Split the message at space characters and get the first word
    
    // Save each word to an array
    String words[MAX_WORD_LENGTH]; // Assume the message has no more than MAX_WORD_LENGTH words
    int i = 0;
    while (word != NULL && i < MAX_WORD_LENGTH) {
      words[i] = String(word);
      word = strtok(NULL, " "); // Get the next word
      i++;
    }

    // Print the words to the serial monitor
    Serial.println("Received the command:");
    for (int j = 0; j < i; j++) {
      Serial.print("Word ");
      Serial.print(j + 1);
      Serial.print(": ");
      Serial.println(words[j]);
    }
  }
  
  // Read voltage
  lux = getLuminance();
  
  // Controller
  /*float y = lux;
  float u = my_pid.compute_control(r, y);
  int pwm = (int)u;
  analogWrite(LED_PIN, pwm);
  my_pid.housekeep(r, y);
  delay(10);*/
  // Visualize
  /*Serial.print("Reference:");
  Serial.print(r);
  Serial.print(",");
  Serial.print("PWM:");
  Serial.print((u)/100.0);
  Serial.print(",");
  Serial.print("Lux:");
  Serial.println(lux);*/
}
