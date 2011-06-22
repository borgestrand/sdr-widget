#include <QtGui/QApplication>
#include "mainwindow.h"

//  This is for the Mac permissions elevation
//  Jeremy McDermond (NH6Z)
#ifdef Q_WS_MAC
#include <sys/param.h>
#include <mach-o/dyld.h>
#include <Authorization.h>
#include <AuthorizationTags.h>
#endif // Q_WS_MAC


void myMessageOutput(QtMsgType type, const char *msg)
 {
     switch (type) {
     case QtDebugMsg:
         fprintf(stderr, "Debug: %s\n", msg);
         break;
     case QtWarningMsg:
         fprintf(stderr, "Warning: %s\n", msg);
         break;
     case QtCriticalMsg:
         fprintf(stderr, "Critical: %s\n", msg);
         break;
     case QtFatalMsg:
         fprintf(stderr, "Fatal: %s\n", msg);
         abort();
     }
 }

int main(int argc, char *argv[])
{

    // install debug message handler
    qInstallMsgHandler(myMessageOutput);


    //  Gain admin privileges on the Mac
#ifdef Q_WS_MAC
    char myPath[PATH_MAX];
    uint32_t pathSize=sizeof(myPath);

    if(geteuid() != 0) {
        AuthorizationItem adminPriv;
        AuthorizationRights adminRights;
        AuthorizationRef adminRightsRef;
        OSStatus result;
        char myPath[MAXPATHLEN];
        uint32_t pathSize = MAXPATHLEN;
        int childStatus;

        adminPriv.name = kAuthorizationRightExecute;
        adminPriv.valueLength = 0;
        adminPriv.value = NULL;
        adminPriv.flags = 0;

        adminRights.count = 1;
        adminRights.items = &adminPriv;

        result = AuthorizationCreate(&adminRights, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults | kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed, &adminRightsRef);

        if(result != errAuthorizationSuccess) {
            fprintf(stderr, "Couldn't Authenticate: %d\n", result);
            exit(1);
        }

        _NSGetExecutablePath(myPath, &pathSize);
        result = AuthorizationExecuteWithPrivileges(adminRightsRef, myPath, kAuthorizationFlagDefaults, NULL, NULL);
        if(result != errAuthorizationSuccess) {
            fprintf(stderr, "Couldn't execute self: %d\n", result);
        }

        waitpid(-1, &childStatus, NULL);
        exit(0);
    }


    _NSGetExecutablePath(myPath,&pathSize);
    char* slash = strrchr(myPath, '/');
    if(slash!=NULL) {
        *slash = '\0';
    }
#endif // Q_WS_MAC



    QApplication a(argc, argv);
    MainWindow w;
#ifdef Q_WS_MAC
    w.setPath(myPath);
#endif
    w.show();

    return a.exec();
}
