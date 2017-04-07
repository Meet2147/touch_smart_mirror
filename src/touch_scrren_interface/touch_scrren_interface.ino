/* Example for analogRead
*  You can change the number of averages, bits of resolution and also the comparison value or range.
*/


#include <ADC.h>


//Define DIGITAL Pins
//Pins 0 ans 1 reserved for Serial 1 COMS
#define H_SIG_SOURCE 2
#define H_SIG_RET 3
#define V_SIG_SOURCE 4
#define V_SIG_RET 5
#define CLK 6
#define RST 7
#define D_A0 8    //Demux A0 pin 
#define D_A1 9    //Demux A1 pin
#define D_A2 10    //Demux A2 pin
#define SENS_PER_H_BANK 8
#define SENS_PER_V_BANK 8

#define SEN_DELAY 1
#define SEN_THRESH 3000
#define H_SCREEN_RES 1920
#define V_SCREEN_RES 1080

const int readPin = A9; // ADC0
const int readPin2 = A2; // ADC1

const int clk_delay = 5;

// Define ADC pins
const int RX0 = A0; // Pin 14
const int RX1 = A1;
const int RX2 = A2;
const int RX3 = A3;
const int RX4 = A4;
const int RX5 = A5;
const int RX6 = A6;
const int RX7 = A7; // Pin 21
const int ledPin = 13;

int rx_pins[] = {RX0, RX1, RX2, RX3, RX4, RX5, RX6, RX7};

int h_count = 0;
int v_count = 0;
int h_bank_count = 0;
int v_bank_count = 0;
int h_cal_vals[1024];
int v_cal_vals[1024];
int h_vals[1024];
int v_vals[1024];
float h_filter[3];
float v_filter[3];

bool horiz = true;
bool vert = false;
bool init = true;
bool debug = false;
bool debug_min = false;
bool hit = false;

ADC *adc = new ADC(); // adc object;

void setup() {
    Mouse.screenSize(H_SCREEN_RES, V_SCREEN_RES);  
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(readPin, INPUT); //pin 23 single ended
    pinMode(readPin2, INPUT); //pin 23 single ended

    pinMode(D_A0, OUTPUT);
    pinMode(D_A1, OUTPUT);
    pinMode(D_A2, OUTPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(H_SIG_SOURCE, OUTPUT);
    pinMode(V_SIG_SOURCE, OUTPUT);
    pinMode(H_SIG_RET, INPUT_PULLDOWN);
    pinMode(V_SIG_RET, INPUT_PULLDOWN);
    attachInterrupt(H_SIG_RET, h_sig_interrupt, RISING);
    attachInterrupt(V_SIG_RET, v_sig_interrupt, RISING);

    //Set states of CLK and RST
    pinMode(CLK, OUTPUT);
    digitalWrite(CLK, LOW);
    pinMode(RST, OUTPUT);
    digitalWrite(RST, HIGH);

    pinMode(RX0, INPUT);
    pinMode(RX1, INPUT);
    pinMode(RX2, INPUT);
    pinMode(RX3, INPUT);
    pinMode(RX4, INPUT);
    pinMode(RX5, INPUT);
    pinMode(RX6, INPUT);
    pinMode(RX7, INPUT);


    
    //Serial.begin(9600);
    if (debug){
      Serial.begin(9600);
      Serial.println("Begin setup");
    }

    ///// ADC0 ////
    // reference can be ADC_REF_3V3, ADC_REF_1V2 (not for Teensy LC) or ADC_REF_EXT.
    //adc->setReference(ADC_REF_1V2, ADC_0); // change all 3.3 to 1.2 if you change the reference to 1V2

    adc->setAveraging(32); // set number of averages
    adc->setResolution(16); // set bits of resolution

    // it can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED_16BITS, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED
    // see the documentation for more information
    adc->setConversionSpeed(ADC_LOW_SPEED); // change the conversion speed
    // it can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED
    adc->setSamplingSpeed(ADC_LOW_SPEED); // change the sampling speed

    //adc->enableInterrupts(ADC_0);

    // always call the compare functions after changing the resolution!
    //adc->enableCompare(1.0/3.3*adc->getMaxValue(ADC_0), 0, ADC_0); // measurement will be ready if value < 1.0V
    //adc->enableCompareRange(1.0*adc->getMaxValue(ADC_0)/3.3, 2.0*adc->getMaxValue(ADC_0)/3.3, 0, 1, ADC_0); // ready if value lies out of [1.0,2.0] V

    ////// ADC1 /////
    #if ADC_NUM_ADCS>1
    adc->setAveraging(4, ADC_1); // set number of averages
    adc->setResolution(16, ADC_1); // set bits of resolution
    adc->setConversionSpeed(ADC_HIGH_SPEED, ADC_1); // change the conversion speed
    adc->setSamplingSpeed(ADC_HIGH_SPEED, ADC_1); // change the sampling speed

    // always call the compare functions after changing the resolution!
    //adc->enableCompare(1.0/3.3*adc->getMaxValue(ADC_1), 0, ADC_1); // measurement will be ready if value < 1.0V
    //adc->enableCompareRange(1.0*adc->getMaxValue(ADC_1)/3.3, 2.0*adc->getMaxValue(ADC_1)/3.3, 0, 1, ADC_1); // ready if value lies out of [1.0,2.0] V
    #endif
    if (debug){
      Serial.println("End setup");
    }

}

int value;
int value2;

void clk_pulse(){
  digitalWrite(CLK, HIGH);
  delayMicroseconds(clk_delay);
  digitalWrite(CLK,LOW);
}

void reset_all(){
  digitalWrite(RST, LOW);
  delayMicroseconds(clk_delay);
  digitalWrite(RST,HIGH);
}

void h_sig_interrupt(){
  horiz = false;
  vert = true;
  digitalWrite(H_SIG_SOURCE, LOW);
}

void v_sig_interrupt(){
  vert = false;
  horiz = true;
  digitalWrite(V_SIG_SOURCE, LOW);
}

void init_count_banks(){
  digitalWrite(H_SIG_SOURCE, HIGH);
  if (debug){
      Serial.println("H SIG INIT");
  }
  bool source_on = true;
  while (horiz){
    h_bank_count += 1; 
    if (debug){
      Serial.println(h_bank_count);
    }
    clk_pulse();
    if (source_on){
      if (debug){
        delay(5000);
      }
      digitalWrite(H_SIG_SOURCE, LOW);
      source_on = false;
    }
  }
  digitalWrite(V_SIG_SOURCE, HIGH);
  if (debug){
      Serial.println("V SIG INIT");
  }
  source_on = true;
  while(vert){
    v_bank_count += 1; 
    clk_pulse();
    if (source_on){
      digitalWrite(V_SIG_SOURCE, LOW);
      source_on = false;
    }
  }
  reset_all();
}

void ring_loop(int *h_vals, int *v_vals, bool init){
  int adc_val = 0;
  digitalWrite(H_SIG_SOURCE, HIGH);
  delayMicroseconds(50);

  if (debug){
      Serial.println("H SIG START");
  }
  bool source_on = true;
  int h_bank_num = 0;
  while (horiz){
    if (debug){
      Serial.print("H Bank Num: ");
      Serial.println(h_bank_num);
    }
    clk_pulse();
    digitalWrite(H_SIG_SOURCE, LOW);

    for (int i = 0; i < SENS_PER_H_BANK; i++){
      adc_val = read_sensor_pair(7-i);
      if (debug){
        Serial.println(adc_val);
      }
      int n = h_bank_num*(SENS_PER_H_BANK) + i;
      //Serial.println(n);
      if (!init){
        adc_val = h_cal_vals[n] - adc_val;
      }
      if (adc_val > -200 && adc_val < 200){
        adc_val = 0;
      }
      h_vals[n] = abs(adc_val);
      delay(SEN_DELAY);
    }
    h_bank_num++;
  }
  
  digitalWrite(V_SIG_SOURCE, HIGH);
  delay(1);
    if (debug){
      Serial.println("V SIG START");
  }
  source_on = true;
  int v_bank_num = 0;
  while (vert){
    if (debug){
      Serial.print("V Bank Num: ");
      Serial.println(v_bank_num);
    }
    clk_pulse();
    digitalWrite(V_SIG_SOURCE, LOW);

    for (int i = 0; i < SENS_PER_V_BANK; i++){
      adc_val = read_sensor_pair(7-i);
      if (debug){
        Serial.println(adc_val);
      }
      int n = v_bank_num*(SENS_PER_V_BANK) + i;
      //Serial.println(n);
      if (!init){
        adc_val = v_cal_vals[n]-adc_val;
      }
      if (adc_val > -200 && adc_val < 200){
        adc_val = 0;
      }
      v_vals[n] = abs(adc_val);
      delay(SEN_DELAY);
    }
    v_bank_num++;
  }
  reset_all();
}

int read_sensor_pair(int i){

  byte num = (byte)i;
  //Send Demux Address for Tx LED
  int a0 = bitRead(num,0);
  int a1 = bitRead(num,1);
  int a2 = bitRead(num,2);
  if (debug){
    Serial.print(a2);
    Serial.print(a1);
    Serial.print(a0);
    Serial.print("\n");
  }
  digitalWrite(D_A0, a0);
  digitalWrite(D_A1, a1);
  digitalWrite(D_A2, a2);
  
  //Give a small delay to let chip setup
  delayMicroseconds(50);
  int val = analogRead(rx_pins[i]);
  return val;
  
}

float get_position(int count, int *values){
    if (debug){
      Serial.println("Starting get_position");
    }
    float total = 0;
    float num_hits = 0;
    float max_val = 0;
    int max_location = -1;
    
    //Find Maximum ADC value and it's location
    for (int i = 0; i < count; i++)
    {
      float val = (float)values[i];
      if ((val > max_val) && (val > 0)){
        max_val = val;
        max_location = i;
      }
    }

    if (max_location == -1){
      if(debug){
        Serial.println("No hits found");
      }
      return -1.0;
    }
    
    if (debug){
      Serial.println("Locating mean");
    }
    //Localize reading to +/- one sensor away from max hit
    for(int i = (max_location - 1); i < (max_location+2); i++)
    {
      int reading = values[i];
      if (reading > SEN_THRESH){
         num_hits = num_hits + 1.0;
         if (i < max_location){
          total = total + (float)(i) + (((float)reading)/max_val);
         }
         else if (i == max_location){
          total = total + (float)(i);
         }
         else if (i > max_location){
          total = total + (float)(i) + 1 - (((float)reading)/max_val);
         }
      }
    }

    if (num_hits == 0){
      return -1.0;
    }
    float pos = (total/num_hits);
    return pos;
}


void loop() {
    digitalWrite(ledPin, HIGH);
    //Init and calibration code on first run
    if (init){
      delay(5000);
      if (debug){
        for(int i = 0; i<6;i++){
          Serial.println("Init test waiting");
          delay(5000);
        }
      }
      init_count_banks();
      h_count = h_bank_count * SENS_PER_H_BANK;
      v_count = v_bank_count * SENS_PER_V_BANK;
      ring_loop(h_cal_vals,v_cal_vals,init);
      init = false;    
      }
     if (debug){
        Serial.println("Start Loop");
        Serial.print("Horiz Banks: ");
        Serial.println(h_bank_count);
        Serial.print("Horiz Sensors: ");
        Serial.println(h_count);
        Serial.print("Vert Banks: ");
        Serial.println(v_bank_count);
        Serial.print("Vert Sensors: ");
        Serial.println(v_count);
      }

    ring_loop(h_vals, v_vals, init);
    digitalWrite(ledPin, LOW);
    if (debug | debug_min){
      Serial.print("H Values: ");
      for(int n = 0; n < h_count;n++){
        Serial.print(h_vals[n]+h_cal_vals[n]);
        Serial.print(" ");
      }
      Serial.print("\n");
      Serial.print("H Normalized Values: ");
      for(int n = 0; n < h_count;n++){
        Serial.print(h_vals[n]);
        Serial.print(" ");
      }
      Serial.print("\n");
      Serial.print("V Values: ");
      for(int n = 0; n < v_count;n++){
        Serial.print(v_vals[n]+v_cal_vals[n]);
        Serial.print(" ");
      }
      Serial.print("\n");
      Serial.print("V Normalized Values: ");
      for(int n = 0; n < v_count;n++){
        Serial.print(v_vals[n]);
        Serial.print(" ");
      }
      Serial.print("\n");
    }

    float h_pos = get_position(h_count, h_vals);
    float v_pos = get_position(v_count, v_vals);
    
    if (debug){
      Serial.print("H pos: ");
      Serial.println(h_pos);
      Serial.print("V pos: ");
      Serial.println(v_pos);
      //Serial.print("V Screen Percentage: ");
      //Serial.println(v_percentage);
    }

    //Populate filter array with first hit value if it is the first detection
    if (h_pos >= 0.0 && v_pos >= 0.0){
      if (!hit){
        h_filter[0]=h_pos;
        h_filter[1]=h_pos;
        v_filter[0]=v_pos;
        v_filter[1]=v_pos;
        hit = true;
      }

      //Mode values down the line
//      for (int i=2;i>0;i--){
//        h_filter[i]=h_filter[i-1];
//      }
//      h_filter[0] = h_pos;

      //Take weighted average
      //float h_w_avg = (h_filter[2] + 3 * h_filter[1] + h_filter [0]) / 5;
      
      //Repeat for vertical... Not enough code to justify making a function
//      for (int i=2;i>0;i--){
//        v_filter[i]=v_filter[i-1];
//      }
//      v_filter[0] = v_pos;
//      float v_w_avg = (v_filter[2] + 3 * v_filter[1] + v_filter [0]) / 5;

      
//      float h_percentage = h_w_avg/((float)(h_count - 1));
//      float v_percentage = v_w_avg/((float)(v_count - 1));
      float h_percentage = h_pos/((float)(h_count - 1));
      float v_percentage = v_pos/((float)(v_count - 1));
      //Rx on top
      //int h = H_SCREEN_RES - (int)(h_percentage * H_SCREEN_RES);
      //int v = (int)(v_percentage * V_SCREEN_RES);

      //Rx on Bottom
      int h = (int)(h_percentage * H_SCREEN_RES);
      int v = V_SCREEN_RES -(int)(v_percentage * V_SCREEN_RES);
      
      Mouse.moveTo(h,v);
      Mouse.press(MOUSE_LEFT);
      if (debug_min){
        Serial.print(h);
        Serial.print(" ");
        Serial.println(v);
      }
    }
    else{
      Mouse.release(MOUSE_LEFT);
      hit = false;
    }
    
}
// If you enable interrupts make sure to call readSingle() to clear the interrupt.
void adc0_isr() {
        adc->adc0->readSingle();
}

