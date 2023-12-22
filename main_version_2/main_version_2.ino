/**
 * Email : openprogramming23@gmail.com
 * Date : 30/10/2023
 * 
 * Auteur : Exaucé KIMBEMBE
 * 
 * @BOARD : ARDUINO
*/

// Vérification de la carte Arduino 
#ifndef __AVR__
	#error "Ce programme a été tester sur une carte Arduino"
#endif

#include <Servo.h>
#include <LiquidCrystal_I2C.h>

#define COL 16 // Nombre de colonne de l'écran
#define ROW 2  // Nombre de ligne de l'écran
// Valeur de temporisation après détection de couleur
#define TEMPOS_1  2000 // Tempos (ms) avant déclanchement du servomoteur 1
#define TEMPOS_2  3000 // Tempos (ms) avant déclanchement du servomoteur 2

// Fréquence des couleurs possible rechercher dans la programme
#define C_BLEU(r,g,b)  ((r>=12 and r<=15) and (g>=14 and g<=17) and (b>=18 and b<=24))  
#define C_ROUGE(r,g,b) ((r>=20 and r<=36) and (g>=29 and g<=34) and (b>=23 and b<=43))
#define C_VERT(r,g,b)  ((r>=2 and r<=7) and (g>=1 and g<=6) and (b>=2 and b<=6))
#define C_JAUNE(r,g,b) ((r>=2 and r<=7) and (g>=1 and g<=6) and (b>=2 and b<=6))
// Valeur de retour après détection de la couleur
#define VAL_BLEU  1 
#define VAL_ROUGE 2 
#define VAL_VERT  3 
#define VAL_JAUNE 4 
// Choix couleur a triée
#define CHOIX_1  "-> BLEU -- ROUGE"
#define CHOIX_2  "-> BLEU -- JAUNE"
#define CHOIX_3  "-> VERT -- JAUNE"
#define CHOIX_4  "-> VERT -- ROUGE"
String tab_choix_couleur[4]={CHOIX_1, CHOIX_2, CHOIX_3, CHOIX_4};
uint8_t index_tab_color = 0; // Choix par défaut
uint8_t choix_coleur_1 = (int) VAL_BLEU; // Choix par défaut
uint8_t choix_coleur_2 = (int) VAL_ROUGE; // Choix par défaut

// Pins utilisés
// --- Capteur TCS3200
#define S0       3
#define S1       4
#define S2       5
#define S3       6
#define output   7

// Pins moteur cc (driver L298N)
#define PIN_IN3 11 
#define PIN_IN4 12  

#define PIN_BNT_O_F  2  // Bouton marche/arrêt
#define PIN_BNT_OK   13 // Bouton OK
#define PIN_BNT_UP   A0 // Bouton up 
#define PIN_BNT_DOWN A1 // Buton  down

#define PIN_SERVO_1 A2 // Servomoteur 1 
#define PIN_SERVO_2 A3 // Servomoteur 2

// Paramètres angles des servomoteurs
#define ANGLE_NEUTRE 180
#define ANGLE_SORTIE 120
#define SORTIE     true // Permet de choisir l'angle du servomoteur

// Paramètres du moteur pas à pas
#define VITESSE_MAX 120  // 120tr/min vitesse du moteur avec couple faible (Vitesse de configuration)
#define VITESSE_MIN 30   
#define PAS 200  		// Nombre de pas pour effectuer un tour complet
#define SENS_H     true // Rotation dans le sens horaire

uint8_t vitesse = VITESSE_MAX;   // vitesse de rotation des tubes

// Menu de demarrage
#define ACCEUIL_l1	"    WELCOME     "
#define ACCEUIL_l2 	"OPENPROGRAMMING "

// Message 
#define MSG_1 	"MODE DE CONTROLE"
#define MSG_2 	"-> AUTOMATIQUE  "
#define MSG_3 	"-> MANUEL       "

// Menu de selection
#define MENU_1 	"-> TRI COULEUR  "
#define MENU_2 	"-> CHOIX COULEUR"
#define MENU_3  "-> PIECES TRIEE "
#define MENU_4  "-> MODE AUTO/MAN"

String tab_menu[4]={ MENU_1, MENU_2, MENU_3, MENU_4};

bool auto_mode    = true; // Variable d'activation du mode manuel ou automatique
bool state_moteur = true;  // Indique si le moteur est en marche ou pas
uint8_t index_option = 0;  // Index de l'option a afficher
uint8_t tab_af[2] = {0,1}; // Tableau contenant l'index du menu à afficher
uint8_t ligne_cursor = 0;  // Position du curseur sur l'ecran 
int Nbre_piece_1 = 0;   // Nombre de pièce trié de couleur 1 
int Nbre_piece_2 = 0;   // Nombre de pièce trié de couleur 2

#define DELAY 3000 // Pause système setup() (ms)
#define ATTENTE 10 // Temps d'incrémentation ou de décrémentation de l'angle du servomoteur

LiquidCrystal_I2C lcd(0x27,COL,ROW);  // Objet écran lcd
Servo servo_1; // Objet servomoteur 1
Servo servo_2; // Objet servomoteur 2

void menu(String *tab);
bool detectionAppui(uint8_t pin);
void gestionBouton_O_F(void); // Utiliser dans l'intérruption
void gestionBouton_OK(void);
void gestionBouton_UP(void);
void gestionBouton_DOWN(void);
void actionnerServo(const bool etat); // Permet de sortir et de rentrer le servomoteur
void revolution(void); // Permet la rotation du moteur
void stopRevolution(void); //Permet l'arrêt du moteur 
int colorRead(char color, int echelle=20);// Lecture de la couleur capteur (TCS3200) echelle 20%
uint8_t detecteCouleur(void);// Detecte la fréquence de couleur rechercher
void setCmdMode(void){auto_mode = not auto_mode;} // Permet de changer de mode de commande
void getCmdMode(void); // Permet d'afficher le mode de commande

void setup(){
	Serial.begin(115200);//Initialisation de la communication série
	// Configuration de l'ecran lcd I2C
	lcd.init();
	lcd.backlight();
	lcd.blink();
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(ACCEUIL_l1);
	lcd.setCursor(0,1);
	lcd.print(ACCEUIL_l2);	

	// Configuration du capteur TCS3202
	pinMode(S0, OUTPUT);
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
	pinMode(output, INPUT);

	// Configuration des boutons de réglage
	pinMode(PIN_BNT_O_F, INPUT_PULLUP);
	pinMode(PIN_BNT_OK,  INPUT_PULLUP);
	pinMode(PIN_BNT_UP,  INPUT_PULLUP);
	pinMode(PIN_BNT_DOWN,INPUT_PULLUP);

	// Configuration du moteur 
	pinMode(PIN_IN3, OUTPUT);
	pinMode(PIN_IN4, OUTPUT);

	// Configuration des servomoteurs 
	servo_1.attach(PIN_SERVO_1);
	servo_2.attach(PIN_SERVO_2);
	servo_1.write(ANGLE_NEUTRE);
	servo_2.write(ANGLE_NEUTRE);

	// Configuration de l'intérruption de changement d'état du moteur
	attachInterrupt(digitalPinToInterrupt(PIN_BNT_O_F), gestionBouton_O_F, FALLING);

	delay(DELAY);

	getCmdMode();

	lcd.setCursor(0,0);
	lcd.print(tab_menu[tab_af[0]]);
	lcd.setCursor(0,1);
	lcd.print(tab_menu[tab_af[1]]);
}

void loop(){
	menu(tab_menu);// Affichage du menu
	// Ne s'exécute que si le moteur ou le tapis est en arrêt 
	if(state_moteur==false){
		gestionBouton_OK();//Permet de lancer l"option selectionner
		gestionBouton_DOWN();
		gestionBouton_UP();
	}
	
	// Mode de commande manuel
	if(auto_mode == false){
		// Mise en marche du moteur, l'extinction se fait dans l'interruption
		if(state_moteur==true){
			revolution();
			if(detectionAppui(PIN_BNT_UP)){
				stopRevolution();
				actionnerServo(servo_1, SORTIE);
				actionnerServo(servo_1, not SORTIE);
				++Nbre_piece_1;
				revolution();
			}
			if(detectionAppui(PIN_BNT_DOWN)){
				stopRevolution();
				actionnerServo(servo_2, SORTIE);
				actionnerServo(servo_2, not SORTIE);
				++Nbre_piece_2;
				revolution();
			}
		}
	}
	// Mode de commande automatique
	else{
		static unsigned long _init_time = millis();
		static unsigned long _init_time_1 = millis();

		// Mise en marche du moteur, l'extinction se fait dans l'interruption
		if(state_moteur==true){
			revolution();
			
			if((millis()-_init_time_1) >= 1500){

				uint8_t value = detecteCouleur();
				Serial.println(value);
				
				// Détection automatique de la première couleur
				if(value == choix_coleur_1){
					_init_time = millis();
					while(true){ 
						revolution();
						if((millis()-_init_time) >= TEMPOS_1){
							stopRevolution();
							actionnerServo(servo_1, SORTIE);
							actionnerServo(servo_1, not SORTIE);
							Serial.println("ICI 1");
							++Nbre_piece_1;
							return;
						}
					}
				}
				
				// Détection automatique de la deuxième couleur
				if(value == choix_coleur_2){
					_init_time = millis();
					while(true){ 
						revolution();
						if((millis()-_init_time) >= TEMPOS_2){
							stopRevolution();
							actionnerServo(servo_2, SORTIE);
							actionnerServo(servo_2, not SORTIE);
							Serial.println("ICI 2");
							++Nbre_piece_2;
							return;
						}
					}
				}
			}
		}// fin if condition moteur en marche
	}// fin else
}

//---------------- Gestion des boutons
void gestionBouton_O_F(void){
	// changement d'état de la variable
	state_moteur = not state_moteur;
	// Arrêt du moteur
	if(state_moteur==false)
		stopRevolution();
}

// Permet de lancer l"option selectionner
void gestionBouton_OK(void){
	if (detectionAppui(PIN_BNT_OK)==true){
		Serial.println("Le bouton ok est cliqué");

		// Affiche les couleurs actuellement entrain d'être triée
		if(index_option==0){
			while(true){
				lcd.setCursor(0,0);
				lcd.print("COULEURS CHOISIE");
				lcd.setCursor(0,1);
				lcd.print(tab_choix_couleur[index_tab_color]);

				if (detectionAppui(PIN_BNT_OK)==true)
					break;
			}
		}
		// Permet de selectionner les couleurs a trier
		if(index_option==1){
			index_option=0;
			while(true){
				gestionBouton_DOWN();
				gestionBouton_UP();
				menu(tab_choix_couleur);
				if (detectionAppui(PIN_BNT_OK)==true)
					break;
			}
			Nbre_piece_1 = 0;
			Nbre_piece_2 = 0;
			index_tab_color = index_option;

			// Mise à jour des couleurs a détecter
			// Choix couleur a triée
			#define CHOIX_1  "-> BLEU -- ROUGE"
			#define CHOIX_2  "-> BLEU -- JAUNE"
			#define CHOIX_3  "-> VERT -- JAUNE"
			#define CHOIX_4  "-> VERT -- ROUGE"
			if(index_tab_color==0){
				choix_coleur_1 = VAL_BLEU;
				choix_coleur_2 = VAL_ROUGE;
			}
			else if(index_tab_color==1){
				choix_coleur_1 = VAL_BLEU;
				choix_coleur_2 = VAL_JAUNE;
			}
			else if(index_tab_color==2){
				choix_coleur_1 = VAL_VERT;
				choix_coleur_2 = VAL_JAUNE;
			}
			else{
				choix_coleur_1 = VAL_VERT;
				choix_coleur_2 = VAL_ROUGE;
			}

			index_option = 1; // Mise a jour de la valeur d'origine
		}
		// Affiche le nombre de pièce triée
		if(index_option==2){
			while(true){
				lcd.setCursor(0,0);
				lcd.print(MENU_3);
				lcd.setCursor(0,1);
				lcd.print("P1 = "+String(Nbre_piece_1)+" P2 = "+String(Nbre_piece_2)+"         ");

				if (detectionAppui(PIN_BNT_OK)==true)
					break;
			}
		}
		// Permet de changer de mode de commande
		else if(index_option==3){
			setCmdMode();
			getCmdMode();
		}
	}
}

// Chaque appui permet de décrémenté la variable pour aller ---> 0 (vers le haut)
void gestionBouton_DOWN(void){
	if (detectionAppui(PIN_BNT_UP)==true){
		if(index_option >= 1){
			--index_option;
			Serial.println(index_option);		
		}
	}
}

// Chaque appui permet d'incrémenté la variable pour aller ---> index + 1 (vers le bas)
void gestionBouton_UP(void){
	if (detectionAppui(PIN_BNT_DOWN)==true){
		if(index_option < ((int)(sizeof(tab_menu)/sizeof(tab_menu[0]))) - 1){
			++index_option;
			Serial.println(index_option);
		}
	}
}

// Fonction permettant de détecter un appui sur un bouton
bool detectionAppui(uint8_t pin){
	if(not digitalRead(pin)==true){
		delay(50);// Attente après chaque appui
		while(not digitalRead(pin)==true){}// Si appui maintenant on ne fait rien
		if(not digitalRead(pin)==false){
			return true;
		}
	}
	return false;
}

// Affiche le menu sur l'écran lcd en fonction 
void menu(String *tab){
	static unsigned long _init = millis();

	// Mise à jour des index dans le tableau a afficher
	if(index_option==0 or index_option==1){
		tab_af[0] = 0;
		tab_af[1] = 1;
	}
	else if(index_option==2 or index_option==3){
		tab_af[0] = 2;
		tab_af[1] = 3;
	}
	
	// Affiche le message à l'écran
	if((millis() - _init) >= 500){
		lcd.setCursor(0,0);
		lcd.print(tab[tab_af[0]]);
		lcd.setCursor(0,1);
		lcd.print(tab[tab_af[1]]);
		
		_init = millis();// initialisation du temps
	}

	// Indique la ligne active sur l'écran
	if(index_option==0 or index_option==2)
		lcd.setCursor(1,0);
	else
		lcd.setCursor(1,1);
}


void revolution(void){
	digitalWrite(PIN_IN3,LOW);
	digitalWrite(PIN_IN4,HIGH);
}


void stopRevolution(void){
	digitalWrite(PIN_IN3,LOW);
	digitalWrite(PIN_IN4,LOW);
}


// Permet de convertir les signaux du capteur TCS3202
int colorRead(char color, int echelle){

	long timeout = 1000; // temps en us

	switch (echelle){
		case 0:
			digitalWrite(S0, LOW); // Set echelle to 0%(echelle is turned OFF)
			digitalWrite(S1, LOW);
			break;

		case 2:
			digitalWrite(S0, LOW); // Set echelle to 2%
			digitalWrite(S1, HIGH);
			break;

		case 20: // Set echelle to 20%
			digitalWrite(S0, HIGH);
			digitalWrite(S1, LOW);
			break;

		case 100: // Set echelle to 100%
			digitalWrite(S0, HIGH);
			digitalWrite(S1, HIGH);
			break;

		default: // Set default echelle (default echelle is 20%)
			digitalWrite(S0, HIGH);
			digitalWrite(S1, LOW);
			break;
	}

	switch (color){
		case 'r': // Setting red filtered photodiodes to be read
			digitalWrite(S2, LOW);
			digitalWrite(S3, LOW);
			break;

		case 'b': // Setting blue filtered photodiodes to be read
			digitalWrite(S2, LOW);
			digitalWrite(S3, HIGH);
			break;

		case 'c': // Setting clear photodiodes(no filters on diodes) to be read
			digitalWrite(S2, HIGH);
			digitalWrite(S3, LOW);
			break;

		case 'g': // Setting green filtered photodiodes to be read
			digitalWrite(S2, HIGH);
			digitalWrite(S3, HIGH);
			break;

		default:
			digitalWrite(S2, HIGH);
			digitalWrite(S3, LOW);
			break;
	}

	unsigned long duration;

	duration = pulseIn(output, LOW, timeout);
	
	if (duration != 0)
		return 1000 / duration; // Reads and returns the frequency of selected color
	else
		return 0;
}

// Detecte la fréquence de couleur du liquide rechercher
uint8_t detecteCouleur(void){
	// Détection des différentes fréqences de couleurs
	int r = colorRead('r');
	int g = colorRead('g');
	int b = colorRead('b');

	Serial.println("\nr = " + String(r) + " g = " + String(g)+" b = " + String(b)+"\n");

	// Detection de la couleur bleu
	if(C_BLEU(r,g,b)){
		Serial.println("Couleur détecté : bleu");
		return VAL_BLEU;
	}
	// Detection de la couleur rouge
	if(C_ROUGE(r,g,b)){
		Serial.println("Couleur détecté : rouge");
		return VAL_ROUGE;
	}
	// Detection de la couleur vert
	if(C_VERT(r,g,b)){
		Serial.println("Couleur détecté : vert");
		return VAL_VERT;
	}
	// Detection de la couleur jaune
	if(C_JAUNE(r,g,b)){
		Serial.println("Couleur détecté : jaune");
		return VAL_JAUNE;
	}
	return 0;
}


void getCmdMode(void){
	if (auto_mode==true){
		lcd.setCursor(0,0);
		lcd.print(MSG_1);
		lcd.setCursor(0,1);
		lcd.print(MSG_2);
	}
	else{
		lcd.setCursor(0,0);
		lcd.print(MSG_1);
		lcd.setCursor(0,1);
		lcd.print(MSG_3);
	}
	delay((int)DELAY/2);
}

// Permet de sortir et de rentrer le servomoteur
void actionnerServo(Servo &servo , const bool etat){
  // Sortie
   if (etat == SORTIE){
    for (int i = servo.read(); i >= ANGLE_SORTIE; --i){
      servo.write(i);
      Serial.println("sortie");
      delay(ATTENTE);
    }
  }
  // Rentrée
  if(etat == not SORTIE){
    for (int i = servo.read(); i <= ANGLE_NEUTRE; ++i){
      servo.write(i);
      Serial.println("entrée");
      delay(ATTENTE);
    }
  }
}