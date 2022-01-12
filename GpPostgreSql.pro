TEMPLATE        = lib
#CONFIG         += staticlib
VER_MAJ		    = 0
VER_MIN		    = 1
VER_PAT		    = 0
QMAKE_CXXFLAGS += -DGP_MODULE_UUID=ba443106-9992-4c29-93db-6e29aef8c576
QMAKE_CXXFLAGS += -DGP_TYPE_SYSTEM_STATIC_ADD_TO_MANAGER
DEFINES		   += GPPOSTGRESQL_LIBRARY
PACKET_NAME     = GpPostgreSql
DIR_LEVEL       = ./..

include(../../QtGlobalPro.pri)

#------------------------------ LIBS BEGIN ---------------------------------
os_windows{
	GP_CORE_LIB_V		= 2
	GP_DB_CLIENT_LIB_V	= 0
	GP_NETWORK_LIB_V	= 0
	GP_LOG_LIB_V		= 0
}

os_linux{
}

LIBS += -lGpCore2$$TARGET_POSTFIX$$GP_CORE_LIB_V
LIBS += -lGpDbClient$$TARGET_POSTFIX$$GP_DB_CLIENT_LIB_V
LIBS += -lGpNetwork$$TARGET_POSTFIX$$GP_NETWORK_LIB_V
LIBS += -lGpLog$$TARGET_POSTFIX$$GP_LOG_LIB_V
LIBS += -lpq

#------------------------------ LIBS END ---------------------------------

SOURCES += \
	GpDbArrayBuilder.cpp \
	GpDbConnectAsyncTask.cpp \
	GpDbConnectionPgSql.cpp \
	GpDbDriverFactoryPgSql.cpp \
	GpDbDriverPgSql.cpp \
	GpDbQueryAsyncTask.cpp \
	GpDbQueryPreparedPgSql.cpp \
	GpDbQueryResPgSql.cpp

HEADERS += \
	GpDbArrayBuilder.hpp \
	GpDbConnectAsyncTask.hpp \
	GpDbConnectionPgSql.hpp \
	GpDbDriverFactoryPgSql.hpp \
	GpDbDriverPgSql.hpp \
	GpDbQueryAsyncTask.hpp \
	GpDbQueryPreparedPgSql.hpp \
	GpDbQueryResPgSql.hpp \
	GpPostgreSql_global.hpp \
	GpPostgreSql.hpp
