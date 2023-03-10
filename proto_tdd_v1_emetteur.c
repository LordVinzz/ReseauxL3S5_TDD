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
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquet; /* paquet utilisé par le protocole */
    paquet_t pdu; /* paquet utilisé pour les réponses */

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    paquet.type = DATA;
    paquet.num_seq = 0;

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }

        /* setup taille et type de paquet */
        paquet.lg_info = taille_msg;
        paquet.type = DATA;

        /* création checksum */
        paquet.somme_ctrl = checksum(&paquet);

        /* on envoie et ecoute tant que la réponse c'est pas un ack */
        do{
            vers_reseau(&paquet);
            de_reseau(&pdu);
        }while(pdu.type != ACK);

        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);

        /* xor 1 <=> mod 2 */
        paquet.num_seq ^= 1;
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
