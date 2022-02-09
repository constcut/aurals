#include "AndroidTools.hpp"
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


void requestFilesPermission() {
    #ifdef Q_OS_ANDROID
    QtAndroid::requestPermissions({"android.permission.WRITE_EXTERNAL_STORAGE"}, permissionCB);
    QtAndroid::requestPermissions({"android.permission.READ_EXTERNAL_STORAGE"}, permissionCB);
    //requestPermissionsSync
    #endif
}


bool chechAndRequestFilesPermission() {
    #ifdef Q_OS_ANDROID
    QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
    QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE");
    #endif
    return true;
}
