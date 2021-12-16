#include "androidtools.h"

#ifdef Q_OS_ANDROID
#include <QtAndroid>

void permissionCB(const QtAndroid::PermissionResultMap &response) {

}
#endif

void requestPermission() {
    #ifdef Q_OS_ANDROID
    QtAndroid::requestPermissions({"android.permission.RECORD_AUDIO"}, permissionCB);
    #endif
}
