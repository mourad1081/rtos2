#include <iostream>
#include <string>
#include <mpi.h>
#include "sha1.h"

using namespace std;
void randomString(char *, const int);

int main(int argc, char *argv[])
{
    /*
    if(argc < 5)
        throw std::string("Error. Usage : ./collisionSearch -b <bytes>"
                                        " -n <number of LSB>");
    */
    char * random = randomString(random, 10);
    cout << "SHA1 de Hello World! = 0x" << sha1("Hello, world!") << endl;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rang);
    if (rang == 1)
    {
        valeur = 18;
        MPI_Send(&valeur, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    }
    else
    {
        if (rang == 0)
        {
            MPI_Recv(&valeur,1,MPI_INT,1,tag, MPI_COMM_WORLD,&status);
            printf("J’ai recu la valeur %d du processus de rang 1",valeur);
        }
    }
    MPI_Finalize();
    return 0;
}

void randomString(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}
