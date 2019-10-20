#include "aliupgpacktool.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    int i;
    for (i = 0; i < argc; i++)
    {
        qDebug() << argv[i];
    }
    qDebug() << endl;
    ALiUpgPackTool w;
    w.start_pack();
}
