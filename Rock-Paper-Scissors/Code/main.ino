#include <ESP32Servo.h>

Servo myservo;

// Define servo positions for different angles
int pos[7] = {0,65,90,115,180};
int currentPos = 2; //Starts at the mid-position

// Define pins for the servo output and joystick inputs
#define SERVO_PIN 23
#define JOYSTICK_X_PIN 34
#define JOYSTICK_Y_PIN 35
#define JOYSTICK_SW_PIN 32

// Define analog reading thresholds for each direction
const int left_threshold = 200;
const int right_threshold = 3800;
const int up_threshold = 3800;


void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo

  // Attach servo motor to pin
  myservo.attach(SERVO_PIN, 500, 2500); 
  myservo.write(pos[currentPos]);
}

void loop() {
  // Generate a random number between 0 and 2
  int randomNumber = random(3);
  
  // Read analog input from joystick
  int x = analogRead(JOYSTICK_X_PIN);
  int y = analogRead(JOYSTICK_Y_PIN);

  // Check joystick input for each direction
  if(y < left_threshold){
    // Left direction
    turnOnLights(randomNumber, 0);
  }
  if(y > right_threshold){
    // Right direction
    turnOnLights(randomNumber, 1);
  }
  if(x > up_threshold){
    // Up direction
    turnOnLights(randomNumber ,2);
  }

  // Check if current position is at extreme ends - games ended
  if(currentPos == 0 ||currentPos == 4){
    // Move servo to center position after delay
    currentPos = 2;
    delay(3000);
    myservo.write(pos[currentPos]);
  }

  // Add delay to avoid rapid readings
  delay(300);
}


// Function to turn on lights based on computer and user input
void turnOnLights(int computerNumber, int userNumber){
  // Turn on all computer leds for tension
  digitalWrite(16, HIGH);
  digitalWrite(17, HIGH);
  digitalWrite(18, HIGH);
  
  // Turn on player's choice led
  digitalWrite(userNumber + 12, HIGH);

  // Turn off unchosen computer leds
  delay(1000);
  digitalWrite(((computerNumber + 1) % 3) + 16, LOW);
  delay(1000);
  digitalWrite(((computerNumber + 2) % 3) + 16, LOW);
  
  // Leave on the Chosen led
  delay(2000);
  
  // Check for winner
  int winner = checkForWinner(computerNumber,userNumber);
  
  // Handle different winning scenarios
  if(winner == 0){ 
    // If it's a tie, leave both leds on
    digitalWrite(computerNumber + 16, LOW);
    digitalWrite(userNumber + 12, LOW);
  }
  if(winner == 1){ 
    // If computer wins, turn off player led first
    digitalWrite(userNumber + 12, LOW);
    delay(1000);
    moveServo(winner);
    digitalWrite(computerNumber + 16, LOW);
  }
  if(winner == 2){ 
    // If player wins, turn off computer led first
    digitalWrite(computerNumber + 16, LOW);
    delay(1000);
    moveServo(winner);
    digitalWrite(userNumber + 12, LOW);
  }
}


// Function to check for winner
int checkForWinner(int computerNumber, int userNumber){
  if(computerNumber == userNumber){
    // Tie
    return 0;
  }
  if((computerNumber == 0 && userNumber == 2) || (computerNumber == 1 && userNumber == 0) || (computerNumber == 2 && userNumber == 1)){
    // Computer won
    return 1;
  }
  // User won
  return 2;
}

//   Moves the servo motor based on the game outcome (user win or computer win).
void moveServo(int winner){
  
  if(winner == 1){ 
    // If computer wins
    currentPos--;
  }
  else{
    // Player wins
    currentPos++;    
  }
  myservo.write(pos[currentPos]);
}