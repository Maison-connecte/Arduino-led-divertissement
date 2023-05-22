//Auteur: Tom Pitou
//Date de création: 24 avril 2023
//Date de dernière modification: 21 mai 2023
//Description: Code permettant au microcontrôleur d'allumer, de modifier la couleur et l'intensité d'une bande DEL en fonction des données reçues par MQTT
//Le microcontrôleur se connecte au réseau WIFI, puis au broker MQTT et attend l'arrivée de données
//Dans le cas de l'allumage de la bande, il reçoit un "1" pour allumer et un "0" pour éteindre
//Les données qu'il reçoit pour la couleur sont un code RGB sous le format suivant: R/G/B
//Il décortique le message pour en ressortir les valeurs nécessaires et les envoie à la bande DEL


//Sources
//https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
//https://docs.arduino.cc/tutorials/uno-wifi-rev2/uno-wifi-r2-mqtt-device-to-device

#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <Adafruit_NeoPixel.h>

//Broche par laquel seront envoyées les données permettant de contrôler la LED
#define LED_PIN    6

// Nombre de LED que notre bande contient 
#define LED_COUNT 60

//Variable de la bande LED
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//Couleur qui sera affectée à la bande LED
uint32_t strip_color = strip.Color(255, 0, 255);

//SSID et mot de passe du réseau WiFi auxquel se connectera le Arduino
char ssid[] = "Externe";        
char mdp[] = "Education2523";    


//Variable de connexion au WiFi
WiFiClient wifiClient;
//Variable de connexion au client MQTT
MqttClient mqttClient(wifiClient);


const char broker[] = "test.mosquitto.org";
int        port     = 1883;
const char topic_allumer[]  = "allumer_led_divertissement";
const char topic_couleur[]  = "couleur_led_divertissement";

void setup() {
  //Initialisation du port série
  Serial.begin(9600);

  strip.begin();
  //Initialisation de la LED éteinte
  strip.show(); 
  //Intensité à 10
  strip.setBrightness(10);
  //Connexion au réseau WiFi
  Serial.print("Connexion au SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, mdp) != WL_CONNECTED) {
    // Échec
    Serial.print(".");
    delay(5000);
  }

  Serial.println("Connecté au réseau");
  Serial.println();

  Serial.print("Connexion au broker MQTT: ");
  Serial.println(broker);

  //Échec de connexion au broker MQTT
  if (!mqttClient.connect(broker, port)) {
    Serial.print("Échec de connexion au broker MQTT\nCode d'erreur = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("Vous êtes connecté au broker MQTT");
  Serial.println();

  //Retour du broker MQTT
  mqttClient.onMessage(messageMqttRecu);

  
  Serial.print("Abonnement au topic: ");
  Serial.println(topic_allumer);
  Serial.println(topic_couleur);

  Serial.println();

  //Abonnement au topic
  mqttClient.subscribe(topic_allumer);
  mqttClient.subscribe(topic_couleur);


  Serial.println();
}

void loop() {
  //Envoi de méssages "keep alive" pour éviter d'être déconnecté par le broker
  mqttClient.poll();
}

//Réception de message du broker MQTT
void messageMqttRecu(int messageTaille) {
  String topic_message=mqttClient.messageTopic();
  String msg="";
  //Lecture du message
  while (mqttClient.available()) {
    char val=(char)mqttClient.read();
    msg+=val;
    }
    //Message d'allumage
    if (topic_message.equals(topic_allumer)) {
      Serial.println("Messsage de changement d'état");
      Serial.println("Message : "+msg);
      bool on=!(msg.equals("0"));
      on_off(on);
    }
    
    //Message de changement de couleur
    else if (topic_message.equals(topic_couleur)) {
      Serial.println("Message de changement de couleur");
      Serial.println("Message : "+msg);
      String r=chercherValeur(msg, '/', 0);
      String g=chercherValeur(msg, '/', 1);
      String b=chercherValeur(msg, '/', 2);
      strip_color=strip.Color(r.toInt(),g.toInt(),b.toInt());
      strip.fill(strip_color, 0, 59);
      strip.show();
    }
   
  Serial.println();
}

//Vérification de si la LED doit allumer
void on_off(bool on) {
  if(on==false)
  {
    strip.clear();
    strip.show();
    Serial.println("État : lumière éteinte");
  }
  else
  {
    strip.fill(strip_color, 0, 59);
    strip.show();
    Serial.println("État : lumière allumée");
  }
}

//Tri du message pour séparer les valeurs RGB
String chercherValeur(String donnee, char separateur, int index)
{
    int trouve = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = donnee.length() - 1;

    for (int i = 0; i <= maxIndex && trouve <= index; i++) {
        if (donnee.charAt(i) == separateur || i == maxIndex) {
            trouve++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return trouve > index ? donnee.substring(strIndex[0], strIndex[1]) : "";
}
