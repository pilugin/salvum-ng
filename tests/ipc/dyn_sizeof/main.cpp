#include <util/array.h>
#include <cstdio>

int main(int arcg, char **argv)
{

    Array<char, 12> arr12;
    printf("size(12)=%ld\n", sizeof(arr12));
    
    Array<char, 0> arr0;
    printf("size(0)=%ld\n", sizeof(arr0));
    
    DynArray<char> darr(12);
    char buf[12];
    printf("dsize(12)=%ld\n", sizeof(darr));
    
    darr.resize(5);
    darr[0] = 'a';
    darr[1] = 'b';
    buf [2] = 'c';
    darr[3] = 'd';
    darr[4] = 0;
    printf("str=%s\n", darr.data() );
    
    printf("darr=%p\nbuf =%p\n", &darr, buf);


    struct P
    {
        DynArray<char> darr;
        char buf[12];
        
        P(): darr(12) {}
    } 
    __attribute__((packed));

    P p;
    p.darr.resize(5);
    p.darr[0] = 'a';
    p.darr[1] = 'b';
    p.buf [2] = 'c';
    p.darr[3] = 'd';
    p.darr[4] = 0;
    printf("str=%s\n", p.darr.data() );
    
    printf("darr=%p\nbuf =%p\n", &p.darr, p.buf);
    
    
    return 0;
}
