#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


/********************    Prototypes    *********************/

static void envoyerData(int sock, const char* dataToSend);
/* Envoie les données contenu dans la chaine de caractère dataToSend sur le socket sock deja initialise */

static void recevoirData(int sock, char* storeData, int tailleStoredata);
/* Recoit les donne sur le socket sock et les stockes dans le buffer storeData de taille tailleStoredata*/

void afficheMenu(void);

char* obtenirInfo(char* storeData);

/*******************************************************/



int main(int argc, char* argv[]) {
	SOCKET sock, flag_sock; /*equivaut:  int sock; */
	SOCKADDR_IN sock_con; /*information du socket (IP, et port)*/
	WSADATA WSAData;
	bool serveur = FALSE; /*determine l'etat client/serveur du processus*/
	bool menu = FALSE;
	char argument[7];
	char ip[15] = "127.0.0.1";
	int port = 3851;
	char dataRcv[1048576]; /*buffer de 1Mo qui contiendra les donnee recues*/
	char dataSnd[1024];
	int sock_size;
	char bufMenu[4];
	
	if(argc>=2) {
		sscanf(argv[1], "%s", argument);		
		if(strncmp(argument, "serveur", 7) == 0)
			serveur = TRUE;
			
		if(argc==3 && serveur)
			sscanf(argv[2], "%d", &port);
		
		else if(argc==3)
			sscanf(argv[2], "%s", ip);
			
		else if(argc==4) {
			sscanf(argv[2], "%s", ip);
			sscanf(argv[3], "%d", &port);
		}
	}

	if(WSAStartup(MAKEWORD(2,0), &WSAData) != 0) {
		printf("Le socket n'a pu s'initialiser");
		/*erreur d'initialisation*/
		return EXIT_FAILURE;
	}
	
	sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP); /*protocole TCP: envoie en mode connecte pour eviter de perde des paquets sur le reseau. protocole UDP: SOCK_DGRAM*/
	if(sock==INVALID_SOCKET) {
		printf("Impossible d'ouvrir le socket");
		/*le socket n'a pu se créer correctement*/
		return EXIT_FAILURE;
	}
	
	printf("Socket correctement initialise\n");
	
	
	
	/* **************************************  SERVEUR  *************************************************/
	if(serveur) {
		printf("Hello server ON. Ecoute sur le port %d\n",port);
		
		sock_con.sin_addr.s_addr = INADDR_ANY;
		sock_con.sin_family = AF_INET;
		sock_con.sin_port = htons(port);
		bind(sock, (SOCKADDR *)&sock_con, sizeof(sock_con));
		listen(sock, 5); /*le serveur ecoute en permanence le port*/
		
		while(1) {
			sock_size = sizeof(sock_con);
			if((flag_sock = accept(sock, (SOCKADDR *)&sock_con, &sock_size)) != INVALID_SOCKET) {
				/*execute....*/
				recevoirData(flag_sock, dataRcv, 1048576);
			}
		}		
	}
	
	
	
	/***************************************   CLIENT   *************************************************/
	else {
		printf("Hello client ON. Serveur distant %s\n",ip);
			
		sock_con.sin_addr.S_un.S_addr = inet_addr(ip);
		sock_con.sin_family = AF_INET;
		sock_con.sin_port = htons(port);
		
		
		/*on commence l'ecriture du code a executer sur le socket ==> envoie d'information*/
		if(!menu) {
			if(connect(sock, (SOCKADDR *)&sock_con, sizeof(sock_con)) != 0) {
				/*Impossible de se connecter*/
				printf("Connexion impossible !\n");
			}
			else {
				printf("Connexion success.\n");
				envoyerData(sock, obtenirInfo(dataSnd));
			}
		}

		while(menu) {
			afficheMenu();
			if(fgets(bufMenu, 2, stdin) != NULL) { /*si pas d'echec lors de l'acquisition des donnes*/
			
				if(strncmp(bufMenu, "q", 1) == 0) { /*si l'utilisateur entre 'q'*/
					menu = FALSE;
				}
				
				else if(strncmp(bufMenu, "s", 2) == 0) { /* si l'utilisateur entre 's'*/
					if(connect(sock, (SOCKADDR *)&sock_con, sizeof(sock_con)) != 0) {
						/*Impossible de se connecter*/
						printf("Connexion impossible !\n");
					}
					else {
						printf("Connexion success.\n");
						envoyerData(sock, dataSnd);
						
					/* Revoir le fait que une fois la connection etablie il n'est possible d'envoyer qu'un seul flot de donnee et pas plusieurs. Necessite une reconnexion ?*/
					}
				}
			}
		}
			
   		/*END*/
	}
	shutdown(sock, 2); /*pour fermer le socket déjà ouvert*/
	printf("Connection interrompue.\n\n");
	return EXIT_SUCCESS;	
}


static void envoyerData(int sock, const char* dataToSend) {
	char accuseRcv[2];
	int tailleDataEnvoyee = send(sock, dataToSend, strlen(dataToSend), 0);
	if(tailleDataEnvoyee == strlen(dataToSend)) {
		printf("Donnees envoyees.......");
		
		if(recv(sock, accuseRcv, 2, 0) == 2) { /*accuse de reception*/
			printf("[OK]\n");
		}
		else {
			printf("[FAILED]\n");
		}
	}
	else {
		printf("Echec a la transmission des donnees\n\t'%s'",dataToSend);
	}
}

static void recevoirData(int sock, char* storeData, int tailleStoreData) {
	time_t Tsec;
	struct tm *Hrecep;
	int tailleDataRecues = recv(sock, storeData, tailleStoreData, 0);
	
	Tsec = time(NULL); /*pour recuperer le nombre de secondes depuis le 1/1/1970 de l'heure actuelle*/
	Hrecep = gmtime(&Tsec);/*pour formatter l'heure dans la structure time_t*/
	
	
	printf("Reception en cours a %dh %dm %ds..........",Hrecep->tm_hour,Hrecep->tm_min,Hrecep->tm_sec);
	if(tailleDataRecues>0) { /*si on a recu quelque chose..*/
		storeData[tailleDataRecues] = '\0';
		printf("[%d octets recus]\n\t%s\n",tailleDataRecues,storeData);
		send(sock, "ok", 2, 0);
	}
	else {
		printf("[FAILED]\n\tEchec a la reception des donnees ou aucun donnees envoyees\n");
	}
}

void afficheMenu(void) {
	printf("'q' pour quitter et 's' pour tester l'envoie de donnee:\t");
}

char* obtenirInfo(char* storeData) {
	char *clientName;
	char* computerName;
	
	clientName = getenv("USERNAME");
	computerName = getenv("COMPUTERNAME");
	
	sprintf(storeData,"Nom du client: %s\r\n\tNom de l'ordinateur: %s",clientName,computerName);
	return storeData;
}

