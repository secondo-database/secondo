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
CCFLAGS += -Werror $(ADDINCLUDES) -std=c++11

DOCU_FILES = CommandBuilder.hpp CommandBuilder.cpp \
			 CommunicationClient.hpp CommunicationClient.cpp \
			 CommunicationProtocol.hpp CommunicationProtocol.cpp \
			 CommunicationServer.hpp CommunicationServer.cpp \
			 CommunicationUtils.hpp CommunicationUtils.cpp \
			 DBServiceAlgebra.hpp DBServiceAlgebra.cpp \
			 DBServiceConnector.hpp DBServiceConnector.cpp \
			 DBServiceManager.hpp DBServiceManager.cpp \
			 DBServicePersistenceAccessor.hpp DBServicePersistenceAccessor.cpp \
			 DebugOutput.hpp DebugOutput.cpp \
			 LocationInfo.hpp LocationInfo.cpp \
			 MetadataObject.hpp MetadataObject.cpp \
			 MultiClientServer.hpp MultiClientServer.cpp \
			 OperatorAddNode.hpp OperatorAddNode.cpp \
			 OperatorCheckDBServiceStatus.hpp OperatorCheckDBServiceStatus.cpp \
			 OperatorDDelete.hpp OperatorDDelete.cpp \
			 OperatorFeedPF.hpp OperatorFeedPF.cpp \
			 OperatorGetConfigParam.hpp OperatorGetConfigParam.cpp \
			 OperatorInitDBServiceWorker.hpp OperatorInitDBServiceWorker.cpp \
			 OperatorLetDConsume.hpp OperatorLetDConsume.cpp \
			 OperatorRead.hpp OperatorRead.cpp \
			 OperatorStartDBService.hpp OperatorStartDBService.cpp \
			 RelationInfo.hpp RelationInfo.cpp \
			 ReplicationClient.hpp ReplicationClient.cpp \
			 ReplicationClientRunnable.hpp ReplicationClientRunnable.cpp \
			 ReplicationServer.hpp ReplicationServer.cpp \
			 ReplicationUtils.hpp ReplicationUtils.cpp \
			 SecondoUtilsLocal.hpp SecondoUtilsLocal.cpp \
			 SecondoUtilsRemote.hpp SecondoUtilsRemote.cpp \
			 ServerRunnable.hpp ServerRunnable.cpp \
			 TraceSettings.hpp TraceSettings.cpp \
			 TraceWriter.hpp TraceWriter.cpp \
			 TriggerFileTransferRunnable.hpp TriggerFileTransferRunnable.cpp \
			 TriggerReplicaDeletionRunnable.hpp TriggerFileTransferRunnable.cpp

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

# generic rule for creating a library
$(LIBOBJ): $(OBJECTS)
	$(create-lib)

# generic rule for cleaning all generated files
.PHONY: clean
clean:
	$(clean-algebra)

docu: $(DOCU_FILES)
	cat $^ > $@
