#include "androidtools.h"
#include <qglobal.h>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QDebug>

void permissionCB(const QtAndroid::PermissionResultMap& response) {
    qDebug() << "Permission CB";
}
#endif

void requestAudioPermission() {
    #ifdef Q_OS_ANDROID
    QtAndroid::requestPermissions({"android.permission.RECORD_AUDIO"}, permissionCB);
    //requestPermissionsSync
    #endif
}
