#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"


int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO];
    paquet_t paquetE, paquetS;
    int fin = 0, connecte = 0, service, taille_msg;
    uint8_t num = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");


    while ( !fin ) {
        
        de_reseau(&paquetE);

        switch(paquetE.type){
            /*
            lorsqu'on demande la connection ca dit qu'il 
            y a eu une demande et le communique 
            qu'une seule fois a la couche app

            */
            case CON_REQ:
                paquetS.type = CON_ACCEPT;
                paquetS.lg_info = 0;
                paquetS.somme_ctrl = checksum(&paquetS);

                if(!connecte){
                    connecte = 1;
                    vers_application_mode_c(T_CONNECT, NULL, 0);
                    de_application_mode_c(&service, NULL, &taille_msg);
                }

                vers_reseau(&paquetS);

                break;
            /*
            lorsqu'on ferme la connection on regarde si qqn est connecté
            si oui on le deconnete et on renvoie le packet de déco
            tant qu'on recoit des demandes de déco
            */
            case CON_CLOSE:

                paquetS.type = CON_CLOSE_ACK;
                paquetS.lg_info = 0;
                paquetS.somme_ctrl = checksum(&paquetS);
                
                int code = 0;
                do{
                    if(code == -1) arret_temporisateur_num(6);

                    de_reseau(&paquetE);
                    if(connecte == 1){
                        connecte = 0;
                        vers_application_mode_c(T_DISCONNECT, NULL, 0);
                    }
                    vers_reseau(&paquetS);
                    depart_temporisateur_num(6, 500);
                    code = attendre(100);
                }while(code == -1);
                
                fin = 1;
                break;
            /*
                traitement data pareil que la v2 sauf que la verif du checksum 
                et du num packet est fait en 2 temps pour eviter
                qu'un paquet avec une mauvaise checksum passe 
            */
            case DATA:
                if(paquetE.somme_ctrl == checksum(&paquetE)){
                    if(paquetE.num_seq == num){
                        for(int i = 0; i < paquetE.lg_info; i++){
                            message[i] = paquetE.info[i];
                        }
                        paquetS.type = DATA;
                        paquetS.lg_info = 0;
                        paquetS.num_seq = num;

                        num += 1;
                        vers_application_mode_c(service, message, paquetE.lg_info);
                    }
                    vers_reseau(&paquetS);
                }
                

                break;
            default:
                fin = 1;
                break;
        }
        
    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
