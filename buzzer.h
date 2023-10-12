#ifndef buzzer_h
#define buzzer_h

class buzzer
{
  private:

    int buzzPin;
    int buzzState = LOW;
    unsigned long previousMillis = 0;
    unsigned long buzzerStartTime;
    int buzzerStatus = 0;
  
  public:
  
    void init(int buzzPin)
    {
      this->buzzPin = buzzPin;
      pinMode(buzzPin, OUTPUT);
    }

    void buzzbeep(unsigned long buzzerDuration){
      digitalWrite(buzzPin, HIGH);
      delay(buzzerDuration);
      digitalWrite(buzzPin, LOW);
    }
};

#endif