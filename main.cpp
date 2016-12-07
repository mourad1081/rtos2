#include <gmp.h>
#include <mpi.h>
#include <time.h>
#include "utils.h"

#define SHA1_BITS_LENGTH 160
#define TAG_SEND_WORD 1
using namespace std;

/*!
 * \brief Function which is intended to manage the red black tree that contains all produced hash
 * \param B Length of a word
 * \param LSB Number last significative bit of the hash.
 */
void master(int B, int LSB);

/*!
 * \brief generator
 * \param B
 * \param LSB
 */
void slave(int B, int LSB);

// ============================================================

int main(int argc, char *argv[])
{
    int id_process;
    int B = 2, LSB = 4;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_process);

    id_process == 0 ? master(B, LSB) : slave(B, LSB);

    MPI_Finalize();
    return 0;
}

// ============================================================

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

void slave(int B, int LSB) {

    // ========= Initialization... =========
    int nbProcess, idProcess;
    string hashed, LSBString, message;
    // x2 parce que 1 byte est codé sur 2 valeurs hexadécimales 00 -> FF
    string startGeneration = Utils::initializeHexString(2*B, 'F');
    string endGeneration;
    MPI_Comm_size(MPI_COMM_WORLD, &nbProcess);   // Dans main(), Affecte a nbProcess le nombre de processus qui executent ce programme
    MPI_Comm_rank(MPI_COMM_WORLD, &idProcess);   // Affecte a idProcess mon numero de processus
    mpz_t maxValue;          // valeur maximale, this is the type of statement is needed to declare the variable
    mpz_t range;
    mpz_t nbProcess_gmp;
    mpz_t start;             // Point de départ de la génération. On va générer "range" mots.
    mpz_t end;               // Fin de la génération. On va générer "range" mots.
    mpz_init(maxValue);      // initialization of the variable maxValue
    mpz_init(nbProcess_gmp); // initialization of the variable nbProcess
    mpz_init(range);         // initialization of the variable range
    mpz_init(start);
    mpz_init(end);
    mpz_set_str(maxValue, startGeneration.c_str(), 16); // Assign the value of "exhaustiveString" to "maxValue" (base 16)
    mpz_set_ui(nbProcess_gmp, nbProcess-1);             // Assign the value of integer "nbProcess"-1  (-1 because the first process is assigned to the handler of the map) to "nbProcess_gmp"
    mpz_cdiv_q(range, maxValue, nbProcess_gmp);         // Divide maxValue by nbProcess_gmp and assign it to range (and truncate)
    mpz_mul_ui(start, range, idProcess-1);
    mpz_mul_ui(end, range, idProcess);
    if(nbProcess > 2) // The max range -1 if nbProcess > 2 (avoid duplicates)
        mpz_sub_ui(end, end, 1);
    startGeneration =  mpz_get_str(NULL, 16, start);
    endGeneration   =  mpz_get_str(NULL, 16, end);
    for (auto & c: startGeneration)
        c = toupper(c); // since the incrementations are in uppercase, we upper this.
    for (auto & c: endGeneration)
        c = toupper(c); // since the incrementations are in uppercase, we upper this.
    // Dans le cas où startGeneration n'est pas représenté sur 2*B caractères (ce qui peut arriver avec gmp),
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
        // 4. Send the word + hash + LSB to master
        message = LSBString + string("#") + startGeneration + string("#") +  hashed;
        MPI_Send(message.c_str(), (2*B + 40 + LSB + 2), MPI_CHAR, 0, TAG_SEND_WORD, MPI_COMM_WORLD);
    }
    // =====================================
}
