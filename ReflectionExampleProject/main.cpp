#include "Test/test.h"
#include <QTemporaryDir>
#include <QString>
#include <QtCore/qassert.h>

int main()
{
    {
        QTemporaryDir tempDir;
        Q_ASSERT(tempDir.isValid());

        const QString filenameWithoutExt =
            tempDir.filePath("test");

        serializationTest(filenameWithoutExt);
        deserializationTest(filenameWithoutExt);
    }

    stringConversionTest();

    equalsTest();

    uniqueStaticPropertyMapTest();

    uniqueStaticHeterogeneousMapTest1();
    uniqueStaticHeterogeneousMapTest2();

    uniqueStaticHeterogeneousArrayTest();

    return 0;
}
