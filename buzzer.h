#ifndef buzzer_h
#define buzzer_h

class Buzzer
{
  private:
    int buzzPin;
  
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
