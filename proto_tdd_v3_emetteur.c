/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v0                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */

void emettreEtAttendre(paquet_t* paquetEntree, paquet_t* paquetSortie, int* service, int tempsAttente){
    int c = 0;
    do{
        vers_reseau(paquetSortie);
        depart_temporisateur(tempsAttente);
          
        c = attendre();

    }while(c != -1);
    de_reseau(paquetEntree);      
    arret_temporisateur();
    *service = paquetEntree->type;
    return;
}

int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquetS, paquetE; /* paquet utilisé par le protocole */

    int service = 0;

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application_mode_c(&service, message, &taille_msg);

    paquetS.num_seq = 0;
    /*
    on envoie le paquet de connection
    et on attend la réponse, si c'est negatif on arrete 
    le programme
    */
    if(service == T_CONNECT){
        paquetS.type = CON_REQ;
        paquetS.lg_info = 0;
        paquetS.somme_ctrl = checksum(&paquetS);
        emettreEtAttendre(&paquetE, &paquetS, &service, 100);

        if(paquetE.type == CON_ACCEPT){
            vers_application_mode_c(T_CONNECT_ACCEPT, NULL, 0);
        }else{
            vers_application_mode_c(T_CONNECT_REFUSE, NULL, 0);
            return 65536;
        }

    }

    de_application_mode_c(&service, message, &taille_msg);
    
    /*
    on traite la data similaire a la v2
    ne serais-ce qu'on change le service
    */

    while(service == T_DATA){
        
        paquetS.type = DATA;
        paquetS.lg_info = taille_msg;

        for (int i=0; i<taille_msg; i++) {
            paquetS.info[i] = message[i];
        }

        
        paquetS.somme_ctrl = checksum(&paquetS);
        emettreEtAttendre(&paquetE, &paquetS, &service, 100);
        paquetS.num_seq += 1;

        de_application_mode_c(&service, message, &taille_msg);
    }

    /*
    on disconnect on attend le paquet de retour et s'il est bon
    on peut fermer l'appli
    */
    if(service == T_DISCONNECT){
        paquetS.type = CON_CLOSE;
        paquetS.lg_info = 0;
        paquetS.somme_ctrl = checksum(&paquetS);
        emettreEtAttendre(&paquetE, &paquetS, &service, 100);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
