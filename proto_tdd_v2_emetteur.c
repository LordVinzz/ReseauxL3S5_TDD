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
    paquet_t pdu;


    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* on init le type du packet */
    paquet.type = DATA;
    paquet.num_seq = 0;

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }

        /* on init les infos */
        paquet.lg_info = taille_msg;
        paquet.type = DATA;

        paquet.somme_ctrl = checksum(&paquet);

        pdu.type = NACK;
        int c;
        /*
            On envoie le paquet tant qu'on recoit pas l'ack qui correspond au bon
            numero de sequence
        */
        do{
            vers_reseau(&paquet);
            depart_temporisateur_num(0, 100);
            
            c = attendre();

            if(c == -1){
                de_reseau(&pdu);
                if(pdu.num_seq != paquet.num_seq){
                    c = attendre();
                }
            }
        }while(c != -1);
        arret_temporisateur_num(0);

        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
        paquet.num_seq ^= 1;
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
