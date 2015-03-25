#include <jpeg/sharedimage.h>
#include <QColor>
#include <QtDebug>

using namespace Jpeg;

ImagePart good(std::shared_ptr<SharedImage> i, int len, QColor c)
{
    ///@TODO: createPart(shared_ptr)
    ImagePart ip= i->createPart(i);
    
    while (len-- > 0) {        
        Block b = ip.addWritableBlock();
        
//        qDebug("%p o=%d", b.line, b.rowOffset);
        
        for (int x=0; x<ip.blockWidth(); ++x)
            for (int y=0; y<ip.blockHeight(); ++y) {
                if (y==0)
                    if (x==0)
                        b.line(y)[x] = QColor(Qt::gray).rgba();
                    else
                        b.line(y)[x] = QColor(Qt::red).rgba();
                else
                    b.line(y)[x] = c.rgba();
            }
    }
    
    return ip;
}

ImagePart bad(std::shared_ptr<SharedImage> i, int len, QColor c)
{
    ImagePart bad = good(i, len, c);
    assert(bad.setBad());
    
    return bad;
}

void dropBad(SharedImage *i)
{
    i->dropBadParts();
}

int main()
{
    int x=8  *5;
    int y=8  *4;    

    SharedImageAllocator alloc;

    std::shared_ptr<SharedImage> si( alloc.alloc(H1V1, x, y, 100), SharedImageDeleter(&alloc) );
    
                    good(si, 4, Qt::white);
                    good(si, 3, Qt::blue);
    ImagePart b =   bad (si, 5, Qt::black);
                    good(si, 3, Qt::white);

                    good(si, 1, Qt::white).setBad();
                    good(si, 1, Qt::green);
                    
    b.setGoodAgain();                    
                    good(si, 3, Qt::yellow).setBad();
                    good(si, 1, Qt::red);

    si->dropBadParts();
    
    toQImage(*si).save("SAVED.png", "png");

    return 0;
}
