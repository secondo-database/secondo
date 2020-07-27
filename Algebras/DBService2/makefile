#This file is part of SECONDO.

#Copyright (C) 2004, University in Hagen, Department of Computer Science, 
#Database Systems for New Applications.

#SECONDO is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#SECONDO is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with SECONDO; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

########################################################################
#
# generic makefile for an algebra module 
#
########################################################################


CURRENT_ALGEBRA := DBServiceAlgebra

# include global definitions
include ../../makefile.env

# workaround, as the Algebras below don't use fully qualified include paths
ADDINCLUDES := -I$(BUILDDIR)/Algebras/OrderedRelation -I$(BUILDDIR)/Algebras/Stream

# expand compile flags if necessary
CCFLAGS += -Werror $(ADDINCLUDES) 

DOCU_FILES =DBServiceAlgebra.hpp DBServiceAlgebra.cpp \
			DBServiceManager.hpp DBServiceManager.cpp \
			DBServiceClient.hpp DBServiceClient.cpp \
			MetadataObject.hpp MetadataObject.cpp \
			LocationInfo.hpp LocationInfo.cpp \
			RelationInfo.hpp RelationInfo.cpp \
			DBServicePersistenceAccessor.hpp DBServicePersistenceAccessor.cpp \
			CommandBuilder.hpp CommandBuilder.cpp \
			MultiClientServer.hpp MultiClientServer.cpp \
			CommunicationServer.hpp CommunicationServer.cpp \
			CommunicationClient.hpp CommunicationClient.cpp \
			ReplicationServer.hpp ReplicationServer.cpp \
			ReplicationClient.hpp ReplicationClient.cpp \
			CommunicationProtocol.hpp CommunicationProtocol.cpp \
			ServerRunnable.hpp ServerRunnable.cpp \
			Replicator.hpp Replicator.cpp \
			ReplicationClientRunnable.hpp ReplicationClientRunnable.cpp \
			TriggerFileTransferRunnable.hpp TriggerFileTransferRunnable.cpp \
			TriggerReplicaDeletionRunnable.hpp TriggerReplicaDeletionRunnable.cpp \
			OperatorAddNode.hpp OperatorAddNode.cpp \
			OperatorCheckDBServiceStatus.hpp OperatorCheckDBServiceStatus.cpp \
			OperatorSetTraceLevel.hpp OperatorSetTraceLevel.cpp \
			OperatorPingDBService.hpp OperatorPingDBService.cpp \
			OperatorWrite.hpp OperatorWrite.cpp \
			OperatorRead.hpp OperatorRead.cpp \
			OperatorDDelete.hpp OperatorDDelete.cpp \
			OperatorInitDBServiceWorker.hpp OperatorInitDBServiceWorker.cpp \
			OperatorGetConfigParam.hpp OperatorGetConfigParam.cpp \
			CommunicationUtils.hpp CommunicationUtils.cpp \
			SecondoUtilsLocal.hpp SecondoUtilsLocal.cpp \
			SecondoUtilsRemote.hpp SecondoUtilsRemote.cpp \
			ReplicationUtils.hpp ReplicationUtils.cpp \
			TraceSettings.hpp TraceSettings.cpp \
			DebugOutput.hpp DebugOutput.cpp \
			TraceWriter.hpp TraceWriter.cpp \
			Test/CommandBuilderTest.cpp \
			Test/CommunicationUtilsTest.cpp \
			Test/LocationInfoTest.cpp \
			Test/RelationInfoTest.cpp \
			Test/ReplicationUtilsTest.cpp \
			Test/TraceSettingsTest.cpp

# create objects and algebra library. The values for the variables
# OBJECTS and LIBOBJ are computed automatically by makefile.env 

.PHONY: all
all: check_dependencies $(OBJECTS) $(LIBOBJ)

.PHONY: test
test: check_dependencies $(OBJECTS) $(LIBOBJ)
	$(MAKE) -C Test
	Test/SuiteDBService

check_dependencies:
	$(check-algebra-dependencies)

# include pre-computed dependencies
ifneq ($(MAKECMDGOALS),clean)
include $(DEP_FILES)
endif

.PHONY: deps
deps: $(DEP_FILES)


# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)

# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)

docu: $(DOCU_FILES)
	cat $^ > $@
