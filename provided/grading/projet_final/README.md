# Projet POS

## Commentaire utiles
### filev6_create
Nous avons décidé de dévier de la donnée pour l'implémentation de cette fonction. Le numéro du nouvel inode ne doit pas être affecté dans la struct filev6 lors de l'appel de cette fonction, l'appel à inode_alloc est à la place fait dans filev6_create. 
Cela nous a permis de mieux respecter la séparation des couches (direntv6 n'a plus besoin d'inclure inode.h) et nous a aussi semblé plus logique car en appelant cette fonction on souhaite créer un nouveau filev6, ce qui passe forcement par la création d'un nouvel inode.

### Points obtenus pour le calcul des SHAs
Nous ne sommes absolument pas d'accord avec la manière dont a été noté notre implémentation de cette partie. 
Nous avons perdu des points pour avoir utilisé les fonctions suivantes de <openssl/sha.h>:

* int SHA1_Init(SHA_CTX *c);
* int SHA1_Update(SHA_CTX *c, const void *data, size_t len);
* int SHA1_Final(unsigned char *md, SHA_CTX *c);

Celles-ci permettent de générer un SHA sans devoir lire tout le fichier en mémoire et c'est d'ailleurs cette façon de faire qui est recommandée dans la man page. Notre implémentation, **contrairement à celle proposée**, est donc à même de gérer de très grands fichiers (double indirection) sans aucune modification. Ceci mériterait à notre avis des points bonus et non des points en moins...

## Réponses aux questions
* jusqu'où avez-vous été: **Nous avons implémenté tout ce qui était demandé.**
* qu'est-ce que vous avez fait/pas fait: **tout/rien**
* nombres d'heure moyennes: **6h au minumum**

## Extensions
### Makefile
* création automatique du disque **ourcode.uv6** via la cible: **build-ourcode-disk**
* tests de non régression: cibles run-all-tests-on-disk, run-all-tests, run-tests-diff-on-disk, run-all-tests-diff
* test d'ajout de fichiers de différentes tailles et comparaison des SHA de ceux-ci: **test-adding-files**