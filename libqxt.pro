#
#
#
#
#
#
#      !!!!!
#
#      keep your hands of
#      configuration is in config.pro 
#
#      !!!!!
#
#
#
#
#
#
#
#
include (config.pri)
TEMPLATE = subdirs

base.path = $${QXTINSTALLDIR}
base.files = *

unix :  docs.path = $${QXTUNIXPREFIX}/doc/qxt
win32:  docs.path = 
macx :  docs.path = 
        docs.files = bin/*


unix :	bin.path = $${QXTUNIXPREFIX}/lib
win32:	bin.path = 
macx :	bin.path = 
      	bin.files = bin/*


unix :	include.path = $${QXTUNIXPREFIX}/include/qxt
win32:	include.path = 
macx :	include.path = 
	include.files = src/QxtDefines.h  src/Qxt.h


#write the paths to prf file
unix :system((echo QXTbase=$${QXTINSTALLDIR}; echo QXTinclude=$${include.path}; echo QXTbin=$${bin.path}; cat qxt.prf.m) > qxt.prf)
win32:system(move qxt.prf.m qxt.prf) 





features.path = $$[QT_INSTALL_DATA]/mkspecs/features
features.files = *.prf	








INSTALLS = base docs bin include features 




contains( QXT, core ){
        message( building the core )
        SUBDIRS += src/core	
	core.files +=  src/core/*.h
	core.files +=  include/core/*
	core.path  =  $${include.path}/core
	INSTALLS += core
        }

contains( QXT, kit ){
        message( building kit module )
        SUBDIRS += src/kit
	kit.files +=  src/kit/*.h
	kit.files +=  include/kit/*
	kit.path  =  $${include.path}/kit
	INSTALLS += kit
        }

contains( QXT, gui ){
        message( building gui module )
        SUBDIRS += src/gui
	gui.files +=  src/gui/*.h
	gui.files +=  include/gui/*
	gui.path  =  $${include.path}/gui
	INSTALLS += gui
        }

contains( QXT, network ){
        message( building network module )
        SUBDIRS += src/network
	network.files +=  src/network/*.h
	network.files +=  include/network/*
	network.path  =  $${include.path}/network
	INSTALLS += network
        }

contains( QXT, sql ){
        message( building sql module )
        SUBDIRS += src/sql
	sql.files +=  src/sql/*.h
	sql.files +=  include/sql/*
	sql.path  =  $${include.path}/sql
	INSTALLS += sql
        }

contains( QXT, media ){
        message( building media module )
        SUBDIRS += src/media
	media.files +=  src/media/*.h
	media.files +=  include/media/*	
	media.path  =  $${include.path}/media
	INSTALLS += media
        }




