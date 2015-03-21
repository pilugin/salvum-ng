#include <jpeg/sharedimage>

#include <QtDebug>

using namespace Jpeg;

int main()
{
    int x=8  *5;
    int y=8  *4;
    void *buffer = malloc( sizeof(SharedImage) + 2*x*y*sizeof(unsigned int) );

    SharedImage *si = new (buffer) SharedImage(x, y, 100);
    
    // 1) create Part. Fill 6 blocks

    // 2) create Part. Fill 7 blocks. Make BAD

    // 3) create Part. Fill 5 blocks. Make BAD

    // 4) restore Part (2)

    // 5) create Part

    free(buffer);

    return 0;
}