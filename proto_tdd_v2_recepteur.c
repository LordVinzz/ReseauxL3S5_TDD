/*************************************************************
* proto_tdd_v0 -  récepteur                                  *
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
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; /* paquet utilisé par le protocole */
    int fin = 0; /* condition d'arrêt */
    int num = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);

        /* extraction des donnees du paquet recu */
        for (int i=0; i<paquet.lg_info; i++) {
            message[i] = paquet.info[i];
        }
        /* remise des données à la couche application */

        // si la checksum est bonne et qu'on a le bon numero de seq,
        // on build ne nouveau packet et on l'envoie
        // sinon on renvoie l'ancien
        if(paquet.somme_ctrl == checksum(&paquet) && paquet.num_seq == num){
            paquet.type = ACK;
            num ^= 1;
            fin = vers_application(message, paquet.lg_info);
        }
        
        vers_reseau(&paquet);
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
