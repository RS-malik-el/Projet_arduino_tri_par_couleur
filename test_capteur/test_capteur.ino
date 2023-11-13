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

// Pins utilisés
// --- Capteur TCS3200
#define S0       3
#define S1       4
#define S2       5
#define S3       6
#define output   7

int colorRead(char color, int echelle=20);// Lecture de la couleur capteur (TCS3200) echelle 20%
void detecteCouleur(void);// Detecte la fréquence de couleur rechercher

void setup(){
	Serial.begin(115200);//Initialisation de la communication série

	// Configuration du capteur TCS3202
	pinMode(S0, OUTPUT);
	pinMode(S1, OUTPUT);
	pinMode(S2, OUTPUT);
	pinMode(S3, OUTPUT);
	pinMode(output, INPUT);
}

void loop(){
	detecteCouleur();
	delay(1000);
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
void detecteCouleur(void){
	// Détection des différentes fréqences de couleurs
	int r = colorRead('r');
	int g = colorRead('g');
	int b = colorRead('b');

	Serial.println("\nr = " + String(r) + " g = " + String(g)+" b = " + String(b)+"\n");
}