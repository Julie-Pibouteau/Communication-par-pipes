//Lauréline Charret et Julie Pibouteau - L3 MIAGE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


//#define ARTICLE "classique"
#define ARTICLE "rustique"
//#define SW "sw1"
#define SW "sw2"
#define ACHTR "Laure"
//#define ACHTR "Philippe"
#define TRANSP "Xavier"
//#define TRANSP "France"
#define PRIXROULEAU 15
#define QTESTOCK "4242"
#define CRYPTO "342"
#define SURFACESOUHAITEE "1515" //en m^2
#define NUMCB "5355 9012 1234 5678"
#define PALETTE 63 //nb de rouleaux par palette
#define MSGSIZE 312
#define WAIT 2


char* lecture(int p[], char* buf){
    int nread;

    //printf("%d : Fermeture acc\u00e8s \u00e9criture du tube.\n", getpid());
    close(p[1]);
    while (1) {
        nread = read(p[0], buf, MSGSIZE);
        switch (nread) {
            case -1:
                if (errno == EAGAIN) {
                    sleep(1);
                    break;
                }
                else {
                    perror("read");
                    exit(5);
                }
            /*case 0:
                printf("Fin de communication re\u00e7ue.\n");
                return buf;*/
            default :
                printf("%d : Message re\u00e7u. Contenu = \"%s\".\n\n", getpid(), buf);
                return buf;
        }
    }
}


void ecriture(int p[], char* msg){
    //printf("%d : Fermeture acc\u00e8s lecture du tube.\n", getpid());
    close(p[0]);
    write(p[1], msg, MSGSIZE);
    printf("%d : Message \u00e9mis. Contenu = \"%s\".\n\n", getpid(), msg);
}

void serveurFonction(int pipeServeurTransporteur[],  int pipeAcheteurServeur[], int pipeServeurAcheteur[], char* buf){
    //lit le nom de l\'article
    char* nomArticle = lecture(pipeAcheteurServeur, buf);
    //renvoie à l\'acheteur la qte en stock
    char qteStock2[MSGSIZE];
    snprintf (qteStock2, MSGSIZE, "Serveur vers Acheteur : Salutations %s, nous disposons de %s rouleaux en stock.", ACHTR, QTESTOCK);
    ecriture(pipeServeurAcheteur, qteStock2);
    //lit la surface souhaitee
    char* surface = lecture(pipeAcheteurServeur, buf);
    //calcule le nombre de palettes et le montant total et transmet à l\'acheteur
    int nbPalettes = atoi(SURFACESOUHAITEE) / PALETTE + 1;
    int total = nbPalettes * PALETTE * PRIXROULEAU;
    char facture[MSGSIZE];
    snprintf (facture, MSGSIZE, "Serveur vers Acheteur : Très bien %s, voici votre facture. Vous avez commande %d palettes, ce qui vous fait un total de %d€.", ACHTR, nbPalettes, total);
    ecriture(pipeServeurAcheteur, facture);
    //lit les infos bancaires de l\'acheteur
    char* coordBanc = lecture(pipeAcheteurServeur, buf);
    //envoie à l\'acheteur un accuse de paiement rappelant le montant total
    char accusePaiement[MSGSIZE];
    snprintf(accusePaiement, MSGSIZE, "Serveur vers Acheteur : Merci pour votre achat %s, veuillez trouver ci-joint votre accuse de paiement.\nVous avez regle un montant total de %d€ avec votre carte bancaire n°%s de cryptogramme %s.\nJe transmets immediatement votre commande à notre transporteur, %s.", ACHTR, total, NUMCB, CRYPTO, TRANSP);
    ecriture(pipeServeurAcheteur, accusePaiement);
    //envoie au transporteur un bon en double, contenant le nombre de palettes
    sleep(WAIT);
    char bonTransporteur[MSGSIZE];
    snprintf(bonTransporteur, MSGSIZE, "Serveur vers Transporteur : Commande de %d palettes pour notre acheteur favori, %s, livre par %s.", nbPalettes, ACHTR, TRANSP);
    ecriture(pipeServeurTransporteur, bonTransporteur);
    sleep(WAIT);
    ecriture(pipeServeurTransporteur, bonTransporteur);
}

void acheteurFonction(int pipeAcheteurTransporteur[], int pipeAcheteurServeur[], int pipeServeurAcheteur[], int pipeTransporteurAcheteur[], char* buf){
    //transmet au serveur le nom de l\'article
    char nomArticle[MSGSIZE];
    snprintf (nomArticle, MSGSIZE, "Acheteur vers Serveur : Bonjour %s ! Je m\'appelle %s. J\'ai besoin de l\'article de type %s.", SW,ACHTR, ARTICLE);
    ecriture(pipeAcheteurServeur, nomArticle);
    //lit la qte en stock
    char* qteStock = lecture(pipeServeurAcheteur, buf);
    //transmet au serveur la surface souhaitee
    char surfaceSouhaitee[MSGSIZE];
    snprintf (surfaceSouhaitee, MSGSIZE, "Acheteur vers Serveur: Parfait %s! J\'ai besoin d\'une surface de %sm^2.", SW, SURFACESOUHAITEE);
    ecriture(pipeAcheteurServeur, surfaceSouhaitee);
    //lit la facture envoyee par le serveur
    char* facture = lecture(pipeServeurAcheteur, buf);
    //transmet au serveur son n° de carte et son cryptogramme
    char coordonnesBancaires[MSGSIZE];
    snprintf (coordonnesBancaires, MSGSIZE, "Acheteur vers Serveur : Magnifique facture ! Où vous fournissez-vous en parchemin ? Passons au paiement : mon numero de carte est %s et mon cryptogramme est %s.",NUMCB, CRYPTO);
    ecriture(pipeAcheteurServeur, coordonnesBancaires);
    //lit l\'accuse de paiement transmis par le serveur
    char* accusePaiement = lecture(pipeServeurAcheteur, buf);
    //reçoit sa livraison
    lecture(pipeTransporteurAcheteur, buf);
    //lit les 2 bons
    char* bonTransporteur1 = lecture(pipeTransporteurAcheteur, buf);
    char* bonTransporteur2 = lecture(pipeTransporteurAcheteur, buf);
    //signe un des bons et transmet le bon signe au transporteur
    char bonSigne[MSGSIZE];
    snprintf(bonSigne, MSGSIZE, "Acheteur vers Transporteur : Merci pour cette incroyable livraison %s, voici votre bon signe de ma plus belle signature : %s ! \"%s\"", TRANSP, ACHTR, bonTransporteur1);
    ecriture(pipeAcheteurTransporteur, bonSigne);
}

void transporteurFonction(int pipeServeurTransporteur[], int pipeAcheteurTransporteur[], int pipeTransporteurAcheteur[], char* buf){
    //lit le nombre de palettes tranmis par le serveur
    char* bonTransporteur1 = lecture(pipeServeurTransporteur, buf);
    char* bonTransporteur2 = lecture(pipeServeurTransporteur, buf);
    char* bonTransporteur3 = &bonTransporteur1[28];
    //livraison
    char livraison[MSGSIZE];
    snprintf(livraison, MSGSIZE, "Transporteur vers Acheteur : C\'est l\'heure de la livraison ! Voici votre commande %s !", ACHTR);
    ecriture(pipeTransporteurAcheteur, livraison);
    //transmet les deux bons à l\'acheteur
    ecriture(pipeTransporteurAcheteur, bonTransporteur3);
    ecriture(pipeTransporteurAcheteur, bonTransporteur3);
    //lit le bon signe par l\'acheteur
    lecture(pipeAcheteurTransporteur, buf);
}


int main() {
    //scenario de vente : Un scenario met en scène 1 serveur web, 1 acheteur, 1 transporteur, 1 article, son prix au rouleau, la quantite de rouleau en stock et une surface en m2.
    printf(ACHTR == "Philippe" ? "\nIl etait une fois un fringant acheteur denomme %s.\nIl souhaitait commander les plus beaux rouleaux de gazon naturel de type %s de tout le royaume.\nPour ce faire, il se tourna vers le meilleur serveur : %s, qui l\'orienta vers la crème des transporteurs, %s.\nComme vous allez l\'apprendre dans l\'epopee commerciale qui suit, le serveur ne dispose que de %s rouleaux en stock, au prix unitaire de %d€. %s decida alors de se porter acquereur d\'une surface de %sm^2 de gazon.\n\n\n\n" : "\nIl etait une fois une charmante acheteuse denommee %s.\nElle souhaitait commander les plus beaux rouleaux de gazon naturel de type %s de tout le royaume.\nPour ce faire, elle se tourna vers le meilleur serveur : %s, qui l\'orienta vers la crème des transporteurs, %s.\nComme vous allez l\'apprendre dans l\'epopee commerciale qui suit, le serveur ne dispose que de %s rouleaux en stock, au prix unitaire de %d€. %s decida alors de se porter acquereur d\'une surface de %sm^2 de gazon.\n\n\n\n", ACHTR, ARTICLE, SW, TRANSP, QTESTOCK, PRIXROULEAU, ACHTR, SURFACESOUHAITEE);

    char* buf = malloc(sizeof(char) * MSGSIZE); //taille de 256

    //convention : le premier cite est celui qui ecrit dans ce pipe
    int pipeServeurTransporteur[2],
        pipeAcheteurTransporteur[2],
        pipeAcheteurServeur[2],
        pipeServeurAcheteur[2],
        pipeTransporteurAcheteur[2];

    if ((pipe(pipeServeurTransporteur) < 0) || (pipe(pipeAcheteurTransporteur) < 0) || (pipe(pipeAcheteurServeur) < 0) || (pipe(pipeServeurAcheteur) < 0) || (pipe(pipeTransporteurAcheteur) < 0)) {
        printf("P\u00e8re. Tubes non support\u00e9s. Fin\n");
        exit(1);
    }

    //#define checkFcntl(nomPipe) fcntl(nomPipe[0], F_SETFL, O_NONBLOCK) < 0
    //if(checkFcntl(pipeServeurTransporteur) || checkFcntl(pipeAcheteurTransporteur))
    if ((fcntl(pipeServeurTransporteur[0], F_SETFL, O_NONBLOCK) < 0) || (fcntl(pipeAcheteurTransporteur[0], F_SETFL, O_NONBLOCK) < 0) || (fcntl(pipeAcheteurServeur[0], F_SETFL, O_NONBLOCK) < 0) || (fcntl(pipeServeurAcheteur[0], F_SETFL, O_NONBLOCK) < 0) || (fcntl(pipeTransporteurAcheteur[0], F_SETFL, O_NONBLOCK) < 0)) {
        printf("Mode non bloquant indisponible. Fin\n");
        exit(2);
    }

    /*
    le processus père est le serveur
    les deux processus fils sont l\'acheteur et le transporteur
    */
    pid_t pidServeur = getpid();
    printf("pidServeur = %d\n\n", pidServeur);

    switch (fork()) {
        case -1:
            printf("P\u00e8re. Fork impossible pour creer Acheteur : Acheteur est envoye aux oubliettes !\n");
            exit(3);
        case 0:;
            pid_t pidAcheteur = getpid();
            printf("pidAcheteur = %d\n\n", pidAcheteur);
            acheteurFonction(pipeAcheteurTransporteur, pipeAcheteurServeur, pipeServeurAcheteur, pipeTransporteurAcheteur, buf);
            break;
        default:
            switch(fork()){
                 case -1:
                    printf("P\u00e8re. Fork impossible pour creer Transporteur : Transporteur est envoye aux oubliettes !\n");
                    exit(4);
                 case 0:;
                    pid_t pidTransporteur = getpid();
                    printf("pidTransporteur = %d\n\n", pidTransporteur);
                    transporteurFonction(pipeServeurTransporteur, pipeAcheteurTransporteur, pipeTransporteurAcheteur, buf);
                    break;
                 default:
                    serveurFonction(pipeServeurTransporteur,  pipeAcheteurServeur, pipeServeurAcheteur, buf);
                    free(buf);
                    break;
            }
            break;
    }
    //int statusA, statusT;
    //waitpid(pidTransporteur, &statusT, 0) && waitpid(pidAcheteur, &statusA, 0);
    sleep(WAIT+1);
    return 0;
}
