//Messages from MQTTLens to Node and Node messages show on Serial Monitor Arduino
// String Format = "(RB_H,RB_X,RB_Y);(Obj1_X,Obj1_Y);(Obj2_X,Obj2_Y);(Obj3_X,Obj3_Y);(Obj4_X,Obj4_Y);(Obj5_X,Obj5_Y);(Obj6_X,Obj6_Y);(Obj7_X,Obj7_Y);(Obj8_X,Obj8_Y);(Obj9_X,Obj9_Y);(BL_X,BL_Y);(BR_X,BR_Y);(TR_X,TR_Y);(TR_X,TR_Y)"
// ex = "(0,10,20);(30,40);(50,60);(70,80);(90,100);(11,22);(33,44);(55,66);(77,88);(99,110);(1,1);(99,1);(1,100);(99,100);";
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include<Servo.h>
#include <hcsr04.h>
Servo myservo0;


#define TRIG_PIN D0
#define ECHO_PIN D8
HCSR04 my_ultra(TRIG_PIN, ECHO_PIN, 20, 4000);


bool catchTheBall = false;
int ultra_Distance;


// Update these with values suitable for your network.
const char* ssid = "xeuslab";
const char* password = "embeddednu2014";
String msg = "";
char msgChar[50];
String msg1 = "";
String msg2 = "";
int Completed;
int minDisIndex;
byte Robot_State = 0;
byte sub_Robot_State = 0;
byte Servo_State = 0;
//the %distance accepted from the central robot to ball
//int Dis_Servo_Accepted = 20;


int Subvalue_ServoAccepted = 16;
int min_ultra_Distance = 40;
int max_ultra_Distance = 65;
float Robot_Ball_AcceptedDirection = 7.0;
float Robot_Ball_AcceptedDistance = 2.6;
//float Robot_Ball_AcceptedDistance = 24.0;

float Robot_Zone_AcceptedDirection = 10.0;
float Robot_Zone_AcceptedDistance = 13.0;
//float Robot_Zone_AcceptedDistance = 24.0;

float Robot_AvoidanceDistance = 30.0;
//60
float Robot_AvoidanceDirection = 30.0;
//30

float Dis_Robot_Ball;

//format charector
boolean stringComplete = false;
//String msg = "";

const byte n_object = 13;
const byte n_area = 4;
float Object_X[n_object]; // this is float array of positions X of objects(n) when n is floateger of amount object
float Object_Y[n_object];

// positions of objects you can call to variable for compute like this
// float object_1X = Object_X[0]
// float object_1Y = Object_Y[0]
// float object_2X = Object_X[1]
// float object_2Y = Object_Y[1]
// float object_3X = Object_X[2]
// float object_3Y = Object_Y[2]
// float object_4X = Object_X[3]
// float object_4Y = Object_Y[3]
//              ....
//              ....
//              ....
// float object_nX = Object_X[n-1]
// float object_nY = Object_Y[n-1]  
// WHEN n IS INTEGER OF AMOUNT OBJECTS call this "INDEX"
// in this case n is integer of n_object that index between 0 to 13-1
// INDEX of float array start at 0 , end at 12
//-----------------------------------------------------------------------


float Robot_H;
float Robot_X;
float Robot_Y;

float Distance[n_object];// this is float array of Distance
// you can call this float array to variable like this 

// float disObj_1 = Distance[0] 
// """ it means Distance between object1, Robot """
// float disObj_2 = Distance[1] 
// """ it means Distance between object1, Robot """
//              ....
//              ....
// float disObj_n = Distance[n-1] 
// """ it means Distance between objectn, Robot """

// in this case n is integer of n_object that index between 0 to 13-1
// INDEX of float array start at 0 , end at 12
//-----------------------------------------------------------------------
float Direction[n_object];// this is float array of Direction
// float Direction_1 = Direction[0]
// float Direction_2 = Direction[1]
// float Direction_3 = Direction[2]
// float Direction_4 = Direction[3]
//      ....
//      ....
//      ....
// float Direction_n = Direction[n-1]
// in this case n is integer of n_object that index between 0 to 13-1
// INDEX of float array start at 0 , end at 12
//----------------------------------------------------------------------
// Position_x area
float BL_X; // Bottom left
float BR_X; // Bottom Right
float TL_X; // Top left
float TR_X; // Top right

// Position_y area
float BL_Y; // Bottom left
float BR_Y; // Bottom Right
float TL_Y; // Top left
float TR_Y; // Top right

float area_X[4] = {BL_X, BR_X, TL_X, TR_X};
float area_Y[4] = {BL_Y, BR_Y, TL_Y, TR_Y};
float Delta_Direction[n_object];





#define IN1 D1    // Pin 10 of L293D IC, D0 Pin of NodeMCU
#define IN2 D2    // Pin 15 of L293D IC, D1 Pin of NodeMCU

//Motor B RIGHT
#define IN3 D3    // Pin  2 of L293D IC, D2 Pin of NodeMCU
#define IN4 D4    // Pin  7 of L293D IC, D3 Pin of NodeMCU

#define EN1 D5                // Pin 1 of L293D IC, D5 Pin of NodeMCU
#define EN2 D6                // Pin 9 of L293D IC, D6 Pin of NodeMCU

#define svport0 D7

float lrHalfAccepted = 0.50;
float lrFullAccepted = 0.8;
float forwardbackAccepted = 0.7;
// Config MQTT Server
#define mqtt_server "192.168.0.101"
#define mqtt_port 1883

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  myservo0.attach(svport0); //D7
  myservo0.write(90); //initial the start angle


  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);


  pinMode(EN1, OUTPUT);   // where the motor is connected to
  pinMode(EN2, OUTPUT);   // where the motor is connected to
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);  
    
}

void loop() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("/ESP/MSG1");
      client.subscribe("/ESP/MSG2");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      return;
    }
  }
  client.loop();
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,"/ESP/MSG1")==0){
      msg1 = "";
      int i=0;     
      while (i<length) msg1 += (char)payload[i++];
    }
  else if(strcmp(topic,"/ESP/MSG2")==0){
      msg2 = "";
      int i=0;
      while (i<length) msg2 += (char)payload[i++];
    }

  if((msg1 != "" )&&(msg2 != "")){
     String msg = "";
     //msg = msg1 + ";"+ msg2;
     msg = msg1 + msg2;
     
     messageSplitToPair(msg);
     getDistance();
     getDirection();
     check_State();
     //Serial.println("split to float value");
     //showInfo();
     msg = "";
     msg1 = "";
     msg2 = "";
     

  }
}

void showInfo()
{
    Serial.print("Robot_H = ");
    Serial.println(Robot_H);
    Serial.print("Robot_X = ");
    Serial.print(Robot_X);
    Serial.print("Robot_Y = ");
    Serial.println(Robot_Y);

    Serial.print("Bottom left = ");
    Serial.print(BL_X);
    Serial.print(",");
    Serial.println(BL_Y);

    Serial.print("Bottom right = ");
    Serial.print(BR_X);
    Serial.print(",");
    Serial.println(BR_Y);

    Serial.print("Top left = ");
    Serial.print(TL_X);
    Serial.print(",");
    Serial.println(TL_Y);
    
    Serial.print("Top right = ");
    Serial.print(TR_X);
    Serial.print(",");
    Serial.println(TR_Y);
    for(byte i=0; i<n_object ;i++)
    {
        Serial.print("Object_X[");
        Serial.print(i+1);
        Serial.print("] = ");
        Serial.println(Object_X[i]);
        
        Serial.print("Object_Y[");
        Serial.print(i+1);
        Serial.print("] = ");
        Serial.println(Object_Y[i]);
    }

    // LAB 5.2 ADD
    Serial.println("==========Distance========");
  
    for(byte index = 0; index < n_object; index++){
      Serial.print("Distance Between Robot,Object[");
      Serial.print(index+1);
      Serial.print("] = ");
      Serial.println(Distance[index]);
    }
    Serial.println("==========Direction========");
    for(byte index = 0; index < n_object; index++){
      Serial.print("Direction Between Degree 0,Object[");
      Serial.print(index+1);
      Serial.print("] = ");
      Serial.println(Direction[index]);
    }

    Serial.println("==========Delta Direction========");
    for(byte index = 0; index < n_object; index++){
      Serial.print("Delta Direction Between Robot,Object[");
      Serial.print(index+1);
      Serial.print("] = ");
      Serial.println(Delta_Direction[index]);
    }
}


void check_State(){

  //(1)Red Ball


  if(Robot_State == 0){
    if((Object_X[0] == -1)&&(Object_Y[0] == -1)){
      Robot_State = 4;
      return;
    }
    Serial.println(Robot_State);
    if(abs(Delta_Direction[0]) <= Robot_Ball_AcceptedDirection){
      Robot_State = 1;
    }
    else{
      if (Delta_Direction[0] > 0){
        Robot_Turn_Right_byDirection(Delta_Direction[0]);
      }
      else{
        Robot_Turn_Left_byDirection(Delta_Direction[0]);
      }
    }
  }

  else if(Robot_State == 1){
    Serial.println(Robot_State);
    if (abs(Delta_Direction[0]) > Robot_Ball_AcceptedDirection){
      Robot_State = 0;
      //return;
    }
    if ((Servo_State == 0)&&(abs(Distance[0]) <= (Robot_Ball_AcceptedDistance + Subvalue_ServoAccepted))){
      //Servo is open
      myservo0.write(180);

      Servo_State = 1;
      //return;
    }
    check_catchTheBall(1);    
    if ((Servo_State == 1)&&((catchTheBall)||(abs(Distance[0]) <= Robot_Ball_AcceptedDistance))){
      //Servo is close
      myservo0.write(90);

      Servo_State = 2;
      //Robot_State = 2;
    }
    
    if ((Servo_State == 2) && (abs(Distance[0]) <= Robot_Ball_AcceptedDistance)){
      check_catchTheBall(1);
      if(catchTheBall){
        Robot_State = 2;
      }
      else{
        Robot_State = 0;
      }
      //return;
    }
    else{
      minDisIndex = findMinDistanceIndex(Robot_State);
      Serial.println("IN STATEminDisIndex: ");
      Serial.println(minDisIndex);
      if (minDisIndex  == -1){
        Serial.println("");
        Robot_Move_Forward_byDistance(Distance[0]);
      }
      else if((Robot_X != -1)&&(Robot_Y != -1)){        
        avoidance();
        Serial.println("Avoid Forward");
        Robot_Move_Forward_AvoidbyDistance(Distance[minDisIndex]);
      }         
    }
  }

  else if (Robot_State == 2){
    Serial.println(Robot_State);
    if (abs(Delta_Direction[0]) > Robot_Ball_AcceptedDirection){
      Robot_State = 0;
      //return;
    }

    if (abs(Distance[0]) > Robot_Ball_AcceptedDistance){
      Robot_State = 1;
      //return;
    }
    Serial.print("Delta_Direction[10]: ");
    Serial.print(Delta_Direction[10]);
    Serial.print(" <= ");
    Serial.print("Robot_Zone_AcceptedDirection: ");
    Serial.println(Robot_Zone_AcceptedDirection);

    if (abs(Delta_Direction[10]) <= Robot_Zone_AcceptedDirection){
      
      Robot_State = 3;
      //return;
    }
    else{
      if (Delta_Direction[10] > 0){
        Robot_Turn_Right_byDirection(Delta_Direction[10]);
      }
      else{
        Robot_Turn_Left_byDirection(Delta_Direction[10]);
      }
      //Robot_Turn_WithBall_1();
    }
  }

  else if (Robot_State == 3){
    Serial.println(Robot_State);
    Completed = sqrt(pow(Object_X[0] - BR_X,2) + pow(Object_Y[0] - BR_Y,2));
    Serial.print("this Complete: "); Serial.println(Completed);
     if ((Robot_State == 3)&&(sub_Robot_State == 99)){
       //Servo is open
       myservo0.write(180);

       robot_Backward(300, 1023.*forwardbackAccepted, 1023.*forwardbackAccepted);       
       //Robot_Move_Backward();
       Serial.print(abs(Distance[0]));
       Serial.print(" > ");
       Serial.println(Robot_Zone_AcceptedDistance + Subvalue_ServoAccepted);
       if ((Servo_State == 2)&&(abs(Distance[10]) > (Robot_Zone_AcceptedDistance))){
           //Servo is close
           Servo_State = 0;
           myservo0.write(90);

           sub_Robot_State = 0;
           catchTheBall = false;
           Robot_State = 4;
           //return;
           
       }
     }
     else{
       if (abs(Delta_Direction[0]) > Robot_Ball_AcceptedDirection){
         Robot_State = 0;
         //return;
       }
  
       if (abs(Distance[0]) > Robot_Ball_AcceptedDistance){
         Robot_State = 1;
         //return;
       }
  
       if (abs(Delta_Direction[10]) > Robot_Zone_AcceptedDirection){
         Robot_State = 2;
         //return;
       }
  
       //ball in Zone
  
       
       //if (abs(Distance[10]) <= Robot_Zone_AcceptedDistance){
       if (Completed <= Robot_Zone_AcceptedDistance){
        //Servo is open and drop the ball
          sub_Robot_State = 99;
       }
       
       else{         
         minDisIndex = findMinDistanceIndex(Robot_State);
         if (minDisIndex == -1){
           //Serial.println("Forward");
           Robot_Move_Forward_byDistance(Distance[10]);
         }
         else if((Robot_X != -1)&&(Robot_Y != -1)){        
           avoidance();
           //Robot_Turn_WithBall_1();
           Serial.println("Avoid Forward");
           Robot_Move_Forward_AvoidbyDistance(Distance[minDisIndex]);
         }         
         //Robot_MoveForward_WithBall_1();
       }
    }
     
  }

  //(2)Green Ball
  else if (Robot_State == 4){
    Serial.println(Robot_State);
    if((Object_X[0] == -1)&&(Object_Y[0] == -1)){
      Serial.println("TRUE0");
      if((Object_X[1] == -1)&&(Object_Y[1] == -1)){
          Serial.println("TRUE1");
          Robot_State = 8;
          return;
      }
      else{
          Serial.println("NOT TRUE1");
      }
    }
    if (abs(Delta_Direction[1]) <= Robot_Ball_AcceptedDirection){
      Robot_State = 5;
    }
    else{
      if (Delta_Direction[1] > 0){
        Robot_Turn_Right_byDirection(Delta_Direction[1]);
      }
      else{
        Robot_Turn_Left_byDirection(Delta_Direction[1]);
      }
    }
  }

  else if (Robot_State == 5){
    Serial.println(Robot_State);
    if (abs(Delta_Direction[1]) > Robot_Ball_AcceptedDirection){
      Robot_State = 4;
      //return;
    }
    
    if ((Servo_State == 0)&&(abs(Distance[1]) <= (Robot_Ball_AcceptedDistance + Subvalue_ServoAccepted))){
      //Servo is open
      myservo0.write(180);

      Servo_State = 1;
      
      
    }    
    check_catchTheBall(5);
    if ((Servo_State == 1)&&((catchTheBall)||(abs(Distance[1]) <= Robot_Ball_AcceptedDistance))){
    //if ((Servo_State == 1)&&(abs(Distance[1]) <= Robot_Ball_AcceptedDistance)){
      //Servo is close
      myservo0.write(90);

      Servo_State = 2;
      //Robot_State = 6;
    }
    if ((Servo_State == 2) && (abs(Distance[1]) <= Robot_Ball_AcceptedDistance)){
      check_catchTheBall(5);
      if(catchTheBall){
        Robot_State = 6;
      }
      else{
        Robot_State = 4;
      }
      
      //return;
    }
    else{
      minDisIndex = findMinDistanceIndex(Robot_State);
      if (minDisIndex == -1){
        Robot_Move_Forward_byDistance(Distance[1]);
      }
      else if((Robot_X != -1)&&(Robot_Y != -1)){        
        avoidance();
        Serial.println("Avoid forward");
        Robot_Move_Forward_AvoidbyDistance(Distance[minDisIndex]);
      }
   }
  }

  else if (Robot_State == 6){
    Serial.println(Robot_State);
    if (abs(Delta_Direction[1]) > Robot_Ball_AcceptedDirection){
      Robot_State = 4;
      //return;
    }

    if (abs(Distance[1]) > Robot_Ball_AcceptedDistance){
      Robot_State = 5;
      //return;
    }

    if (abs(Delta_Direction[11]) <= Robot_Zone_AcceptedDirection){
      
      Robot_State = 7;
      //return;
    }
    else{
      if (Delta_Direction[11] > 0){
        Robot_Turn_Right_byDirection(Delta_Direction[11]);
      }
      else{
        Robot_Turn_Left_byDirection(Delta_Direction[11]);
      }
      //Robot_Turn_WithBall_2();
    }
  }

  else if (Robot_State == 7){
    Serial.println(Robot_State);
    Completed = sqrt(pow(Object_X[1] - TR_X,2) + pow(Object_Y[1] - TR_Y,2));
    Serial.print("this Complete: "); Serial.println(Completed);
    if ((Robot_State == 7)&&(sub_Robot_State == 99)){
       //Servo is open
       myservo0.write(180);

       
       robot_Backward(300, 1023.*forwardbackAccepted, 1023.*forwardbackAccepted);       
       //Robot_Move_Backward();
       Serial.print(abs(Distance[1]));
       Serial.print(" > ");
       Serial.println(Robot_Zone_AcceptedDistance + Subvalue_ServoAccepted);
       if ((Servo_State == 2)&&(abs(Distance[11]) > (Robot_Zone_AcceptedDistance))){
           //Servo is close
           Servo_State = 0;
           myservo0.write(90);

           sub_Robot_State = 0;
           catchTheBall = false;
           Robot_State = 8;
           //return;
           
       }
     }
     else{
       if (abs(Delta_Direction[1]) > Robot_Ball_AcceptedDirection){
         Robot_State = 4;
         //return;
       }
  
       if (abs(Distance[1]) > Robot_Ball_AcceptedDistance){
         Robot_State = 5;
         //return;
       }
  
       if (abs(Delta_Direction[11]) > Robot_Zone_AcceptedDirection){
         Robot_State = 6;
         //return;
       }
       //ball in Zone
       
       /*if (abs(Distance[11]) <= Robot_Zone_AcceptedDistance){
       if (Completed <= Robot_Zone_AcceptedDistance){
         
         Robot_State = 8;
         //return;
         //Robot_Move_Backward();
       }*/
       Serial.print("This is Complte");
       Serial.print("Robot_Zone_AcceptedDistance");
       Serial.print(Completed);
       Serial.print(" <= ");
       Serial.println(Robot_Zone_AcceptedDistance);
       if (Completed <= Robot_Zone_AcceptedDistance){
        //Servo is open and drop the ball
          sub_Robot_State = 99;
          //return;
       }
       else{
         minDisIndex = findMinDistanceIndex(Robot_State);
         if (minDisIndex == -1){
           Robot_Move_Forward_byDistance(Distance[11]);
         }
         else if((Robot_X != -1)&&(Robot_Y != -1)){        
           avoidance();
           //Robot_Turn_WithBall_2();
           Serial.println("Avoid forward");
           Robot_Move_Forward_AvoidbyDistance(Distance[minDisIndex]);
         }
         //Robot_MoveForward_WithBall_2();
      }
     }

  }
  //(3) Blue Ball
  else if (Robot_State == 8){
    Serial.println(Robot_State);
     if (abs(Delta_Direction[2]) <= Robot_Ball_AcceptedDirection){
       Robot_State = 9;
       //return;
     }
     else{
       if (Delta_Direction[2] > 0){
         Robot_Turn_Right_byDirection(Delta_Direction[2]);
       }
       else{
         Robot_Turn_Left_byDirection(Delta_Direction[2]);
       }
     }
  }

  else if (Robot_State == 9){
    Serial.println(Robot_State);
     if (abs(Delta_Direction[2]) > Robot_Ball_AcceptedDirection){
       Robot_State = 8;
       //return;
     }

     if (abs(Distance[2]) <= Robot_Ball_AcceptedDistance){
       Robot_State = 10;
       //return;
     }
     
     if ((Servo_State == 0)&&(abs(Distance[2]) <= (Robot_Ball_AcceptedDistance + Subvalue_ServoAccepted))){
       //Servo is open
       myservo0.write(180);

       Servo_State = 1;
      
     }    
     check_catchTheBall(9);
     if ((Servo_State == 1)&&((catchTheBall)||(abs(Distance[2]) <= Robot_Ball_AcceptedDistance))){
     //if ((Servo_State == 1)&&(abs(Distance[2]) <= Robot_Ball_AcceptedDistance )){
       //Servo is close
       myservo0.write(90);
       Servo_State = 2;
       //Robot_State = 2;
     }
     if ((Servo_State == 2) && (abs(Distance[2]) <= Robot_Ball_AcceptedDistance)){
       check_catchTheBall(9);
       if(catchTheBall){
        Robot_State = 10;
       }
       else{
        Robot_State = 8;
       }
     }
     else{
       minDisIndex = findMinDistanceIndex(Robot_State);
       if (minDisIndex == -1){
         Robot_Move_Forward_byDistance(Distance[2]);
       }
       else if((Robot_X != -1)&&(Robot_Y != -1)){        
           avoidance();
         Serial.println("Avoid forward");
         Robot_Move_Forward_AvoidbyDistance(Distance[minDisIndex]);
       }
     }
  }

  else if (Robot_State == 10){
    Serial.println(Robot_State);
     if (abs(Delta_Direction[2]) > Robot_Ball_AcceptedDirection){
       Robot_State = 8;
       //return;
     }

     if (abs(Distance[2]) > Robot_Ball_AcceptedDistance){
       Robot_State = 9;
       //return;
     }

     if (abs(Delta_Direction[12]) <= Robot_Zone_AcceptedDirection){
       
       Robot_State = 11;
       //return;
     }
     else{
       if (Delta_Direction[12] > 0){
         Robot_Turn_Right_byDirection(Delta_Direction[12]);
       }
       else{
         Robot_Turn_Left_byDirection(Delta_Direction[12]);
       }
       //Robot_Turn_WithBall_3();
     }
  }

  else if (Robot_State == 11){
    
    Serial.println(Robot_State);
    Completed = sqrt(pow(Object_X[2] - TL_X,2) + pow(Object_Y[2] - TL_Y,2));
    Serial.print("this Complete: "); Serial.println(Completed);
     if ((Robot_State == 11)&&(sub_Robot_State == 99)){
       //Servo is open
       myservo0.write(180);

       robot_Backward(300, 1023.*forwardbackAccepted, 1023.*forwardbackAccepted);       
       //Robot_Move_Backward();
       Serial.print(abs(Distance[2]));
       Serial.print(" > ");
       Serial.println(Robot_Zone_AcceptedDistance + Subvalue_ServoAccepted);
       if ((Servo_State == 2)&&(abs(Distance[12]) > (Robot_Zone_AcceptedDistance))){
           //Servo is close
           Servo_State = 0;
           myservo0.write(90);

           sub_Robot_State = 0;
           catchTheBall = false;
           Robot_State = 12;
           
           //return;
           
       }
     }
     else{
       if (abs(Delta_Direction[2]) > Robot_Ball_AcceptedDirection){
         Robot_State = 8;
         //return;
       }
  
       if (abs(Distance[2]) > Robot_Ball_AcceptedDistance){
         Robot_State = 9;
         //return;
       }
  
       if (abs(Delta_Direction[12]) > Robot_Zone_AcceptedDirection){
         Robot_State = 10;
         //return;
       }
      
       /*if (abs(Distance[12]) <= Robot_Zone_AcceptedDistance){
      if (Completed <= Robot_Zone_AcceptedDistance){
        
        Robot_State = 12;
        return;
        //Robot_Move_Backward();
      }*/

       if (Completed <= Robot_Zone_AcceptedDistance){
          //Servo is open and drop the ball
            sub_Robot_State = 99;
       }
       else{
          minDisIndex = findMinDistanceIndex(Robot_State);
          if (minDisIndex == -1){
            Robot_Move_Forward_byDistance(Distance[12]);
          }
         else if((Robot_X != -1)&&(Robot_Y != -1)){        
           avoidance();
            //Robot_Turn_WithBall_3();
            Serial.println("Avoid forward");
            Robot_Move_Forward_AvoidbyDistance(Distance[minDisIndex]);
          }
        //Robot_MoveForward_WithBall_3();
       }
     }

  }

  else if (Robot_State == 12){
    Serial.println(Robot_State);
    if (abs(Delta_Direction[9]) <= Robot_Zone_AcceptedDirection){
       Robot_State = 13;
       //return;
    }
    else{
       if (Delta_Direction[9] > 0){
        Robot_Turn_Right_byDirection(Delta_Direction[9]);
      }
       else{
        Robot_Turn_Left_byDirection(Delta_Direction[9]);
      }         
    }
  }

  else if (Robot_State == 13){
    Serial.println(Robot_State);
    Completed = sqrt(pow(Robot_X - BL_X,2) + pow(Robot_Y - BL_Y,2));
    Serial.print("this Complete: "); Serial.println(Completed);
    if (abs(Delta_Direction[9]) > Robot_Zone_AcceptedDirection){
      Robot_State = 12;
      //return;
    }

    //if (abs(Distance[9]) <= Robot_Zone_AcceptedDistance){
    if (Completed <= Robot_Zone_AcceptedDistance){
      
      Robot_State = 14;
      //return;
    }
    else{
      minDisIndex = findMinDistanceIndex(Robot_State);
      if (minDisIndex == -1){
        Robot_Move_Forward_byDistance(Distance[9]);
      }
      else if((Robot_X != -1)&&(Robot_Y != -1)){        
        avoidance();
        Serial.println("Avoid forward");
        Robot_Move_Forward_AvoidbyDistance(Distance[minDisIndex]);
      }
    }
  }
  
}


int findMinDistanceIndex(int state){
  float minDis = 10000;
  int minIndex = -1;
  int iInitial = 0;
  if (state < 4){
    iInitial = 1;
  }
  else if (state < 8){
    iInitial = 2;
  }
  else if (state < 12){
    iInitial = 3;
  }

  for (int i = iInitial; i < 9; i++){
     if((Object_X[i] >0) && (Object_Y[i] > 0)){
       if (Distance[i] < minDis){
          minDis = Distance[i];
          minIndex = i;
       }

     }       
  }


  Serial.print("minDisIndex0: ");
  Serial.println(minIndex);
  if (minIndex != -1){
    if ((Distance[minIndex] <= Robot_AvoidanceDistance)&&
          (Delta_Direction[minIndex] <= Robot_AvoidanceDirection)){
            Serial.print("Distance[minIndex]: ");
            Serial.print(Distance[minIndex]);
            Serial.print(" <= ");
            Serial.print("Robot_AvoidanceDistance: ");
            Serial.println(Robot_AvoidanceDistance);
            Serial.println(" && ");
            Serial.print("Delta_Direction[minIndex]: ");
            Serial.print(Delta_Direction[minIndex]);
            Serial.print(" <= ");
            Serial.print("Robot_AvoidanceDirection: ");
            Serial.println(Robot_AvoidanceDirection);
      return minIndex;
    }
    else{
      return -1;
    }
  }
  Serial.print("minDisIndex1: ");
  Serial.print(minIndex);
  return minIndex;
}

void avoidance(){
  if (Delta_Direction[minDisIndex] > 0){
    Robot_Turn_Left_AvoidbyDirection(Delta_Direction[minDisIndex]);
  }
  else{
    Robot_Turn_Right_AvoidbyDirection(Delta_Direction[minDisIndex]);
  }
}

void Robot_Turn_Left_AvoidbyDirection(float Direction){
  
  robot_turnLeft(Direction, 1023.*lrFullAccepted, 1023.*lrHalfAccepted);

}



void Robot_Turn_Right_AvoidbyDirection(float Direction){
  robot_turnRight(Direction, 1023.*lrHalfAccepted, 1023.*lrFullAccepted);

}

void Robot_Move_Forward_AvoidbyDistance(float Distance){
  robot_Forward(Distance, 1023.*forwardbackAccepted, 1023.*forwardbackAccepted);

}

void Robot_Move_Forward_byDistance(float Distance){
  robot_Forward(Distance, 1023.*forwardbackAccepted, 1023.*forwardbackAccepted);
}

void Robot_Turn_Right_byDirection(float Direction){
  robot_turnRight(Direction, 1023.*lrHalfAccepted, 1023.*lrFullAccepted);
}

void Robot_Turn_Left_byDirection(float Direction){
  robot_turnLeft(Direction, 1023.*lrFullAccepted, 1023.*lrHalfAccepted);
}

void robot_Stop(float Delay){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(Delay);
}

void robot_turnLeft(float delta_Direc, float speedMotor1, float speedMotor2){
//  robot_Stop(250);
//  Serial.print("motor1: ");
//  Serial.println((int)speedMotor1);
//  Serial.print("motor2: ");
//  Serial.println((int)speedMotor2);
  int Delay;
  if((delta_Direc >= -40) && (delta_Direc >= 40)){
    Delay = 50;
  }
  else{
    Delay = 150;
  }
  Serial.print("Delay: ");
  Serial.println(Delay);
  analogWrite(EN1, (int)speedMotor1); // Run in half speed
  analogWrite(EN2, (int)speedMotor2); // Run in full speed
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(Delay);
  robot_Stop(150);
  
}


void robot_turnRight(float delta_Direc, float speedMotor1, float speedMotor2){
//  robot_Stop(250);
//  Serial.print("motor1: ");
//  Serial.println((int)speedMotor1);
//  Serial.print("motor2: ");
//  Serial.println((int)speedMotor2);
  int Delay;
  if((delta_Direc >= -40) && (delta_Direc <= 40)){
    Delay = 50;
  }
  else{
    Delay = 150;
  }
  Serial.print("Delay: ");
  Serial.println(Delay);
  analogWrite(EN1, (int)speedMotor1); // Run in half speed
  analogWrite(EN2, (int)speedMotor2); // Run in full speed
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(Delay);
  robot_Stop(150);
  
}

void robot_Forward(int Distance, float speedMotor1, float speedMotor2){
  //robot_Stop(100);
  int Delay;
  if(Distance < 10){
    Delay = 80;
  }
  else if(Distance < 15){
    Delay = 100;
  }
  else{
    Delay = 400;
  }
  analogWrite(EN1, (int)speedMotor1);
  analogWrite(EN2, (int)speedMotor2);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(Delay);
  robot_Stop(100);
  
}

void robot_Backward(int Delay, float speedMotor1, float speedMotor2){
  //robot_Stop(100);
  analogWrite(EN1, (int)speedMotor1);
  analogWrite(EN2, (int)speedMotor2);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(Delay);
  robot_Stop(100);
}

//Servo catch the ball
//void ServoCatch(int Delay){
//  Serial.println("This is catch the ball");
//  myservo.write(180); // servo splay 180 degree
//  delay(Delay);
//  myservo.write(90);
//}


void check_catchTheBall(int state){
  int ultra_Distance = my_ultra.distanceInMillimeters();
  Serial.println(ultra_Distance);
  delay(50);
  if(ultra_Distance > 0 && ultra_Distance <=300){
    if ((ultra_Distance >= min_ultra_Distance)&&(ultra_Distance <= max_ultra_Distance)){
      catchTheBall = true;
    }
    else{
      catchTheBall = false;
    }
  }

}


//this function find the distance between balls, Zone

void getDistance(){
  for(byte i = 0; i < n_object; i++){
    if(Object_X[i] == -1 and Object_Y[i] == -1){
      Distance[i] = -1;
    }
    else{
      Distance[i] = sqrt(pow(Object_X[i] - Robot_X,2) + pow(Object_Y[i] - Robot_Y,2));
    }
    
  }
}

void getDirection(){
  for(byte i = 0; i< n_object; i++){
    if(Object_X[i] == -1 and Object_Y[i] == -1){
      Direction[i] = 361;
    }
    else{
      float value = degrees(atan2((Object_Y[i]-Robot_Y), (Object_X[i]-Robot_X)));
      if(value<0){
        //360 is value that make the degree is positive (0-2PI)
        value += 360;
      }
      else{
        value = value;
      }
      Direction[i] = value;

      Delta_Direction[i] = Robot_H - Direction[i];

      if(Delta_Direction[i] <= -180){
        Delta_Direction[i] += 360;
      }
//      if(Delta_Direction[i] >= 180){
//        Delta_Direction[i] = 360 - Delta_Direction[i];
//      }
      if(Delta_Direction[i] == Robot_H){
        Delta_Direction[i] = 0;
      }
   }
      
  }
}
void messageSplitToPair(String msgInput)
{
    Serial.println(msgInput);
    msgInput.replace("\n","");
    char buf[msgInput.length()+1];
    msgInput.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *str;
    byte i = 0;
    while ((str = strtok_r(p, ";", &p)) != NULL) // delimiter is the semicolon
    {
      //Serial.println(str);
      
      if(i==0)
      {
        strTripleToValue(str);
      }
      else
      {
        strPairToValue(str,i-1);
      }
      
      i=i+1;
    }
    BL_X = Object_X[9];
    BR_X = Object_X[10];
    TR_X = Object_X[11];
    TL_X = Object_X[12];
    
    BL_Y = Object_Y[9];
    BR_Y = Object_Y[10];
    TR_Y = Object_Y[11];
    TL_Y = Object_Y[12];
    

    //Fix bug position of Ball when the Robot is catching the ball
    if((Servo_State == 2)&&(Robot_State == 0 || Robot_State == 1 || Robot_State == 2 || Robot_State ==3)){
      Object_X[0] = Robot_X;
      Object_Y[0] = Robot_Y;
      Serial.print("(");
      Serial.print(Robot_X);
      Serial.print(",");
      Serial.print(Robot_Y);
      Serial.println(")");
      Serial.print("(");
      Serial.print(Object_X[0]);
      Serial.print(",");
      Serial.print(Object_Y[0]);
      Serial.println(")");

      Serial.println("Fixing the bug position ball state PART1");
    }
    if((Servo_State == 2)&&(Robot_State == 4 || Robot_State == 5 || Robot_State == 6 || Robot_State ==7)){
      Object_X[1] = Robot_X;
      Object_Y[1] = Robot_Y;
      Serial.println("Fixing the bug position ball state PART2");
    }
    if((Servo_State == 2)&&(Robot_State == 8 || Robot_State == 9 || Robot_State == 10 || Robot_State ==11)){
      Object_X[2] = Robot_X;
      Object_Y[2] = Robot_Y;
      Serial.println("Fixing the bug position ball state PART3");
    }
}
void strPairToValue(String strPair, byte index)
{
    strPair.replace("(","");
    strPair.replace(")","");
    char buf[strPair.length()+1];
    strPair.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *str;
    byte i=0;
    while ((str = strtok_r(p, ",", &p)) != NULL) // delimiter is the comma
    {
      //Serial.println(str);
      
      if(i==0)
      {
       
        Object_X[index] = atof(str);

        
      }
      else if(i==1)
      {
        
        Object_Y[index] = atof(str);
        
      }
      
      i=i+1;
    }
}
void strTripleToValue(String strTriple)
{
    strTriple.replace("(","");
    strTriple.replace(")","");
    char buf[strTriple.length()+1];
    strTriple.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *str;
    byte i = 0;
    while ((str = strtok_r(p, ",", &p)) != NULL) // delimiter is the comma
    {
      //Serial.println(str);
      
      if(i == 0)
      {
        Robot_H = atof(str);
      }
      else if(i == 1)
      {
        Robot_X = atof(str);
      } 
      else if(i == 2)
      {
        Robot_Y = atof(str);
      }
      i=i+1;
    }
}
