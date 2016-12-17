#include <gmp.h>
#include <mpi.h>
#include <time.h>
#include "utils.h"

#define SHA1_BITS_LENGTH               160

#define TAG_SEND_WORD                    1
#define TAG_COLLISION_NOT_FOUND          2
#define TAG_COLLISION_FOUND              4
#define TAG_CHECK_LSB                    5
#define TAG_CHECK_THIS_LSB               6
#define TAG_THIS_LSB_FOUND               7
#define TAG_THIS_LSB_NOT_FOUND           8
#define TAG_NO_COLLISIONS_ANYWHERE       9

#define NB_SLAVES                       12
#define NB_MANAGERS                      3
#define MAX_SIZE_LSB_BASE_60            27

using namespace std;

/*!
 * \brief Function which is intended to manage the red black tree that contains
 *        all produced hash.
 * \param B Length of a word
 * \param LSB Number last significative bit of the hash.
 */
void master();

/*!
 * \brief generator
 * \param B
 * \param LSB
 */
void slave(int B, int LSB);

void writer();

// ============================================================

int main(int argc, char *argv[])
{
    int id_process;
    int B = 2, LSB = 4;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_process);

    switch (id_process) {
    case 0:                 writer();      break;
    case 1: case 2: case 3: master();      break;
    default:                slave(B, LSB); break;
    }

    MPI_Finalize();
    return 0;
}

// ============================================================

void master() {
    cout << "Im a fucking manager" << endl;
}


void writer() {
    cout << "Im the fucking writer" << endl;
}

void slave(int B, int LSB) {

    // ========= Initialization... =========
    // Each slave has its own map.
    map<string, string> treeHashes;
    vector<string> messageSplited;
    int nbProcess, idProcess, idManager, tag, dest, msgReceived;
    char * msgRecv = (char *) malloc(MAX_SIZE_LSB_BASE_60 + B + 1);
    string hashed, LSBString, message;
    MPI_Status status;
    // x2 parce que 1 byte est codé sur 2 valeurs hexadécimales 00 -> FF
    string startGeneration = Utils::initializeHexString(2*B, 'F');
    string endGeneration;
    // Affecte a nbProcess le nombre de processus qui executent ce programme
    MPI_Comm_size(MPI_COMM_WORLD, &nbProcess);
    // Affecte a idProcess mon numero de processus
    MPI_Comm_rank(MPI_COMM_WORLD, &idProcess);
    idManager = nbProcess % 4;
    mpz_t maxValue;          // valeur maximale
    mpz_t range;
    mpz_t nbProcess_gmp;
    mpz_t start;             // Point de départ de la génération.
    mpz_t end;               // Fin de la génération.
    mpz_t lsbBase60;
    mpz_init(maxValue);      // initialization of the variable maxValue
    mpz_init(nbProcess_gmp); // initialization of the variable nbProcess
    mpz_init(range);         // initialization of the variable range
    mpz_init(start);
    mpz_init(end);
    mpz_init(lsbBase60);
    // Assign the value of "exhaustiveString" to "maxValue" (base 16)
    mpz_set_str(maxValue, startGeneration.c_str(), 16);
    // Assign the value "nbProcess"-1  (-1 because the first process is assigned
    // to the handler of the map) to "nbProcess_gmp"
    mpz_set_ui(nbProcess_gmp, nbProcess-1);
    // Divide maxValue by nbProcess_gmp and assign it to range (and truncate)
    mpz_cdiv_q(range, maxValue, nbProcess_gmp);
    mpz_mul_ui(start, range,    idProcess-1);
    mpz_mul_ui(end,   range,    idProcess);

    mpz_sub_ui(end, end, 1);

    startGeneration =  mpz_get_str(NULL, 16, start);
    endGeneration   =  mpz_get_str(NULL, 16, end);
    for (auto & c: startGeneration)
        c = toupper(c); // since the incrementations are in uppercase, we upper.
    for (auto & c: endGeneration)
        c = toupper(c); // since the incrementations are in uppercase, we upper.

    // Dans le cas où startGeneration n'est pas représenté sur 2*B caractères
    // (ce qui peut arriver avec gmp),
    // on le préfice avec autant de 0 que nécessaire. Pareil pour endGeneration.
    startGeneration.insert(startGeneration.begin(), 2*B - startGeneration.size(), '0');
    endGeneration.insert(endGeneration.begin(), 2*B - endGeneration.size(), '0');
    // =====================================


    // =========== Generation... ===========
    while(startGeneration.compare(endGeneration) != 0) {
        // 1. Generate hash of the input
        hashed = sha1(Utils::hexToString(startGeneration));
        // 2. Increment the hex value of the string
        Utils::incrementString(startGeneration);
        // 3. Get the LSB
        LSBString = (Utils::hexToBin(hashed).substr(SHA1_BITS_LENGTH - LSB));
        // 4. Convert it in base 60
        mpz_set_str(lsbBase60, LSBString.c_str(), 60);
        LSBString = string(mpz_get_str(NULL, 60,  lsbBase60));
        // 5. Check if there is a collision on my local map.
        if(treeHashes.find(LSBString) == treeHashes.end()) {
            // No, inform my manager
            tag = TAG_COLLISION_NOT_FOUND;
            dest = idManager;
        }
        else {  // Yes, inform the writer
            tag = TAG_COLLISION_FOUND;
            dest = 0;
        }
        // Message = <LSB base 60 # word>
        message = LSBString + string("#") + Utils::hexToString(startGeneration);
        MPI_Send(message.c_str(), MAX_SIZE_LSB_BASE_60 + B + 1, MPI_CHAR, dest,
                 tag, MPI_COMM_WORLD);

        // Check si mon manager veut que je regarde dans ma map
        // s'il y a une collision avec le LSB qu'il m'envoie.
        MPI_Iprobe(idManager, TAG_CHECK_LSB, MPI_COMM_WORLD, &msgReceived,
                   &status);

        if(msgReceived) { // Yes, my manager wants me to do so
            MPI_Recv(msgRecv, MAX_SIZE_LSB_BASE_60, MPI_CHAR, idManager,
                     TAG_CHECK_THIS_LSB, MPI_COMM_WORLD, &status);
            dest = status.MPI_SOURCE;
            tag = (treeHashes.find(string(msgRecv)) == treeHashes.end()) ?
                        TAG_THIS_LSB_NOT_FOUND : TAG_THIS_LSB_FOUND;
            MPI_Send(string("").c_str(), 0, MPI_CHAR, dest,tag, MPI_COMM_WORLD);
        }

        // Check si mon manager m'informe qu'un des mots que j'ai envoyé
        // n'a de collisions avec personne.
        MPI_Iprobe(idManager, TAG_NO_COLLISIONS_ANYWHERE, MPI_COMM_WORLD,
                   &msgReceived, &status);
        if(msgReceived) { // Add the word in my map
            MPI_Recv(msgRecv, MAX_SIZE_LSB_BASE_60 + B + 1, MPI_CHAR, idManager,
                     TAG_CHECK_THIS_LSB, MPI_COMM_WORLD, &status);
            messageSplited = Utils::split(string(msgRecv), '#');
            treeHashes[messageSplited[0]] = messageSplited[1];
        }
    }
    // =====================================



}

/*
void master(int B, int LSB) {
    int nbProcess, nbProcessFinished = 0;
    // The key will contains the hashed value of the input
    // The value will contains the list of all input that
    // have a collision with the key.
    // An HashWord is defined as <the word, it's hashed value>
    map<string, vector<HashWord>> treeHashes;
    bool finished = false;
    vector<string> messageSplited;
    // 2*B = word in hexa ; 40 = sizeof sha1 ; LSB = LSB ; 2 = number of delimiters
    char * message      = (char *)       malloc(2*B + 40 + LSB + 2);
    MPI_Status * status = (MPI_Status *) malloc(sizeof(MPI_Status));
    MPI_Comm_size(MPI_COMM_WORLD, &nbProcess);

    while(nbProcessFinished < (nbProcess - 1)) {
        MPI_Recv(message, (2*B + 40 + LSB + 2), MPI_CHAR, MPI_ANY_SOURCE, TAG_SEND_WORD, MPI_COMM_WORLD, status);
        messageSplited = Utils::split(string(message), '#');
        treeHashes[messageSplited[0]].push_back({messageSplited[1], messageSplited[2]});
    }

    Utils::exportToCSV(treeHashes, LSB, B);

}
